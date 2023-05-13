#include "CameraReader.h"

#include <libcamera/libcamera.h>

#include <iostream>
#include <thread>

using std::clog;
using std::endl;

struct CameraReader::Guts
{
    void handle_request(libcamera::Request *req);

    std::unique_ptr<ReqQueue> queue;
    std::shared_ptr<libcamera::Camera> camera;
    std::unique_ptr<libcamera::FrameBufferAllocator> allocator;
    std::vector<std::unique_ptr<libcamera::Request>> reqs;
    std::atomic<int> is_playing{0};

    std::int64_t fuck_pair[2] = {};
    int use_w = 0;
    int use_h = 0;
};

void CameraReader::Guts::handle_request(libcamera::Request *req)
{
    if (req->status() == libcamera::Request::RequestComplete)
        queue->add(req);
}

CameraReader::CameraReader(std::shared_ptr<libcamera::Camera> camera) : guts(new Guts)
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

CameraReader::ReqQueue *CameraReader::filledRequests()
{
    return guts->queue.get();
}

void CameraReader::sendBackFinishedRequest(libcamera::Request*req)
{
    // queue for next frame
    if (guts->is_playing != 0)
    {
        req->reuse(libcamera::Request::ReuseBuffers);
        guts->camera->queueRequest(req);
    }
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
    guts->reqs.clear();
    guts->fuck_pair[0] = expectedIntervalUS;
    guts->fuck_pair[1] = expectedIntervalUS;
    for (unsigned int i = 0; i < buffers.size(); ++i)
    {
        auto req = guts->camera->createRequest();
        if (req == nullptr)
        {
            clog << "failed to create request for camera " << guts->camera->id() << endl;
            return false;
        }

        req->controls().set(controls::FrameDurationLimits, Span<const int64_t, 2>(guts->fuck_pair));

        const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
        int ret = req->addBuffer(stream, buffer.get());
        if (ret < 0)
        {
            clog << "Can't set buffer for request" << std::endl;
            return false;
        }

        guts->reqs.push_back(std::move(req));
    }
    guts->queue.reset(new ReqQueue(unsigned(buffers.size())));
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
    for (auto &req : guts->reqs)
    {
        guts->camera->queueRequest(req.get());
    }
    return true;
}

void CameraReader::stop()
{
    using namespace std::chrono_literals;
    guts->is_playing = 0;
    std::this_thread::sleep_for(10ms);
    guts->camera->stop();
}