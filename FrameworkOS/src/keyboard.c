#include "keyboard.h"
#include "io.h"

static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int alt_pressed = 0;
static int super_pressed = 0;

static char scancode_to_char(unsigned char scancode) {
    static char normal[128] = {
        0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
        '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
        0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
        0,  '\\','z','x','c','v','b','n','m',',','.','/',
        0,  '*',
        0,  ' '
    };

    static char shifted[128] = {
        0,  27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
        '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
        0,  'A','S','D','F','G','H','J','K','L',':','"','~',
        0,  '|','Z','X','C','V','B','N','M','<','>','?',
        0,  '*',
        0,  ' '
    };

    if (scancode >= 128) return 0;
    return shift_pressed ? shifted[scancode] : normal[scancode];
}

KeyEvent keyboard_read_event(void) {
    unsigned char scancode;

    while (1) {
        while ((inb(0x64) & 1) == 0) {}

        scancode = inb(0x60);

        int released = scancode & 0x80;
        unsigned char key = scancode & 0x7F;

        if (key == 0x2A || key == 0x36) {
            shift_pressed = !released;
            continue;
        }

        if (key == 0x1D) {
            ctrl_pressed = !released;
            continue;
        }

        if (key == 0x38) {
            alt_pressed = !released;
            continue;
        }

        if (key == 0x5B || key == 0x5C) {
            super_pressed = !released;
            continue;
        }

        if (released) {
            continue;
        }

        if (key == 0x1C) {
            if (ctrl_pressed && shift_pressed) {
                return (KeyEvent){ KEY_CTRL_SHIFT_ENTER, 0 };
            }

            return (KeyEvent){ KEY_ENTER, '\n' };
        }

        if (key == 0x0E) {
            return (KeyEvent){ KEY_BACKSPACE, '\b' };
        }

        char c = scancode_to_char(key);

        if (c != 0) {
            return (KeyEvent){ KEY_CHAR, c };
        }

        return (KeyEvent){ KEY_NONE, 0 };
    }
}