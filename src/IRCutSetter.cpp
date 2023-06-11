#include "IRCutSetter.h"

#include "Settings.h"

#include <string>

#include <gpiod.h>

bool setIRCutGPIOState(int value)
{
    auto &settings = Settings::getInstance();
    auto chip_name = std::string("/dev/gpiochip") + std::to_string(settings.get_gpio_chip_index());
    int re = gpiod_ctxless_set_value(chip_name.c_str(), settings.get_ircut_gpio_index(), value, false, "test", nullptr,
                                     nullptr);
    return re == 0;
}