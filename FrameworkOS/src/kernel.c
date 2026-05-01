#include "terminal.h"
#include "keyboard.h"

#define INPUT_MAX 128

static int string_equals(const char* a, const char* b) {
    int i = 0;

    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) {
            return 0;
        }

        i++;
    }

    return a[i] == '\0' && b[i] == '\0';
}

static void prompt(void) {
    terminal_write("Kernel> ");
}

static void run_command(const char* command) {
    if (string_equals(command, "help")) {
        terminal_writeline("Commands: help, clear, color, echo");
    } else if (string_equals(command, "clear")) {
        terminal_initialize();
    } else if (string_equals(command, "color")) {
        terminal_writeline("Color system online.");
        terminal_write_color_test();
        terminal_set_color(0x0F);
    } else if (string_equals(command, "echo")) {
        terminal_writeline("echo");
    } else if (command[0] == '\0') {
        // Empty command does nothing.
    } else {
        terminal_writeline("Unknown command.");
    }
}

static void wait_for_debug_gate(void) {
    terminal_initialize();
    terminal_draw_color_strip();

    terminal_writeline("FrameworkOS debug boot state.");
    terminal_writeline("Minimal systems initialized:");
    terminal_writeline("- VGA terminal");
    terminal_writeline("- Keyboard polling");
    terminal_writeline("- Color output");
    terminal_putchar('\n');
    terminal_writeline("Press Ctrl + Shift + Enter to enter kernel shell.");

    while (1) {
        KeyEvent event = keyboard_read_event();

        if (event.type == KEY_CTRL_SHIFT_ENTER) {
            terminal_initialize();
            terminal_writeline("Entering kernel shell...");
            terminal_putchar('\n');
            return;
        }
    }
}

void kernel_main(void) {
    char input[INPUT_MAX];
    int input_length = 0;

    wait_for_debug_gate();
    prompt();

    while (1) {
        KeyEvent event = keyboard_read_event();

        if (event.type == KEY_CHAR) {
            if (input_length < INPUT_MAX - 1) {
                input[input_length] = event.character;
                input_length++;
                terminal_putchar(event.character);
            }
        }

        if (event.type == KEY_BACKSPACE) {
            if (input_length > 0) {
                input_length--;
                terminal_backspace();
            }
        }

        if (event.type == KEY_ENTER) {
            input[input_length] = '\0';

            terminal_putchar('\n');
            run_command(input);

            input_length = 0;
            prompt();
        }
    }
}