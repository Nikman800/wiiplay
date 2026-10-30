#include "MetroTRK/debugger/Portable/msghndlr.h"

#include "MetroTRK/debugger/Portable/mem_TRK.h"
#include "MetroTRK/debugger/Portable/msg.h"
#include "MetroTRK/debugger/Portable/nubevent.h"
#include "MetroTRK/debugger/Os/dolphin/dolphin_trk_glue.h"
#include "MetroTRK/debugger/Os/dolphin/usr_put.h"

BOOL IsTRKConnected;
u32 g_CurrentSequence;

extern void SetUseSerialIO(u8 useSerialIO);
extern void __TRK_copy_vectors(void);
extern void __TRK_reset(void);
extern DSError TRKTargetStop(void);
extern DSError TRKTargetContinue(void);
extern BOOL TRKTargetStopped(void);
extern u32 TRKTargetGetPC(void);
extern DSError TRKTargetSingleStep(u32 count, BOOL stepOver);
extern DSError TRKTargetStepOutOfRange(u32 start, u32 end, BOOL stepOver);
extern DSError TRKTargetAccessMemory(void* data, u32 start, u32* length,
                                     BOOL userView, BOOL read);
extern DSError TRKTargetAccessDefault(u32 start, u32 end, void* values,
                                      u32* count, BOOL read);
extern DSError TRKTargetAccessFP(u32 start, u32 end, void* values,
                                 u32* count, BOOL read);
extern DSError TRKTargetAccessExtended1(u32 start, u32 end, void* values,
                                        u32* count, BOOL read);
extern DSError TRKTargetAccessExtended2(u32 start, u32 end, void* values,
                                        u32* count, BOOL read);

void SetTRKConnected(BOOL connected);
BOOL GetTRKConnected(void);

typedef struct {
    u32 size;       // 0x0
    u8 command;     // 0x4
    u8 padding0[3];
    u8 error;       // 0x8
    u8 padding1[3];
    u32 sequence;   // 0xC
    u8 padding2[0x30];
} TRKHeader;

static inline DSError SendACK(u8 error) {
    TRKHeader header;

    TRK_memset(&header, 0, 0x40);
    header.command = 0x80;
    header.sequence = g_CurrentSequence++;
    header.size = 0x40;
    header.error = error;
    header.sequence = g_CurrentSequence++;
    TRKWriteUARTN(&header, 0x40);
    return kNoError;
}

static inline DSError MapWriteError(DSError error) {
    switch (error) {
    case 0x703:
        return 0x12;
    case 0x701:
        return 0x14;
    case 0x302:
        return 0x02;
    case 0x702:
        return 0x15;
    case 0x704:
        return 0x21;
    case 0x705:
        return 0x22;
    case 0x706:
        return 0x20;
    default:
        return 0x03;
    }
}

static inline DSError MapReadError(DSError error) {
    switch (error) {
    case 0x703:
        return 0x12;
    case 0x701:
        return 0x14;
    case 0x702:
        return 0x15;
    case 0x704:
        return 0x21;
    case 0x705:
        return 0x22;
    case 0x706:
        return 0x20;
    default:
        return 0x03;
    }
}

BOOL GetTRKConnected(void) {
    return IsTRKConnected;
}

void SetTRKConnected(BOOL connected) {
    IsTRKConnected = connected;
}

DSError TRKDoConnect(TRKMessageBuffer* buf) {
    IsTRKConnected = 1;
    return SendACK(0);
}

DSError TRKDoDisconnect(TRKMessageBuffer* buf) {
    TRKEvent event;

    IsTRKConnected = 0;
    SendACK(0);
    TRKConstructEvent(&event, kEventWait);
    TRKPostEvent(&event);
    return kNoError;
}

DSError TRKDoReset(TRKMessageBuffer* buf) {
    SendACK(0);
    __TRK_reset();
    return kNoError;
}

DSError TRKDoOverride(TRKMessageBuffer* buf) {
    SendACK(0);
    __TRK_copy_vectors();
    return kNoError;
}

DSError TRKDoVersions(TRKMessageBuffer* buf) {
    return kNoError;
}

DSError TRKDoSupportMask(TRKMessageBuffer* buf) {
    return kNoError;
}

