
#include "ws2812bTest.h"

#include <unistd.h>
#include <iostream>


ws2812bTest::ws2812bTest()
    :wsPixel(new ws2812b(2))
{

}

ws2812bTest::~ws2812bTest()
{

}


void ws2812bTest::EffectsDemo()
{
    colorWipe();
}

void ws2812bTest::colorWipe()
{
    std::cout << "COLOR WIPE" << std::endl;

    for(uint8_t i = 0; i < 20; i++)
    {
        for (uint32_t index = 0; index < wsPixel->GetPixelAmount(); index++)
        {
            wsPixel->SetPixelColour(index, i, i, i);
            wsPixel->Show();
        }
        usleep(5000);
    }
}
/*------------------- LOCAL FUNCTIONS ---------------------*/


