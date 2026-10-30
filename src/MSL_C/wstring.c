#include <internal/wstring.h>

size_t wcslen(const wchar_t* str) {
    const wchar_t* cursor = str - 1;
    size_t length = (size_t)-1;

    do {
        ++length;
    } while (*++cursor != 0);

    return length;
}

wchar_t* wcscpy(wchar_t* destination, const wchar_t* source) {
    register const wchar_t* source_cursor = source - 1;
    register wchar_t* destination_cursor = destination - 1;

    while ((*++destination_cursor = *++source_cursor) != 0) {
    }

    return destination;
}

wchar_t* wcsncpy(wchar_t* destination, const wchar_t* source, size_t count) {
    const wchar_t* source_cursor = source - 1;
    wchar_t* destination_cursor = destination - 1;
    wchar_t character;

    count = count + 1;

    while (--count != 0) {
        character = *++source_cursor;
        *++destination_cursor = character;

        if (character == 0) {
            while (--count != 0) {
                *++destination_cursor = 0;
            }

            break;
        }
    }

    return destination;
}

int wcscmp(const wchar_t* str1, const wchar_t* str2) {
    register const wchar_t* left = str1 - 1;
    register const wchar_t* right = str2 - 1;
    wchar_t c1;
    wchar_t c2;

    while ((c1 = *++left) == (c2 = *++right)) {
        if (c1 == 0) {
            return 0;
        }
    }

    return c1 - c2;
}

wchar_t* wcschr(const wchar_t* str, wchar_t chr) {
    const wchar_t* cursor = str - 1;
    wchar_t character;

    while ((character = *++cursor) != 0) {
        if (character == chr) {
            return (wchar_t*)cursor;
        }
    }

    if (chr == 0) {
        return (wchar_t*)cursor;
    }

    return 0;
}
