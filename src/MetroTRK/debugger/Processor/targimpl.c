#include <MetroTRK.h>

typedef enum {
    kInvalidRegister = 0x0701,
    kCWDSException = 0x0702,
    kUnsupportedError = 0x0703,
} TargetError;

typedef enum {
    DSMSG_NotifyStopped = 0x90,
    DSMSG_NotifyException = 0x91,
    DSMSG_WriteFile = 0xD0,
    DSMSG_ReadFile,
    DSMSG_OpenFile,
    DSMSG_CloseFile,
    DSMSG_PositionFile,
} DSMessageID;

typedef enum {
    DSSTEP_IntoCount = 0x00,
    DSSTEP_IntoRange = 0x01,
    DSSTEP_OverCount = 0x10,
    DSSTEP_OverRange = 0x11,
} DSMessageStepOptions;

typedef enum {
    VALIDMEM_Readable = 0,
    VALIDMEM_Writeable = 1,
} ValidMemoryOptions;

typedef enum {
    MEMACCESS_UserMemory = 0,
} MemoryAccessOptions;

typedef enum {
    DS_IONoError = 0,
    DS_IOError = 1,
} DSIOResult;

#define PPC_Trace 0xD00

typedef struct memRange {
    u8* start;      // at 0x0
    u8* end;        // at 0x4
    BOOL readable;  // at 0x8
    BOOL writeable; // at 0xC
} memRange;

typedef struct StopInfo_PPC {
    u32 PC;            // at 0x0
    u32 PCInstruction; // at 0x4
    u16 exceptionID;   // at 0x8
} StopInfo_PPC;

typedef struct TRKExceptionStatus {
    StopInfo_PPC exceptionInfo; // at 0x0
    u8 inTRK;                   // at 0xC
    u8 exceptionDetected;       // at 0xD
} TRKExceptionStatus;

typedef struct TRKStepStatus {
    BOOL active;               // at 0x0
    DSMessageStepOptions type; // at 0x4
    u32 count;                 // at 0x8
    u32 rangeStart;            // at 0xC
    u32 rangeEnd;              // at 0x10
} TRKStepStatus;

typedef struct ProcessorRestoreFlags_PPC {
    u8 TBR;          // at 0x0
    u8 DEC;          // at 0x1
    u8 linker_padding[7];
} ProcessorRestoreFlags_PPC;

typedef struct Default_PPC {
    u32 GPR[32]; // at 0x0
    u32 PC;      // at 0x80
    u32 LR;      // at 0x84
    u32 CR;      // at 0x88
    u32 CTR;     // at 0x8C
    u32 XER;     // at 0x90
} Default_PPC;

typedef struct Float_PPC {
    u64 FPR[32];
    u64 FPSCR;
    u64 FPECR;
} Float_PPC;

typedef struct Extended1_PPC_6xx_7xx {
    u32 SR[16];
    u32 TBL;
    u32 TBU;
    u32 HID0;
    u32 HID1;
    u32 MSR;
    u32 PVR;
    u32 IBAT0U;
    u32 IBAT0L;
    u32 IBAT1U;
    u32 IBAT1L;
    u32 IBAT2U;
    u32 IBAT2L;
    u32 IBAT3U;
    u32 IBAT3L;
    u32 DBAT0U;
    u32 DBAT0L;
    u32 DBAT1U;
    u32 DBAT1L;
    u32 DBAT2U;
    u32 DBAT2L;
    u32 DBAT3U;
    u32 DBAT3L;
    u32 DMISS;
    u32 DCMP;
    u32 HASH1;
    u32 HASH2;
    u32 IMISS;
    u32 ICMP;
    u32 RPA;
    u32 SDR1;
    u32 DAR;
    u32 DSISR;
    u32 SPRG0;
    u32 SPRG1;
    u32 SPRG2;
    u32 SPRG3;
    u32 DEC;
    u32 IABR;
    u32 EAR;
    u32 DABR;
    u32 PMC1;
    u32 PMC2;
    u32 PMC3;
    u32 PMC4;
    u32 SIA;
    u32 MMCR0;
    u32 MMCR1;
    u32 THRM1;
    u32 THRM2;
    u32 THRM3;
    u32 ICTC;
    u32 L2CR;
    u32 UMMCR2;
    u32 UBAMR;
    u32 UMMCR0;
    u32 UPMC1;
    u32 UPMC2;
    u32 USIA;
    u32 UMMCR1;
    u32 UPMC3;
    u32 UPMC4;
    u32 USDA;
    u32 MMCR2;
    u32 BAMR;
    u32 SDA;
    u32 MSSCR0;
    u32 MSSCR1;
    u32 PIR;
    u32 exceptionID; // at 0x150
    u32 GQR[8];
    u32 HID_G;
    u32 WPAR;
    u32 DMA_U;
    u32 DMA_L;
} Extended1_PPC_6xx_7xx;

typedef struct Extended2_PPC_6xx_7xx {
    u32 PSR[32][2];
} Extended2_PPC_6xx_7xx;

