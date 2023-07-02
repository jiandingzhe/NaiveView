#pragma once

class ScreenBrightnessMapper
{
  public:
    virtual ~ScreenBrightnessMapper();

    virtual float getScreenBrightnessNormByEnvLuminance(float lux) = 0;
};

class SteppedScreenBrightnessMapper : public ScreenBrightnessMapper
{
  public:
    ~SteppedScreenBrightnessMapper() override = default;
    float getScreenBrightnessNormByEnvLuminance(float lux) override;
};

class LinnearScreenBrightnessMapper : public ScreenBrightnessMapper
{
  public:
    ~LinnearScreenBrightnessMapper() override = default;
    float getScreenBrightnessNormByEnvLuminance(float lux) override;
};
