#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

#include "5x8_font.h"

#include "display.h"

void display_clear(display *dsp) {
    memset(dsp->buffer, 0, dsp->bufferLength);
}

void display_init(display *dsp, uint8_t width, uint8_t height) {
    dsp->width = width;
    dsp->height = height;
    dsp->pages = height / 8;

    // heights below DISPLAY_PAGE_SIZE will result in zero pages, and heights that are not even multiples of
    // DISPLAY_PAGE_SIZE will "bleed off" the bottom edge of the screen unless we add another, partial page
    // of data to the buffer.
    if (dsp->pages == 0 || height % DISPLAY_PAGE_SIZE != 0) {
        dsp->pages++;
    }

    dsp->bufferLength = dsp->pages * width;
    dsp->buffer = malloc(dsp->bufferLength);
    display_clear(dsp);
}

display *display_create(uint8_t width, uint8_t height) {
    display *dsp = malloc(sizeof(display));
    display_init(dsp, width, height);
    return dsp;
}

void display_destroy(display *dsp) {
    free(dsp->buffer);
    free(dsp);
}

void display_setPixel(display *dsp, uint8_t x, uint8_t y, bool on) {
    uint8_t page = y / DISPLAY_PAGE_SIZE;
    uint8_t bit = 0x01 << y % DISPLAY_PAGE_SIZE;
    uint16_t index = x + (page * dsp->width);
    if (on) {
        dsp->buffer[index] |= bit;
    } else {
        dsp->buffer[index] &= !bit;
    }
}

/*
 * Prints a line of text into the display's buffer.
 * Begins by scrolling the entire display up by one page, then writes the contents of str into the bottom of the
 * display. Truncates if more than 25 characters are present.
 */
void display_println(display *dsp, const char *str) {
    // "scroll" up by a page
    memcpy(dsp->buffer, dsp->buffer + 128, dsp->bufferLength - 128);
    int idx = 128 * 6; // Beginning of page 7
    memset(dsp->buffer + idx, 0, 128); // Write zeroes in to erase the contents of the page
    // Run for 25 characters, or until we reach a null terminator
    for (int c = 0; c < 25 && str[c] != '\0'; c++) {
        char glyph = str[c];
        memcpy(dsp->buffer + idx, font[glyph], 5);
        idx+=5;
    }
}