typedef struct ProcessorState_PPC {
    Default_PPC Default;             // at 0x0
    Float_PPC Float;                 // at 0x98
    Extended1_PPC_6xx_7xx Extended1; // at 0x1A8
    Extended2_PPC_6xx_7xx Extended2; // at 0x32C
    u32 transport_handler_saved_ra;  // at 0x42C
} ProcessorState_PPC;

typedef struct TRKState_PPC {
    u32 GPR[32];          // at 0x0
    u32 LR;               // at 0x80
    u32 CTR;              // at 0x84
    u32 XER;              // at 0x88
    u32 MSR;              // at 0x8C
    u32 DAR;              // at 0x90
    u32 DSISR;            // at 0x94
    BOOL isStopped;       // at 0x98
    BOOL inputActivated;  // at 0x9C
    u8* inputPendingPtr;  // at 0xA0
} TRKState_PPC;

typedef struct TRKStopInfoReply {
    u32 length;      // at 0x0
    u8 command;      // at 0x4
    u8 pad0[3];
    u32 PC;          // at 0x8
    u32 instruction; // at 0xC
    u32 exceptionID; // at 0x10
    u8 pad1[0x2C];
} TRKStopInfoReply;

typedef unsigned char u128[16];

#define SPR_FPECR 1022
#define SPR_HID2 920
#define SPR_GQR0 912

#define INSTR_NOP 0x60000000
#define INSTR_BLR 0x4E800020
#define INSTR_PSQ_ST(psr, offset, rDest, w, gqr)                               \
    (0xF0000000 | (psr << 21) | (rDest << 16) | (w << 15) | (gqr << 12) |      \
     offset)
#define INSTR_PSQ_L(psr, offset, rSrc, w, gqr)                                 \
    (0xE0000000 | (psr << 21) | (rSrc << 16) | (w << 15) | (gqr << 12) |       \
     offset)
#define INSTR_STW(rSrc, offset, rDest)                                         \
    (0x90000000 | (rSrc << 21) | (rDest << 16) | offset)
#define INSTR_LWZ(rDest, offset, rSrc)                                         \
    (0x80000000 | (rDest << 21) | (rSrc << 16) | offset)
#define INSTR_STFD(fprSrc, offset, rDest)                                      \
    (0xD8000000 | (fprSrc << 21) | (rDest << 16) | offset)
#define INSTR_LFD(fprDest, offset, rSrc)                                       \
    (0xC8000000 | (fprDest << 21) | (rSrc << 16) | offset)
#define INSTR_MFSPR(rDest, spr)                                                \
    (0x7C000000 | (rDest << 21) | ((spr & 0xFE0) << 6) |                       \
     ((spr & 0x1F) << 16) | 0x2A6)
#define INSTR_MTSPR(spr, rSrc)                                                 \
    (0x7C000000 | (rSrc << 21) | ((spr & 0xFE0) << 6) |                        \
     ((spr & 0x1F) << 16) | 0x3A6)

extern void TRKUARTInterruptHandler(void);
extern void TRKSaveExtended1Block(void);
extern void TRKRestoreExtended1Block(void);
extern void TRK_flush_cache(u32 addr, size_t size);
extern void* TRKTargetTranslate(void* address);
extern DSError TRKDoNotifyStopped(DSMessageID event);
extern DSError TRKSuppAccessFile(u32 fileHandle, u8* data, u32* count,
                                 int* result, BOOL sendReply, BOOL read);
extern DSError HandleOpenFileSupportRequest(const char* path, u8 openMode,
                                            u32* fileHandle, int* result);
extern DSError HandleCloseFileSupportRequest(u32 fileHandle, int* result);
extern DSError HandlePositionFileSupportRequest(u32 fileHandle, u32* position,
                                                u8 mode, int* result);

const memRange gTRKMemMap[1] = {{(u8*)0, (u8*)-1, TRUE, TRUE}};

ProcessorRestoreFlags_PPC gTRKRestoreFlags = {FALSE, FALSE};
static TRKExceptionStatus gTRKExceptionStatus = {{0, 0, 0}, TRUE, 0};
TRKStepStatus gTRKStepStatus = {FALSE, DSSTEP_IntoCount, 0, 0};

static u16 TRK_saved_exceptionID = 0;
TRKState_PPC gTRKState;
ProcessorState_PPC gTRKCPUState;
Default_PPC gTRKSaveState;
u128 TRKvalue128_temp;

static inline DSError TRKPPCAccessSPR(void* value, u32 spr_register_num, BOOL read);
static inline DSError TRKPPCAccessPairedSingleRegister(void* srcDestPtr, u32 psr, BOOL read);
static inline DSError TRKPPCAccessFPRegister(void* srcDestPtr, u32 fpr, BOOL read);
static inline DSError TRKPPCAccessSpecialReg(void* value, u32* access_func, BOOL read);
void TRKExceptionHandler(u16 exceptionID);
void TRKInterruptHandlerEnableInterrupts(void);
void WriteFPSCR(register f64* p);
void ReadFPSCR(register f64* p);
void __TRK_set_MSR(register u32 msr);
u32 __TRK_get_MSR(void);
void TRK_ppc_memcpy(register void* dest, register const void* src,
                    register int n, register u32 param_4,
                    register u32 param_5);
void TRKTargetSetStopped(unsigned int stopped);
void TRKPostInterruptEvent(void);

