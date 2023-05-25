#include "RenderThread_EglDma.h"

#include "glhelpers/Buffer.h"
#include "glhelpers/Program.h"
#include "glhelpers/Shader.h"
#include "glhelpers/Texture.h"
#include "glhelpers/Utils.h"

#include "shaders/camera_frame_dma_fs.h"
#include "shaders/camera_frame_vs.h"

#include <SDL_video.h>
#include <epoxy/egl.h>
#include <epoxy/gl.h>
#include <libcamera/framebuffer.h>
#include <libdrm/drm_fourcc.h>

#include <atomic>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <sys/mman.h>

using std::clog;
using std::endl;

struct GLObjectsDMA
{
    GLObjectsDMA();
    GLTexture2D *getTextureFor(libcamera::FrameBuffer *, const libcamera::StreamConfiguration &cfg);

    std::map<int, std::unique_ptr<GLTexture2D>> textures; // fd -> texture object
    GLTexture2D *prev_set_texture = nullptr;
    GLShader frame_vs{GL_VERTEX_SHADER};
    GLShader frame_fs{GL_FRAGMENT_SHADER};
    GLProgram frame_prog;
    GLuint attr_loc_frame_prog_pos = -1;
    GLuint attr_loc_frame_prog_tex_coord = -1;
    GLint uni_loc_frame_prog_yuv = -1;
    bool ok = false;
};

GLObjectsDMA::GLObjectsDMA()
{
    if (!frame_vs.compile((const char *)rcs::shaders_camera_frame_vs_data))
    {
        clog << "failed to compile camera frame GLSL vertex shader:" << endl << frame_vs.getLastError() << endl;
        return;
    }
    if (!frame_fs.compile((const char *)rcs::shaders_camera_frame_dma_fs_data))
    {
        clog << "failed to compile camera frame GLSL fragment shader:" << endl << frame_vs.getLastError() << endl;
        return;
    }
    frame_prog.attachShader(frame_vs.id);
    frame_prog.attachShader(frame_fs.id);
    if (!frame_prog.link())
    {
        clog << "failed to link camera frame GLSL program:" << endl << frame_prog.getLastError() << endl;
        return;
    }
    attr_loc_frame_prog_pos = frame_prog.getAttribute("position").location;
    attr_loc_frame_prog_tex_coord = frame_prog.getAttribute("tex_coord").location;
    uni_loc_frame_prog_yuv = frame_prog.getUniform("frame_yuv").location;
    ok = true;
}

static void get_colour_space_info(std::optional<libcamera::ColorSpace> const &cs, EGLint &encoding, EGLint &range)
{
    encoding = EGL_ITU_REC601_EXT;
    range = EGL_YUV_NARROW_RANGE_EXT;

    if (cs == libcamera::ColorSpace::Sycc)
        range = EGL_YUV_FULL_RANGE_EXT;
    else if (cs == libcamera::ColorSpace::Smpte170m)
        /* all good */;
    else if (cs == libcamera::ColorSpace::Rec709)
        encoding = EGL_ITU_REC709_EXT;
    else
        clog << "unexpected colour space " << libcamera::ColorSpace::toString(cs) << endl;
}

GLTexture2D *GLObjectsDMA::getTextureFor(libcamera::FrameBuffer *fb, const libcamera::StreamConfiguration &cfg)
{
    const auto &planes = fb->planes();
    if (planes.size() != 3)
        abort();
    auto find_re = textures.find(planes[0].fd.get());
    if (find_re != textures.end())
    {
        return find_re->second.get();
    }
    else
    {
        // create EGL image
        EGLint encoding, range;
        get_colour_space_info(cfg.colorSpace, encoding, range);
        EGLint attribs[] = {EGL_WIDTH,
                            static_cast<EGLint>(cfg.size.width),
                            EGL_HEIGHT,
                            static_cast<EGLint>(cfg.size.height),
                            EGL_LINUX_DRM_FOURCC_EXT,
                            DRM_FORMAT_YUV420,
                            EGL_DMA_BUF_PLANE0_FD_EXT,
                            planes[0].fd.get(),
                            EGL_DMA_BUF_PLANE0_OFFSET_EXT,
                            EGLint(planes[0].offset),
                            EGL_DMA_BUF_PLANE0_PITCH_EXT,
                            EGLint(cfg.stride),
                            EGL_DMA_BUF_PLANE1_FD_EXT,
                            planes[1].fd.get(),
                            EGL_DMA_BUF_PLANE1_OFFSET_EXT,
                            EGLint(planes[1].offset),
                            EGL_DMA_BUF_PLANE1_PITCH_EXT,
                            EGLint(cfg.stride / 2),
                            EGL_DMA_BUF_PLANE2_FD_EXT,
                            planes[2].fd.get(),
                            EGL_DMA_BUF_PLANE2_OFFSET_EXT,
                            EGLint(planes[2].offset),
                            EGL_DMA_BUF_PLANE2_PITCH_EXT,
                            EGLint(cfg.stride / 2),
                            EGL_YUV_COLOR_SPACE_HINT_EXT,
                            encoding,
                            EGL_SAMPLE_RANGE_HINT_EXT,
                            range,
                            EGL_NONE};
        auto display = eglGetCurrentDisplay();
        EGLImage image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs);
        if (!image)
            throw std::runtime_error("failed create EGL image from fd " + std::to_string(planes[0].fd.get()));

        // create texture
        std::unique_ptr<GLTexture2D> tex(new GLTexture2D(GL_TEXTURE_EXTERNAL_OES, true, false, false));
        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);

        // finalize
        eglDestroyImage(display, image);
        GLTexture2D *re = tex.get();
        textures[planes[0].fd.get()] = std::move(tex);
        return re;
    }
}

struct RenderThread_EglDma::Guts
{
    std::unique_ptr<GLObjectsDMA> globjs;
};

#define BREAK_WHEN_FAIL(EXPR)                                                                                          \
    if (!bool(EXPR))                                                                                                   \
    {                                                                                                                  \
        assert(false);                                                                                                 \
        reader.sendBackFinishedRequest(req);                                                                           \
        break;                                                                                                         \
    }

RenderThread_EglDma::RenderThread_EglDma(CameraReader &reader) : RenderThreadBase(reader), guts(new Guts)
{
}

RenderThread_EglDma::~RenderThread_EglDma()
{
    stop();
}

void RenderThread_EglDma::setupGL()
{
    guts->globjs.reset(new GLObjectsDMA);
}

void RenderThread_EglDma::handleBuffer(libcamera::FrameBuffer *buf)
{
    const auto &planes = buf->planes();
    if (buf->planes().size() != 3)
    {
        assert(false);
        return;
    }
    auto *cam_cfg = reader().cameraConfig();
    guts->globjs->prev_set_texture = guts->globjs->getTextureFor(buf, cam_cfg->at(0));
}

void RenderThread_EglDma::renderFrame()
{
    if (guts->globjs->prev_set_texture == nullptr)
        return;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, guts->globjs->prev_set_texture->id);

    GLProgram::BindScope prog_scope(guts->globjs->frame_prog);

    guts->globjs->frame_prog.configAttribute(guts->globjs->attr_loc_frame_prog_pos, 0, 16);
    guts->globjs->frame_prog.configAttribute(guts->globjs->attr_loc_frame_prog_tex_coord, 8, 16);
    prog_scope.setUniform(guts->globjs->uni_loc_frame_prog_yuv, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    CHECK_GL;
}

void RenderThread_EglDma::shutdownGL()
{
    guts->globjs.reset();
}
