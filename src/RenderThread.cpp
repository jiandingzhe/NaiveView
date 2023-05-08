#include "RenderThread.h"

#include "SdlHelpers.h"

#include "glhelpers/Buffer.h"
#include "glhelpers/Program.h"
#include "glhelpers/Shader.h"
#include "glhelpers/Texture.h"

#include <SDL_video.h>

#include <atomic>
#include <stdexcept>
#include <thread>

struct RenderThread::Guts
{
    Guts(TaskQueue<FrameTask> &queue);
    void thread_body();
    TaskQueue<FrameTask> &task_queue;
    std::unique_ptr<SDL_Window> window;
    SDL_GLContext gl_ctx = nullptr;
    std::atomic<int> gl_stop_flag;
    std::unique_ptr<std::thread> gl_thread;
};

RenderThread::Guts::Guts(TaskQueue<FrameTask> &queue) : task_queue(queue)
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

    while (gl_stop_flag == 0)
    {
        // try to fetch frames
        FrameTask* task = task_queue.fetch();
    }

    SDL_GL_DeleteContext(gl_ctx);
}

RenderThread::RenderThread(TaskQueue<FrameTask> &queue) : guts(new Guts(queue))
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