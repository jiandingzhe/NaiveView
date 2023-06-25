#pragma once

#include "ScreenBrightnessSetter.h"

#include <limits>
#include <memory>

class WaveshareScreenBrightnessSetter: public ScreenBrightnessSetter
{
  public:
    WaveshareScreenBrightnessSetter();
    ~WaveshareScreenBrightnessSetter() override;

    bool setBrightness(float brightnessNorm) override;

    float getLastAppliedBrightness() const override
    {
        return m_last_brightness;
    }

  private:
    void try_open_device();

    int m_fd = -1;
    float m_last_brightness = std::numeric_limits<float>::signaling_NaN();
};