DSError TRKValidMemory32(const void* addr, size_t length,
                         ValidMemoryOptions readWriteable) {
    DSError err = kInvalidMemory;
    const u8* start;
    const u8* end;
    s32 i;

    start = (const u8*)addr;
    end = ((const u8*)addr + (length - 1));

    if (end < start) {
        return kInvalidMemory;
    }

    for (i = 0; i < (s32)(sizeof(gTRKMemMap) / sizeof(memRange)); i++) {
        if ((start <= (const u8*)gTRKMemMap[i].end) &&
            (end >= (const u8*)gTRKMemMap[i].start)) {
            if (((readWriteable == VALIDMEM_Readable) &&
                 !gTRKMemMap[i].readable) ||
                ((readWriteable == VALIDMEM_Writeable) &&
                 !gTRKMemMap[i].writeable)) {
                err = kInvalidMemory;
            } else {
                err = kNoError;

                if (start < (const u8*)gTRKMemMap[i].start) {
                    err = TRKValidMemory32(
                        start, (u32)((const u8*)gTRKMemMap[i].start - start),
                        readWriteable);
                }

                if ((err == kNoError) &&
                    (end > (const u8*)gTRKMemMap[i].end)) {
                    err = TRKValidMemory32(
                        (const u8*)gTRKMemMap[i].end,
                        (u32)(end - (const u8*)gTRKMemMap[i].end),
                        readWriteable);
                }
            }

            break;
        }
    }

    return err;
}

DSError TRKTargetAccessMemory(void* data, u32 start, size_t* length,
                              MemoryAccessOptions accessOptions, BOOL read) {
    DSError error;
    u32 uVar5;
    void* addr;
    u32 param4;
    TRKExceptionStatus tempExceptionStatus = gTRKExceptionStatus;
    gTRKExceptionStatus.exceptionDetected = FALSE;

    addr = TRKTargetTranslate((void*)start);
    error = TRKValidMemory32(addr, *length, read == FALSE);

    if (error != kNoError) {
        *length = 0;
    } else {
        uVar5 = __TRK_get_MSR();
        param4 = uVar5 | gTRKCPUState.Extended1.MSR & 0x10;

        if (read) {
            TRK_ppc_memcpy(data, addr, *length, uVar5, param4);
        } else {
            TRK_ppc_memcpy(addr, data, *length, param4, uVar5);
            TRK_flush_cache((u32)addr, *length);
            if ((void*)start != addr) {
                TRK_flush_cache(start, *length);
            }
        }
    }

    if (gTRKExceptionStatus.exceptionDetected) {
        *length = 0;
        error = kCWDSException;
    }

    gTRKExceptionStatus = tempExceptionStatus;
    return error;
}

DSError TRKTargetReadInstruction(void* data, u32 start) {
    DSError error;
    size_t registersLength = 4;

    error = TRKTargetAccessMemory(data, start, &registersLength,
                                  MEMACCESS_UserMemory, TRUE);

    if (error == kNoError && registersLength != 4) {
        error = kInvalidMemory;
    }

    return error;
}

DSError TRKTargetAccessDefault(u32 firstRegister, u32 lastRegister,
                               TRKMessageBuffer* b, size_t* registersLengthPtr,
                               BOOL read) {
    DSError error;
    u32 count;
    u32* data;
    TRKExceptionStatus tempExceptionStatus;

    if (lastRegister > 0x24) {
        return kInvalidRegister;
    }

    tempExceptionStatus = gTRKExceptionStatus;
    gTRKExceptionStatus.exceptionDetected = FALSE;

    data = gTRKCPUState.Default.GPR + firstRegister;
    count = (lastRegister - firstRegister) + 1;
    *registersLengthPtr = count * sizeof(u32);

    if (read) {
        error = TRKAppendBuffer_ui32(b, (unsigned int*)data, count);
    } else {
        error = TRKReadBuffer_ui32(b, (unsigned int*)data, count);
    }

    if (gTRKExceptionStatus.exceptionDetected) {
        *registersLengthPtr = 0;
        error = kCWDSException;
    }

    gTRKExceptionStatus = tempExceptionStatus;
    return error;
}

DSError TRKTargetAccessFP(u32 firstRegister, u32 lastRegister,
                          TRKMessageBuffer* b, size_t* registersLengthPtr,
                          BOOL read) {
    u64 temp;
    DSError error;
    TRKExceptionStatus tempExceptionStatus;
    u32 current;

    if (lastRegister > 0x21) {
        return kInvalidRegister;
    }

    tempExceptionStatus = gTRKExceptionStatus;
    gTRKExceptionStatus.exceptionDetected = FALSE;

    __TRK_set_MSR(__TRK_get_MSR() | 0x2000);

    *registersLengthPtr = 0;
    error = kNoError;

    for (current = firstRegister;
         (current <= lastRegister) && (error == kNoError);
         current++, *registersLengthPtr += sizeof(f64)) {
        if (read) {
            TRKPPCAccessFPRegister(&temp, current, read);
            error = TRKAppendBuffer1_ui64(b, temp);
        } else {
            TRKReadBuffer1_ui64(b, &temp);
            error = TRKPPCAccessFPRegister(&temp, current, read);
        }
    }

    if (gTRKExceptionStatus.exceptionDetected) {
        *registersLengthPtr = 0;
        error = kCWDSException;
    }

    gTRKExceptionStatus = tempExceptionStatus;
    return error;
}

