#include "terminal.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile char*)0xB8000)

static int row = 0;
static int column = 0;
static char color = 0x0F;

static void scroll(void) {
    // Move all lines up
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            int from = (y * VGA_WIDTH + x) * 2;
            int to = ((y - 1) * VGA_WIDTH + x) * 2;

            VGA_MEMORY[to] = VGA_MEMORY[from];
            VGA_MEMORY[to + 1] = VGA_MEMORY[from + 1];
        }
    }

    // Clear last line
    for (int x = 0; x < VGA_WIDTH; x++) {
        int index = ((VGA_HEIGHT - 1) * VGA_WIDTH + x) * 2;
        VGA_MEMORY[index] = ' ';
        VGA_MEMORY[index + 1] = color;
    }

    row = VGA_HEIGHT - 1;
}

void terminal_initialize(void) {
    row = 0;
    column = 0;

    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            int index = (y * VGA_WIDTH + x) * 2;
            VGA_MEMORY[index] = ' ';
            VGA_MEMORY[index + 1] = color;
        }
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        column = 0;
        row++;
    } else {
        int index = (row * VGA_WIDTH + column) * 2;
        VGA_MEMORY[index] = c;
        VGA_MEMORY[index + 1] = color;
        column++;
    }

    if (column >= VGA_WIDTH) {
        column = 0;
        row++;
    }

    if (row >= VGA_HEIGHT) {
        scroll();
    }
}

void terminal_write(const char* data) {
    for (int i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_writeline(const char* data) {
    terminal_write(data);
    terminal_putchar('\n');
}