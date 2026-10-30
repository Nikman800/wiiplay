#include <internal/file_io.h>
#include <revolution/EUART.h>
#include <revolution/OS.h>

#pragma cplusplus on

void __TRK_write_console(__file_handle file, unsigned char* buffer,
                         unsigned long* count, __idle_proc idle_proc);

extern "C" {
int __write_console(__file_handle file, unsigned char* buffer, unsigned long* count,
                    __idle_proc idle_proc);
int __close_console(__file_handle file);

}

static int __uart_initialized = 0;

int __write_console(__file_handle file, unsigned char* buffer, unsigned long* count,
                    __idle_proc idle_proc) {
    int error;

    if ((OSGetConsoleType() & OS_CONSOLE_MASK_TDEV) == 0) {
        error = 0;
        if (__uart_initialized == 0) {
            error = ((EUARTError (*)(unsigned long))InitializeUART)(0xe100);
            if (error == 0) {
                __uart_initialized = 1;
            }
        }

        if (error != 0) {
            return 1;
        }

        error = WriteUARTN((const char*)buffer, *count);
        if (error != 0) {
            *count = 0;
            return 1;
        }
    }

    __TRK_write_console(file, buffer, count, idle_proc);
    return 0;
}

int __close_console(__file_handle file) {
    return 0;
}
