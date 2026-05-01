#include "terminal.h"
#include "keyboard.h"

#define INPUT_MAX 128

#define MAX_FILES 16
#define FILE_NAME_MAX 32
#define FILE_DATA_MAX 256

typedef struct {
    int used;
    char name[FILE_NAME_MAX];
    char data[FILE_DATA_MAX];
    int persistent;
} File;

static File files[MAX_FILES];

static int auth_initialized = 0;
static int fs_initialized = 0;
static int userspace_initialized = 0;
static void handle_fs_command(const char* command);

static int string_equals(const char* a, const char* b) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return a[i] == b[i];
}

static int string_starts_with(const char* text, const char* prefix) {
    int i = 0;
    while (prefix[i]) {
        if (text[i] != prefix[i]) return 0;
        i++;
    }
    return 1;
}

static void string_copy(char* dest, const char* src, int max) {
    int i = 0;

    while (src[i] != '\0' && i < max - 1) {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';
}

static int string_length(const char* text) {
    int i = 0;
    while (text[i] != '\0') i++;
    return i;
}

static int fs_find_file(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && string_equals(files[i].name, name)) {
            return i;
        }
    }

    return -1;
}

static int fs_find_free_slot(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) {
            return i;
        }
    }

    return -1;
}

static void fs_write_file(const char* name, const char* data, int persistent) {
    int index = fs_find_file(name);

    if (index == -1) {
        index = fs_find_free_slot();

        if (index == -1) {
            terminal_writeline("FS error: no free file slots.");
            return;
        }

        files[index].used = 1;
        string_copy(files[index].name, name, FILE_NAME_MAX);
    }

    string_copy(files[index].data, data, FILE_DATA_MAX);
    files[index].persistent = persistent;

    terminal_writeline("File written.");
}

static void fs_read_file(const char* name) {
    int index = fs_find_file(name);

    if (index == -1) {
        terminal_writeline("File not found.");
        return;
    }

    terminal_writeline(files[index].data);
}

static void fs_list_files(void) {
    int found = 0;

    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used) {
            terminal_write(files[i].name);
            terminal_write(files[i].persistent ? " [persistent-ram]" : " [temporary]");
            terminal_putchar('\n');
            found = 1;
        }
    }

    if (!found) {
        terminal_writeline("No files.");
    }
}

static void prompt(void) {
    terminal_write("Kernel> ");
}

static void handle_init(const char* command) {
    if (string_equals(command, "init Auth")) {
        auth_initialized = 1;
        terminal_writeline("Authentication system initialized.");
    } else if (string_equals(command, "init FS")) {
        if (!auth_initialized) {
            terminal_writeline("Permission denied.");
            terminal_writeline("File System is locked behind user Authentication.");
            terminal_writeline("Run \"init Auth\" to proceed.");
            return;
        }

        fs_initialized = 1;
        terminal_writeline("File System initialized.");
    } else if (string_equals(command, "init UserSpace")) {
        userspace_initialized = 1;
        auth_initialized = 1;
        fs_initialized = 1;
        terminal_writeline("UserSpace initialized.");
        terminal_writeline("[Stub] Auth, FS, and desktop environment staged.");
    } else {
        terminal_writeline("Init target not implemented.");
    }
}

static void handle_debug(const char* command) {
    if (string_equals(command, "debug color")) {
        terminal_write_color_test();
        terminal_set_color(0x0F);
    } else if (string_equals(command, "debug keyboard")) {
        terminal_writeline("Keyboard debug mode. Press Ctrl+C to exit.");

        while (1) {
            KeyEvent event = keyboard_read_event();

            if (event.type == KEY_CTRL_C) {
                terminal_writeline("");
                terminal_writeline("Exiting keyboard debug.");
                return;
            }

            if (event.type == KEY_CHAR) {
                terminal_putchar(event.character);
            }

            if (event.type == KEY_ENTER) {
                terminal_putchar('\n');
            }

            if (event.type == KEY_BACKSPACE) {
                terminal_backspace();
            }
        }
    } else if (string_equals(command, "debug video")) {
        terminal_writeline("[Not implemented] 2D/3D split renderer.");
    } else if (string_equals(command, "debug video shading")) {
        terminal_writeline("[Not implemented] 3D shaded renderer.");
    } else {
        terminal_writeline("Debug target not implemented.");
    }
}

static void run_command(const char* command) {
    if (string_equals(command, "help")) {
        terminal_writeline("Commands:");
        terminal_writeline("init Auth");
        terminal_writeline("init FS");
        terminal_writeline("init UserSpace");
        terminal_writeline("debug color");
        terminal_writeline("debug keyboard");
        terminal_writeline("debug video");
        terminal_writeline("debug video shading");
        terminal_writeline("clear");
        terminal_writeline("ls");
        terminal_writeline("read <file>");
        terminal_writeline("write <file> <text>");
    } else if (string_equals(command, "clear")) {
        terminal_initialize();
    } else if (string_starts_with(command, "init ")) {
        handle_init(command);
    } else if (string_starts_with(command, "debug ")) {
        handle_debug(command);
    } else if (command[0] == '\0') {
        return;
    } else if (
        string_equals(command, "ls") ||
        string_starts_with(command, "read ") ||
        string_starts_with(command, "write ")
    ) {
        handle_fs_command(command);
    } else {
        terminal_writeline("Unknown command. Type help.");
    }
}

static int find_space(const char* text, int start) {
    int i = start;

    while (text[i] != '\0') {
        if (text[i] == ' ') {
            return i;
        }

        i++;
    }

    return -1;
}

static void copy_range(char* dest, const char* src, int start, int end, int max) {
    int j = 0;

    for (int i = start; i < end && j < max - 1; i++) {
        dest[j] = src[i];
        j++;
    }

    dest[j] = '\0';
}

static void handle_fs_command(const char* command) {
    if (!fs_initialized) {
        terminal_writeline("File System is not initialized. Run \"init FS\" first.");
        return;
    }

    if (!auth_initialized) {
        terminal_writeline("Permission denied.");
        terminal_writeline("File System is locked behind user Authentication.");
        terminal_writeline("Run \"init Auth\" to proceed through login.");
        return;
    }

    if (string_equals(command, "ls")) {
        fs_list_files();
        return;
    }

    if (string_starts_with(command, "read ")) {
        fs_read_file(command + 5);
        return;
    }

    if (string_starts_with(command, "write ")) {
        char filename[FILE_NAME_MAX];

        int name_start = 6;
        int name_end = find_space(command, name_start);

        if (name_end == -1) {
            terminal_writeline("Usage: write <file> <text>");
            return;
        }

        copy_range(filename, command, name_start, name_end, FILE_NAME_MAX);

        const char* data = command + name_end + 1;
        fs_write_file(filename, data, 1);
        return;
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
