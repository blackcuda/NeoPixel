
#include "ws2812b.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits>
#include <sys/mman.h>
#include <iostream>

struct control_data_s* ws2812b::m_ctlPtr=0;
uint8_t* ws2812b::m_virtbasePtr = 0;
volatile unsigned int* ws2812b::pwm_reg = 0;
volatile unsigned int* ws2812b::clk_reg = 0;
volatile unsigned int* ws2812b::dma_reg = 0;
volatile unsigned int* ws2812b::gpio_reg = 0;

ws2812b::ws2812b(uint16_t numLeds)
    :m_numLEDs(numLeds),
     m_brightness(DEFAULT_BRIGHTNESS)
{
    initHardware();
    initLEDBuffer();
}

ws2812b::~ws2812b()
{
    terminate(0);
}

bool ws2812b::SetPixelColour(uint32_t pixelNr, uint8_t r, uint8_t g, uint8_t b)
{
    std::cout << "setPixelColour " << pixelNr << std::endl;

    if (pixelNr < 0)
    {
        printf("Unable to set pixel %d (less than zero?)\n", pixelNr);
        return false;
    }

    if (pixelNr > m_LEDBuffer.size())
    {
        printf("Unable to set pixel %d (LED buffer is %d pixels long)\n", pixelNr, m_numLEDs);
        return false;
    }

    Color_t color;

    color.r = r;
    color.g = g;
    color.b = b;

    uint8_t ledBufferSize = m_LEDBuffer.size();

    std::cout << "Red = " << std::to_string(color.r) << ", Green = " << std::to_string(color.g) << ", Blue = " << std::to_string(color.b) <<  std::endl;
    std::cout << "LED buffer size " << std::to_string(ledBufferSize) << std::endl;

    m_LEDBuffer.at(pixelNr) = color;
}

void ws2812b::Show()
{
    setColourBits();

    m_ctlPtr = (struct control_data_s *)m_virtbasePtr;
    dma_cb_t *cbp = m_ctlPtr->cb;

    for (int index = 0; index < (cbp->length / 4); index++)
    {
        m_ctlPtr->sample[index] = m_PWMWaveform[index];
    }

    startTransfer();

    float bitTimeUsec = (float)(NUM_DATA_WORDS * 32) * 0.4;
    usleep((int)bitTimeUsec);
}

uint32_t ws2812b::GetPixelAmount()
{
    return m_numLEDs;
}

/* ---------- LOCAL FUNCTIONS ---------- */

void ws2812b::initHardware()
{
    int pid = 0;
    int fd = 0;
    char pagemap_fn[64];

    // Clear the PWM buffer
    clearPWMBuffer();

    // Set up peripheral access
    initPheripherals();

    // Set PWM alternate function for GPIO18
    SET_GPIO_ALT(18, 5);

    // Allocate memory for the DMA control block & data to be sent
    m_virtbasePtr = (uint8_t*) mmap(
        NULL,
        NUM_PAGES * PAGE_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED |
        MAP_ANONYMOUS |
        MAP_NORESERVE |
        MAP_LOCKED,
        -1,
        0);

    if (m_virtbasePtr == MAP_FAILED)
    {
        fatal("Failed to mmap physical pages: %m\n");
    }

    if ((unsigned long)m_virtbasePtr & (PAGE_SIZE-1))
    {
        fatal("Virtual address is not page aligned\n");
    }

    // Allocate page map (pointers to the control block(s) and data for each CB
    page_map =(page_map_t*) malloc(NUM_PAGES * sizeof(*page_map));

    if (page_map == 0)
    {
        fatal("Failed to malloc page_map: %m\n");
    }

    pid = getpid();
    sprintf(pagemap_fn, "/proc/%d/pagemap", pid);
    fd = open(pagemap_fn, O_RDONLY);

    if (fd < 0)
    {
        fatal("Failed to open %s: %m\n", pagemap_fn);
    }

    if (lseek(fd, (unsigned long)m_virtbasePtr >> 9, SEEK_SET) != (unsigned long)m_virtbasePtr >> 9)
    {
        fatal("Failed to seek on %s: %m\n", pagemap_fn);
    }

    for (int i = 0; i < NUM_PAGES; i++)
    {
        uint64_t pfn;
        page_map[i].virtaddr = m_virtbasePtr + i * PAGE_SIZE;

        page_map[i].virtaddr[0] = 0;

        if (read(fd, &pfn, sizeof(pfn)) != sizeof(pfn))
        {
            fatal("Failed to read %s: %m\n", pagemap_fn);
        }

        if ((pfn >> 55)&0xfbf != 0x10c)
        {
            fatal("Page %d not present (pfn 0x%016llx)\n", i, pfn);
        }

        page_map[i].physaddr = (unsigned int)pfn << PAGE_SHIFT | 0x40000000;
    }

    initControlBlock();
    initPWMClock();
    initPWM();
}

