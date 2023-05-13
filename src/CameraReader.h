#pragma once

#include "TaskQueue.h"

#include <libcamera/camera.h>

#include <memory>

class CameraReader
{
  public:
    typedef TaskQueue<libcamera::Request> ReqQueue;
    
    CameraReader(std::shared_ptr<libcamera::Camera> camera);
    ~CameraReader();

    ReqQueue* filledRequests();
    void sendBackFinishedRequest(libcamera::Request*);
    bool configure(int width, int height, int expectedIntervalUS);
    bool start();
    void stop();

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};