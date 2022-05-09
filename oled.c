#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define PAGE_SIZE 8

typedef struct s_display {
    uint8_t width;
    uint8_t height;
    uint8_t pages;
    int bufferLength;
    uint8_t *buffer;
} display;

void init_display(display *dsp, uint8_t width, uint8_t height) {
    dsp->width = width;
    dsp->height = height;
    dsp->pages = height / 8;

    // heights below PAGE_SIZE will result in zero pages, and heights that are not even multiples of
    // PAGE_SIZE will "bleed off" the bottom edge of the screen unless we add another, partial page
    // of data to the buffer.
    if (dsp->pages == 0 || height % PAGE_SIZE != 0) {
        dsp->pages++;
    }

    dsp->bufferLength = dsp->pages * width;
    dsp->buffer = malloc(dsp->bufferLength);
    memset(dsp->buffer, 0, dsp->bufferLength);
}

display* createDisplay(uint8_t width, uint8_t height) {
    display *dsp = malloc(sizeof(display));
    init_display(dsp, width, height);
    return dsp;
}

void destroyDisplay(display *dsp) {
    free(dsp->buffer);
    free(dsp);
}

void setPixel(display *dsp, uint8_t x, uint8_t y, bool on) {
    uint8_t page = y / PAGE_SIZE;
    uint8_t bit = 0x01 << y % PAGE_SIZE;
    uint16_t index = x + (page * dsp->width);
    if (on) {
        dsp->buffer[index] |= bit;
    } else {
        dsp->buffer[index] &= !bit;
    }
}

void oled_send_cmd(uint8_t cmd) {
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command

    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(i2c_default, (OLED_ADDR & OLED_WRITE_MODE), buf, 2, false);
}

void oled_send_buf(const uint8_t buf[], int bufferLength) {
    // in horizontal addressing mode, the column address pointer auto-increments
    // and then wraps around to the next page

    // copy our frame buffer into a new buffer because we need to add the control byte
    // to the beginning

    // TODO find a more memory-efficient way to do this..
    // maybe break the data transfer into pages?
    uint8_t *temp_buf = malloc(bufferLength + 1);

    for (int i = 1; i < bufferLength + 1; i++) {
        temp_buf[i] = buf[i - 1];
    }
    // Co = 0, D/C = 1 => the driver expects data to be written to RAM
    temp_buf[0] = 0x40;
    i2c_write_blocking(i2c_default, (OLED_ADDR & OLED_WRITE_MODE), temp_buf, bufferLength + 1, false);

    free(temp_buf);
}

void oled_init() {
    oled_send_cmd(OLED_SET_DISP | 0x00); // set display off

    /* memory mapping */
    oled_send_cmd(OLED_SET_MEM_ADDR); // set memory address mode
    oled_send_cmd(0x00); // horizontal addressing mode

    /* resolution and layout */
    oled_send_cmd(OLED_SET_DISP_START_LINE); // set display start line to 0

    oled_send_cmd(OLED_SET_SEG_REMAP | 0x01); // set segment re-map
    // column address 127 is mapped to SEG0

    oled_send_cmd(OLED_SET_MUX_RATIO); // set multiplex ratio
    oled_send_cmd(63); // default mux ratio

    oled_send_cmd(OLED_SET_COM_OUT_DIR | 0x08); // set COM (common) output scan direction
    // scan from bottom up, COM[N-1] to COM0

    oled_send_cmd(OLED_SET_DISP_OFFSET); // set display offset
    oled_send_cmd(0x00); // no offset

    oled_send_cmd(OLED_SET_COM_PIN_CFG); // set COM (common) pins hardware configuration
    oled_send_cmd(0x10); // manufacturer magic number

    /* timing and driving scheme */
    oled_send_cmd(OLED_SET_DISP_CLK_DIV); // set display clock divide ratio
    oled_send_cmd(0x80); // div ratio of 1, standard freq

    oled_send_cmd(OLED_SET_PRECHARGE); // set pre-charge period
    oled_send_cmd(0xF1); // Vcc internally generated on our board

    oled_send_cmd(OLED_SET_VCOM_DESEL); // set VCOMH deselect level
    oled_send_cmd(0x30); // 0.83xVcc

    /* display */
    oled_send_cmd(OLED_SET_CONTRAST); // set contrast control
    oled_send_cmd(0x80);

    oled_send_cmd(OLED_SET_ENTIRE_ON); // set entire display on to follow RAM content

    oled_send_cmd(OLED_SET_NORM_INV); // set normal (not inverted) display

    oled_send_cmd(OLED_SET_CHARGE_PUMP); // set charge pump
    oled_send_cmd(0x14); // Vcc internally generated on our board

    oled_send_cmd(OLED_SET_SCROLL | 0x00); // deactivate horizontal scrolling if set
    // this is necessary as memory writes will corrupt if scrolling was enabled

    oled_send_cmd(OLED_SET_DISP | 0x01); // turn display on
}


#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17

int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    printf("I2C bus is online...\n");

    oled_init();

    display *dsp = createDisplay(128, 64);

    oled_send_cmd(OLED_SET_COL_ADDR);
    oled_send_cmd(0);
    oled_send_cmd(127);

    oled_send_cmd(OLED_SET_PAGE_ADDR);
    oled_send_cmd(0);
    oled_send_cmd(7);

    oled_send_buf(dsp->buffer, dsp->bufferLength);

    // Draw a diagonal across the screen
    uint8_t x = 0;
    uint8_t y = 0;
    for (y = 0; y < 64; y++) {
        setPixel(dsp, x++, y, true);
        setPixel(dsp, x++, y, true);
    }
    // Draw a square in the middle
    for (y = 30; y < 38; y++) {
        for (x = 60; x < 68; x++) {
            setPixel(dsp, x , y, true);
        }
    }
    // Draw a line at the bottom of the yellow
    y = 15;
    for (x = 0; x < 128; x++) {
        setPixel(dsp, x, y, true);
    }
    oled_send_buf(dsp->buffer, dsp->bufferLength);

    oled_send_cmd(OLED_SET_CONTRAST);
    oled_send_cmd(0x01);

    return 0;
}
