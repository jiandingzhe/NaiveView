// automatically generated, do not modify manually!
#include "Settings.h"

#include "Utils.h"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

using std::clog;
using std::endl;

namespace fs = std::filesystem;

struct Settings::Guts
{
    bool reload_from_config_file();
    bool save_to_config_file() const;

    fs::path config_file;
    int camera_width = 1280;
    int camera_height = 720;
    float camera_brightness = 0.0f;
    float camera_contrast = 1.0f;
    int i2c_index = 1;
    int gpio_chip_index = 0;
    int ircut_gpio_index = 17;
    float ircut_lux = 10;
    RotateCCW display_rotate = RotateCCW0;
    bool display_hflip = true;
    float screen_min_brightness = 0.3f;
    float screen_mid_brightness = 0.65f;
    float lux_bound1 = 30;
    float lux_bound2 = 500;
    UISide ui_side = UIOnLeft;
    ScreenBacklightMode bl_mode = WaveshareBacklight;
    int pwm_bl_chip = 0;
    int pwm_bl_index = 0;
    int pwm_bl_period = 50000;
    float pwm_bl_full_duty_cycle = 0.0;
    float pwm_bl_zero_duty_cycle = 0.7333;
}; // end of Settings::Guts

static bool try_load_float(float& re, const std::string& str)
{
    const auto* headptr = str.c_str();
    char* endptr = nullptr;
    errno = 0;
    float tmp = strtof(headptr, &endptr);
    if (errno == 0)
    {
        re = tmp;
        return true;
    }
    clog << "failed to parse float expression \"" << str << "\": " << strerror(errno) << endl;
    return false;
}

static bool try_load_norm(float& re, const std::string& str)
{
    float tmp;
    if (try_load_float(tmp, str))
    {
        re = std::max(0.0f, std::min(1.0f, tmp));
        return true;
    }
    return false;
}

static bool try_load_bool(bool& re, const std::string& str)
{
    if (str == "true")
    {
        re = true;
        return true;
    }
    else if (str == "false")
    {
        re = false;
        return true;
    }
    clog << "failed to parse boolean expression \"" << str << "\"" << endl;
    return false;
}

static bool try_load_int(int& re, const std::string& str)
{
    const auto* headptr = str.c_str();
    char* endptr = nullptr;
    errno = 0;
    auto tmp = strtol(headptr, &endptr, 0);
    if (errno == 0)
    {
        re = int(tmp);
        return true;
    }
    clog << "failed to parse integer expression \"" << str << "\": " << strerror(errno) << endl;
    return false;
}

static bool try_load_uint(unsigned& re, const std::string& str)
{
    const auto* headptr = str.c_str();
    char* endptr = nullptr;
    errno = 0;
    auto tmp = strtol(headptr, &endptr, 0);
    if (errno == 0)
    {
        re = unsigned(tmp);
        return true;
    }
    clog << "failed to parse integer expression \"" << str << "\": " << strerror(errno) << endl;
    return false;
}

template<typename T>
static bool try_load_int_enum(T& re, const std::string& str)
{
    int tmp;
    if (try_load_int(tmp, str))
    {
        re = T(tmp);
        return true;
    }
    return false;
}

bool Settings::Guts::reload_from_config_file()
{
    if (!fs::is_regular_file(config_file))
        return false;
    
    std::ifstream fh(config_file);
    if (!fh.is_open())
    {
        clog << "failed to open config file \"" << config_file << "\" for read:" << strerror(errno) << endl;
        return false;
    }
    
    std::string line;
    while (std::getline(fh, line))
    {
        auto delim = line.find('\t');
        if (delim == std::string::npos || delim == 0)
            continue;
        std::string key = line.substr(0, delim);
        std::string val_str = line.substr(delim+1);
        if (key == "camera_width") try_load_int(camera_width, val_str);
        else if (key == "camera_height") try_load_int(camera_height, val_str);
        else if (key == "camera_brightness") try_load_float(camera_brightness, val_str);
        else if (key == "camera_contrast") try_load_float(camera_contrast, val_str);
        else if (key == "i2c_index") try_load_int(i2c_index, val_str);
        else if (key == "gpio_chip_index") try_load_int(gpio_chip_index, val_str);
        else if (key == "ircut_gpio_index") try_load_int(ircut_gpio_index, val_str);
        else if (key == "ircut_lux") try_load_float(ircut_lux, val_str);
        else if (key == "display_rotate") try_load_int_enum(display_rotate, val_str);
        else if (key == "display_hflip") try_load_bool(display_hflip, val_str);
        else if (key == "screen_min_brightness") try_load_norm(screen_min_brightness, val_str);
        else if (key == "screen_mid_brightness") try_load_norm(screen_mid_brightness, val_str);
        else if (key == "lux_bound1") try_load_float(lux_bound1, val_str);
        else if (key == "lux_bound2") try_load_float(lux_bound2, val_str);
        else if (key == "ui_side") try_load_int_enum(ui_side, val_str);
        else if (key == "bl_mode") try_load_int_enum(bl_mode, val_str);
        else if (key == "pwm_bl_chip") try_load_int(pwm_bl_chip, val_str);
        else if (key == "pwm_bl_index") try_load_int(pwm_bl_index, val_str);
        else if (key == "pwm_bl_period") try_load_int(pwm_bl_period, val_str);
        else if (key == "pwm_bl_full_duty_cycle") try_load_norm(pwm_bl_full_duty_cycle, val_str);
        else if (key == "pwm_bl_zero_duty_cycle") try_load_norm(pwm_bl_zero_duty_cycle, val_str);
    }
    return true;
} // end of reload_from_config_file()

