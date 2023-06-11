#include "WaveshareScreenBrightnessSetter.h"

#include "Settings.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <thread>

#include <fcntl.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace fs = std::filesystem;
using std::clog;
using std::endl;

#define WAVESHARE_REPORT_ID 9
#define WAVESHARE_VENDOR_ID 1810
#define WAVESHARE_PRODUCT_ID 10

WaveshareScreenBrightnessSetter::WaveshareScreenBrightnessSetter()
{
    try_open_device();
}

WaveshareScreenBrightnessSetter::~WaveshareScreenBrightnessSetter()
{
    if (m_fd >= 0)
        close(m_fd);
}

bool WaveshareScreenBrightnessSetter::setBrightness(float brightnessNorm)
{
    if (m_fd < 0)
        try_open_device();

    if (m_fd < 0)
    {
        clog << "failed to locate USB HID port for waveshare screen brightnesss" << endl;
        m_last_brightness = std::numeric_limits<float>::signaling_NaN();
        return false;
    }

    brightnessNorm = std::max(0.0f, std::min(1.0f, brightnessNorm));
    if (m_last_brightness == brightnessNorm)
    {
        return true;
    }

    int bright_i = (int)std::round(brightnessNorm * 255);
    unsigned char v = (unsigned char)bright_i;
    unsigned char vxor = v ^ (unsigned char)(0xFF);
    unsigned char mesg[5] = {WAVESHARE_REPORT_ID, 0x08, 0xF7, v, vxor};
    if (write(m_fd, mesg, 5) == 5)
    {
        m_last_brightness = brightnessNorm;
        return true;
    }
    else
    {
        close(m_fd);
        m_fd = -1;
        m_last_brightness = std::numeric_limits<float>::signaling_NaN();
        return false;
    }
}

void WaveshareScreenBrightnessSetter::try_open_device()
{
    // traverse all hidraw devices to find expected one
    fs::path dev_path("/dev");
    for (const auto &dir_entry : fs::directory_iterator(dev_path))
    {
        if (dir_entry.path().filename().string().find("hidraw") != 0)
            continue;

        int curr_fd = open(dir_entry.path().c_str(), O_RDWR);
        if (curr_fd < 0)
            continue;

        struct hidraw_devinfo info = {};
        ioctl(curr_fd, HIDIOCGRAWINFO, &info);
        if (info.vendor != WAVESHARE_VENDOR_ID || info.product != WAVESHARE_PRODUCT_ID)
        {
            close(curr_fd);
            continue;
        }

        m_fd = curr_fd;
        break;
    }
}

struct WaveshareScreenBrightnessSetter::Runner::Guts
{
    void thread_body();
    std::atomic<int> stop_flag{0};
    std::atomic<float> incoming_lux = std::numeric_limits<float>::signaling_NaN();
    std::unique_ptr<std::thread> thread;
    WaveshareScreenBrightnessSetter setter;
};

WaveshareScreenBrightnessSetter::Runner::Runner() : guts(new Guts)
{
    guts->thread.reset(new std::thread([this]() { guts->thread_body(); }));
}

#define NEW_LUX_WEIGHT 0.5F

void WaveshareScreenBrightnessSetter::Runner::Guts::thread_body()
{
    using namespace std::chrono_literals;
    unsigned iter_count = 0;
    float curr_lux = std::numeric_limits<float>::signaling_NaN();
    while (stop_flag == 0)
    {
        // update luminance
        float new_lux = incoming_lux;
        if (std::isnan(curr_lux))
            curr_lux = new_lux;
        else
            curr_lux = new_lux * NEW_LUX_WEIGHT + curr_lux * (1.0f - NEW_LUX_WEIGHT);

        // set screen brightness
        if (iter_count % 8 == 0)
        {
            auto &settings = Settings::getInstance();
            float bright = 0.67f;
            if (!std::isnan(curr_lux))
            {
                if (curr_lux < settings.get_lux_bound1())
                    bright = settings.get_screen_min_brightness();
                else if (curr_lux < settings.get_lux_bound2())
                    bright = settings.get_screen_mid_brightness();
                else
                    bright = 1.0f;
            }
            setter.setBrightness(bright);
        }

        // finalize this iteration
        std::this_thread::sleep_for(200ms);
        iter_count += 1;
    }
}

WaveshareScreenBrightnessSetter::Runner::~Runner()
{
    guts->stop_flag = 1;
    guts->thread->join();
    guts->thread.reset();
    guts->stop_flag = 0;
}

void WaveshareScreenBrightnessSetter::Runner::setSensorLuminance(float lux)
{
    guts->incoming_lux = lux;
}