DSError TRKDoReadMemory(TRKMessageBuffer* buf) {
    u8 memory[0x820] ALIGN(32);
    TRKHeader header;
    u8 options = TRKMessageBufferGet(buf, u8, 0x08);
    DSError error;
    DSError ackError;
    u32 start = TRKMessageBufferGet(buf, u32, 0x10);
    u16 size = TRKMessageBufferGet(buf, u16, 0x0C);
    u32 length;

    if (options & 0x02) {
        return SendACK(0x12);
    }

    length = size;
    error = TRKTargetAccessMemory(memory, start, &length,
                                  (options & 0x08) == 0, TRUE);
    TRKResetBuffer(buf, FALSE);

    if (error == kNoError) {
        TRK_memset(&header, 0, 0x40);
        header.size = length + 0x40;
        header.error = error;
        header.command = 0x80;
        header.sequence = g_CurrentSequence++;
        TRKAppendBuffer(buf, &header, 0x40);
        if (options & 0x40) {
            error = TRKAppendBuffer(buf, memory + (start & 0x1f), length);
        } else {
            error = TRKAppendBuffer(buf, memory, length);
        }
    }

    if (error != kNoError) {
        switch (error) {
        case 0x702:
            ackError = 0x15;
            break;
        case 0x700:
            ackError = 0x13;
            break;
        case 0x704:
            ackError = 0x21;
            break;
        case 0x705:
            ackError = 0x22;
            break;
        case 0x706:
            ackError = 0x20;
            break;
        default:
            ackError = 0x03;
            break;
        }
        return SendACK(ackError);
    }
    return TRKMessageSend(buf);
}

DSError TRKDoWriteMemory(TRKMessageBuffer* buf) {
    u8 memory[0x820] ALIGN(32);
    TRKHeader header;
    u8 options = TRKMessageBufferGet(buf, u8, 0x08);
    DSError error;
    DSError ackError;
    u32 start = TRKMessageBufferGet(buf, u32, 0x10);
    u16 size = TRKMessageBufferGet(buf, u16, 0x0C);
    u32 length;

    if (options & 0x02) {
        return SendACK(0x12);
    }

    length = size;
    TRKSetBufferPosition(buf, 0x40);
    TRKReadBuffer(buf, memory, length);
    error = TRKTargetAccessMemory(memory, start, &length,
                                  (options & 0x08) == 0, FALSE);
    TRKResetBuffer(buf, FALSE);

    if (error == kNoError) {
        TRK_memset(&header, 0, 0x40);
        header.size = 0x40;
        header.command = 0x80;
        header.error = error;
        header.sequence = g_CurrentSequence++;
        error = TRKAppendBuffer(buf, &header, 0x40);
    }

    if (error != kNoError) {
        switch (error) {
        case 0x702:
            ackError = 0x15;
            break;
        case 0x700:
            ackError = 0x13;
            break;
        case 0x704:
            ackError = 0x21;
            break;
        case 0x705:
            ackError = 0x22;
            break;
        case 0x706:
            ackError = 0x20;
            break;
        default:
            ackError = 0x03;
            break;
        }
        return SendACK(ackError);
    }
    return TRKMessageSend(buf);
}

DSError TRKDoReadRegisters(TRKMessageBuffer* buf) {
    TRKHeader header;
    DSError error;
    u32 count;

    if (TRKMessageBufferGet(buf, u16, 0x0C) > TRKMessageBufferGet(buf, u16, 0x10)) {
        return SendACK(0x14);
    }

    header.command = 0x80;
    header.size = 0x468;
    header.sequence = g_CurrentSequence++;
    TRKResetBuffer(buf, FALSE);
    TRKAppendBuffer_ui8(buf, (u8*)&header, 0x40);

    error = TRKTargetAccessDefault(0, 0x24, buf, &count, TRUE);
    if (error == kNoError) {
        error = TRKTargetAccessFP(0, 0x21, buf, &count, TRUE);
    }
    if (error == kNoError) {
        error = TRKTargetAccessExtended1(0, 0x60, buf, &count, TRUE);
    }
    if (error == kNoError) {
        error = TRKTargetAccessExtended2(0, 0x1f, buf, &count, TRUE);
    }

    if (error != kNoError) {
        return SendACK(MapReadError(error));
    }
    return TRKMessageSend(buf);
}

