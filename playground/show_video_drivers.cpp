#include <SDL.h>

#include <iostream>
#include <cstring>
using std::clog;
using std::endl;

int main()
{
    int n_drv = SDL_GetNumVideoDrivers();
    clog << n_drv << " SDL video drivers:" << endl;
    const char* sel_drv = nullptr;
    for ( int i = 0; i < n_drv; i+=1 )
    {
        auto* name = SDL_GetVideoDriver(i);
        clog << "  " << name << endl;
        if (strcmp(name, "KMSDRM") == 0)
            sel_drv = name;
    }

    auto init_re = SDL_VideoInit(sel_drv);
    clog << "init video " << sel_drv << " returned " << init_re << endl;
    SDL_VideoQuit();

    SDL_Window* window = SDL_CreateWindow
        (
        "SDL2",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        640, 480,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
        );
    if( nullptr == window )
    {
        std::cerr << "SDL_CreateWindow(): " << SDL_GetError() << '\n';
        return EXIT_FAILURE;
    }
    else
    {
        clog << "window created" << endl;
    }

    auto gl = SDL_GL_CreateContext(window);
    clog << "create OpenGL context " << gl << endl;
    auto make_current_re = SDL_GL_MakeCurrent(window, gl);
    clog << "make current ctx returned " << make_current_re << endl;
    

    SDL_GL_DeleteContext(gl);
    SDL_DestroyWindow( window );
    SDL_Quit();
}