// automatically generated, do not modify manually!
#pragma once

#include <memory>

enum UISide
{
    UIOnLeft,
    UIOnRight
};

class Settings
{
  public:
    ~Settings();
    static Settings& getInstance();

    int get_i2c_index() const;
    static int get_i2c_index_default() { return 1; }
    void set_i2c_index(int);
    void reset_i2c_index();

    float get_screen_min_brightness() const;
    static float get_screen_min_brightness_default() { return 0.3f; }
    void set_screen_min_brightness(float);
    void reset_screen_min_brightness();

    float get_screen_mid_brightness() const;
    static float get_screen_mid_brightness_default() { return 0.65f; }
    void set_screen_mid_brightness(float);
    void reset_screen_mid_brightness();

    float get_lux_bound1() const;
    static float get_lux_bound1_default() { return 30; }
    void set_lux_bound1(float);
    void reset_lux_bound1();

    float get_lux_bound2() const;
    static float get_lux_bound2_default() { return 500; }
    void set_lux_bound2(float);
    void reset_lux_bound2();

    UISide get_ui_side() const;
    static UISide get_ui_side_default() { return UIOnLeft; }
    void set_ui_side(UISide);
    void reset_ui_side();

  private:
    Settings();
    struct Guts;
    std::unique_ptr<Guts> guts;
};
