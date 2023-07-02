#include "ScreenBrightnessMapper.h"

#include "Settings.h"

ScreenBrightnessMapper::~ScreenBrightnessMapper()
{
}

float SteppedScreenBrightnessMapper::getScreenBrightnessNormByEnvLuminance(float lux)
{
    auto &settings = Settings::getInstance();
    if (lux < settings.get_lux_bound1())
        return settings.get_screen_min_brightness();
    else if (lux < settings.get_lux_bound2())
        return settings.get_screen_mid_brightness();
    else
        return 1.0f;
}

inline float ratio(float b1, float b2, float x)
{
    return (x - b1) / (b2 - b1);
}

inline float interp(float v1, float v2, float r)
{
    return v1 + (v2 - v1) * r;
}

float LinnearScreenBrightnessMapper::getScreenBrightnessNormByEnvLuminance(float lux)
{
    auto &settings = Settings::getInstance();
    float lux_bound1 = settings.get_lux_bound1();
    float lux_bound2 = settings.get_lux_bound2();
    float lux_mid = (lux_bound1 + lux_bound2) / 2.0f;
    float min_bright = settings.get_screen_min_brightness();
    float mid_bright = settings.get_screen_mid_brightness();
    if (lux < lux_bound1)
    {
        return min_bright;
    }
    else if (lux < lux_mid)
    {
        float r = ratio(lux_bound1, lux_mid, lux);
        return interp(min_bright, mid_bright, r);
    }
    else if (lux < lux_bound2)
    {
        float r = ratio(lux_mid, lux_bound2, lux);
        return interp(mid_bright, 1.0f, r);
    }
    else
    {
        return 1.0f;
    }
}