#pragma once

#include "CameraReader.h"

#include <memory>

class RenderThreadBase
{
  public:
    RenderThreadBase(CameraReader &reader);
    virtual ~RenderThreadBase();
    
    CameraReader& reader();

    void start();
    void stop();

  protected:
    virtual void setupGL() = 0;
    virtual void handleBuffer(libcamera::FrameBuffer *) = 0;
    virtual void renderFrame() = 0;
    virtual void shutdownGL() = 0;

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};