#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#define DISPLAY_PAGE_SIZE 8

typedef struct s_display {
    uint8_t width;
    uint8_t height;
    uint8_t pages;
    int bufferLength;
    uint8_t *buffer;
} display;

void display_clear(display *dsp);

void display_init(display *dsp, uint8_t width, uint8_t height);

display* display_create(uint8_t width, uint8_t height);

void display_destroy(display *dsp);

void display_setPixel(display *dsp, uint8_t x, uint8_t y, bool on);

void display_println(display *dsp, const char *str);

#endif //OLED_DISPLAY_H
