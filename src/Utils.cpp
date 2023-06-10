#include "Utils.h"

#include <cstdlib>

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

namespace fs = std::filesystem;

fs::path getSettingsDir()
{
    fs::path settings_root;

    // determine by XDG_CONFIG_HOME
    const char *root_by_xdg = getenv("XDG_CONFIG_HOME");
    if (root_by_xdg != nullptr)
    {
        settings_root = fs::path(root_by_xdg);
    }
    else
    {
        // determine by home dir
        const char *home_name = getenv("HOME");
        if (home_name == nullptr)
            home_name = getpwuid(getuid())->pw_dir;
        settings_root = fs::path(home_name) / ".config";
    }

    return settings_root / "NaiveView";
}

fs::path getSettingsFile(std::string filename)
{
    return getSettingsDir() / filename;
}