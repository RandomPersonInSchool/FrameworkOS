#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEY_NONE 0
#define KEY_CHAR 1
#define KEY_ENTER 2
#define KEY_BACKSPACE 3
#define KEY_CTRL_SHIFT_ENTER 4

typedef struct {
    int type;
    char character;
} KeyEvent;

KeyEvent keyboard_read_event(void);

#endif