DSError TRKTargetAccessExtended1(u32 firstRegister, u32 lastRegister,
                                 TRKMessageBuffer* b,
                                 size_t* registersLengthPtr, BOOL read) {
    TRKExceptionStatus tempExceptionStatus;
    int error;
    u32* data;
    int count;

    if (lastRegister > 0x60) {
        return kInvalidRegister;
    }

    tempExceptionStatus = gTRKExceptionStatus;
    gTRKExceptionStatus.exceptionDetected = FALSE;

    *registersLengthPtr = 0;

    if (firstRegister <= lastRegister) {
        data = (u32*)&gTRKCPUState.Extended1 + firstRegister;
        count = lastRegister - firstRegister + 1;
        *registersLengthPtr += count * sizeof(u32);

        if (read) {
            error = TRKAppendBuffer_ui32(b, (unsigned int*)data, count);
        } else {
            if (data <= &gTRKCPUState.Extended1.TBU &&
                (data + count - 1) >= &gTRKCPUState.Extended1.TBL) {
                gTRKRestoreFlags.TBR = 1;
            }

            if (data <= &gTRKCPUState.Extended1.DEC &&
                (data + count - 1) >= &gTRKCPUState.Extended1.DEC) {
                gTRKRestoreFlags.DEC = 1;
            }
            error = TRKReadBuffer_ui32(b, (unsigned int*)data, count);
        }
    }
    if (gTRKExceptionStatus.exceptionDetected) {
        *registersLengthPtr = 0;
        error = kCWDSException;
    }

    gTRKExceptionStatus = tempExceptionStatus;
    return error;
}

DSError TRKTargetAccessExtended2(u32 firstRegister, u32 lastRegister,
                                 TRKMessageBuffer* b,
                                 size_t* registerStorageSize, BOOL read) {
    TRKExceptionStatus savedException;
    u32 i;
    u32 value_buf0[1];
    u32 value_buf[2];
    DSError err;

    if (lastRegister > 0x1f) {
        return kInvalidRegister;
    }

    savedException = gTRKExceptionStatus;
    gTRKExceptionStatus.exceptionDetected = FALSE;

    TRKPPCAccessSPR(value_buf0, SPR_HID2, TRUE);

    value_buf0[0] |= 0xA0000000;
    TRKPPCAccessSPR(value_buf0, SPR_HID2, FALSE);

    value_buf0[0] = 0;
    TRKPPCAccessSPR(value_buf0, SPR_GQR0, FALSE);

    *registerStorageSize = 0;
    err = kNoError;

    for (i = firstRegister; (i <= lastRegister) && (err == kNoError); i++) {
        if (read) {
            err = TRKPPCAccessPairedSingleRegister((u64*)value_buf, i, read);
            err = TRKAppendBuffer1_ui64(b, *(u64*)value_buf);
        } else {
            err = TRKReadBuffer1_ui64(b, (u64*)value_buf);
            err = TRKPPCAccessPairedSingleRegister((u64*)value_buf, i, read);
        }

        *registerStorageSize += sizeof(u64);
    }

    if (gTRKExceptionStatus.exceptionDetected) {
        *registerStorageSize = 0;
        err = kCWDSException;
    }

    gTRKExceptionStatus = savedException;

    return err;
}

void TRKPostInterruptEvent(void) {
    TRKEventType eventType;
    u32 inst;
    TRKEvent event;

    if (gTRKState.inputActivated) {
        gTRKState.inputActivated = FALSE;
    } else {
        switch (gTRKCPUState.Extended1.exceptionID & 0xFFFF) {
        case 0xd00:
        case 0x700:
            TRKTargetReadInstruction(&inst, gTRKCPUState.Default.PC);

            if (inst == 0xfe00000) {
                eventType = kEventSupportRequest;
            } else {
                eventType = kEventException;
            }
            break;
        default:
            eventType = kEventBreakpoint;
            break;
        }

        TRKConstructEvent(&event, eventType);
        TRKPostEvent(&event);
    }
}

static inline DSError TRKTargetEnableTrace(BOOL val) {
    if (val) {
        gTRKCPUState.Extended1.MSR = (gTRKCPUState.Extended1.MSR | 0x400);
    } else {
        gTRKCPUState.Extended1.MSR = (gTRKCPUState.Extended1.MSR & ~0x400);
    }
    return kNoError;
}

static inline BOOL TRKTargetStepDone(void) {
    BOOL result = TRUE;

    if (gTRKStepStatus.active &&
        ((u16)gTRKCPUState.Extended1.exceptionID) == PPC_Trace) {
        switch (gTRKStepStatus.type) {
        case DSSTEP_IntoCount:
            if (gTRKStepStatus.count > 0) {
                result = FALSE;
            }
            break;
        case DSSTEP_IntoRange:
            if (gTRKCPUState.Default.PC >= gTRKStepStatus.rangeStart &&
                gTRKCPUState.Default.PC <= gTRKStepStatus.rangeEnd) {
                result = FALSE;
            }
            break;
        default:
            break;
        }
    }

    return result;
}

