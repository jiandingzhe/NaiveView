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

#include <filesystem>
#include <fstream>

#include <cstdlib>

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
        return false;
HEREDOC
foreach my $data (@all_settings)
{
    my ($name, $mode, $real_type, $defvalue) = @$data;
    my $fmt;
    if ($real_type eq 'float') { $fmt = '%f' }
    elsif ($real_type eq 'int' or $mode eq 'enum') { $fmt = '%d' }
    elsif ($real_type eq 'unsigned') { $fmt = '%u' }
    else { die "unrecognized type $mode $real_type for $name" }
    print $fh_src <<HEREDOC
    fprintf(fh, "$name\\t$fmt", $name);
HEREDOC
}
print $fh_src <<HEREDOC;
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
i2c_index	int	1
screen_min_brightness	norm	0.3f
screen_mid_brightness	norm	0.65f
lux_bound1	float	30
lux_bound2	float	500
ui_side	enum UISide	UIOnLeft
