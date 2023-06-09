#include "LightSensorThread.h"

#include <iostream>
#include <thread>
using std::clog;
using std::cout;
using std::endl;
using namespace std::chrono_literals;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        clog << "Usage: " << argv[0] << " i2c_device_file" << endl;
        exit(EXIT_FAILURE);
    }

    LightSensorThread thread(argv[1]);
    while (true)
    {
        std::this_thread::sleep_for(200ms);
        auto lum = thread.getRawLuminance();
        cout << "luminance: " << lum.first << " " << lum.second << endl;
    }
}