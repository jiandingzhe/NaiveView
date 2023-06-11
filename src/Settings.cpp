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
    int i2c_index = 1;
    int ircut_gpio_index = 17;
    RotateCCW display_rotate = RotateCCW0;
    bool display_hflip = true;
    float screen_min_brightness = 0.3f;
    float screen_mid_brightness = 0.65f;
    float lux_bound1 = 30;
    float lux_bound2 = 500;
    UISide ui_side = UIOnLeft;
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
        else if (key == "i2c_index") try_load_int(i2c_index, val_str);
        else if (key == "ircut_gpio_index") try_load_int(ircut_gpio_index, val_str);
        else if (key == "display_rotate") try_load_int_enum(display_rotate, val_str);
        else if (key == "display_hflip") try_load_bool(display_hflip, val_str);
        else if (key == "screen_min_brightness") try_load_norm(screen_min_brightness, val_str);
        else if (key == "screen_mid_brightness") try_load_norm(screen_mid_brightness, val_str);
        else if (key == "lux_bound1") try_load_float(lux_bound1, val_str);
        else if (key == "lux_bound2") try_load_float(lux_bound2, val_str);
        else if (key == "ui_side") try_load_int_enum(ui_side, val_str);
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
    fprintf(fh, "i2c_index\t%d\n", i2c_index);
    fprintf(fh, "ircut_gpio_index\t%d\n", ircut_gpio_index);
    fprintf(fh, "display_rotate\t%d\n", display_rotate);
    fprintf(fh, "display_hflip\t%s\n", display_hflip ? "true" : "false");
    fprintf(fh, "screen_min_brightness\t%f\n", screen_min_brightness);
    fprintf(fh, "screen_mid_brightness\t%f\n", screen_mid_brightness);
    fprintf(fh, "lux_bound1\t%f\n", lux_bound1);
    fprintf(fh, "lux_bound2\t%f\n", lux_bound2);
    fprintf(fh, "ui_side\t%d\n", ui_side);
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

