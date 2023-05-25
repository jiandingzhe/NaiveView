#pragma once
#include "RenderThreadBase.h"

#include <memory>

class RenderThread_EglDma : public RenderThreadBase
{
  public:
    RenderThread_EglDma(CameraReader &reader);
    ~RenderThread_EglDma() override;

  protected:
    void setupGL() override;
    void handleBuffer(libcamera::FrameBuffer *) override;
    void renderFrame() override;
    void shutdownGL() override;

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};