#include "RenderThreadBase.h"

#include "SDL_video.h"
#include "SdlHelpers.h"
#include "glhelpers/Buffer.h"

#include <chrono>
#include <iostream>
#include <thread>

#define FPS_INTERVAL 10

using namespace std::chrono_literals;

using std::clog;
using std::endl;

struct ScreenPoint
{
    float x;
    float y;
    float s;
    float t;
};

ScreenPoint screen_rect_landscape[4] = {
    {-1.0f, -1.0f, 0.0, 0.0f}, {-1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, -1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}};

ScreenPoint screen_rect_portrait[4] = {
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
    size_t frame_count = 0;
    std::chrono::steady_clock::time_point prev_fps_time;
    float fps = 0.0f;
};

RenderThreadBase::Guts::Guts(RenderThreadBase &self, CameraReader &reader) : self(self), reader(reader)
{
}

void RenderThreadBase::Guts::thread_body()
{
    // get window size
    int win_w = 0;
    int win_h = 0;
    SDL_GetWindowSize(window.get(), &win_w, &win_h);
    clog << "window size obtained in OpenGL thread: " << win_w << " " << win_h << endl;

    // create OpenGL context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    gl_ctx = SDL_GL_CreateContext(window.get());
    if (gl_ctx == nullptr)
        throw std::runtime_error("failed to create SDL OpenGL context");

    SDL_GL_SetSwapInterval(1);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // create OpenGL objects
    screen_rect_vbuf.reset(new GLBuffer(GL_ARRAY_BUFFER));
    {
        GLBuffer::BindScope scope(*screen_rect_vbuf);
        if (win_w >= win_h)
            scope.setData(screen_rect_landscape, sizeof(screen_rect_landscape), GL_STATIC_DRAW);
        else
            scope.setData(screen_rect_portrait, sizeof(screen_rect_portrait), GL_STATIC_DRAW);
    }
    self.setupGL();

    while (gl_stop_flag == 0)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // try to fetch frame
        libcamera::Request *req = nullptr;
        if (reader.filledRequests()->fetch(req))
        {
            if (req->buffers().size() > 0)
            {
                auto *buf = req->buffers().begin()->second;
                self.handleBuffer(buf);
            }
            reader.sendBackFinishedRequest(req);
            frame_count += 1;
            if (frame_count % FPS_INTERVAL == 0)
            {
                auto curr_time = std::chrono::steady_clock::now();
                if (frame_count > 0)
                {
                    auto ten_frame_time = curr_time - prev_fps_time;
                    double ten_frame_sec = double(ten_frame_time.count()) *
                                           double(decltype(ten_frame_time)::period::num) /
                                           double(decltype(ten_frame_time)::period::den);
                    fps = float(10.0 / ten_frame_sec);
                }
                prev_fps_time = curr_time;
            }
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
    gl_ctx = nullptr;
}

RenderThreadBase::RenderThreadBase(CameraReader &reader) : guts(new Guts(*this, reader))
{
}

RenderThreadBase::~RenderThreadBase()
{
    assert(guts->window == nullptr);
    assert(guts->gl_thread == nullptr);
}

CameraReader &RenderThreadBase::reader()
{
    return guts->reader;
}

void RenderThreadBase::start()
{
    guts->window.reset(
        SDL_CreateWindow("NaiveView", 0, 0, 1080, 1920,
                         SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL));
    if (guts->window == nullptr)
        throw std::runtime_error("failed to create SDL window");

    guts->gl_thread.reset(new std::thread([this]() { guts->thread_body(); }));
}

void RenderThreadBase::stop()
{
    if (guts->gl_thread != nullptr)
    {
        guts->gl_stop_flag = 1;
        guts->gl_thread->join();
        guts->gl_thread.reset();
        guts->gl_stop_flag = 0;
        guts->window.reset();
    }
}