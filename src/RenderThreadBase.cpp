#include "RenderThreadBase.h"

#include "SdlHelpers.h"
#include "glhelpers/Buffer.h"

#include <thread>
struct ScreenPoint
{
    float x;
    float y;
    float s;
    float t;
};

ScreenPoint screen_rect[4] = {
    {-1.0f, -1.0f, 1.0, 0.0f}, {-1.0f, 1.0f, 0.0f, 0.0f}, {1.0f, -1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}};

struct RenderThreadBase::Guts
{
    Guts(RenderThreadBase &self, CameraReader &reader);
    void thread_body();
    RenderThreadBase &self;
    CameraReader &reader;
    std::unique_ptr<SDL_Window> window;
    SDL_GLContext gl_ctx = nullptr;
    std::unique_ptr<GLBuffer> screen_rect_vbuf;
    std::atomic<int> gl_stop_flag{0};
    std::unique_ptr<std::thread> gl_thread;
};

RenderThreadBase::Guts::Guts(RenderThreadBase &self, CameraReader &reader) : self(self), reader(reader)
{
}

void RenderThreadBase::Guts::thread_body()
{
    // create OpenGL context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    gl_ctx = SDL_GL_CreateContext(window.get());
    if (gl_ctx == nullptr)
        throw std::runtime_error("failed to create SDL OpenGL context");

    SDL_GL_SetSwapInterval(0);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // create OpenGL objects
    screen_rect_vbuf.reset(new GLBuffer(GL_ARRAY_BUFFER));
    {
        GLBuffer::BindScope scope(*screen_rect_vbuf);
        scope.setData(screen_rect, sizeof(screen_rect), GL_STATIC_DRAW);
    }
    self.setupGL();

    while (gl_stop_flag == 0)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // try to fetch frame
        auto *req = reader.filledRequests()->fetch();
        if (req != nullptr)
        {
            if (req->buffers().size() > 0)
            {
                auto *buf = req->buffers().begin()->second;
                self.handleBuffer(buf);
            }
            reader.sendBackFinishedRequest(req);
        }
        {
            GLBuffer::BindScope buf_scope(*screen_rect_vbuf);
            self.renderFrame();
        }

        // finish one frame
        SDL_GL_SwapWindow(window.get());
    }

    // finalize
    self.shutdownGL();
    SDL_GL_DeleteContext(gl_ctx);
}

RenderThreadBase::RenderThreadBase(CameraReader &reader) : guts(new Guts(*this, reader))
{
    guts->window.reset(
        SDL_CreateWindow("NaiveView", 0, 0, 1080, 1920,
                         SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL));
    if (guts->window == nullptr)
        throw std::runtime_error("failed to create SDL window");
}

RenderThreadBase::~RenderThreadBase()
{
    assert(guts->gl_thread == nullptr);
}

CameraReader& RenderThreadBase::reader()
{
    return guts->reader;
}

void RenderThreadBase::start()
{
    guts->gl_thread.reset(new std::thread([this]() { guts->thread_body(); }));
}

void RenderThreadBase::stop()
{
    guts->gl_stop_flag = 1;
    guts->gl_thread->join();
    guts->gl_thread.reset();
    guts->gl_stop_flag = 0;
}