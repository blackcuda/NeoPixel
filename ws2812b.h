

#ifndef WS2812B_H
#define WS2812B_H

#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>

#include "ws2812-rpi-defines.h"

class ws2812b
{
public:
    ws2812b(uint16_t numLeds);
    ~ws2812b();

    void Show(void);

    bool SetPixelColour(uint32_t pixelNr, uint8_t r, uint8_t g, uint8_t b);
    bool SetPixelColour(uint32_t pixelNr, uint8_t r, uint8_t g, uint8_t b, uint8_t w);

    void SetBrightness();

    uint32_t GetPixelAmount();

private:

    void initHardware();

    void startTransfer();

    void clearPWMBuffer();

    void initLEDBuffer();
    void clearLEDBuffer();

    void setColourBits();
    void setColourBit();
    void setPWMBit(unsigned int bitPos, bool bit);

    void* map_peripheral(uint32_t base, uint32_t len);

    void terminate(int dummy);
    void fatal(std::string fmt, ...);

    unsigned int mem_virt_to_phys(void *virt);

    void initPheripherals(void);
    void initControlBlock(void);
    void initPWMClock(void);
    void initPWM(void);

    uint16_t m_numLEDs;
    float m_brightness;

    std::vector<Color_t> m_LEDBuffer;

    unsigned int m_PWMWaveform[NUM_DATA_WORDS];

    static struct control_data_s *m_ctlPtr;

    page_map_t *page_map;
    static uint8_t *m_virtbasePtr;

    static volatile unsigned int *pwm_reg;
    static volatile unsigned int *clk_reg;
    static volatile unsigned int *dma_reg;
    static volatile unsigned int *gpio_reg;
};

#endif //WS2812B_H