DSError TRKDoWriteRegisters(TRKMessageBuffer* buf) {
    TRKHeader header;
    u8 type = TRKMessageBufferGet(buf, u8, 0x08);
    DSError error;
    u16 start = TRKMessageBufferGet(buf, u16, 0x0C);
    u16 end = TRKMessageBufferGet(buf, u16, 0x10);
    u32 count;

    TRKSetBufferPosition(buf, 0);
    if (start > end) {
        return SendACK(0x14);
    }

    TRKSetBufferPosition(buf, 0x40);
    switch (type) {
    case 0:
        error = TRKTargetAccessDefault(start, end, buf, &count, FALSE);
        break;
    case 1:
        error = TRKTargetAccessFP(start, end, buf, &count, FALSE);
        break;
    case 2:
        error = TRKTargetAccessExtended1(start, end, buf, &count, FALSE);
        break;
    case 3:
        error = TRKTargetAccessExtended2(start, end, buf, &count, FALSE);
        break;
    default:
        error = 0x703;
        break;
    }

    TRKResetBuffer(buf, FALSE);
    if (error == kNoError) {
        TRK_memset(&header, 0, 0x40);
        header.size = 0x40;
        header.command = 0x80;
        header.error = error;
        header.sequence = g_CurrentSequence++;
        error = TRKAppendBuffer(buf, &header, 0x40);
    }

    if (error != kNoError) {
        return SendACK(MapWriteError(error));
    }

    return TRKMessageSend(buf);
}

DSError TRKDoContinue(TRKMessageBuffer* buf) {
    if (TRKTargetStopped() == FALSE) {
        return SendACK(0x16);
    }

    SendACK(0);
    return TRKTargetContinue();
}

DSError TRKDoStep(TRKMessageBuffer* buf) {
    DSError error;
    u8 type;
    u8 count;
    u32 start;
    u32 end;
    u32 pc;

    TRKSetBufferPosition(buf, 0);
    type = TRKMessageBufferGet(buf, u8, 0x08);
    start = TRKMessageBufferGet(buf, u32, 0x10);
    end = TRKMessageBufferGet(buf, u32, 0x14);

    switch (type) {
    case 0:
    case 0x10:
        count = TRKMessageBufferGet(buf, u8, 0x0C);
        if (count < 1) {
            return SendACK(0x11);
        }
        break;
    case 1:
    case 0x11:
        pc = TRKTargetGetPC();
        if (pc < start || pc > end) {
            return SendACK(0x11);
        }
        break;
    default:
        return SendACK(0x12);
    }

    if (TRKTargetStopped() == FALSE) {
        return SendACK(0x16);
    }

    error = SendACK(0);
    switch (type) {
    case 0:
    case 0x10:
        error = TRKTargetSingleStep(count, type == 0x10);
        break;
    case 1:
    case 0x11:
        error = TRKTargetStepOutOfRange(start, end, type == 0x11);
        break;
    }

    return error;
}

DSError TRKDoStop(TRKMessageBuffer* buf) {
    u8 ackError;

    switch (TRKTargetStop()) {
    case 0:
        ackError = 0;
        break;
    case 0x704:
        ackError = 0x21;
        break;
    case 0x705:
        ackError = 0x22;
        break;
    case 0x706:
        ackError = 0x20;
        break;
    default:
        ackError = 1;
        break;
    }

    return SendACK(ackError);
}

DSError TRKDoSetOption(TRKMessageBuffer* buf) {
    static const char sSerialIO[] = "\nMetroTRK Option : SerialIO - ";
    static const char sEnable[] = "Enable\n";
    static const char sDisable[] = "Disable\n";
    u8 type = TRKMessageBufferGet(buf, u8, 0x08);
    u8 useSerialIO = TRKMessageBufferGet(buf, u8, 0x0C);

    if (type == 1) {
        usr_puts_serial(sSerialIO);
        if (useSerialIO != 0) {
            usr_puts_serial(sEnable);
        } else {
            usr_puts_serial(sDisable);
        }
        SetUseSerialIO(useSerialIO);
    }

    return SendACK(0);
}
