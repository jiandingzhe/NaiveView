#pragma once

#include <memory>

class LightSensorThread
{
  public:
    enum DeviceAddr
    {
        Addr_Low = 0x23,
        Addr_High = 0x5c,
    };
    enum Mode
    {
        Mode_HighRes = 0x10,
        Mode_HighRes2 = 0x11,
        Mode_LowRes = 0x13,
    };
    LightSensorThread(const char *i2cDeviceFile, DeviceAddr addr);
    ~LightSensorThread();
    Mode getCurrentWorkingMode() const;
    void setMode(Mode);
    float getLuminance() const;

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};