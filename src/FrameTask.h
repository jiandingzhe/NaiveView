#pragma once

#include <atomic>
#include <libcamera/request.h>

struct FrameTask
{
    enum TaskStatus
    {
        TaskStatus_Idle = 0,
        TaskStatus_Snapping = 1,
        TaskStatus_Drawing = 2,
        TaskStatus_Deleting = 3
    };

    ~FrameTask();

    std::atomic<int> status{0};
    std::unique_ptr<libcamera::Request> request;
};

const char* toString(FrameTask::TaskStatus);