static inline DSError TRKTargetDoStep(void) {
    gTRKStepStatus.active = TRUE;
    TRKTargetEnableTrace(TRUE);

    if (gTRKStepStatus.type == DSSTEP_IntoCount ||
        gTRKStepStatus.type == DSSTEP_OverCount) {
        gTRKStepStatus.count--;
    }

    TRKTargetSetStopped(FALSE);
    return kNoError;
}

static inline BOOL TRKTargetCheckStep(void) {
    if (gTRKStepStatus.active) {
        TRKTargetEnableTrace(FALSE);

        if (TRKTargetStepDone()) {
            gTRKStepStatus.active = FALSE;
        } else {
            TRKTargetDoStep();
        }
    }

    return gTRKStepStatus.active;
}

DSError TRKTargetInterrupt(TRKEvent* event) {
    DSError error = kNoError;
    switch (event->type) {
    case kEventBreakpoint:
    case kEventException:
        if (TRKTargetCheckStep() == FALSE) {
            TRKTargetSetStopped(TRUE);
            error = TRKDoNotifyStopped(DSMSG_NotifyStopped);
        }
        break;
    default:
        break;
    }

    return error;
}

DSError TRKTargetAddStopInfo(TRKMessageBuffer* b) {
    DSError error;
    TRKStopInfoReply reply;
    u32 inst;

    TRK_memset(&reply, 0, sizeof(reply));
    reply.length = sizeof(reply);
    reply.command = DSMSG_NotifyStopped;
    reply.PC = gTRKCPUState.Default.PC;
    TRKTargetReadInstruction(&inst, gTRKCPUState.Default.PC);

    reply.instruction = inst;
    reply.exceptionID = gTRKCPUState.Extended1.exceptionID & 0xFFFF;
    error = TRKAppendBuffer_ui8(b, (u8*)&reply, sizeof(reply));
    return error;
}

void TRKTargetAddExceptionInfo(TRKMessageBuffer* b) {
    TRKStopInfoReply reply;
    u32 inst;

    TRK_memset(&reply, 0, sizeof(reply));

    reply.length = sizeof(reply);
    reply.command = DSMSG_NotifyException;
    reply.PC = gTRKExceptionStatus.exceptionInfo.PC;

    TRKTargetReadInstruction(&inst, gTRKExceptionStatus.exceptionInfo.PC);

    reply.instruction = inst;
    reply.exceptionID = gTRKExceptionStatus.exceptionInfo.exceptionID;

    TRKAppendBuffer_ui8(b, (u8*)&reply, sizeof(reply));
}

DSError TRKTargetSingleStep(u32 count, BOOL stepOver) {
    DSError error = kNoError;

    if (stepOver) {
        error = kUnsupportedError;
    } else {
        gTRKStepStatus.type = DSSTEP_IntoCount;
        gTRKStepStatus.count = count;
        error = TRKTargetDoStep();
    }

    return error;
}

DSError TRKTargetStepOutOfRange(u32 rangeStart, u32 rangeEnd, BOOL stepOver) {
    DSError error = kNoError;

    if (stepOver) {
        error = kUnsupportedError;
    } else {
        gTRKStepStatus.type = DSSTEP_IntoRange;
        gTRKStepStatus.rangeStart = rangeStart;
        gTRKStepStatus.rangeEnd = rangeEnd;
        error = TRKTargetDoStep();
    }

    return error;
}

u32 TRKTargetGetPC(void) {
    return gTRKCPUState.Default.PC;
}

DSError TRKTargetSupportRequest(void) {
    int ioResult;
    u32* length;
    DSMessageID commandId;
    DSError error;
    u32 position;
    TRKEvent event;

    commandId = gTRKCPUState.Default.GPR[3];
    if (commandId != DSMSG_ReadFile && commandId != DSMSG_WriteFile &&
        commandId != DSMSG_OpenFile && commandId != DSMSG_CloseFile &&
        commandId != DSMSG_PositionFile) {
        TRKConstructEvent(&event, kEventBreakpoint);
        TRKPostEvent(&event);
        return kNoError;
    } else if (commandId == DSMSG_OpenFile) {
        error = HandleOpenFileSupportRequest(
            (const char*)gTRKCPUState.Default.GPR[4],
            (u8)gTRKCPUState.Default.GPR[5],
            (u32*)gTRKCPUState.Default.GPR[6], &ioResult);

        if (ioResult == DS_IONoError && error != kNoError) {
            ioResult = DS_IOError;
        }

        gTRKCPUState.Default.GPR[3] = ioResult;
    } else if (commandId == DSMSG_CloseFile) {
        error = HandleCloseFileSupportRequest(gTRKCPUState.Default.GPR[4],
                                              &ioResult);

        if (ioResult == DS_IONoError && error != kNoError) {
            ioResult = DS_IOError;
        }

        gTRKCPUState.Default.GPR[3] = ioResult;
    } else if (commandId == DSMSG_PositionFile) {
        position = *(u32*)gTRKCPUState.Default.GPR[5];
        error = HandlePositionFileSupportRequest(
            gTRKCPUState.Default.GPR[4], &position,
            (u8)gTRKCPUState.Default.GPR[6], &ioResult);

        if (ioResult == DS_IONoError && error != kNoError) {
            ioResult = DS_IOError;
        }

        gTRKCPUState.Default.GPR[3] = ioResult;
        *(u32*)gTRKCPUState.Default.GPR[5] = position;
    } else {
        length = (u32*)gTRKCPUState.Default.GPR[5];
        error = TRKSuppAccessFile(gTRKCPUState.Default.GPR[4],
                                  (u8*)gTRKCPUState.Default.GPR[6], length,
                                  &ioResult, TRUE,
                                  commandId == DSMSG_ReadFile);

        if (ioResult == DS_IONoError && error != kNoError) {
            ioResult = DS_IOError;
        }

        gTRKCPUState.Default.GPR[3] = ioResult;

        if (commandId == DSMSG_ReadFile) {
            TRK_flush_cache(gTRKCPUState.Default.GPR[6], *length);
        }
    }

    gTRKCPUState.Default.PC += 4;
    return error;
}

