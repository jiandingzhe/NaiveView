#include "RenderThread_RGB888.h"

#include "shaders/camera_frame_rgb_fs.h"
#include "shaders/camera_frame_vs.h"

#include "glhelpers/Buffer.h"
#include "glhelpers/Program.h"
#include "glhelpers/Shader.h"
#include "glhelpers/Texture.h"
#include "glhelpers/Utils.h"

#include <iostream>

#include <sys/mman.h>
#include <cstring>

using std::clog;
using std::endl;

struct GLObjectsRGB
{
    GLObjectsRGB();
    GLTexture2D tex_frame{true, false, false};
    GLShader frame_vs{GL_VERTEX_SHADER};
    GLShader frame_fs{GL_FRAGMENT_SHADER};
    GLProgram frame_prog;
    GLuint attr_loc_frame_prog_pos = -1;
    GLuint attr_loc_frame_prog_tex_coord = -1;
    GLint uni_loc_frame_prog_rgb = -1;
    bool ok = false;
};

GLObjectsRGB::GLObjectsRGB()
{
    if (!frame_vs.compile((const char *)rcs::shaders_camera_frame_vs_data))
    {
        clog << "failed to compile camera frame GLSL vertex shader:" << endl << frame_vs.getLastError() << endl;
        return;
    }
    if (!frame_fs.compile((const char *)rcs::shaders_camera_frame_rgb_fs_data))
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
    uni_loc_frame_prog_rgb = frame_prog.getUniform("frame_rgb").location;
    ok = true;
}

struct RenderThread_RGB888::Guts
{
    std::unique_ptr<GLObjectsRGB> globjs;
};

RenderThread_RGB888::RenderThread_RGB888(CameraReader &reader) : RenderThreadBase(reader), guts(new Guts)
{
}

RenderThread_RGB888::~RenderThread_RGB888()
{
    stop();
}

void RenderThread_RGB888::setupGL()
{
    guts->globjs.reset(new GLObjectsRGB);
}

void RenderThread_RGB888::handleBuffer(libcamera::FrameBuffer * buf)
{
    if (buf->planes().size() != 1)
    {
        assert(false);
        return;
    }
    const auto& plane = buf->planes()[0];
    auto fd = plane.fd.get();
    auto *addr = (unsigned char *)mmap(nullptr, plane.length, PROT_READ, MAP_PRIVATE, fd, plane.offset);
    if (addr == MAP_FAILED)
    {
        clog << "mmap failed for camera dma fd " << fd << " size " << plane.length << ": " << strerror(errno) << endl;
        abort();
    }
    int w = reader().getActualWidth();
    int h = reader().getActualHeight();
    if (w * h * 3 != plane.length)
    {
        assert(false);
        return;
    }
    {
        GLTexture2D::BindScope scope_y(guts->globjs->tex_frame);
        scope_y.setRGBImage(w, h, addr);
    }
    
    glFinish();
    if (munmap(addr, plane.length) != 0)
    {
        clog << "munmap failed for camera dma fd " << plane.fd.get() << " address " << std::hex << addr << std::dec
             << " size " << plane.length << ": " << strerror(errno) << endl;
        abort();
    }
}

void RenderThread_RGB888::renderFrame()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, guts->globjs->tex_frame.id);

    GLProgram::BindScope prog_scope(guts->globjs->frame_prog);

    guts->globjs->frame_prog.configAttribute(guts->globjs->attr_loc_frame_prog_pos, 0, 16);
    guts->globjs->frame_prog.configAttribute(guts->globjs->attr_loc_frame_prog_tex_coord, 8, 16);
    prog_scope.setUniform(guts->globjs->uni_loc_frame_prog_rgb, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    CHECK_GL;
}

void RenderThread_RGB888::shutdownGL()
{
    guts->globjs.reset();
}