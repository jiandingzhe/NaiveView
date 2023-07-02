#pragma once

#include <filesystem>

std::filesystem::path getSettingsDir();

std::filesystem::path getSettingsFile(const std::string &filename);

std::string sysfsReadStr(const std::filesystem::path &file);

int sysfsReadStrToInt(const std::filesystem::path &file, int fallbackValue = 0);

bool sysfsWriteStr(const std::filesystem::path &filename, const std::string &mesg);

bool sysfsWriteToStr(const std::filesystem::path &filename, int mesg);