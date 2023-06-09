#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <thread>

using std::clog;
using std::endl;

#define I2C_DEVFILE "/dev/i2c-1"
#define ADDR_LOW 0x23
#define ADDR_HIGH 0x5c

#define CMD_POWER_DOWN 0x00
#define CMD_POWER_ON 0x01
#define CMD_RESET 0x07
#define CMD_MODE_CONT_HIRES 0X10
#define CMD_MODE_CONT_HIRES2 0x11
#define CMD_MODE_CONT_LOWRES 0x13

#define CMD_MODE_ONCE_HIRES 0X20
#define CMD_MODE_ONCE_HIRES2 0x21
#define CMD_MODE_ONCE_LOWRES 0x23

int main(int argc, char **argv)
{
    using namespace std::chrono_literals;
    if (argc != 2)
    {
        clog << "Usage: " << argv[0] << " low/high" << endl;
        exit(EXIT_FAILURE);
    }

    int addr = 0;
    if (strcmp(argv[1], "low") == 0)
    {
        addr = ADDR_LOW;
    }
    else if (strcmp(argv[1], "high") == 0)
    {
        addr = ADDR_HIGH;
    }
    else
    {
        clog << "invalid address selection \"" << argv[1] << "\", specify \"low\" or \"high\"" << endl;
        exit(EXIT_FAILURE);
    }

    int fd = open(I2C_DEVFILE, O_RDWR);
    if (fd < 0)
    {
        clog << "failed to open " << I2C_DEVFILE << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
    clog << "opened " << I2C_DEVFILE << ": fd " << fd << endl;

    if (ioctl(fd, I2C_SLAVE, addr) != 0)
    {
        clog << "failed to set slave address: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
    clog << "set address " << std::hex << ADDR_LOW << " to I2C_SLAVE" << endl;

    char val;
    char buf[3];

    val = CMD_POWER_ON;
    if (write(fd, &val, 1) != 1)
    {
        clog << "write CMD_POWER_ON failed: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    val = CMD_RESET;
    if (write(fd, &val, 1) != 1)
    {
        clog << "write CMD_RESET failed: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    val = CMD_MODE_CONT_HIRES2;
    if (write(fd, &val, 1) != 1)
    {
        clog << "write CMD_MODE_CONT_HIRES2 failed: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        std::this_thread::sleep_for(200ms);
        buf[0] = 0xff;
        buf[1] = 0xff;
        buf[2] = 0xff;
        int nread = read(fd, buf, 2);
        float lx = float(buf[0] * 256 + buf[1]) / 1.2f / 2.0f;
        printf("read result: %f lx, %02x %02x %02x (%d bytes actually read)\n", lx, buf[0], buf[1], buf[2], nread);
    }

    val = CMD_POWER_DOWN;
    write(fd, &val, 1);
}