BOOL TRKTargetStopped(void) {
    return gTRKState.isStopped;
}

void TRKTargetSetStopped(unsigned int stopped) {
    gTRKState.isStopped = stopped;
}

u32 TRKTargetStop(void) {
    gTRKState.isStopped = TRUE;
    return 0;
}

static inline DSError TRKPPCAccessSPR(void* value, u32 spr_register_num, BOOL read) {
    u32 access_func[10] = {INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP,
                           INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP,
                           INSTR_NOP, INSTR_NOP};

    if (read) {
        access_func[0] = INSTR_MFSPR(4, spr_register_num);
        access_func[1] = (u32)INSTR_STW(4, 0, 3);
    } else {
        access_func[0] = (u32)INSTR_LWZ(4, 0, 3);
        access_func[1] = INSTR_MTSPR(spr_register_num, 4);
    }

    return TRKPPCAccessSpecialReg(value, access_func, read);
}

static inline DSError TRKPPCAccessPairedSingleRegister(void* srcDestPtr, u32 psr,
                                         BOOL read) {
    u32 instructionData[] = {INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP,
                             INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP,
                             INSTR_NOP, INSTR_NOP};

    if (read) {
        instructionData[0] = INSTR_PSQ_ST(psr, 0, 3, 0, 0);
    } else {
        instructionData[0] = INSTR_PSQ_L(psr, 0, 3, 0, 0);
    }

    return TRKPPCAccessSpecialReg(srcDestPtr, instructionData, read);
}

static inline DSError TRKPPCAccessFPRegister(void* srcDestPtr, u32 fpr, BOOL read) {
    DSError error = kNoError;
    u32 instructionData1[] = {INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP,
                              INSTR_NOP, INSTR_NOP, INSTR_NOP, INSTR_NOP,
                              INSTR_NOP, INSTR_NOP};

    if (fpr < 0x20) {
        if (read) {
            instructionData1[0] = INSTR_STFD(fpr, 0, 3);
        } else {
            instructionData1[0] = INSTR_LFD(fpr, 0, 3);
        }

        error = TRKPPCAccessSpecialReg(srcDestPtr, instructionData1, read);
    } else if (fpr == 0x20) {
        if (read) {
            ReadFPSCR(srcDestPtr);
        } else {
            WriteFPSCR(srcDestPtr);
        }

        *(u64*)srcDestPtr &= 0xFFFFFFFF;
    } else if (fpr == 0x21) {
        if (!read) {
            *(u32*)srcDestPtr = *((u32*)(srcDestPtr) + 1);
        }

        error = TRKPPCAccessSPR(srcDestPtr, SPR_FPECR, read);
        if (read) {
            *(u64*)srcDestPtr = *(u32*)srcDestPtr & 0xffffffffLL;
        }
    }

    return error;
}

static inline DSError TRKPPCAccessSpecialReg(void* value, u32* access_func, BOOL read) {
    typedef void (*asm_access_type)(void*, void*);
    asm_access_type asm_access;

    access_func[9] = INSTR_BLR;
    asm_access = (asm_access_type)access_func;

    TRK_flush_cache((u32)access_func, sizeof(access_func) * 10);
    (*asm_access)((u32*)value, (void*)&TRKvalue128_temp);

    return kNoError;
}

void TRKTargetSetInputPendingPtr(void* ptr) {
    gTRKState.inputPendingPtr = ptr;
}

asm u32 __TRK_get_MSR(void) {
    // clang-format off
    nofralloc
    mfmsr r3
    blr
    // clang-format on
}

asm void __TRK_set_MSR(register u32 msr) {
    // clang-format off
    nofralloc
    mtmsr msr
    blr
    // clang-format on
}

asm void TRK_ppc_memcpy(register void* dest, register const void* src,
                        register int n, register u32 param_4,
                        register u32 param_5) {
    // clang-format off
    nofralloc

    mfmsr r8
    li r10, 0

top_loop:
    cmpw r10, n
    beq out_loop

    mtmsr param_5
    sync

    lbzx r9, r10, src

    mtmsr param_4
    sync

    stbx r9, r10, dest

    addi r10, r10, 1

    b top_loop
out_loop:
    mtmsr r8
    sync

    blr
    // clang-format on
}

