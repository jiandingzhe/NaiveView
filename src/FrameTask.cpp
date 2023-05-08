#include "FrameTask.h"

#include <thread>

FrameTask::~FrameTask()
{
    using namespace std::chrono_literals;
    int expect_status = TaskStatus_Idle;
    while (!status.compare_exchange_weak(expect_status, TaskStatus_Deleting))
    {
        expect_status = TaskStatus_Idle;
        std::this_thread::sleep_for(5ms);
    }
}

const char *toString(FrameTask::TaskStatus s)
{
    switch (s)
    {
    case FrameTask::TaskStatus_Idle:
        return "idle";
    case FrameTask::TaskStatus_Snapping:
        return "snapping";
    case FrameTask::TaskStatus_Drawing:
        return "drawing";
    case FrameTask::TaskStatus_Deleting:
        return "deleting";
    default:
        assert(false);
        return "INVALID";
    }
}
