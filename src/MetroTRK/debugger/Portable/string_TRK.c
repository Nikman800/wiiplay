#include <types.h>

int TRK_strlen(const unsigned char* str) {
    int length;
    const unsigned char* p;

    length = -1;
    p = str - 1;

    do {
        p++;
        length++;
    } while (*p != '\0');

    return length;
}
