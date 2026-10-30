#include <internal/printf.h>
#include <internal/wchar_io.h>
#include <stdarg.h>
#include <stddef.h>

extern FILE __files[];

int __FileWrite(void* file, const char* buffer, size_t count);
int __StringWrite(void* write_ptr, const char* buffer, size_t count);
int __pformatter(int (*write_proc)(void*, const char*, size_t), void* write_ptr,
                 const char* format, va_list arg);

typedef struct __StringWriteState {
    char* string;
    size_t max_length;
    size_t length;
} __StringWriteState;

int vprintf(const char* format, va_list arg) {
    if (fwide(&__files[1], -1) >= 0)
        return -1;

    return __pformatter(__FileWrite, &__files[1], format, arg);
}

int vsnprintf(char* string, size_t max_length, const char* format, va_list arg) {
    __StringWriteState write_state;
    size_t count;

    write_state.string = string;
    write_state.max_length = max_length;
    write_state.length = 0;

    count = __pformatter(__StringWrite, &write_state, format, arg);

    if (string) {
        if (count < max_length) {
            string[count] = 0;
        } else if (max_length) {
            string[max_length - 1] = 0;
        }
    }

    return count;
}

int vsprintf(char* string, const char* format, va_list arg) {
    __StringWriteState write_state;
    size_t count;

    write_state.string = string;
    write_state.max_length = (size_t)-1;
    write_state.length = 0;

    count = __pformatter(__StringWrite, &write_state, format, arg);

    if (string) {
        if (count < (size_t)-1)
            string[count] = 0;
        else
            string[-2] = 0;
    }

    return count;
}

int snprintf(char* string, size_t max_length, const char* format, ...) {
    va_list args;
    __StringWriteState write_state;
    size_t count;

    va_start(args, format);
    write_state.string = string;
    write_state.max_length = max_length;
    write_state.length = 0;
    count = __pformatter(__StringWrite, &write_state, format, args);

    if (string) {
        if (count < max_length) {
            string[count] = 0;
        } else if (max_length) {
            string[max_length - 1] = 0;
        }
    }
    va_end(args);

    return count;
}

int sprintf(char* string, const char* format, ...) {
    va_list args;
    __StringWriteState write_state;
    size_t count;

    va_start(args, format);
    write_state.string = string;
    write_state.max_length = (size_t)-1;
    write_state.length = 0;
    count = __pformatter(__StringWrite, &write_state, format, args);

    if (string) {
        if (count < (size_t)-1) {
            string[count] = 0;
        } else {
            string[-2] = 0;
        }
    }
    va_end(args);

    return count;
}
