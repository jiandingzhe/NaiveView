#pragma once
#include "RenderThreadBase.h"

#include <memory>

class RenderThread_RGB888 : public RenderThreadBase
{
  public:
    RenderThread_RGB888(CameraReader &reader);
    ~RenderThread_RGB888() override;

  protected:
    void setupGL() override;
    void handleBuffer(libcamera::FrameBuffer *) override;
    void renderFrame() override;
    void shutdownGL() override;

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};