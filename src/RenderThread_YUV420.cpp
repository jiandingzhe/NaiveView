#include "RenderThread_YUV420.h"

#include "glhelpers/Buffer.h"
#include "glhelpers/Program.h"
#include "glhelpers/Shader.h"
#include "glhelpers/Texture.h"
#include "glhelpers/Utils.h"

#include "shaders/camera_frame_vs.h"
#include "shaders/camera_frame_yuv420_fs.h"

#include <SDL_video.h>
#include <libcamera/framebuffer.h>

#include <atomic>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <sys/mman.h>

using std::clog;
using std::endl;

struct GLObjectsYUV420
{
    GLObjectsYUV420();

    GLTexture2D tex_frame_y{true, false, false};
    GLTexture2D tex_frame_u{true, false, false};
    GLTexture2D tex_frame_v{true, false, false};
    GLShader frame_vs{GL_VERTEX_SHADER};
    GLShader frame_fs{GL_FRAGMENT_SHADER};
    GLProgram frame_prog;
    GLuint attr_loc_frame_prog_pos = -1;
    GLuint attr_loc_frame_prog_tex_coord = -1;
    GLint uni_loc_frame_prog_comp_y = -1;
    GLint uni_loc_frame_prog_comp_u = -1;
    GLint uni_loc_frame_prog_comp_v = -1;
    bool ok = false;
};

GLObjectsYUV420::GLObjectsYUV420()
{
    if (!frame_vs.compile((const char *)rcs::shaders_camera_frame_vs_data))
    {
        clog << "failed to compile camera frame GLSL vertex shader:" << endl << frame_vs.getLastError() << endl;
        return;
    }
    if (!frame_fs.compile((const char *)rcs::shaders_camera_frame_yuv420_fs_data))
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
    uni_loc_frame_prog_comp_y = frame_prog.getUniform("frame_y").location;
    uni_loc_frame_prog_comp_u = frame_prog.getUniform("frame_u").location;
    uni_loc_frame_prog_comp_v = frame_prog.getUniform("frame_v").location;
    ok = true;
}

struct RenderThread_YUV420::Guts
{
    std::unique_ptr<GLObjectsYUV420> globjs;
};

#define BREAK_WHEN_FAIL(EXPR)                                                                                          \
    if (!bool(EXPR))                                                                                                   \
    {                                                                                                                  \
        assert(false);                                                                                                 \
        reader.sendBackFinishedRequest(req);                                                                           \
        break;                                                                                                         \
    }

static void copy_plane_to_texture(const unsigned char *addr, const libcamera::FrameBuffer::Plane &plane, int w, int h,
                                  GLTexture2D &texture)
{
    if (w * h != plane.length)
    {
        assert(false);
        return;
    }
    GLTexture2D::BindScope scope_y(texture);
    scope_y.setMono8Image(w, h, addr + plane.offset);
}

RenderThread_YUV420::RenderThread_YUV420(CameraReader &reader) : RenderThreadBase(reader), guts(new Guts)
{
}

RenderThread_YUV420::~RenderThread_YUV420()
{
    stop();
}

void RenderThread_YUV420::setupGL()
{
    guts->globjs.reset(new GLObjectsYUV420);
}
void RenderThread_YUV420::handleBuffer(libcamera::FrameBuffer *buf)
{
    const auto &planes = buf->planes();
    if (buf->planes().size() != 3)
    {
        assert(false);
        return;
    }
    assert(planes[0].offset == 0);
    auto fd = planes[0].fd.get();
    assert(fd == planes[1].fd.get());
    assert(fd == planes[2].fd.get());
    size_t total_size = planes[0].length + planes[1].length + planes[2].length;
    auto *addr = (unsigned char *)mmap(nullptr, total_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED)
    {
        clog << "mmap failed for camera dma fd " << fd << " size " << total_size << ": " << strerror(errno) << endl;
        abort();
    }
    int cam_w = reader().getActualWidth();
    int cam_h = reader().getActualHeight();
    copy_plane_to_texture(addr, planes[0], cam_w, cam_h, guts->globjs->tex_frame_y);
    copy_plane_to_texture(addr, planes[1], cam_w / 2, cam_h / 2, guts->globjs->tex_frame_u);
    copy_plane_to_texture(addr, planes[2], cam_w / 2, cam_h / 2, guts->globjs->tex_frame_v);
    glFinish();
    if (munmap(addr, total_size) != 0)
    {
        clog << "munmap failed for camera dma fd " << planes[0].fd.get() << " address " << std::hex << addr << std::dec
             << " size " << total_size << ": " << strerror(errno) << endl;
        abort();
    }
}

void RenderThread_YUV420::renderFrame()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, guts->globjs->tex_frame_y.id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, guts->globjs->tex_frame_u.id);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, guts->globjs->tex_frame_v.id);

    GLProgram::BindScope prog_scope(guts->globjs->frame_prog);

    guts->globjs->frame_prog.configAttribute(guts->globjs->attr_loc_frame_prog_pos, 0, 16);
    guts->globjs->frame_prog.configAttribute(guts->globjs->attr_loc_frame_prog_tex_coord, 8, 16);
    prog_scope.setUniform(guts->globjs->uni_loc_frame_prog_comp_y, 0);
    prog_scope.setUniform(guts->globjs->uni_loc_frame_prog_comp_u, 1);
    prog_scope.setUniform(guts->globjs->uni_loc_frame_prog_comp_v, 2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    CHECK_GL;
}

void RenderThread_YUV420::shutdownGL()
{
    guts->globjs.reset();
}
