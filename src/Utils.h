#pragma once

#include <filesystem>

std::filesystem::path getSettingsDir();

std::filesystem::path getSettingsFile(const std::string &filename);

std::string rawReadMesg(const std::filesystem::path &file);

bool rawWriteMesg(const std::filesystem::path &filename, const std::string &mesg);