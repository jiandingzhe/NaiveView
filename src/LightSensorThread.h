#pragma once

#include <memory>

class LightSensorThread
{
  public:
    enum Mode
    {
        Mode_HighRes = 0x10,
        Mode_HighRes2 = 0x11,
        Mode_LowRes = 0x13,
    };
    LightSensorThread(const char *i2cDeviceFile);
    ~LightSensorThread();
    Mode getCurrentWorkingMode() const;
    void setMode(Mode);
    std::pair<float, float> getRawLuminance() const;
    float getLuminance() const;

  private:
    struct Guts;
    std::unique_ptr<Guts> guts;
};