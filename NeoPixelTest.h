
#ifndef NEOPIXELTEST_H
#define NEOPIXELTEST_H

#include <stdint.h>

#include "ws2812-rpi-defines.h"
#include "ws2812-rpi.h"
#include "ws2812b.h"

class NeoPixelTest {

public:

    NeoPixelTest();

    ~NeoPixelTest();

    void ColorWipe(Color_t c, uint8_t wait);
    void Rainbow(uint8_t wait);
    void RainbowCycle(uint8_t wait);
    void TheaterChase(Color_t c, uint8_t wait);
    void TheaterChaseRainbow(uint8_t wait);

    void EffectsDemo();

private:

    NeoPixel *pixel;
    ws2812b *wsPixel;
};

#endif //NEOPIXELTEST_H
