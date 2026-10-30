#include <MetroTRK/debugger/Os/dolphin/dolphin_trk_glue.h>

typedef int (*DBCommRead)(void* dst, size_t size);
typedef int (*DBCommWrite)(const void* src, size_t size);
typedef int (*DBCommInitialize)(void* inputPendingPtr, void* callback);

typedef struct DBCommTable {
    DBCommInitialize initialize;
    void (*enableInterrupts)(void);
    void (*shutdown)(void);
    int (*peek)(void);
    DBCommRead read;
    DBCommWrite write;
    void (*close)(void);
    void (*open)(void);
    int (*preContinue)(void);
    int (*postStop)(void);
} DBCommTable;

DBCommTable gDBCommTable = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};
u8 TRK_Use_BBA;

void EnableEXI2Interrupts(void) {
    if (TRK_Use_BBA == FALSE && gDBCommTable.enableInterrupts != NULL) {
        gDBCommTable.enableInterrupts();
    }
}

void TRKPollUART(void) {
    gDBCommTable.peek();
}

UARTError TRKReadUARTN(void* dst, size_t size) {
    s32 result = gDBCommTable.read(dst, size);
    return (UARTError)((-result | result) >> 31);
}

UARTError TRKWriteUARTN(const void* src, size_t size) {
    s32 result = gDBCommTable.write(src, size);
    return (UARTError)((-result | result) >> 31);
}
