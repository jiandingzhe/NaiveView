#pragma once
#include "RenderThreadBase.h"

#include <memory>

class RenderThread_YUV420 : public RenderThreadBase
{
  public:
    RenderThread_YUV420(CameraReader &reader);
    ~RenderThread_YUV420() override;

  protected:
    void setupGL() override;
    void handleBuffer(libcamera::FrameBuffer *) override;
    void renderFrame() override;
    void shutdownGL() override;

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};