bool Settings::Guts::save_to_config_file() const
{
    fs::create_directories(config_file.parent_path());
    auto* fh = fopen(config_file.c_str(), "w");
    if (fh == nullptr)
    {
        clog << "failed to open config file \"" << config_file << "\" for write: " << strerror(errno) << endl;
        return false;
    }
    fprintf(fh, "camera_width\t%d\n", camera_width);
    fprintf(fh, "camera_height\t%d\n", camera_height);
    fprintf(fh, "camera_brightness\t%f\n", camera_brightness);
    fprintf(fh, "camera_contrast\t%f\n", camera_contrast);
    fprintf(fh, "i2c_index\t%d\n", i2c_index);
    fprintf(fh, "gpio_chip_index\t%d\n", gpio_chip_index);
    fprintf(fh, "ircut_gpio_index\t%d\n", ircut_gpio_index);
    fprintf(fh, "ircut_lux\t%f\n", ircut_lux);
    fprintf(fh, "display_rotate\t%d\n", display_rotate);
    fprintf(fh, "display_hflip\t%s\n", display_hflip ? "true" : "false");
    fprintf(fh, "screen_min_brightness\t%f\n", screen_min_brightness);
    fprintf(fh, "screen_mid_brightness\t%f\n", screen_mid_brightness);
    fprintf(fh, "lux_bound1\t%f\n", lux_bound1);
    fprintf(fh, "lux_bound2\t%f\n", lux_bound2);
    fprintf(fh, "ui_side\t%d\n", ui_side);
    fprintf(fh, "bl_mode\t%d\n", bl_mode);
    fprintf(fh, "pwm_bl_chip\t%d\n", pwm_bl_chip);
    fprintf(fh, "pwm_bl_index\t%d\n", pwm_bl_index);
    fprintf(fh, "pwm_bl_period\t%d\n", pwm_bl_period);
    fprintf(fh, "pwm_bl_full_duty_cycle\t%f\n", pwm_bl_full_duty_cycle);
    fprintf(fh, "pwm_bl_zero_duty_cycle\t%f\n", pwm_bl_zero_duty_cycle);
    fclose(fh);
    return true;
}

Settings& Settings::getInstance()
{
    static Settings obj;
    return obj;
}

Settings::Settings(): guts(new Guts)
{
    guts->config_file = getSettingsFile("settings.txt");
    if (fs::is_regular_file(guts->config_file))
        guts->reload_from_config_file();
    else
        guts->save_to_config_file();
}

Settings::~Settings() {}

int Settings::get_camera_width() const { return guts->camera_width; }
void Settings::set_camera_width(int v)
{
    guts->camera_width = v;
    guts->save_to_config_file();
}
void Settings::reset_camera_width()
{
    guts->camera_width = 1280;
    guts->save_to_config_file();
}

int Settings::get_camera_height() const { return guts->camera_height; }
void Settings::set_camera_height(int v)
{
    guts->camera_height = v;
    guts->save_to_config_file();
}
void Settings::reset_camera_height()
{
    guts->camera_height = 720;
    guts->save_to_config_file();
}

float Settings::get_camera_brightness() const { return guts->camera_brightness; }
void Settings::set_camera_brightness(float v)
{
    guts->camera_brightness = v;
    guts->save_to_config_file();
}
void Settings::reset_camera_brightness()
{
    guts->camera_brightness = 0.0f;
    guts->save_to_config_file();
}

float Settings::get_camera_contrast() const { return guts->camera_contrast; }
void Settings::set_camera_contrast(float v)
{
    guts->camera_contrast = v;
    guts->save_to_config_file();
}
void Settings::reset_camera_contrast()
{
    guts->camera_contrast = 1.0f;
    guts->save_to_config_file();
}

int Settings::get_i2c_index() const { return guts->i2c_index; }
void Settings::set_i2c_index(int v)
{
    guts->i2c_index = v;
    guts->save_to_config_file();
}
void Settings::reset_i2c_index()
{
    guts->i2c_index = 1;
    guts->save_to_config_file();
}

int Settings::get_gpio_chip_index() const { return guts->gpio_chip_index; }
void Settings::set_gpio_chip_index(int v)
{
    guts->gpio_chip_index = v;
    guts->save_to_config_file();
}
void Settings::reset_gpio_chip_index()
{
    guts->gpio_chip_index = 0;
    guts->save_to_config_file();
}

int Settings::get_ircut_gpio_index() const { return guts->ircut_gpio_index; }
void Settings::set_ircut_gpio_index(int v)
{
    guts->ircut_gpio_index = v;
    guts->save_to_config_file();
}
void Settings::reset_ircut_gpio_index()
{
    guts->ircut_gpio_index = 17;
    guts->save_to_config_file();
}

