#include "LuxSetterThread.h"

#include "Settings.h"
#include "elements/IRCutSetter.h"
#include "elements/PWMBrightnessSetter.h"
#include "elements/ScreenBrightnessMapper.h"
#include "elements/ScreenBrightnessSetter.h"
#include "elements/WaveshareScreenBrightnessSetter.h"

#include <atomic>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <thread>

using std::clog;
using std::endl;

struct LuxSetterThread::Guts
{
    void thread_body();
    std::atomic<int> stop_flag{0};
    std::atomic<float> incoming_lux{std::numeric_limits<float>::signaling_NaN()};
    std::unique_ptr<std::thread> thread;
    std::unique_ptr<ScreenBrightnessMapper> bright_mapper;
    std::unique_ptr<ScreenBrightnessSetter> screen_setter;
};

#define NEW_LUX_WEIGHT 0.5f

void LuxSetterThread::Guts::thread_body()
{
    using namespace std::chrono_literals;
    auto &settings = Settings::getInstance();
    unsigned iter_count = 0;
    float curr_lux = std::numeric_limits<float>::signaling_NaN();
    int old_ircut_state = -1;
    while (stop_flag == 0)
    {
        // update luminance
        float new_lux = incoming_lux;
        if (std::isnan(curr_lux))
            curr_lux = new_lux;
        else
            curr_lux = new_lux * NEW_LUX_WEIGHT + curr_lux * (1.0f - NEW_LUX_WEIGHT);

        // output state
        if (iter_count % 8 == 0)
        {
            // default output values
            int ircut_state = 1;
            float screen_brightness = 0.67f;

            // determine real values if there exists luminance value
            if (!std::isnan(curr_lux))
            {
                ircut_state = curr_lux >= settings.get_ircut_lux() ? 1 : 0;
                screen_brightness = bright_mapper->getScreenBrightnessNormByEnvLuminance(curr_lux);
            }

            // do set values
            if (old_ircut_state != ircut_state)
            {
                old_ircut_state = ircut_state;
                if (!setIRCutGPIOState(settings.get_gpio_chip_index(), settings.get_ircut_gpio_index(), ircut_state))
                    old_ircut_state = -1;
            }
            clog << "set brightness " << screen_brightness << endl;
            screen_setter->setBrightness(screen_brightness);
        }

        // finalize this iteration
        std::this_thread::sleep_for(200ms);
        iter_count += 1;
    }
}

LuxSetterThread::LuxSetterThread() : guts(new Guts)
{
    const auto &settings = Settings::getInstance();
    switch (settings.get_bl_mode())
    {
    case WaveshareBacklight:
        guts->bright_mapper.reset(new SteppedScreenBrightnessMapper);
        guts->screen_setter.reset(new WaveshareScreenBrightnessSetter);
        break;
    case PWMBacklight:
        guts->bright_mapper.reset(new LinnearScreenBrightnessMapper);
        guts->screen_setter.reset(new PWMScreenBrightnessSetter(
            settings.get_pwm_bl_chip(), settings.get_pwm_bl_index(), settings.get_pwm_bl_period(),
            settings.get_pwm_bl_zero_duty_cycle(), settings.get_pwm_bl_full_duty_cycle()));
        break;
    default:
        assert(false);
        break;
    }
    guts->thread.reset(new std::thread([this]() { guts->thread_body(); }));
}

LuxSetterThread::~LuxSetterThread()
{
    guts->stop_flag = 1;
    guts->thread->join();
    guts->thread.reset();
    guts->stop_flag = 0;
}

void LuxSetterThread::setSensorLuminance(float lux)
{
    guts->incoming_lux = lux;
}