void ws2812b::startTransfer()
{
    dma_reg[DMA_CONBLK_AD] = mem_virt_to_phys(m_ctlPtr->cb);
    dma_reg[DMA_CS] = DMA_CS_CONFIGWORD | (1 << DMA_CS_ACTIVE);
    usleep(100);

    SETBIT(pwm_reg[PWM_CTL], PWM_CTL_PWEN1);
}

void ws2812b::clearPWMBuffer()
{
    memset(m_PWMWaveform, 0, NUM_DATA_WORDS * 4);
}

void ws2812b::initLEDBuffer()
{
    std::cout << "initLEDBuffer" << std::endl;

    m_LEDBuffer.resize(m_numLEDs);

    for(Color_t led : m_LEDBuffer)
    {
        led.r = 0;
        led.g = 0;
        led.b = 0;
    }
}

void ws2812b::clearLEDBuffer(){
    int i;
    for(i=0; i<m_numLEDs; i++) {
        m_LEDBuffer[i].r = 0;
        m_LEDBuffer[i].g = 0;
        m_LEDBuffer[i].b = 0;
    }
}

void ws2812b::setColourBits()
{
    unsigned int colorBits = 0;
    unsigned char colorBit = 0;
    unsigned int wireBit = 0;

    uint8_t sizeOfColorBits = sizeof(colorBits) * 8;
    uint8_t sizeOfColorBit = sizeof(colorBit) * 8;
    uint8_t colorBitAmount = sizeOfColorBit * 3; // Amount of colorbit in Colorbits. 8 bits for every color (R,G,B).

    for (Color_t led : m_LEDBuffer)
    {
        colorBits = (led.b | (unsigned int)led.r << 8 | ((unsigned int)led.g << 16));

        for (int bitIndex = (colorBitAmount - 1); bitIndex >= 0; bitIndex--)
        {
            colorBit = (colorBits & (1 << bitIndex)) ? 1 : 0;

            if (colorBit)
            {
                setPWMBit(wireBit++, 1);
                setPWMBit(wireBit++, 1);
                setPWMBit(wireBit++, 0);
            }
            else
            {
                setPWMBit(wireBit++, 1);
                setPWMBit(wireBit++, 0);
                setPWMBit(wireBit++, 0);
            }
        }
    }
}

void ws2812b::setPWMBit(unsigned int bitPos, bool bit)
{
    uint8_t sizeOfUInt = std::numeric_limits<unsigned>::digits;
    unsigned int wordOffset = bitPos / sizeOfUInt;
    unsigned int bitIdx = bitPos - (wordOffset *sizeOfUInt);

    if (bit)
    {
        m_PWMWaveform[wordOffset] |= (1 << ((sizeOfUInt - 1) - bitIdx));
    }
    else
    {
        m_PWMWaveform[wordOffset] &= ~(1 << ((sizeOfUInt - 1) - bitIdx));
    }
}

void* ws2812b::map_peripheral(uint32_t base, uint32_t len)
{
    int fd = open("/dev/mem", O_RDWR);
    void* vaddr;

    if (fd < 0)
    {
        fatal("Failed to open /dev/mem: %m\n");
    }

    vaddr = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, base);

    if (vaddr == MAP_FAILED)
    {
        fatal("Failed to map peripheral at 0x%08x: %m\n", base);
    }
    close(fd);

    return vaddr;
}

void ws2812b::fatal(std::string fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt.c_str(), ap);
    va_end(ap);
    terminate(0);
}

