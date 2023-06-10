// automatically generated, do not modify manually!
#include "Settings.h"

#include "Utils.h"

#include <filesystem>
#include <fstream>

#include <cstdlib>

namespace fs = std::filesystem;

struct Settings::Guts
{
    bool reload_from_config_file();
    bool save_to_config_file() const;

    fs::path config_file;
    int i2c_index = 1;
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
        return false;
    
    std::string line;
    while (std::getline(fh, line))
    {
        auto delim = line.find('\t');
        if (delim == std::string::npos || delim == 0)
            continue;
        std::string key = line.substr(0, delim);
        std::string val_str = line.substr(delim+1);
        if (key == "i2c_index") try_load_int(i2c_index, val_str);
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
        return false;
    fprintf(fh, "i2c_index\t%d", i2c_index);
    fprintf(fh, "screen_min_brightness\t%f", screen_min_brightness);
    fprintf(fh, "screen_mid_brightness\t%f", screen_mid_brightness);
    fprintf(fh, "lux_bound1\t%f", lux_bound1);
    fprintf(fh, "lux_bound2\t%f", lux_bound2);
    fprintf(fh, "ui_side\t%d", ui_side);
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
    guts->reload_from_config_file();
}

Settings::~Settings() {}

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

