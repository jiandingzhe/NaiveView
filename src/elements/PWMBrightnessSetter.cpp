#include "PWMBrightnessSetter.h"

#include "Utils.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
using std::clog;
using std::endl;

static fs::path get_chip_path(int chip_id)
{
    fs::path path("/sys/class/pwm/pwmchip");
    path += std::to_string(chip_id);
    return path;
}

static fs::path get_pwm_path(int chip_id, int exportID)
{
    auto path = get_chip_path(chip_id);
    path /= "pwm";
    path += std::to_string(exportID);
    return path;
}

PWMScreenBrightnessSetter::PWMScreenBrightnessSetter(int chipID, int exportID, int periodNS, float dimmostDutyCycle,
                                                     float brightmostDutyCycle)
    : m_chip_id(chipID), m_export_id(exportID), m_period_ns(periodNS), m_dim_duty_cycle(dimmostDutyCycle),
      m_bright_duty_cycle(brightmostDutyCycle)
{
    try_setup_sysfs();
    setBrightness(0.667f);
}

PWMScreenBrightnessSetter::~PWMScreenBrightnessSetter()
{
    auto pwm_path = get_pwm_path(m_chip_id, m_export_id);
    if (fs::is_directory(pwm_path))
    {
        auto unexport_path = get_chip_path(m_chip_id) / "unexport";
        sysfsWriteToStr(unexport_path, m_export_id);
    }
}

bool PWMScreenBrightnessSetter::setBrightness(float brightnessNorm)
{
    auto pwm_path = get_pwm_path(m_chip_id, m_export_id);

    // setup pwm sysfs
    if (!fs::is_directory(pwm_path))
    {
        if (!try_setup_sysfs())
            return false;
    }
    assert(fs::is_directory(pwm_path));

    auto period_file = pwm_path / "period";
    auto duty_cycle_file = pwm_path / "duty_cycle";
    auto en_file = pwm_path / "enable";
    assert(fs::is_directory(pwm_path));
    assert(fs::is_regular_file(period_file));
    assert(fs::is_regular_file(duty_cycle_file));
    assert(fs::is_regular_file(en_file));

    // set period if not already set
    if (sysfsReadStrToInt(period_file) != m_period_ns && !sysfsWriteToStr(period_file, m_period_ns))
        return false;

    // calculate and set duty cycle
    brightnessNorm = std::max(0.0f, std::min(1.0f, brightnessNorm));
    float duty_cycle_norm = m_dim_duty_cycle + (m_bright_duty_cycle - m_dim_duty_cycle) * brightnessNorm;
    int duty_cycle_ns = (int)std::round(duty_cycle_norm * m_period_ns);
    sysfsWriteToStr(duty_cycle_file, duty_cycle_ns);

    // turn on PWM
    if (sysfsReadStrToInt(en_file) != 1 && !sysfsWriteToStr(en_file, 1))
        return false;
    return true;
}

float PWMScreenBrightnessSetter::getLastAppliedBrightness() const
{
    auto pwm_path = get_pwm_path(m_chip_id, m_export_id);
    auto period_path = pwm_path / "period";
    auto duty_cycle_path = pwm_path / "duty_cycle";
    if (!fs::is_regular_file(period_path) || !fs::is_regular_file(duty_cycle_path))
    {
        return std::numeric_limits<float>::signaling_NaN();
    }

    int period_ns = sysfsReadStrToInt(pwm_path);
    int duty_cycle_ns = sysfsReadStrToInt(duty_cycle_path);
    float duty_cycle_norm = float(duty_cycle_ns) / float(period_ns);
    float brightness = (duty_cycle_norm - m_dim_duty_cycle) / (m_bright_duty_cycle - m_dim_duty_cycle);
    return brightness;
}

bool PWMScreenBrightnessSetter::try_setup_sysfs()
{
    // setup pwm sysfs
    auto export_path = get_chip_path(m_chip_id) / "export";
    if (!fs::is_regular_file(export_path))
        return false;
    if (!sysfsWriteToStr(export_path, m_export_id))
        return false;
    return true;
}