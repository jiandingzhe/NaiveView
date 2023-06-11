#include "LightSensorThread.h"

#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <thread>

#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

using std::clog;
using std::endl;

enum LightSensorAddr
{
    AddrLow = 0x23,
    AddrHigh = 0x5c
};

#define CMD_POWER_DOWN 0x00
#define CMD_POWER_ON 0x01
#define CMD_RESET 0x07

struct LightSensorThread::Guts
{
    Guts(const std::string &devFile);
    void try_open_dev();

    bool &ready_flag(LightSensorAddr);
    bool prepare(LightSensorAddr);
    bool choose(LightSensorAddr);
    bool command(char cmd);
    float readout(LightSensorAddr);
    void stop(LightSensorAddr);
    void down();

    void thread_body();
    char mode = Mode_HighRes;
    char incoming_mode = Mode_HighRes;
    bool low_ready = false;
    bool high_ready = false;
    float last_lux_low = std::numeric_limits<float>::signaling_NaN();
    float last_lux_high = std::numeric_limits<float>::signaling_NaN();
    std::string dev_file;
    int fd = -1;
    std::atomic<int> stop_flag{0};
    std::unique_ptr<std::thread> thread;
};

LightSensorThread::Guts::Guts(const std::string &devFile) : dev_file(devFile)
{
}

void LightSensorThread::Guts::try_open_dev()
{
    assert(fd < 0);
    fd = open(dev_file.c_str(), O_RDWR);
    if (fd < 0)
    {
        down();
        return;
    }
}

bool &LightSensorThread::Guts::ready_flag(LightSensorAddr addr)
{
    if (addr == AddrLow)
        return low_ready;
    else if (addr == AddrHigh)
        return high_ready;
    assert(false);
    abort();
}

bool LightSensorThread::Guts::prepare(LightSensorAddr addr)
{
    assert(fd >= 0);
    if (!choose(addr))
        return false;
    if (!command(CMD_POWER_ON))
        return false;
    if (!command(CMD_RESET))
        return false;
    if (!command(mode))
        return false;

    // finalize
    ready_flag(addr) = true;
    return true;
}

bool LightSensorThread::Guts::choose(LightSensorAddr addr)
{
    int re = ioctl(fd, I2C_SLAVE, addr);
    if (re == 0)
    {
        return true;
    }
    else
    {
        clog << "failed to ioctl I2C_SLAVE address " << int(addr) << ": " << strerror(errno) << endl;
        return false;
    }
}

bool LightSensorThread::Guts::command(char cmd)
{
    auto num_write = write(fd, &cmd, 1);
    if (num_write == 1)
    {
        return true;
    }
    else
    {
        clog << "failed to write command " << int(cmd) << ": " << strerror(errno) << endl;
        return false;
    }
}

float LightSensorThread::Guts::readout(LightSensorAddr addr)
{
    if (!choose(addr))
    {
        stop(addr);
        return std::numeric_limits<float>::signaling_NaN();
    }
    char buf[3] = {0xff, 0xff, 0xff};
    int nread = read(fd, buf, 2);
    if (nread != 2)
    {
        stop(addr);
        return std::numeric_limits<float>::signaling_NaN();
    }
    float result = float(buf[0] * 256 + buf[1]);
    if (mode == Mode_HighRes || mode == Mode_LowRes)
        return result / 1.2f;
    else if (mode == Mode_HighRes2)
        return result / 2.4f;
    assert(false);
    return std::numeric_limits<float>::signaling_NaN();
}

void LightSensorThread::Guts::stop(LightSensorAddr addr)
{
    choose(addr);
    command(CMD_POWER_DOWN);
    ready_flag(addr) = false;
}

void LightSensorThread::Guts::down()
{
    last_lux_low = std::numeric_limits<float>::signaling_NaN();
    last_lux_high = std::numeric_limits<float>::signaling_NaN();
    if (low_ready)
    {
        assert(fd >= 0);
        stop(AddrLow);
    }
    if (high_ready)
    {
        assert(fd >= 0);
        stop(AddrHigh);
    }
    if (fd >= 0)
    {
        close(fd);
        fd = -1;
    }
}

void LightSensorThread::Guts::thread_body()
{
    using namespace std::chrono_literals;
    while (stop_flag == 0)
    {
        if (fd < 0)
            try_open_dev();

        if (!low_ready)
            prepare(AddrLow);
        if (!high_ready)
            prepare(AddrHigh);

        // do read out
        if (fd >= 0)
        {
            last_lux_low = low_ready ? readout(AddrLow) : std::numeric_limits<float>::signaling_NaN();
            last_lux_high = high_ready ? readout(AddrHigh) : std::numeric_limits<float>::signaling_NaN();
        }
        else
        {
            last_lux_low = std::numeric_limits<float>::signaling_NaN();
            last_lux_high = std::numeric_limits<float>::signaling_NaN();
        }

        // apply mode update
        auto curr_incoming_mode = incoming_mode;
        if (mode != curr_incoming_mode)
        {
            bool any_success = false;
            if ((choose(AddrLow) && command(curr_incoming_mode)))
                any_success = true;
            else
                stop(AddrLow);
            if (choose(AddrHigh) && command(curr_incoming_mode))
                any_success = true;
            else
                stop(AddrHigh);
            if (any_success)
                mode = curr_incoming_mode;
        }

        // sleep according to mode resolution
        if (mode == Mode_HighRes || mode == Mode_HighRes2)
            std::this_thread::sleep_for(200ms);
        else if (mode == Mode_LowRes)
            std::this_thread::sleep_for(25ms);
        else
        {
            assert(false);
            mode = Mode_HighRes;
            incoming_mode = Mode_HighRes;
            std::this_thread::sleep_for(200ms);
        }
    }

    down();
}

LightSensorThread::LightSensorThread(const std::string &i2cDeviceFile) : guts(new Guts(i2cDeviceFile))
{
    guts->thread.reset(new std::thread([this]() { guts->thread_body(); }));
}

LightSensorThread::~LightSensorThread()
{
    guts->stop_flag = 1;
    guts->thread->join();
}

LightSensorThread::Mode LightSensorThread::getCurrentWorkingMode() const
{
    return Mode(guts->mode);
}

void LightSensorThread::setMode(Mode m)
{
    guts->incoming_mode = m;
}

std::pair<float, float> LightSensorThread::getRawLuminance() const
{
    return {guts->last_lux_low, guts->last_lux_high};
}

float LightSensorThread::getLuminance() const
{
    float curr_low = guts->last_lux_low;
    float curr_high = guts->last_lux_high;
    if (std::isnan(curr_low))
    {
        if (std::isnan(curr_high))
        {
            return std::numeric_limits<float>::signaling_NaN();
        }
        else
        {
            return curr_high;
        }
    }
    else
    {
        if (std::isnan(curr_high))
        {
            return curr_low;
        }
        else
        {
            return (curr_low + curr_high) / 2.0f;
        }
    }
}