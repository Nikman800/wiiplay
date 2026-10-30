#include <MSL/internal/mem.h>


void* memmove(void* destination, const void* source, size_t count) {
    u32 difference = (u32)destination ^ (u32)source;
    u32 direction = ((u32)destination << __cntlzw(difference)) >> 31;

    if (count >= 0x20) {
        if ((difference & 3) != 0) {
            if (direction == 0)
                __copy_longs_unaligned(destination, source, count);
            else
                __copy_longs_rev_unaligned(destination, source, count);
        } else {
            if (direction == 0)
                __copy_longs_aligned(destination, source, count);
            else
                __copy_longs_rev_aligned(destination, source, count);
        }

        return destination;
    }

    if (direction == 0) {
        const unsigned char* second = (const unsigned char*)source - 1;
        unsigned char* first = (unsigned char*)destination - 1;

        count = count + 1;
        while (--count != 0)
            *++first = *++second;
    } else {
        const unsigned char* second = (const unsigned char*)source + count;
        unsigned char* first = (unsigned char*)destination + count;

        count = count + 1;
        while (--count != 0)
            *--first = *--second;
    }

    return destination;
}

void* memchr(const void* source, int character, size_t count) {
    const unsigned char target = (unsigned char)character;
    const unsigned char* position = (const unsigned char*)source - 1;

    count = count + 1;
    while (--count != 0) {
        if (*++position == target)
            return (void*)position;
    }

    return NULL;
}

void* __memrchr(const void* source, int character, size_t count) {
    const unsigned char* position = (const unsigned char*)source + count;
    const unsigned char target = (unsigned char)character;

    count = count + 1;
    while (--count != 0) {
        if (*--position == target)
            return (void*)position;
    }

    return NULL;
}

int memcmp(const void* left, const void* right, size_t count) {
    const unsigned char* first = (const unsigned char*)left - 1;
    const unsigned char* second = (const unsigned char*)right - 1;

    count = count + 1;
    while (--count != 0) {
        if (*++first != *++second) {
            if (*first < *second)
                return -1;

            return 1;
        }
    }

    return 0;
}
