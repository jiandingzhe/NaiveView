#!/usr/bin/perl

use strict;
use File::Basename;
use File::Path;
use feature qw/say/;

die "output prefix not specified" if @ARGV == 0;

my $prefix_out = shift;

my $d_out = dirname $prefix_out;
mkpath $d_out;

my $f_header = $prefix_out.'.h';
my $f_source = $prefix_out.'.cpp';

# read input
my @all_settings;
while (<DATA>)
{
    chomp;
    my ($name, $type_expr, $defvalue) = split /\t/;
    
    my $mode;
    my $real_type;
    if ($type_expr eq 'norm')
    {
        $mode = 'norm';
        $real_type = 'float';
    }
    elsif ($type_expr =~ /^enum +(\w+)/)
    {
        $mode = 'enum';
        $real_type = $1;
    }
    else
    {
        $mode = $type_expr;
        $real_type = $type_expr;
    }
    push @all_settings, [$name, $mode, $real_type, $defvalue];
}

#
# write header
#
open my $fh_hdr, '>', $f_header or die "failed to open output header file \"$f_header\": $!";

print $fh_hdr <<HEREDOC;
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

enum ScreenBacklightMode
{
    WaveshareBacklight,
    PWMBacklight,
};

class Settings
{
  public:
    ~Settings();
    static Settings& getInstance();

HEREDOC

foreach my $data (@all_settings)
{
    my ($name, $mode, $real_type, $defvalue) = @$data;
    print $fh_hdr <<HEREDOC;
    $real_type get_$name() const;
    static $real_type get_${name}_default() { return $defvalue; }
    void set_$name($real_type);
    void reset_$name();

HEREDOC
}
print $fh_hdr <<HEREDOC;
  private:
    Settings();
    struct Guts;
    std::unique_ptr<Guts> guts;
};
HEREDOC

close $fh_hdr;

#
# source file
#
open my $fh_src, '>', $f_source or die "failed to open output source file \"$f_source\": $!";

my $hdr_base = basename $f_header;
print $fh_src <<HEREDOC;
// automatically generated, do not modify manually!
#include "$hdr_base"

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
HEREDOC
foreach my $data (@all_settings)
{
    my ($name, $mode, $real_type, $defvalue) = @$data;
    print $fh_src <<HEREDOC;
    $real_type $name = $defvalue;
HEREDOC
}
print $fh_src <<HEREDOC;
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
    clog << "failed to parse float expression \\"" << str << "\\": " << strerror(errno) << endl;
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
    clog << "failed to parse boolean expression \\"" << str << "\\"" << endl;
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
    clog << "failed to parse integer expression \\"" << str << "\\": " << strerror(errno) << endl;
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
    clog << "failed to parse integer expression \\"" << str << "\\": " << strerror(errno) << endl;
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
        clog << "failed to open config file \\"" << config_file << "\\" for read:" << strerror(errno) << endl;
        return false;
    }
    
    std::string line;
    while (std::getline(fh, line))
    {
        auto delim = line.find('\\t');
        if (delim == std::string::npos || delim == 0)
            continue;
        std::string key = line.substr(0, delim);
        std::string val_str = line.substr(delim+1);
HEREDOC
foreach my $i (0..$#all_settings)
{
    my $data = $all_settings[$i];
    my ($name, $mode, $real_type, $defvalue) = @$data;
    my $if_expr = $i == 0 ? 'if' : 'else if';
    my $load_expr;
    if ($mode eq 'float')       { $load_expr = "try_load_float($name, val_str)" }
    elsif ($mode eq 'bool')     { $load_expr = "try_load_bool($name, val_str)" }
    elsif ($mode eq 'norm')     { $load_expr = "try_load_norm($name, val_str)" }
    elsif ($mode eq 'int')      { $load_expr = "try_load_int($name, val_str)"}
    elsif ($mode eq 'unsigned') { $load_expr = "try_load_uint($name, val_str)"}
    elsif ($mode eq 'enum')     { $load_expr = "try_load_int_enum($name, val_str)" }
    else { die "unrecognized type mode \"$mode\" for property $name" }
    print $fh_src <<HEREDOC;
        $if_expr (key == \"$name\") $load_expr;
HEREDOC
}
print $fh_src <<HEREDOC;
    }
    return true;
} // end of reload_from_config_file()

bool Settings::Guts::save_to_config_file() const
{
    fs::create_directories(config_file.parent_path());
    auto* fh = fopen(config_file.c_str(), "w");
    if (fh == nullptr)
    {
        clog << "failed to open config file \\"" << config_file << "\\" for write: " << strerror(errno) << endl;
        return false;
    }
HEREDOC
foreach my $data (@all_settings)
{
    my ($name, $mode, $real_type, $defvalue) = @$data;
    my $dump_expr;
    if ($real_type eq 'float') { $dump_expr = "fprintf(fh, \"$name\\t%f\\n\", $name)" }
    elsif ($real_type eq 'int' or $mode eq 'enum') { $dump_expr = "fprintf(fh, \"$name\\t%d\\n\", $name)" }
    elsif ($real_type eq 'unsigned') { $dump_expr = "fprintf(fh, \"$name\\t%u\\n\", $name)" }
    elsif ($real_type eq 'bool') { $dump_expr = "fprintf(fh, \"$name\\t%s\\n\", $name ? \"true\" : \"false\")" }
    else { die "unrecognized type $mode $real_type for $name" }
    print $fh_src <<HEREDOC
    $dump_expr;
HEREDOC
}
print $fh_src <<HEREDOC;
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

HEREDOC

foreach my $data (@all_settings)
{
    my ($name, $mode, $real_type, $defvalue) = @$data;

    print $fh_src <<HEREDOC;
$real_type Settings::get_$name() const { return guts->$name; }
void Settings::set_$name($real_type v)
{
    guts->$name = v;
    guts->save_to_config_file();
}
void Settings::reset_$name()
{
    guts->$name = $defvalue;
    guts->save_to_config_file();
}

HEREDOC
}

close $fh_src;

__DATA__
camera_width	int	1280
camera_height	int	720
i2c_index	int	1
gpio_chip_index	int	0
ircut_gpio_index	int	17
ircut_lux	float	10
display_rotate	enum RotateCCW	RotateCCW0
display_hflip	bool	true
screen_min_brightness	norm	0.3f
screen_mid_brightness	norm	0.65f
lux_bound1	float	30
lux_bound2	float	500
ui_side	enum UISide	UIOnLeft
bl_mode	enum ScreenBacklightMode	WaveshareBacklight
pwm_bl_chip	int	0
pwm_bl_index	int	0
pwm_bl_period	int	50000
pwm_bl_full_duty_cycle	norm	0.0
pwm_bl_zero_duty_cycle	norm	0.7333
