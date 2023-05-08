#include "LightSensorThread.h"

#include <atomic>
#include <cassert>
#include <chrono>
#include <limits>
#include <thread>

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define CMD_POWER_DOWN 0x00
#define CMD_POWER_ON 0x01
#define CMD_RESET 0x07

struct LightSensorThread::Guts
{
    Guts(const char *defFile, char addr);
    void try_open_dev();
    bool prepare();
    bool command(char cmd);
    float readout();
    void down();

    void thread_body();
    const char addr;
    char mode = Mode_HighRes;
    char incoming_mode = Mode_HighRes;
    bool ready = false;
    float last_lux = std::numeric_limits<float>::signaling_NaN();
    const char *const dev_file = nullptr;
    int fd = -1;
    std::atomic<int> stop_flag{0};
    std::unique_ptr<std::thread> thread;
};

void LightSensorThread::Guts::try_open_dev()
{
    assert(fd < 0);
    fd = open(dev_file, O_RDWR);
    if (fd < 0)
    {
        down();
        return;
    }
    if (ioctl(fd, I2C_SLAVE, addr) != 0)
    {
        down();
        return;
    }
}

bool LightSensorThread::Guts::prepare()
{
    assert(fd >= 0);
    if (!command(CMD_POWER_ON))
        return false;
    if (!command(CMD_RESET))
        return false;
    if (!command(mode))
        return false;
    // finalize
    ready = true;
    return true;
}

bool LightSensorThread::Guts::command(char cmd)
{
    auto num_write = write(fd, &cmd, 1);
    return num_write == 1;
}

float LightSensorThread::Guts::readout()
{
    char buf[3] = {0xff, 0xff, 0xff};
    int nread = read(fd, buf, 2);
    if (nread != 2)
    {
        down();
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

void LightSensorThread::Guts::down()
{
    last_lux = std::numeric_limits<float>::signaling_NaN();
    if (ready)
    {
        assert(fd >= 0);
        command(CMD_POWER_DOWN);
        ready = false;
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

        if (!ready)
            prepare();

        if (fd >= 0 && ready)
        {
            last_lux = readout();
        }

        // apply mode update
        auto curr_incoming_mode = incoming_mode;
        if (mode != curr_incoming_mode)
        {
            if (command(curr_incoming_mode))
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

LightSensorThread::LightSensorThread(const char *i2cDeviceFile, DeviceAddr addr) : guts(new Guts(i2cDeviceFile, addr))
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

float LightSensorThread::getLuminance() const
{
    return guts->last_lux;
}