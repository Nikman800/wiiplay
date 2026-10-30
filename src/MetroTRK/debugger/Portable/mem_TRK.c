#include <MetroTRK.h>

DECL_SECTION(".init")
void* TRK_memcpy(void* dest, const void* src, size_t count) {
    const char* csrc = (const char*)src;
    char* cdest = (char*)dest;

    csrc--;
    cdest--;
    count++;

    while (--count) {
        *++cdest = *++csrc;
    }

    return dest;
}

DECL_SECTION(".init") void* TRK_memset(void* dest, int val, size_t count) {
    TRK_fill_mem(dest, val, count);
    return dest;
}

void TRK_fill_mem(void* dest, int value, size_t length) {
    u32 val = (u8)value;
    u32 i;
    ((u32*)dest) = (u32*)dest;
    ((u8*)dest) = (u8*)dest;

    ((u8*)dest)--;

    if (length >= 32) {
        i = ~(u32)dest & 3;

        if (i) {
            length -= i;
            do {
                *++((u8*)dest) = val;
            } while (--i);
        }

        if (val) {
            val |= val << 24 | val << 16 | val << 8;
        }

        ((u32*)dest) = (u32*)(((u8*)dest) + 1) - 1;

        i = length >> 5;
        if (i) {
            do {
                *++((u32*)dest) = val;
                *++((u32*)dest) = val;
                *++((u32*)dest) = val;
                *++((u32*)dest) = val;
                *++((u32*)dest) = val;
                *++((u32*)dest) = val;
                *++((u32*)dest) = val;
                *++((u32*)dest) = val;
            } while (--i);
        }

        i = (length & 31) >> 2;

        if (i) {
            do {
                *++((u32*)dest) = val;
            } while (--i);
        }

        ((u8*)dest) = (u8*)(((u32*)dest) + 1) - 1;

        length &= 3;
    }

    if (length) {
        do {
            *++((u8*)dest) = val;
        } while (--length);
    }
}
