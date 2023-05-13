#include "CameraReader.h"

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>

#include <chrono>
#include <iostream>
#include <thread>

#include <signal.h>

using std::clog;
using std::endl;

bool should_exit = false;

void on_term(int)
{
    should_exit = true;
}

int main()
{
    using namespace std::chrono_literals;
    signal(SIGTERM, on_term);

    libcamera::CameraManager mgr;
    mgr.start();
    auto cameras = mgr.cameras();
    if (cameras.size() == 0)
    {
        clog << "no camera found" << endl;
        exit(EXIT_FAILURE);
    }

    CameraReader reader(cameras[0]);
    reader.configure(1280, 720, 16000);
    auto* queue = reader.filledRequests();

    reader.start();
    std::chrono::steady_clock::time_point prev_time;
    while (!should_exit)
    {
        auto *req = queue->fetch();
        if (req == nullptr)
        {
            continue;
        }
        
        reader.sendBackFinishedRequest(req);

        // count FPS
        auto curr_time = std::chrono::steady_clock::now();
        if (prev_time != std::chrono::steady_clock::time_point())
        {
            auto duration = curr_time - prev_time;
            clog << "frame interval " << std::dec << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << endl;
        }
        prev_time = curr_time;
        
        
    }
    reader.stop();
    mgr.stop();
}