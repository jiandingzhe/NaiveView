#pragma once

#include "CameraReader.h"

#include <memory>

class RenderThread
{
  public:
    RenderThread(CameraReader &reader);
    ~RenderThread();
    void start();
    void stop();

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};