asm void TRKInterruptHandler(void) {
    // clang-format off
    nofralloc
    mtsrr0 r2
    mtsrr1 r4
    mfsprg r4, 3
    mfcr r2
    mtsprg 3, r2
    lis r2, gTRKState@h
    ori r2, r2, gTRKState@l
    lwz r2, TRKState_PPC.MSR(r2)
    ori r2, r2, 0x8002
    xori r2, r2, 0x8002
    sync
    mtmsr r2
    sync
    lis r2, TRK_saved_exceptionID@h
    ori r2, r2, TRK_saved_exceptionID@l
    sth r3, 0(r2)
    cmpwi r3, 0x500
    bne L_DefaultHandler
    lis r2, gTRKCPUState@h
    ori r2, r2, gTRKCPUState@l
    mflr r3
    stw r3, ProcessorState_PPC.transport_handler_saved_ra(r2)
    bl TRKUARTInterruptHandler
    lis r2, gTRKCPUState@h
    ori r2, r2, gTRKCPUState@l
    lwz r3, ProcessorState_PPC.transport_handler_saved_ra(r2)
    mtlr r3
    lis r2, gTRKState@h
    ori r2, r2, gTRKState@l
    lwz r2, TRKState_PPC.inputPendingPtr(r2)
    lbz r2, 0(r2)
    cmpwi r2, 0
    beq L_Return
    lis r2, gTRKExceptionStatus@h
    ori r2, r2, gTRKExceptionStatus@l
    lbz r2, TRKExceptionStatus.inTRK(r2)
    cmpwi r2, 1
    beq L_Return
    lis r2, gTRKState@h
    ori r2, r2, gTRKState@l
    li r3, 1
    stb r3, TRKState_PPC.inputActivated(r2)
    b L_DefaultHandler
L_Return:
    lis r2, gTRKSaveState@h
    ori r2, r2, gTRKSaveState@l
    lwz r3, Default_PPC.CR(r2)
    mtcrf 0xff, r3
    lwz r3, Default_PPC.GPR[3](r2)
    lwz r2, Default_PPC.GPR[2](r2)
    rfi
L_DefaultHandler:
    lis r2, TRK_saved_exceptionID@h
    ori r2, r2, TRK_saved_exceptionID@l
    lhz r3, 0(r2)
    lis r2, gTRKExceptionStatus@h
    ori r2, r2, gTRKExceptionStatus@l
    lbz r2, TRKExceptionStatus.inTRK(r2)
    cmpwi r2, 0
    bne TRKExceptionHandler
    lis r2, gTRKCPUState@h
    ori r2, r2, gTRKCPUState@l
    stw r0, ProcessorState_PPC.Default.GPR[0](r2)
    stw r1, ProcessorState_PPC.Default.GPR[1](r2)
    mfsprg r0, 1
    stw r0, ProcessorState_PPC.Default.GPR[2](r2)
    sth r3, ProcessorState_PPC.Extended1.exceptionID(r2)
    sth r3, (ProcessorState_PPC.Extended1.exceptionID + 2)(r2)
    mfsprg r0, 2
    stw r0, ProcessorState_PPC.Default.GPR[3](r2)
    stmw r4, ProcessorState_PPC.Default.GPR[4](r2)
    mfsrr0 r27
    mflr r28
    mfsprg r29, 3
    mfctr r30
    mfxer r31
    stmw r27, ProcessorState_PPC.Default.PC(r2)
    bl TRKSaveExtended1Block
    lis r2, gTRKExceptionStatus@h
    ori r2, r2, gTRKExceptionStatus@l
    li r3, 1
    stb r3, TRKExceptionStatus.inTRK(r2)
    lis r2, gTRKState@h
    ori r2, r2, gTRKState@l
    lwz r0, TRKState_PPC.MSR(r2)
    sync
    mtmsr r0
    sync
    lwz r0, TRKState_PPC.LR(r2)
    mtlr r0
    lwz r0, TRKState_PPC.CTR(r2)
    mtctr r0
    lwz r0, TRKState_PPC.XER(r2)
    mtxer r0
    lwz r0, TRKState_PPC.DSISR(r2)
    mtdsisr r0
    lwz r0, TRKState_PPC.DAR(r2)
    mtdar r0
    lmw r3, TRKState_PPC.GPR[3](r2)
    lwz r0, TRKState_PPC.GPR[0](r2)
    lwz r1, TRKState_PPC.GPR[1](r2)
    lwz r2, TRKState_PPC.GPR[2](r2)
    b TRKPostInterruptEvent
    // clang-format on
}

