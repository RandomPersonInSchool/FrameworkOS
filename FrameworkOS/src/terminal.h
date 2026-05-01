#ifndef TERMINAL_H
#define TERMINAL_H

void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* data);
void terminal_writeline(const char* data);
void terminal_draw_color_strip(void);
void terminal_set_color(char new_color);
void terminal_write_color_test(void);
void terminal_backspace(void);

#endif