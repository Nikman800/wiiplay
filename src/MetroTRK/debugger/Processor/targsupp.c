#include <MetroTRK.h>

#pragma push
#pragma function_align 16
asm void TRKAccessFile(void) {
    // clang-format off
    nofralloc

    twi 31, r0, 0
    blr
    // clang-format on
}
#pragma pop

asm void TRKOpenFile(void) {
    // clang-format off
    nofralloc

    twi 31, r0, 0
    blr
    // clang-format on
}

asm void TRKCloseFile(void) {
    // clang-format off
    nofralloc

    twi 31, r0, 0
    blr
    // clang-format on
}

asm void TRKPositionFile(void) {
    // clang-format off
    nofralloc

    twi 31, r0, 0
    blr
    // clang-format on
}
