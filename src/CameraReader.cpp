#include "CameraReader.h"
#include "FrameTask.h"

#include <libcamera/libcamera.h>

#include <iostream>
#include <thread>

using std::clog;
using std::endl;

struct CameraReader::Guts
{
    Guts(TaskQueue<FrameTask> &queue);
    void handle_request(libcamera::Request *req);

    TaskQueue<FrameTask> &queue;
    std::shared_ptr<libcamera::Camera> camera;
    std::unique_ptr<libcamera::FrameBufferAllocator> allocator;
    std::vector<std::unique_ptr<FrameTask>> tasks;
    std::atomic<int> is_playing{0};

    std::int64_t fuck_pair[2] = {};
    int use_w = 0;
    int use_h = 0;
};

CameraReader::Guts::Guts(TaskQueue<FrameTask> &queue) : queue(queue)
{
}

void CameraReader::Guts::handle_request(libcamera::Request *req)
{
    using namespace std::chrono_literals;
    auto *task = (FrameTask *)req->cookie();
    assert(task->status == FrameTask::TaskStatus_Snapping);
    if (is_playing == 0)
        return;

    // send frame to consumer
    int expect_status = FrameTask::TaskStatus_Snapping;
    if (!task->status.compare_exchange_weak(expect_status, FrameTask::TaskStatus_Drawing))
    {
        clog << "failed to exchange task from snapping to drawing: current status "
             << toString(FrameTask::TaskStatus(expect_status)) << endl;
        return;
    }
    queue.add(task);
    while (is_playing != 0 && task->status != FrameTask::TaskStatus_Idle)
        std::this_thread::sleep_for(1ms);

    // queue for next frame
    if (is_playing != 0)
    {
        int expect_status = FrameTask::TaskStatus_Idle;
        if (task->status.compare_exchange_weak(expect_status, FrameTask::TaskStatus_Snapping))
        {
            req->reuse(libcamera::Request::ReuseBuffers);
            camera->queueRequest(req);
        }
        else
        {
            assert(expect_status == FrameTask::TaskStatus_Deleting);
        }
    }
}

CameraReader::CameraReader(std::shared_ptr<libcamera::Camera> camera, TaskQueue<FrameTask> &queue)
    : guts(new Guts(queue))
{
    guts->camera = camera;
    guts->camera->acquire();
    guts->allocator.reset(new libcamera::FrameBufferAllocator(camera));
    guts->camera->requestCompleted.connect(guts.get(), &CameraReader::Guts::handle_request);
}

CameraReader::~CameraReader()
{
    guts->camera->release();
    guts->camera->requestCompleted.disconnect(guts.get());
}

bool CameraReader::configure(int expectWidth, int expectHeight, int expectedIntervalUS)
{
    using namespace libcamera;

    if (guts->is_playing != 0)
        return false;

    // pick stream config
    std::unique_ptr<CameraConfiguration> camera_config =
        guts->camera->generateConfiguration({StreamRole::VideoRecording});
    if (camera_config->empty())
    {
        clog << "failed to create any camera config from " << guts->camera->id() << endl;
        assert(false);
        return false;
    }

    auto &stream_cfg = camera_config->at(0);

    stream_cfg.size.width = expectWidth;
    stream_cfg.size.height = expectHeight;

    auto cfg_state = camera_config->validate();
    if (cfg_state == CameraConfiguration::Invalid)
    {
        clog << "invalid camera stream configuration " << stream_cfg.toString() << " (via width " << expectWidth
             << " height " << expectHeight << ")" << endl;
        return false;
    }
    guts->use_w = stream_cfg.size.width;
    guts->use_h = stream_cfg.size.height;

    if (guts->camera->configure(camera_config.get()) != 0)
    {
        clog << "camera configure failed" << endl;
        assert(false);
        return false;
    }

    // allocate buffer
    if (guts->allocator->allocate(stream_cfg.stream()) < 0)
    {
        clog << "failed to allocate buffer for " << stream_cfg.toString() << endl;
        return false;
    }
    auto *stream = stream_cfg.stream();
    auto &buffers = guts->allocator->buffers(stream);

    // create request for each buffer
    guts->tasks.clear();
    guts->fuck_pair[0] = expectedIntervalUS;
    guts->fuck_pair[1] = expectedIntervalUS;
    for (unsigned int i = 0; i < buffers.size(); ++i)
    {
        std::unique_ptr<FrameTask> task(new FrameTask);
        task->request = guts->camera->createRequest((std::uint64_t)task.get());
        if (task->request == nullptr)
        {
            clog << "failed to create request for camera " << guts->camera->id() << endl;
            return false;
        }

        task->request->controls().set(controls::FrameDurationLimits, Span<const int64_t, 2>(guts->fuck_pair));

        const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
        int ret = task->request->addBuffer(stream, buffer.get());
        if (ret < 0)
        {
            clog << "Can't set buffer for request" << std::endl;
            return false;
        }

        guts->tasks.push_back(std::move(task));
    }
    return true;
}

bool CameraReader::start()
{
    using namespace std::chrono_literals;
    if (guts->camera->start() != 0)
    {
        clog << "failed to start camera " << guts->camera->id() << endl;
        return false;
    }
    guts->is_playing = 1;
    for (auto &task : guts->tasks)
    {
        int expect_status = FrameTask::TaskStatus_Idle;
        while (!task->status.compare_exchange_weak(expect_status, FrameTask::TaskStatus_Snapping))
        {
            if (expect_status == FrameTask::TaskStatus_Deleting)
                return false;
            expect_status = FrameTask::TaskStatus_Idle;
            std::this_thread::sleep_for(1ms);
        }
        guts->camera->queueRequest(task->request.get());
    }
    return true;
}

void CameraReader::stop()
{
    using namespace std::chrono_literals;
    guts->camera->stop();
    std::this_thread::sleep_for(10ms);
    guts->is_playing = 0;
}