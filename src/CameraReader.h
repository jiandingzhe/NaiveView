#pragma once

#include "FrameTask.h"
#include "TaskQueue.h"

#include <libcamera/camera.h>

#include <memory>

class CameraReader
{
  public:
    CameraReader(std::shared_ptr<libcamera::Camera> camera, TaskQueue<FrameTask> &queue);
    ~CameraReader();

    bool configure(int width, int height, int expectedIntervalUS);
    bool start();
    void stop();

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};