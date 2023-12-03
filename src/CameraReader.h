#pragma once

#include "TaskQueue.h"

#include <libcamera/camera.h>

#include <memory>

class CameraReader
{
  public:
    typedef TaskQueue<libcamera::Request*> ReqQueue;
    
    CameraReader(std::shared_ptr<libcamera::Camera> camera);
    ~CameraReader();

    ReqQueue* filledRequests();
    void sendBackFinishedRequest(libcamera::Request*);
    bool configure(libcamera::PixelFormat fmt, int width, int height, int expectedIntervalUS);
    libcamera::CameraConfiguration* cameraConfig() const;
    int getActualWidth() const;
    int getActualHeight() const;
    float getBrightness() const;
    float getContrast() const;
    void setBrightness(float);
    void setContrast(float);
    bool start();
    void stop();

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};