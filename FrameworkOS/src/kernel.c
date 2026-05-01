#include "terminal.h"

void kernel_main(void) {
    terminal_initialize();

    terminal_writeline("FrameworkOS booted.");
    terminal_writeline("VGA terminal driver online.");
    terminal_writeline("Next: scrolling, colors, and keyboard input.");
}