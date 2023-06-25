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

PWMScreenBrightnessSetter::PWMScreenBrightnessSetter(int chipID, int exportID, double frequency)
    : m_chip_id(chipID), m_export_id(exportID), m_period_ns(int(std::round(1e9 / frequency)))
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
        rawWriteMesg(unexport_path, std::to_string(m_export_id));
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

    auto period_ns_str = std::to_string(m_period_ns);
    if (rawReadMesg(period_file) != period_ns_str)
        rawWriteMesg(period_file, period_ns_str);

    brightnessNorm = std::max(0.0f, std::min(1.0f, brightnessNorm));
    rawWriteMesg(duty_cycle_file, std::to_string((int)std::round(double(m_period_ns) * brightnessNorm)));

    if (rawReadMesg(en_file) != "1")
        rawWriteMesg(en_file, "1");
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

    auto period_str = rawReadMesg(pwm_path);
    auto duty_cycle_str = rawReadMesg(duty_cycle_path);

    char *pend = nullptr;
    double period = strtod(period_str.c_str(), &pend);
    if (pend == period_str.c_str())
    {
        clog << "failed to parse any number from period string \"" << period_str << "\"" << endl;
        return std::numeric_limits<float>::signaling_NaN();
    }
    pend = nullptr;
    double duty_cycle = strtod(duty_cycle_str.c_str(), &pend);
    if (pend == duty_cycle_str.c_str())
    {
        clog << "failed to parse any number from duty cycle string \"" << duty_cycle_str << "\"" << endl;
        return std::numeric_limits<float>::signaling_NaN();
    }

    return float(duty_cycle / period);
}

bool PWMScreenBrightnessSetter::try_setup_sysfs()
{
    // setup pwm sysfs
    auto export_path = get_chip_path(m_chip_id) / "export";
    if (!fs::is_regular_file(export_path))
        return false;
    if (!rawWriteMesg(export_path, std::to_string(m_export_id)))
        return false;
    return true;
}