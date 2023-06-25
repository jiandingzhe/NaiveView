#pragma once
#include "ScreenBrightnessSetter.h"

class PWMScreenBrightnessSetter: public ScreenBrightnessSetter
{
public:
    PWMScreenBrightnessSetter(int chipID, int exportID, double frequency);
    ~PWMScreenBrightnessSetter() override;
    bool setBrightness(float brightnessNorm) override;
    float getLastAppliedBrightness() const override;

private:
    bool try_setup_sysfs();
    const int m_chip_id;
    const int m_export_id;
    const int m_period_ns;
};