#include "RenderThread.h"

#include "SdlHelpers.h"

#include "glhelpers/Buffer.h"
#include "glhelpers/Program.h"
#include "glhelpers/Shader.h"
#include "glhelpers/Texture.h"
#include "glhelpers/Utils.h"

#include "libcamera/framebuffer.h"
#include "shaders/camera_frame_fs.h"
#include "shaders/camera_frame_vs.h"

#include <SDL_video.h>

#include <atomic>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <sys/mman.h>

using std::clog;
using std::endl;

struct RenderThread::Guts
{
    struct GLObjects
    {
        GLObjects();
        GLBuffer screen_rect_vbuf{GL_ARRAY_BUFFER};
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

    Guts(CameraReader &reader);
    void thread_body();
    CameraReader &reader;
    std::unique_ptr<SDL_Window> window;
    SDL_GLContext gl_ctx = nullptr;
    std::unique_ptr<GLObjects> globjs;
    std::atomic<int> gl_stop_flag;
    std::unique_ptr<std::thread> gl_thread;
};

struct ScreenPoint
{
    float x;
    float y;
    float s;
    float t;
};

ScreenPoint screen_rect[4] = {
    {-1.0f, -1.0f, 1.0, 0.0f}, {-1.0f, 1.0f, 0.0f, 0.0f}, {1.0f, -1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}};

RenderThread::Guts::GLObjects::GLObjects()
{
    {
        GLBuffer::BindScope scope(screen_rect_vbuf);
        scope.setData(screen_rect, sizeof(screen_rect), GL_STATIC_DRAW);
    }
    if (!frame_vs.compile((const char *)rcs::shaders_camera_frame_vs_data))
    {
        clog << "failed to compile camera frame GLSL vertex shader:" << endl << frame_vs.getLastError() << endl;
        return;
    }
    if (!frame_fs.compile((const char *)rcs::shaders_camera_frame_fs_data))
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

RenderThread::Guts::Guts(CameraReader &reader) : reader(reader)
{
}

#define BREAK_WHEN_FAIL(EXPR)                                                                                          \
    if (!bool(EXPR))                                                                                                   \
    {                                                                                                                  \
        assert(false);                                                                                                 \
        goto REQ_BACK;                                                                                                 \
    }

static void copy_plane_to_texture(const libcamera::FrameBuffer::Plane &plane, int w, int h, GLTexture2D &texture)
{
    if (w * h != plane.length)
    {
        assert(false);
        return;
    }
    GLTexture2D::BindScope scope_y(texture);
    void *addr = mmap(nullptr, plane.length, PROT_NONE, MAP_PRIVATE, plane.fd.get(), plane.offset);
    scope_y.setMono8Image(w, h, addr);
    glFinish();
    munmap(addr, plane.length);
}

void RenderThread::Guts::thread_body()
{
    // create OpenGL context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    gl_ctx = SDL_GL_CreateContext(window.get());
    if (gl_ctx == nullptr)
        throw std::runtime_error("failed to create SDL OpenGL context");

    SDL_GL_SetSwapInterval(0);

    // create OpenGL objects
    globjs.reset(new GLObjects);
    if (!globjs->ok)
        goto GL_THREAD_END;

    while (gl_stop_flag == 0)
    {
        // try to fetch frame
        auto *req = reader.filledRequests()->fetch();
        if (req != nullptr)
        {
            libcamera::FrameBuffer *buf;
            int cam_w, cam_h;
            BREAK_WHEN_FAIL(req->buffers().size() > 0);
            buf = req->buffers().begin()->second;
            BREAK_WHEN_FAIL(buf->planes().size() == 3);
            cam_w = reader.getActualWidth();
            cam_h = reader.getActualHeight();
            copy_plane_to_texture(buf->planes()[0], cam_w, cam_h, globjs->tex_frame_y);
            copy_plane_to_texture(buf->planes()[1], cam_w, cam_h, globjs->tex_frame_u);
            copy_plane_to_texture(buf->planes()[2], cam_w, cam_h, globjs->tex_frame_v);
        REQ_BACK:
            reader.sendBackFinishedRequest(req);
        }

        // draw camera frame
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, globjs->tex_frame_y.id);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, globjs->tex_frame_u.id);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, globjs->tex_frame_v.id);

            GLProgram::BindScope prog_scope(globjs->frame_prog);
            GLBuffer::BindScope buf_scope(globjs->screen_rect_vbuf);
            globjs->frame_prog.configAttribute(globjs->attr_loc_frame_prog_pos, 0, sizeof(ScreenPoint));
            globjs->frame_prog.configAttribute(globjs->attr_loc_frame_prog_tex_coord, 8, sizeof(ScreenPoint));
            prog_scope.setUniform(globjs->uni_loc_frame_prog_comp_y, 0);
            prog_scope.setUniform(globjs->uni_loc_frame_prog_comp_u, 1);
            prog_scope.setUniform(globjs->uni_loc_frame_prog_comp_v, 2);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            CHECK_GL;
        }

        // finalize
        SDL_GL_SwapWindow(window.get());
    }

// finalize
GL_THREAD_END:
    globjs.reset();
    SDL_GL_DeleteContext(gl_ctx);
}

RenderThread::RenderThread(CameraReader &reader) : guts(new Guts(reader))
{
    guts->window.reset(
        SDL_CreateWindow("NaiveView", 0, 0, 1080, 1920,
                         SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL));
    if (guts->window == nullptr)
        throw std::runtime_error("failed to create SDL window");
}

RenderThread::~RenderThread()
{
    stop();
}

void RenderThread::start()
{
    guts->gl_thread.reset(new std::thread([this]() { guts->thread_body(); }));
}
void RenderThread::stop()
{

    guts->gl_stop_flag = 1;
    guts->gl_thread->join();
    guts->gl_thread.reset();
}