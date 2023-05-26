#include <gpiod.h>


#include <cstdlib>
#include <iostream>
using std::clog;
using std::endl;

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        clog << "Usage: " << argv[0] << " pin state" << endl;
        exit(EXIT_FAILURE);
    }
    int pin = strtol(argv[1], nullptr, 0);
    int to_set = strtol(argv[2], nullptr, 0);
    clog << "going to set " << pin << " to " << to_set << endl;
    if (gpiod_ctxless_set_value("/dev/gpiochip0", pin, to_set, false, "test", nullptr, nullptr) != 0)
    {
        clog << "gpiod_ctxless_set_value failed" << endl;
    }
}