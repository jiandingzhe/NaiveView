#pragma once

#include <SDL_video.h>
#include <memory>

namespace std
{
template <> struct default_delete<SDL_Window>
{
    void operator()(SDL_Window *window)
    {
        SDL_DestroyWindow(window);
    }
};
} // namespace std