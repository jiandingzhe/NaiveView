#pragma once

struct ScreenBrightnessSetter
{
    virtual ~ScreenBrightnessSetter() = default;
    virtual bool setBrightness(float brightnessNorm) = 0;
    virtual float getLastAppliedBrightness() const = 0;
};