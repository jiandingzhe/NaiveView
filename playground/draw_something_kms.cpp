#include <SDL.h>

#include <cstring>
#include <iostream>
#include <thread>

#include <SDL_opengles2.h>

using namespace std::chrono_literals;
using std::clog;
using std::endl;

int main()
{
    auto init_re = SDL_VideoInit("KMSDRM");
    if (init_re != 0)
        exit(EXIT_FAILURE);

    SDL_Window *window = SDL_CreateWindow("SDL2", 0, 0, 1920, 1080, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (nullptr == window)
    {
        std::cerr << "SDL_CreateWindow(): " << SDL_GetError() << '\n';
        exit(EXIT_FAILURE);
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    auto gl = SDL_GL_CreateContext(window);
    clog << "create OpenGL context " << gl << endl;
    auto make_current_re = SDL_GL_MakeCurrent(window, gl);
    clog << "make current ctx returned " << make_current_re << endl;
    clog << "OpenGL version: " << (const char *)glGetString(GL_VERSION) << ", GLSL "
         << (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glClearColor(0.5, 0.5, 0.5, 1.0);
    size_t frame_count = 0;
    SDL_Event e;
    bool should_break = false;
    while (!should_break)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                should_break = true;
            else if (e.type == SDL_KEYDOWN)
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    should_break = true;
        }
        frame_count += 1;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
        clog << "frame " << frame_count << endl;
        std::this_thread::sleep_for(30ms);
    }

    SDL_GL_DeleteContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
