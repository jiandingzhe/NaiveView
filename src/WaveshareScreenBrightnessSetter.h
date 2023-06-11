#pragma once

#include <limits>
#include <memory>

class WaveshareScreenBrightnessSetter
{
  public:
    WaveshareScreenBrightnessSetter();
    ~WaveshareScreenBrightnessSetter();

    bool setBrightness(float brightnessNorm);

    float getLastSetBrightness() const
    {
        return m_last_brightness;
    }

  private:
    void try_open_device();

    int m_fd = -1;
    float m_last_brightness = std::numeric_limits<float>::signaling_NaN();
};
