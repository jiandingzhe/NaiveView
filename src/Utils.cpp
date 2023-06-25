#include "Utils.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using std::clog;
using std::endl;
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

fs::path getSettingsFile(const std::string &filename)
{
    return getSettingsDir() / filename;
}

std::string rawReadMesg(const std::filesystem::path &file)
{
    int fh = open(file.c_str(), O_RDONLY);
    if (fh < 0)
    {
        clog << "failed to open \"" << file << "\" for read: " << strerror(errno) << endl;
        return {};
    }

    // obtain whole length
    auto length = lseek(fh, 0, SEEK_END);
    if (length < 0)
    {
        clog << "failed to seek \"" << file << "\" with file descriptor " << fh << ": " << strerror(errno) << endl;
        close(fh);
        return {};
    }
    lseek(fh, 0, SEEK_SET);

    // read content
    std::string result(length, 0);
    read(fh, result.data(), length);
    close(fh);

    // remove last newline
    while (result.back() == '\n')
        result = result.substr(0, result.size() - 1);
    return result;
}

bool rawWriteMesg(const std::filesystem::path &filename, const std::string &mesg)
{
    int fh = open(filename.c_str(), O_WRONLY);
    if (fh < 0)
    {
        clog << "failed to open \"" << filename << "\" for write: " << strerror(errno) << endl;
        return false;
    }
    if (write(fh, mesg.c_str(), mesg.size()) != mesg.size())
    {
        clog << "failed to write message \"" << mesg << "\" to file \"" << filename << "\": " << strerror(errno)
             << endl;
        return false;
    }
    close(fh);
    return true;
}