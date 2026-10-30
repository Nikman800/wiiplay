#include <string.h>
#include <stddef.h>

wchar_t* wmemcpy(wchar_t* destination, const wchar_t* source, size_t count) {
    return memcpy(destination, source, count * sizeof(wchar_t));
}

wchar_t* wmemchr(const wchar_t* ptr, wchar_t chr, size_t count) {
    const wchar_t* cursor = ptr;

    while (count-- != 0) {
        if (*cursor == chr) {
            return (void*)cursor;
        }

        ++cursor;
    }

    return NULL;
}
