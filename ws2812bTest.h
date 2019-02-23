
#ifndef WS2812BTEST_H
#define WS2812BTEST_H

#include <stdint.h>

#include "ws2812b.h"

class ws2812bTest {

public:

    ws2812bTest();

    ~ws2812bTest();

    void EffectsDemo();

private:

    ws2812b *wsPixel;

    void colorWipe();
};

#endif //WS2812BTEST_H
