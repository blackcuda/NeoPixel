#include "ws2812bTest.h"

int main(int argc, char **argv){

    ws2812bTest wsTest;

    std::cout << "NEOPIXEL" << std::endl;

    while(true)
    {
        wsTest.EffectsDemo();
    }

    return 0;
}


