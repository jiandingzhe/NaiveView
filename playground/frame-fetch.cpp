#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

#include <libcamera/libcamera.h>
using namespace libcamera;
using std::cout;
using std::clog;
using std::endl;

static std::shared_ptr<Camera> camera;

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
    
    // pick stream config
    clog << "supported stream configs:" << endl;
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::Raw } );
    for (auto& stream_config : *config)
        clog << "  " << stream_config.toString() << endl;
        
    auto& stream_cfg = config->at(0);
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
    if (ret < 0) {
       std::cerr << "Can't allocate buffers" << std::endl;
       return -ENOMEM;
    }

    size_t allocated = allocator->buffers(stream_cfg.stream()).size();
    std::cout << "Allocated " << allocated << " buffers for stream" << std::endl;

    // destruct
    camera->release();
    camera = nullptr;   
}