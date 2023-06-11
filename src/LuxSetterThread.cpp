#include "LuxSetterThread.h"

#include "IRCutSetter.h"
#include "Settings.h"
#include "WaveshareScreenBrightnessSetter.h"

#include <atomic>
#include <cmath>
#include <limits>
#include <string>
#include <thread>

struct LuxSetterThread::Guts
{
    void thread_body();
    std::atomic<int> stop_flag{0};
    std::atomic<float> incoming_lux{std::numeric_limits<float>::signaling_NaN()};
    std::unique_ptr<std::thread> thread;
    WaveshareScreenBrightnessSetter screen_setter;
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
                if (curr_lux < settings.get_lux_bound1())
                    screen_brightness = settings.get_screen_min_brightness();
                else if (curr_lux < settings.get_lux_bound2())
                    screen_brightness = settings.get_screen_mid_brightness();
                else
                    screen_brightness = 1.0f;
            }

            // do set values
            if (old_ircut_state != ircut_state)
            {
                old_ircut_state = ircut_state;
                if (!setIRCutGPIOState(ircut_state))
                    old_ircut_state = -1;
            }
            screen_setter.setBrightness(screen_brightness);
        }

        // finalize this iteration
        std::this_thread::sleep_for(200ms);
        iter_count += 1;
    }
}

LuxSetterThread::LuxSetterThread() : guts(new Guts)
{
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
