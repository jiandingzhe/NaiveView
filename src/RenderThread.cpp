#include "RenderThread.h"

#include "SdlHelpers.h"

#include "glhelpers/Buffer.h"
#include "glhelpers/Program.h"
#include "glhelpers/Shader.h"
#include "glhelpers/Texture.h"

#include "shaders/camera_frame_fs.h"
#include "shaders/camera_frame_vs.h"

#include <SDL_video.h>

#include <atomic>
#include <stdexcept>
#include <thread>

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
    };

    Guts(CameraReader &reader);
    void thread_body();
    CameraReader& reader;
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
    frame_vs.compile((const char*)rcs::shaders_camera_frame_vs_data);
    frame_fs.compile((const char*)rcs::shaders_camera_frame_fs_data);
    frame_prog.attachShader(frame_vs.id);
    frame_prog.attachShader(frame_fs.id);
    frame_prog.link();
}

RenderThread::Guts::Guts(CameraReader &reader) : reader(reader)
{
}

void RenderThread::Guts::thread_body()
{
    // create OpenGL context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    gl_ctx = SDL_GL_CreateContext(window.get());
    if (gl_ctx == nullptr)
        throw std::runtime_error("failed to create SDL OpenGL context");

    // create OpenGL objects
    globjs.reset(new GLObjects);

    while (gl_stop_flag == 0)
    {
        // try to fetch frames
        auto* req = reader.filledRequests()->fetch();
    }

    // finalize
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

    guts->gl_thread.reset(new std::thread([this]() { guts->thread_body(); }));
}

RenderThread::~RenderThread()
{
    guts->gl_stop_flag = 1;
    guts->gl_thread->join();
}