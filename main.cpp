#include "NeoPixelTest.h"
#include "ws2812bTest.h"

int main(int argc, char **argv){

    //NeoPixelTest test;
    ws2812bTest wsTest;

    std::cout << "NEOPIXEL" << std::endl;

    while(true)
    {
//        test.EffectsDemo();
        wsTest.EffectsDemo();
    }

    return 0;
}


