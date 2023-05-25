#include "CameraReader.h"
#include "RenderThread_EglDma.h"

#include <SDL.h>
#include <libcamera/camera_manager.h>
#include <libcamera/formats.h>

#include <iostream>
using std::clog;
using std::endl;

int main()
{
    libcamera::CameraManager mgr;
    mgr.start();
    auto cameras = mgr.cameras();
    if (cameras.size() == 0)
    {
        clog << "no camera found" << endl;
        exit(EXIT_FAILURE);
    }

    CameraReader cam_reader(cameras[0]);
    cam_reader.configure(libcamera::formats::YUV420, 1280, 720, 20000);

    RenderThread_EglDma render(cam_reader);

    cam_reader.start();
    render.start();

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
    }

    render.stop();
    cam_reader.stop();
}