float Settings::get_ircut_lux() const { return guts->ircut_lux; }
void Settings::set_ircut_lux(float v)
{
    guts->ircut_lux = v;
    guts->save_to_config_file();
}
void Settings::reset_ircut_lux()
{
    guts->ircut_lux = 10;
    guts->save_to_config_file();
}

RotateCCW Settings::get_display_rotate() const { return guts->display_rotate; }
void Settings::set_display_rotate(RotateCCW v)
{
    guts->display_rotate = v;
    guts->save_to_config_file();
}
void Settings::reset_display_rotate()
{
    guts->display_rotate = RotateCCW0;
    guts->save_to_config_file();
}

bool Settings::get_display_hflip() const { return guts->display_hflip; }
void Settings::set_display_hflip(bool v)
{
    guts->display_hflip = v;
    guts->save_to_config_file();
}
void Settings::reset_display_hflip()
{
    guts->display_hflip = true;
    guts->save_to_config_file();
}

float Settings::get_screen_min_brightness() const { return guts->screen_min_brightness; }
void Settings::set_screen_min_brightness(float v)
{
    guts->screen_min_brightness = v;
    guts->save_to_config_file();
}
void Settings::reset_screen_min_brightness()
{
    guts->screen_min_brightness = 0.3f;
    guts->save_to_config_file();
}

float Settings::get_screen_mid_brightness() const { return guts->screen_mid_brightness; }
void Settings::set_screen_mid_brightness(float v)
{
    guts->screen_mid_brightness = v;
    guts->save_to_config_file();
}
void Settings::reset_screen_mid_brightness()
{
    guts->screen_mid_brightness = 0.65f;
    guts->save_to_config_file();
}

float Settings::get_lux_bound1() const { return guts->lux_bound1; }
void Settings::set_lux_bound1(float v)
{
    guts->lux_bound1 = v;
    guts->save_to_config_file();
}
void Settings::reset_lux_bound1()
{
    guts->lux_bound1 = 30;
    guts->save_to_config_file();
}

float Settings::get_lux_bound2() const { return guts->lux_bound2; }
void Settings::set_lux_bound2(float v)
{
    guts->lux_bound2 = v;
    guts->save_to_config_file();
}
void Settings::reset_lux_bound2()
{
    guts->lux_bound2 = 500;
    guts->save_to_config_file();
}

UISide Settings::get_ui_side() const { return guts->ui_side; }
void Settings::set_ui_side(UISide v)
{
    guts->ui_side = v;
    guts->save_to_config_file();
}
void Settings::reset_ui_side()
{
    guts->ui_side = UIOnLeft;
    guts->save_to_config_file();
}

ScreenBacklightMode Settings::get_bl_mode() const { return guts->bl_mode; }
void Settings::set_bl_mode(ScreenBacklightMode v)
{
    guts->bl_mode = v;
    guts->save_to_config_file();
}
void Settings::reset_bl_mode()
{
    guts->bl_mode = WaveshareBacklight;
    guts->save_to_config_file();
}

int Settings::get_pwm_bl_chip() const { return guts->pwm_bl_chip; }
void Settings::set_pwm_bl_chip(int v)
{
    guts->pwm_bl_chip = v;
    guts->save_to_config_file();
}
void Settings::reset_pwm_bl_chip()
{
    guts->pwm_bl_chip = 0;
    guts->save_to_config_file();
}

int Settings::get_pwm_bl_index() const { return guts->pwm_bl_index; }
void Settings::set_pwm_bl_index(int v)
{
    guts->pwm_bl_index = v;
    guts->save_to_config_file();
}
void Settings::reset_pwm_bl_index()
{
    guts->pwm_bl_index = 0;
    guts->save_to_config_file();
}

int Settings::get_pwm_bl_period() const { return guts->pwm_bl_period; }
void Settings::set_pwm_bl_period(int v)
{
    guts->pwm_bl_period = v;
    guts->save_to_config_file();
}
void Settings::reset_pwm_bl_period()
{
    guts->pwm_bl_period = 50000;
    guts->save_to_config_file();
}

float Settings::get_pwm_bl_full_duty_cycle() const { return guts->pwm_bl_full_duty_cycle; }
void Settings::set_pwm_bl_full_duty_cycle(float v)
{
    guts->pwm_bl_full_duty_cycle = v;
    guts->save_to_config_file();
}
void Settings::reset_pwm_bl_full_duty_cycle()
{
    guts->pwm_bl_full_duty_cycle = 0.0;
    guts->save_to_config_file();
}

float Settings::get_pwm_bl_zero_duty_cycle() const { return guts->pwm_bl_zero_duty_cycle; }
void Settings::set_pwm_bl_zero_duty_cycle(float v)
{
    guts->pwm_bl_zero_duty_cycle = v;
    guts->save_to_config_file();
}
void Settings::reset_pwm_bl_zero_duty_cycle()
{
    guts->pwm_bl_zero_duty_cycle = 0.7333;
    guts->save_to_config_file();
}

