#include <MetroTRK.h>

u32 TRK_mainError_803A26A0;
static u8 gap_09_803A26A4_bss[4];
void* lc_base[2];
extern u8 gTRKCPUState[];

void* TRK_memcpy(void* dst, const void* src, size_t size);
void TRK_flush_cache(void* addr, size_t size);
extern u8 gTRKInterruptVectorTable[];
void EnableEXI2Interrupts(void);

static u32 TRK_ISR_OFFSETS[16] = {
    0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
    0x0800, 0x0900, 0x0C00, 0x0D00, 0x0F00, 0x1300, 0x1400,
    0x1700, 0,
};

void EnableMetroTRKInterrupts(void) {
    EnableEXI2Interrupts();
}

void* TRKTargetTranslate(void* address) {
    if (address >= (void*)lc_base[0] && (u32)address < (u32)lc_base[0] + 0x4000 &&
        (*(u32*)(gTRKCPUState + 0x238) & 3) != 0) {
        return address;
    }

    if ((u32)address >= 0x7E000000 && (u32)address <= 0x80000000) {
        return address;
    }

    return (void*)(((u32)address & 0x3FFFFFFF) | 0x80000000);
}

void* TRKTargetTranslate(void* address);

void __TRK_copy_vectors(void) {
    s32 bit;
    u32 exceptions;
    u32* isrOffsets;
    u8* cpuState;
    u32* exceptionAddress;

    exceptionAddress = (u32*)TRKTargetTranslate((void*)0x44);
    exceptions = *exceptionAddress;
    isrOffsets = TRK_ISR_OFFSETS;
    cpuState = gTRKCPUState;
    for (bit = 0; bit <= 14; bit++) {
        if ((exceptions & (1 << bit)) != 0 && bit != 4) {
            u32 offset = isrOffsets[bit];
            u32 destination;

            destination = (u32)TRKTargetTranslate((void*)offset);
            TRK_memcpy((void*)destination, gTRKInterruptVectorTable + offset, 0x100);
            TRK_flush_cache((void*)destination, 0x100);
        }
    }
}