asm void TRKExceptionHandler(u16 exceptionID) {
    // clang-format off
    nofralloc
    lis r2, gTRKExceptionStatus@h
    ori r2, r2, gTRKExceptionStatus@l
    sth r3, TRKExceptionStatus.exceptionInfo.exceptionID(r2)
    mfsrr0 r3
    stw r3, TRKExceptionStatus.exceptionInfo.PC(r2)
    lhz r3, TRKExceptionStatus.exceptionInfo.exceptionID(r2)
    cmpwi r3, 0x200
    beq L_SkipInstruction
    cmpwi r3, 0x300
    beq L_SkipInstruction
    cmpwi r3, 0x400
    beq L_SkipInstruction
    cmpwi r3, 0x600
    beq L_SkipInstruction
    cmpwi r3, 0x700
    beq L_SkipInstruction
    cmpwi r3, 0x800
    beq L_SkipInstruction
    cmpwi r3, 0x1000
    beq L_SkipInstruction
    cmpwi r3, 0x1100
    beq L_SkipInstruction
    cmpwi r3, 0x1200
    beq L_SkipInstruction
    cmpwi r3, 0x1300
    beq L_SkipInstruction
    b L_Detected
L_SkipInstruction:
    mfsrr0 r3
    addi r3, r3, 0x4
    mtsrr0 r3
L_Detected:
    lis r2, gTRKExceptionStatus@h
    ori r2, r2, gTRKExceptionStatus@l
    li r3, 0x1
    stb r3, TRKExceptionStatus.exceptionDetected(r2)
    mfsprg r3, 3
    mtcrf 0xff, r3
    mfsprg r2, 1
    mfsprg r3, 2
    rfi
    // clang-format on
}

asm void TRKSwapAndGo(void) {
    // clang-format off
    nofralloc
    lis r3, gTRKState@h
    ori r3, r3, gTRKState@l
    stmw r0, TRKState_PPC.GPR[0](r3)
    mfmsr r0
    stw r0, TRKState_PPC.MSR(r3)
    mflr r0
    stw r0, TRKState_PPC.LR(r3)
    mfctr r0
    stw r0, TRKState_PPC.CTR(r3)
    mfxer r0
    stw r0, TRKState_PPC.XER(r3)
    mfdsisr r0
    stw r0, TRKState_PPC.DSISR(r3)
    mfdar r0
    stw r0, TRKState_PPC.DAR(r3)
    li r1, -0x7ffe
    nor r1, r1, r1
    mfmsr r3
    and r3, r3, r1
    mtmsr r3
    lis r2, gTRKState@h
    ori r2, r2, gTRKState@l
    lwz r2, TRKState_PPC.inputPendingPtr(r2)
    lbz r2, 0(r2)
    cmpwi r2, 0
    beq L_NoInput
    lis r2, gTRKState@h
    ori r2, r2, gTRKState@l
    li r3, 1
    stb r3, TRKState_PPC.inputActivated(r2)
    b TRKInterruptHandlerEnableInterrupts
L_NoInput:
    lis r2, gTRKExceptionStatus@h
    ori r2, r2, gTRKExceptionStatus@l
    li r3, 0
    stb r3, TRKExceptionStatus.inTRK(r2)
    bl TRKRestoreExtended1Block
    lis r2, gTRKCPUState@h
    ori r2, r2, gTRKCPUState@l
    lmw r27, ProcessorState_PPC.Default.PC(r2)
    mtsrr0 r27
    mtlr r28
    mtcrf 0xff, r29
    mtctr r30
    mtxer r31
    lmw r3, ProcessorState_PPC.Default.GPR[3](r2)
    lwz r0, ProcessorState_PPC.Default.GPR[0](r2)
    lwz r1, ProcessorState_PPC.Default.GPR[1](r2)
    lwz r2, ProcessorState_PPC.Default.GPR[2](r2)
    rfi
    // clang-format on
}

asm void TRKInterruptHandlerEnableInterrupts(void) {
    // clang-format off
    nofralloc
    lis r2, gTRKState@h
    ori r2, r2, gTRKState@l
    lwz r0, TRKState_PPC.MSR(r2)
    sync
    mtmsr r0
    sync
    lwz r0, TRKState_PPC.LR(r2)
    mtlr r0
    lwz r0, TRKState_PPC.CTR(r2)
    mtctr r0
    lwz r0, TRKState_PPC.XER(r2)
    mtxer r0
    lwz r0, TRKState_PPC.DSISR(r2)
    mtdsisr r0
    lwz r0, TRKState_PPC.DAR(r2)
    mtdar r0
    lmw r3, TRKState_PPC.GPR[3](r2)
    lwz r0, TRKState_PPC.GPR[0](r2)
    lwz r1, TRKState_PPC.GPR[1](r2)
    lwz r2, TRKState_PPC.GPR[2](r2)
    b TRKPostInterruptEvent
    // clang-format on
}

asm void ReadFPSCR(register f64* p) {
    // clang-format off
    nofralloc
    stwu r1, -0x40(r1)
    stfd f31, 0x10(r1)
    psq_st f31, 0x20(r1), 0, 0
    mffs f31
    stfd f31, 0(p)
    psq_l f31, 0x20(r1), 0, 0
    lfd f31, 0x10(r1)
    addi r1, r1, 0x40
    blr
    // clang-format on
}

asm void WriteFPSCR(register f64* p) {
    // clang-format off
    nofralloc
    stwu r1, -0x40(r1)
    stfd f31, 0x10(r1)
    psq_st f31, 0x20(r1), 0, 0
    lfd f31, 0(p)
    mtfsf 0xff, f31
    psq_l f31, 0x20(r1), 0, 0
    lfd f31, 0x10(r1)
    addi r1, r1, 0x40
    blr
    // clang-format on
}
