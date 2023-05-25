#include <linux/hidraw.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using std::clog;
using std::endl;

#define REPORT_ID 9
#define VENDOR_ID 1810
#define PRODUCT_ID 10

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        clog << "Usage: " << argv[0] << " hidraw_device brightness_value(0~255)" << endl;
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0)
    {
        clog << "failed to open device " << argv[1] << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    struct hidraw_devinfo info = {};
    ioctl(fd, HIDIOCGRAWINFO, &info);
    clog << "vendor " << info.vendor << ", product " << info.product << endl;

    if (info.vendor != VENDOR_ID || info.product != PRODUCT_ID)
    {
        clog << "device is not a waveshare 5.5inch AMOLED screen, exit" << endl;
        exit(EXIT_FAILURE);
    }

    auto target_brightness = (int)strtol(argv[2], nullptr, 0);
    target_brightness = std::max(0, std::min(255, target_brightness));
    unsigned char v = (unsigned char)target_brightness;
    unsigned char vxor = v ^ (unsigned char)(0xFF);
    unsigned char mesg[5] = {REPORT_ID, 0x08, 0xF7, v, vxor};
    write(fd, mesg, 5);
}