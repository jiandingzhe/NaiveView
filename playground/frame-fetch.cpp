#include <libcamera/libcamera.h>

#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
using namespace std::chrono_literals;
using namespace libcamera;
using std::clog;
using std::cout;
using std::endl;

static std::shared_ptr<Camera> camera;

uint64_t prev_time = 0;

static void requestComplete(Request *req)
{
    if (req->status() == Request::RequestCancelled)
        return;
    clog << "on req complete" << endl;
    for (const auto &stream_and_buf : req->buffers())
    {
        auto *stream = stream_and_buf.first;
        auto *buf = stream_and_buf.second;
        if (prev_time > 0)
        {
            auto delta_time = buf->metadata().timestamp - prev_time;
            clog << "  stream " << std::hex << size_t(stream) << " buffer " << std::hex << size_t(buf) << ", interval "
                 << std::dec << double(delta_time) / 1000000.0 << endl;
        }
        for (const auto &plane : buf->planes())
        {
            clog << "    plane: fd " << plane.fd.get() << " position " << plane.offset << " length " << plane.length
                 << endl;
        }
        prev_time = buf->metadata().timestamp;
    }

    req->reuse(Request::ReuseBuffers);
    camera->queueRequest(req);
}

int64_t frame_duraion_limit[2] = {15000, 15000};
int main()
{
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();

    // pick first camera
    if (cm->cameras().empty())
    {
        clog << "no camera found" << endl;
        exit(EXIT_FAILURE);
    }
    camera = cm->cameras()[0];
    camera->acquire();

    // show camera controls
    clog << "camera controls:" << endl;
    for (auto &ctrl_pair : camera->controls())
    {
        clog << "  " << ctrl_pair.first->id() << " " << ctrl_pair.first->name() << ": " << ctrl_pair.second.toString()
             << endl;
    }

    // pick stream config
    clog << "supported stream configs:" << endl;
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration({StreamRole::VideoRecording});
    //std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration({StreamRole::Raw});
    for (auto &stream_config : *config)
        clog << "  " << stream_config.toString() << endl;

    auto &stream_cfg = config->at(0);
    clog << "stream config 0 pixel formats:" << endl;
    for (auto &px_fmt : stream_cfg.formats().pixelformats())
    {
        clog << "  " << px_fmt.toString() << ":";
        for (auto &px_fmt : stream_cfg.formats().sizes(px_fmt))
            clog << " " << px_fmt.toString();
        clog << endl;
    }
    stream_cfg.size.width = 1280;
    stream_cfg.size.height = 720;

    auto cfg_state = config->validate();
    if (cfg_state == CameraConfiguration::Valid)
        clog << "stream config is valid: " << stream_cfg.toString() << endl;
    else if (cfg_state == CameraConfiguration::Adjusted)
        clog << "stream config is adjusted to: " << stream_cfg.toString() << endl;
    else if (cfg_state == CameraConfiguration::Invalid)
    {
        clog << "stream config is invalid: " << stream_cfg.toString() << endl;
        exit(EXIT_FAILURE);
    }

    camera->configure(config.get());

    // allocate buffer
    FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);

    int ret = allocator->allocate(stream_cfg.stream());
    if (ret < 0)
    {
        std::cerr << "Can't allocate buffers" << std::endl;
        return -ENOMEM;
    }

    auto *stream = stream_cfg.stream();
    auto &buffers = allocator->buffers(stream);
    size_t allocated = buffers.size();
    std::cout << "Allocated " << allocated << " buffers for stream" << std::endl;

    // create request for each buffer
    clog << "creating requests" << endl;
    std::vector<std::unique_ptr<Request>> requests;
    for (unsigned int i = 0; i < buffers.size(); ++i)
    {
        std::unique_ptr<Request> request = camera->createRequest();
        if (!request)
        {
            std::cerr << "Can't create request" << std::endl;
            return -ENOMEM;
        }

        request->controls().set(controls::FrameDurationLimits, Span<const int64_t, 2>(frame_duraion_limit));

        const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
        int ret = request->addBuffer(stream, buffer.get());
        if (ret < 0)
        {
            std::cerr << "Can't set buffer for request" << std::endl;
            return ret;
        }

        clog << "  " << request->toString() << endl;
        requests.push_back(std::move(request));
    }

    // run
    camera->requestCompleted.connect(requestComplete);
    camera->start();
    for (std::unique_ptr<Request> &request : requests)
        camera->queueRequest(request.get());

    // wait for a while
    std::this_thread::sleep_for(3000ms);

    // destruct
    camera->stop();
    allocator->free(stream);
    delete allocator;
    camera->release();
    camera = nullptr;
    cm->stop();
}