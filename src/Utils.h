#pragma once

#include <filesystem>

std::filesystem::path getSettingsDir();

std::filesystem::path getSettingsFile(std::string filename);