void ws2812b::terminate(int /*dummy*/)
{
    if(dma_reg)
    {
        CLRBIT(dma_reg[DMA_CS], DMA_CS_ACTIVE);
        usleep(100);
        SETBIT(dma_reg[DMA_CS], DMA_CS_RESET);
        usleep(100);
    }

    // Shut down PWM
    if(pwm_reg)
    {
        CLRBIT(pwm_reg[PWM_CTL], PWM_CTL_PWEN1);
        usleep(100);
        pwm_reg[PWM_CTL] = (1 << PWM_CTL_CLRF1);
    }

    // Free the allocated memory
    if(page_map != 0)
    {
        free(page_map);
    }
}

unsigned int ws2812b::mem_virt_to_phys(void *virt)
{
    unsigned int offset = (uint8_t *)virt - m_virtbasePtr;

    return page_map[offset >> PAGE_SHIFT].physaddr + (offset % PAGE_SIZE);
}

void ws2812b::initPheripherals()
{
    dma_reg = (unsigned int*)map_peripheral(DMA_BASE, DMA_LEN);
    dma_reg += 0x000;
    pwm_reg = (unsigned int*)map_peripheral(PWM_BASE, PWM_LEN);
    clk_reg = (unsigned int*)map_peripheral(CLK_BASE, CLK_LEN);
    gpio_reg = (unsigned int*)map_peripheral(GPIO_BASE, GPIO_LEN);
}

void ws2812b::initControlBlock()
{
    m_ctlPtr = (struct control_data_s *)m_virtbasePtr;
    dma_cb_t *cbp = m_ctlPtr->cb;
    unsigned int phys_pwm_fifo_addr = 0x7e20c000 + 0x18;

    cbp->info = DMA_TI_CONFIGWORD;
    cbp->src = mem_virt_to_phys(m_ctlPtr->sample);
    cbp->dst = phys_pwm_fifo_addr;
    cbp->length = ((m_numLEDs * 2.25) + 1) * 4;

    if (cbp->length > NUM_DATA_WORDS * 4)
    {
        cbp->length = NUM_DATA_WORDS * 4;
    }

    cbp->stride = 0;
    cbp->pad[0] = 0;
    cbp->pad[1] = 0;
    cbp->next = 0;

    dma_reg[DMA_CS] |= (1 << DMA_CS_ABORT);
    usleep(100);
    dma_reg[DMA_CS] |= (1 << DMA_CS_RESET);
    usleep(100);
}

void ws2812b::initPWMClock()
{
    clk_reg[PWM_CLK_CNTL] = 0x5A000000 | (1 << 5);
    usleep(100);

    CLRBIT(pwm_reg[PWM_DMAC], PWM_DMAC_ENAB);
    usleep(100);

    unsigned int idiv = 400;
    unsigned short fdiv = 0;
    clk_reg[PWM_CLK_DIV] = 0x5A000000 | (idiv << 12) | fdiv;
    usleep(100);

    clk_reg[PWM_CLK_CNTL] = 0x5A000015;
    usleep(100);
}

void ws2812b::initPWM()
{
    pwm_reg[PWM_CTL] = 0;
    pwm_reg[PWM_RNG1] = 32;
    usleep(100);

    pwm_reg[PWM_DMAC] =
        (1 << PWM_DMAC_ENAB) |
        (8 << PWM_DMAC_PANIC) |
        (8 << PWM_DMAC_DREQ);

    usleep(1000);

    SETBIT(pwm_reg[PWM_CTL], PWM_CTL_CLRF1);
    usleep(100);

    CLRBIT(pwm_reg[PWM_CTL], PWM_CTL_RPTL1);
    usleep(100);

    CLRBIT(pwm_reg[PWM_CTL], PWM_CTL_SBIT1);
    usleep(100);

    CLRBIT(pwm_reg[PWM_CTL], PWM_CTL_POLA1);
    usleep(100);

    SETBIT(pwm_reg[PWM_CTL], PWM_CTL_MODE1);
    usleep(100);

    SETBIT(pwm_reg[PWM_CTL], PWM_CTL_USEF1);
    usleep(100);

    CLRBIT(pwm_reg[PWM_CTL], PWM_CTL_MSEN1);
    usleep(100);

    SETBIT(dma_reg[DMA_CS], DMA_CS_INT);
    usleep(100);

    SETBIT(dma_reg[DMA_CS], DMA_CS_END);
    usleep(100);

    dma_reg[DMA_CONBLK_AD] = mem_virt_to_phys(m_ctlPtr->cb);
    usleep(100);

    dma_reg[DMA_DEBUG] = 7;
    usleep(100);
}
