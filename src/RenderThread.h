#pragma once

#include "FrameTask.h"
#include "TaskQueue.h"

#include <memory>

class RenderThread
{
  public:
    RenderThread(TaskQueue<FrameTask> &queue);
    ~RenderThread();
  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};