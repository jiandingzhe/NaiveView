#include "IRCutSetter.h"

#include <string>

#include <gpiod.h>

bool setIRCutGPIOState(int chip, int gpio, int value)
{
    auto chip_name = std::string("/dev/gpiochip") + std::to_string(chip);
    int re = gpiod_ctxless_set_value(chip_name.c_str(), gpio, value, false, "test", nullptr,
                                     nullptr);
    return re == 0;
}