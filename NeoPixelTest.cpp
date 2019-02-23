
#include "NeoPixelTest.h"


NeoPixelTest::NeoPixelTest()
    :pixel(new NeoPixel(2))
    ,wsPixel(new ws2812b(2))
{

}

NeoPixelTest::~NeoPixelTest()
{

}

void NeoPixelTest::ColorWipe(Color_t c, uint8_t wait)
{
    uint16_t i;
    for(i=0; i<pixel->numPixels(); i++) {
        pixel->setPixelColor(i, c);
        pixel->show();
        usleep(wait * 1000);
    }
}

void NeoPixelTest::Rainbow(uint8_t wait)
{
    uint16_t i, j;

    for(j=0; j<256; j++) {
        for(i=0; i<pixel->numPixels(); i++) {
            pixel->setPixelColor(i, pixel->wheel((i+j) & 255));
        }
        pixel->show();
        usleep(wait * 1000);
    }
}

void NeoPixelTest::RainbowCycle(uint8_t wait)
{
    uint16_t i, j;

    for(j=0; j<256*5; j++) {
        for(i=0; i<pixel->numPixels(); i++) {
            pixel->setPixelColor(i, pixel->wheel(((i * 256 / pixel->numPixels()) + j) & 255));
        }
        pixel->show();
        usleep(wait * 1000);
    }
}

void NeoPixelTest::TheaterChase(Color_t c, uint8_t wait)
{
    unsigned int j, q, i;
    for (j=0; j<15; j++) {
        for (q=0; q < 3; q++) {
            for (i=0; i < pixel->numPixels(); i=i+3) {
                pixel->setPixelColor(i+q, c);
            }
            pixel->show();

            usleep(wait * 1000);

            for (i=0; i < pixel->numPixels(); i=i+3) {
                pixel->setPixelColor(i+q, 0, 0, 0);
            }
        }
    }
}

void NeoPixelTest::TheaterChaseRainbow(uint8_t wait)
{
    int j, q, i;
    for (j=0; j < 256; j+=4) {
        for (q=0; q < 3; q++) {
            for (i=0; i < pixel->numPixels(); i=i+3) {
                pixel->setPixelColor(i+q, pixel->wheel((i+j) % 255));
            }
            pixel->show();

            usleep(wait * 1000);

            for (i=0; i < pixel->numPixels(); i=i+3) {
                pixel->setPixelColor(i+q, 0, 0, 0);
            }
        }
    }
}

void NeoPixelTest::EffectsDemo()
{
    int i = 0;
    int j = 0;
    float k = 0.0;

    // Default effects from the Arduino lib
//    ColorWipe(pixel->Color(255, 0, 0), 50); // Red
//    ColorWipe(pixel->Color(0, 255, 0), 50); // Green
//    ColorWipe(pixel->Color(0, 0, 255), 50); // Blue

//    TheaterChase(pixel->Color(127, 127, 127), 50); // White
//    TheaterChase(pixel->Color(127,   0,   0), 50); // Red
//    TheaterChase(pixel->Color(  0,   0, 127), 50); // Blue

    Rainbow(5);
    RainbowCycle(5);
    TheaterChaseRainbow(50);

    // Watermelon fade :)
    for(k=0; k<0.5; k+=.01)
    {
        pixel->setBrightness(k);
        for(i=0; i < (pixel->numPixels()); i++)
        {
            pixel->setPixelColor(i, i*5, 64, i*2);
        }

        pixel->show();
    }

    for(k=0.5; k>=0; k-=.01)
    {
        pixel->setBrightness(k);
        for(i=0; i < pixel->numPixels(); i++)
        {
            pixel->setPixelColor(i, i*5, 64, i*2);
        }
        pixel->show();
    }
    usleep(1000);

    // Random color fade
    srand(time(NULL));
    uint8_t lastRed = 0;
    uint8_t lastGreen = 0;
    uint8_t lastBlue = 0;
    uint8_t red, green, blue;
    Color_t curPixel;
    pixel->setBrightness(DEFAULT_BRIGHTNESS);
    for(j=1; j<16; j++)
    {
        if(j % 3)
        {
            red = 120;
            green = 64;
            blue = 48;
        }
        else if(j % 7)
        {
            red = 255;
            green = 255;
            blue = 255;
        }
        else
        {
            red = rand();
            green = rand();
            blue = rand();
        }
        for(k=0; k<1; k+=.01) {
            for(i=0; i<pixel->numPixels(); i++) {
                pixel->setPixelColor(
                    i,
                    (red * k) + (lastRed * (1-k)),
                    i * (255 / pixel->numPixels()), //(green * k) + (lastGreen * (1-k)),
                    (blue * k) + (lastBlue * (1-k))
                    );
                curPixel = pixel->getPixelColor(i);
            }
            pixel->show();
        }
        lastRed = red;
        lastGreen = green;
        lastBlue = blue;
    }
}

/*------------------- LOCAL FUNCTIONS ---------------------*/


