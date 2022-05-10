#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "display.h"
#include <math.h>

#define DISPLAY_PAGE_SIZE 8
#define PI 3.14159265
#define TAU PI * 2;

void render(display *dsp) {
    oled_send_cmd(OLED_SET_COL_ADDR);
    oled_send_cmd(0);
    oled_send_cmd(127);

    oled_send_cmd(OLED_SET_PAGE_ADDR);
    oled_send_cmd(0);
    oled_send_cmd(7);

    oled_send_buf(dsp->buffer, dsp->bufferLength);
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

    oled_send_cmd(OLED_SET_CONTRAST);
    oled_send_cmd(0x01);

    // Create an empty display and render it
    display *dsp = display_create(128, 64);
    render(dsp);

    // Draw a sine wave
    for (int x = 0; x < 128; x++) {
        double val = ((double)x / 128) * TAU;
        double y = sin(val);
        y = y / 2 + 0.5;
        display_setPixel(dsp, x, (int)(y * 64), true);
    }
    render(dsp);


    // configure horizontal scrolling
    oled_send_cmd(OLED_SET_HORIZ_SCROLL | 0x00);
    oled_send_cmd(0x00); // dummy byte
    oled_send_cmd(0x00); // start page 0
    oled_send_cmd(0x00); // time interval
    oled_send_cmd(0x07); // end page 7
    oled_send_cmd(0x00); // dummy byte
    oled_send_cmd(0xFF); // dummy byte

    // let's goooo!
    oled_send_cmd(OLED_SET_SCROLL | 0x01);

    return 0;
}
