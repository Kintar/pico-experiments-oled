#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "display.h"

#define DISPLAY_PAGE_SIZE 8

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

    display_println(dsp, "This is a test.");
    display_println(dsp, "And only a test!");
    display_println(dsp, "0123456789ABCDEF0123456789");

    render(dsp);

    sleep_ms(2000);
    display_println(dsp, "Are we done yet?");
    render(dsp);

    sleep_ms(2000);
    display_println(dsp, "I wanna go home...");
    render(dsp);

    sleep_ms(1000);
    display_println(dsp, "Why won't you let me go...");
    render(dsp);

    return 0;
}
