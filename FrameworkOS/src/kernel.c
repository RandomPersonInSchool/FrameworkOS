#include "terminal.h"

void kernel_main(void) {
    terminal_initialize();

    for (int i = 0; i < 40; i++) {
        terminal_writeline("Testing scroll...");
    }
}