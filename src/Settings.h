// automatically generated, do not modify manually!
#pragma once

#include <memory>

enum UISide
{
    UIOnLeft,
    UIOnRight
};

enum RotateCCW
{
    RotateCCW0,
    RotateCCW90,
    RotateCCW180,
    RotateCCW270
};

class Settings
{
  public:
    ~Settings();
    static Settings& getInstance();

    int get_camera_width() const;
    static int get_camera_width_default() { return 1280; }
    void set_camera_width(int);
    void reset_camera_width();

    int get_camera_height() const;
    static int get_camera_height_default() { return 720; }
    void set_camera_height(int);
    void reset_camera_height();

    int get_i2c_index() const;
    static int get_i2c_index_default() { return 1; }
    void set_i2c_index(int);
    void reset_i2c_index();

    int get_gpio_chip_index() const;
    static int get_gpio_chip_index_default() { return 0; }
    void set_gpio_chip_index(int);
    void reset_gpio_chip_index();

    int get_ircut_gpio_index() const;
    static int get_ircut_gpio_index_default() { return 17; }
    void set_ircut_gpio_index(int);
    void reset_ircut_gpio_index();

    float get_ircut_lux() const;
    static float get_ircut_lux_default() { return 10; }
    void set_ircut_lux(float);
    void reset_ircut_lux();

    RotateCCW get_display_rotate() const;
    static RotateCCW get_display_rotate_default() { return RotateCCW0; }
    void set_display_rotate(RotateCCW);
    void reset_display_rotate();

    bool get_display_hflip() const;
    static bool get_display_hflip_default() { return true; }
    void set_display_hflip(bool);
    void reset_display_hflip();

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
