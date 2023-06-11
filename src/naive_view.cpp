#include "CameraReader.h"
#include "LightSensorThread.h"
#include "RenderThread_EglDma.h"
#include "Settings.h"
#include "LuxSetterThread.h"

#include <SDL.h>
#include <libcamera/camera_manager.h>
#include <libcamera/formats.h>

#include <iostream>
#include <thread>
using std::clog;
using std::endl;

int main()
{
    auto &settings = Settings::getInstance();

    std::string i2c_devfile = std::string("/dev/i2c-") + std::to_string(settings.get_i2c_index());

    LuxSetterThread lux_applier;
    LightSensorThread lumi_sensor(i2c_devfile);

    if (SDL_VideoInit("KMSDRM") != 0)
    {
        clog << "failed to initialize SDL video in KMSDRM mode" << endl;
        exit(EXIT_FAILURE);
    }
    using namespace std::chrono_literals;
    libcamera::CameraManager mgr;
    mgr.start();
    auto cameras = mgr.cameras();
    if (cameras.size() == 0)
    {
        clog << "no camera found" << endl;
        exit(EXIT_FAILURE);
    }

    CameraReader cam_reader(cameras[0]);
    cam_reader.configure(libcamera::formats::YUV420, settings.get_camera_width(), settings.get_camera_height(), 20000);

    RenderThread_EglDma render(cam_reader);

    cam_reader.start();
    render.start();

    // main loop
    SDL_Event e;
    bool should_break = false;
    while (!should_break)
    {
        // poll SDL events
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                should_break = true;
            else if (e.type == SDL_KEYDOWN)
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    should_break = true;
        }

        // process luminance
        float lux = lumi_sensor.getLuminance();
        lux_applier.setSensorLuminance(lux);
    }

    // finalize
    render.stop();
    cam_reader.stop();
    SDL_Quit();
}