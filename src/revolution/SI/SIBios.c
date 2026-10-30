#include <revolution/OS.h>
#include <revolution/SI.h>
#include <revolution/VI.h>

#define SI_ERROR_NOREP 0x0008
#define SI_ERROR_BUSY 0x0080

#define SI_TYPE_MASK 0x18000000
#define SI_TYPE_GC 0x08000000
#define SI_GC_WIRELESS 0x80000000
#define SI_WIRELESS_RECEIVED 0x40000000
#define SI_WIRELESS_IR 0x04000000
#define SI_WIRELESS_FIX_ID 0x00100000
#define SI_WIRELESS_TYPE_ID 0x00CFFF00

typedef void (*SITypeAndStatusCallback)(s32 chan, u32 type);

typedef struct SIMain {
    s32 chan;
    u32 poll;
    u32 inSize;
    void* inAddr;
    SICallback callback;
} SIMain;

typedef struct SIPacket {
    s32 chan;
    void* outAddr;
    u32 outSize;
    void* inAddr;
    u32 inSize;
    SICallback callback;
    s64 fire;
} SIPacket;

const char* __SIVersion =
    "<< RVL_SDK - SI \trelease build: Nov 30 2006 03:31:44 (0x4199_60831) >>";

static SIMain Si = {SI_CHAN_NONE};
static SIPacket Packet[SI_MAX_CHAN];
static s64 XferTime[SI_MAX_CHAN];
static s64 TypeTime[SI_MAX_CHAN];
static OSAlarm Alarm[SI_MAX_CHAN];
static u32 Type[SI_MAX_CHAN] = {SI_ERROR_NOREP, SI_ERROR_NOREP, SI_ERROR_NOREP,
                                SI_ERROR_NOREP};
static u32 InputBuffer[SI_MAX_CHAN][2];
static BOOL InputBufferValid[SI_MAX_CHAN];
static volatile u32 InputBufferVcount[SI_MAX_CHAN];
static OSInterruptHandler RDSTHandler[SI_MAX_CHAN];
static SITypeAndStatusCallback TypeCallback[SI_MAX_CHAN][4];

u32 __PADFixBits;

static void GetTypeCallback(s32 chan, u32 error, OSContext* ctx);
static BOOL SIGetResponseRaw(s32 chan);
static BOOL __SITransfer(s32 chan, void* outAddr, u32 outSize, void* inAddr,
                         u32 inSize, SICallback callback);

BOOL SIBusy(void) {
    return Si.chan != SI_CHAN_NONE ? TRUE : FALSE;
}

BOOL SIIsChanBusy(s32 chan) {
    return Packet[chan].chan != SI_CHAN_NONE || Si.chan == chan;
}

static u32 CompleteTransfer(void) {
    u32 i;
    u32 sr;
    u8* dst;
    u32 size;
    u32 imm;

    sr = SI_HW_REGS[SI_SISR];
    SI_HW_REGS[SI_SICOMSCR] =
        (SI_HW_REGS[SI_SICOMSCR] | SI_SICOMCSR_TCINT) & ~SI_SICOMCSR_TSTART;

    if (Si.chan != SI_CHAN_NONE) {
        XferTime[Si.chan] = __OSGetSystemTime();

        dst = (u8*)Si.inAddr;

        size = Si.inSize / sizeof(u32);
        for (i = 0; i < size; dst += sizeof(u32), i++) {
            *(u32*)dst = SI_HW_REGS[SI_RAM_BASE + i];
        }

        size = Si.inSize % sizeof(u32);
        if (size > 0) {
            imm = SI_HW_REGS[SI_RAM_BASE + i];

            for (i = 0; i < size; dst++, i++) {
                *dst = imm >> (3 - i) * 8;
            }
        }

        if (SI_HW_REGS[SI_SICOMSCR] & SI_SICOMCSR_COMERR) {
            sr >>= ((SI_MAX_CHAN - 1) - Si.chan) * 8;
            sr &= (SI_UNRUN | SI_OVRUN | SI_COLL | SI_NOREP);

            if ((sr & SI_NOREP) && (Type[Si.chan] & SI_ERROR_BUSY) == 0) {
                Type[Si.chan] = SI_ERROR_NOREP;
            }

            if (sr == 0) {
                sr = SI_COLL;
            }
        } else {
            TypeTime[Si.chan] = __OSGetSystemTime();
            sr = 0;
        }

        Si.chan = SI_CHAN_NONE;
    }

    return sr;
}

static void SITransferNext(s32 chan) {
    int i;
    SIPacket* packet;

    for (i = 0; i < SI_MAX_CHAN; i++) {
        chan++;
        chan %= SI_MAX_CHAN;
        packet = &Packet[chan];

        if (packet->chan != SI_CHAN_NONE &&
            packet->fire <= __OSGetSystemTime()) {
            if (__SITransfer(packet->chan, packet->outAddr, packet->outSize,
                             packet->inAddr, packet->inSize,
                             packet->callback)) {
                OSCancelAlarm(&Alarm[chan]);
                packet->chan = SI_CHAN_NONE;
            }

            break;
        }
    }
}

static void SIInterruptHandler(s16 intr, OSContext* ctx) {
    u32 reg;

    reg = SI_HW_REGS[SI_SICOMSCR];

    if ((reg & (SI_SICOMCSR_TCINT | SI_SICOMCSR_TCINTMSK)) ==
        (SI_SICOMCSR_TCINT | SI_SICOMCSR_TCINTMSK)) {
        s32 chan;
        u32 sr;
        SICallback callback;

        chan = Si.chan;
        sr = CompleteTransfer();
        callback = Si.callback;
        Si.callback = NULL;

        SITransferNext(chan);

        if (callback != NULL) {
            callback(chan, sr, ctx);
        }

        sr = SI_HW_REGS[SI_SISR];
        sr &= 0x0F000000 >> (chan * 8);
        SI_HW_REGS[SI_SISR] = sr;

        if (Type[chan] == SI_ERROR_BUSY && !SIIsChanBusy(chan)) {
            static u32 cmdTypeAndStatus = 0 << 24;
            SITransfer(chan, &cmdTypeAndStatus, 1, &Type[chan], 3,
                       GetTypeCallback, OS_USEC_TO_TICKS(65));
        }
    }

    if ((reg & (SI_SICOMCSR_RDSTINT | SI_SICOMCSR_RDSTINTMSK)) ==
        (SI_SICOMCSR_RDSTINT | SI_SICOMCSR_RDSTINTMSK)) {
        int i;
        u32 vcount;
        u32 x;

        vcount = VIGetCurrentLine() + 1;
        x = (Si.poll & SI_SIPOLL_X) >> 16;

        for (i = 0; i < SI_MAX_CHAN; i++) {
            if (SIGetResponseRaw(i)) {
                InputBufferVcount[i] = vcount;
            }
        }

        for (i = 0; i < SI_MAX_CHAN; i++) {
            if (!(Si.poll & (0x80000000 >> (31 - 7 + i)))) {
                continue;
            }

            if (InputBufferVcount[i] == 0 ||
                InputBufferVcount[i] + (x / 2) < vcount) {
                return;
            }
        }

        for (i = 0; i < SI_MAX_CHAN; i++) {
            InputBufferVcount[i] = 0;
        }

        for (i = 0; i < SI_MAX_CHAN; i++) {
            if (RDSTHandler[i] != NULL) {
                RDSTHandler[i](intr, ctx);
            }
        }
    }
}

void SIInit(void) {
    static BOOL Initialized = FALSE;

    if (Initialized) {
        return;
    }

    OSRegisterVersion(__SIVersion);

    Packet[SI_CHAN_0].chan = Packet[SI_CHAN_1].chan = Packet[SI_CHAN_2].chan =
        Packet[SI_CHAN_3].chan = SI_CHAN_NONE;

    Si.poll = 0;
    SISetSamplingRate(0);

    while (SI_HW_REGS[SI_SICOMSCR] & SI_SICOMCSR_TSTART) {
        ;
    }

    SI_HW_REGS[SI_SICOMSCR] = SI_SICOMCSR_TCINT;

    __OSSetInterruptHandler(OS_INTR_PI_SI, SIInterruptHandler);
    __OSUnmaskInterrupts(OS_INTR_MASK(OS_INTR_PI_SI));

    SIGetType(SI_CHAN_0);
    SIGetType(SI_CHAN_1);
    SIGetType(SI_CHAN_2);
    SIGetType(SI_CHAN_3);

    Initialized = TRUE;
}

static BOOL __SITransfer(s32 chan, void* outAddr, u32 outSize, void* inAddr,
                         u32 inSize, SICallback callback) {
    BOOL enabled;
    u32 sr;
    u32 alignSize;
    u32 i;

    union {
        struct {
            u32 TCINT : 1;
            u32 TCINTMSK : 1;
            u32 COMERR : 1;
            u32 RDSTINT : 1;
            u32 RDSTINTMSK : 1;
            u32 reserved2 : 4;
            u32 OUTLNGTH : 7;
            u32 reserved1 : 1;
            u32 INLNGTH : 7;
            u32 reserved0 : 5;
            u32 CHANNEL : 2;
            u32 TSTART : 1;
        };

        u32 reg;
    } comscr;

    enabled = OSDisableInterrupts();

    if (Si.chan != SI_CHAN_NONE) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    sr = SI_HW_REGS[SI_SISR];
    sr &= (SI_SISR_UNRUN0 | SI_SISR_OVRUN0 | SI_SISR_COLL0 | SI_SISR_NOREP0) >>
          (chan * 8);
    SI_HW_REGS[SI_SISR] = sr;

    Si.chan = chan;
    Si.callback = callback;
    Si.inSize = inSize;
    Si.inAddr = inAddr;

    alignSize = (outSize + 3) / 4;
    for (i = 0; i < alignSize; i++) {
        SI_HW_REGS[SI_RAM_BASE + i] = ((u32*)outAddr)[i];
    }

    comscr.reg = SI_HW_REGS[SI_SICOMSCR];

    comscr.TCINT = TRUE;
    comscr.TCINTMSK = callback != NULL;
    comscr.OUTLNGTH = outSize == 128 ? 0 : outSize;
    comscr.INLNGTH = inSize == 128 ? 0 : inSize;
    comscr.CHANNEL = chan;
    comscr.TSTART = TRUE;

    SI_HW_REGS[SI_SICOMSCR] = comscr.reg;

    OSRestoreInterrupts(enabled);
    return TRUE;
}

u32 SIGetStatus(s32 chan) {
    BOOL enabled;
    u32 sr;
    int chanShift;

    enabled = OSDisableInterrupts();
    sr = SI_HW_REGS[SI_SISR];
    chanShift = (SI_MAX_CHAN - 1 - chan) * 8;
    sr >>= chanShift;

    if (sr & SI_NOREP) {
        if (!(Type[chan] & SI_ERROR_BUSY)) {
            Type[chan] = SI_ERROR_NOREP;
        }
    }

    OSRestoreInterrupts(enabled);
    return sr;
}

void SISetCommand(s32 chan, u32 command) {
    SI_HW_REGS[chan * 3] = command;
}

void SITransferCommands(void) {
    SI_HW_REGS[SI_SISR] = SI_SISR_WR;
}

u32 SISetXY(u32 lines, u32 times) {
    BOOL enabled;
    u32 poll;

    poll = lines << 16;
    poll |= times << 8;

    enabled = OSDisableInterrupts();

    Si.poll &= ~(SI_SIPOLL_X | SI_SIPOLL_Y);
    Si.poll |= poll;

    poll = Si.poll;
    SI_HW_REGS[SI_SIPOLL] = poll;

    OSRestoreInterrupts(enabled);
    return poll;
}

static BOOL SIGetResponseRaw(s32 chan) {
    u32 sr;

    sr = SIGetStatus(chan);
    if (sr & SI_RDST) {
        InputBuffer[chan][0] = SI_HW_REGS[chan * 3 + 1];
        InputBuffer[chan][1] = SI_HW_REGS[chan * 3 + 2];
        InputBufferValid[chan] = TRUE;
        return TRUE;
    }

    return FALSE;
}

static void AlarmHandler(OSAlarm* alarm, OSContext* ctx) {
    s32 chan;
    SIPacket* packet;

    chan = alarm - Alarm;
    packet = &Packet[chan];

    if (packet->chan != SI_CHAN_NONE) {
        if (__SITransfer(packet->chan, packet->outAddr, packet->outSize,
                         packet->inAddr, packet->inSize, packet->callback)) {
            packet->chan = SI_CHAN_NONE;
        }
    }
}

BOOL SITransfer(s32 chan, void* outAddr, u32 outSize, void* inAddr, u32 inSize,
                SICallback callback, s64 wait) {
    BOOL enabled;
    SIPacket* packet = &Packet[chan];
    s64 now;
    s64 fire;

    enabled = OSDisableInterrupts();

    if (packet->chan != SI_CHAN_NONE || Si.chan == chan) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }

    now = __OSGetSystemTime();

    if (wait == 0) {
        fire = now;
    } else {
        fire = XferTime[chan] + wait;
    }

    if (now < fire) {
        wait = fire - now;
        OSSetAlarm(&Alarm[chan], wait, AlarmHandler);
    } else if (__SITransfer(chan, outAddr, outSize, inAddr, inSize, callback)) {
        OSRestoreInterrupts(enabled);
        return TRUE;
    }

    packet->chan = chan;
    packet->outAddr = outAddr;
    packet->outSize = outSize;
    packet->inAddr = inAddr;
    packet->inSize = inSize;
    packet->callback = callback;
    packet->fire = fire;

    OSRestoreInterrupts(enabled);
    return TRUE;
}

static void CallTypeAndStatusCallback(s32 chan, u32 type) {
    SITypeAndStatusCallback callback;
    int i;

    for (i = 0; i < 4; i++) {
        callback = TypeCallback[chan][i];
        if (callback != NULL) {
            TypeCallback[chan][i] = NULL;
            callback(chan, type);
        }
    }
}

static void GetTypeCallback(s32 chan, u32 error, OSContext* ctx) {
    static u32 cmdFixDevice[SI_MAX_CHAN];
    u32 type;
    u32 chanBit;
    BOOL fix;
    u32 id;

    Type[chan] &= ~SI_ERROR_BUSY;
    Type[chan] |= error;
    TypeTime[chan] = __OSGetSystemTime();

    type = Type[chan];

    chanBit = 0x80000000 >> chan;
    fix = __PADFixBits & chanBit;
    __PADFixBits &= ~chanBit;

    if ((error & (SI_UNRUN | SI_OVRUN | SI_NOREP | SI_COLL)) ||
        (type & SI_TYPE_MASK) != SI_TYPE_GC || !(type & SI_GC_WIRELESS) ||
        (type & SI_WIRELESS_IR)) {
        OSSetWirelessID(chan, 0);
        CallTypeAndStatusCallback(chan, Type[chan]);
        return;
    }

    id = (u32)(OSGetWirelessID(chan) << 8);

    if (fix && (id & SI_WIRELESS_FIX_ID)) {
        cmdFixDevice[chan] = 0x4E << 24 | (id & SI_WIRELESS_TYPE_ID) |
                             SI_WIRELESS_FIX_ID;
        Type[chan] = SI_ERROR_BUSY;
        SITransfer(chan, &cmdFixDevice[chan], 3, &Type[chan], 3,
                   GetTypeCallback, 0);
        return;
    }

    if (type & SI_WIRELESS_FIX_ID) {
        if ((id & SI_WIRELESS_TYPE_ID) != (type & SI_WIRELESS_TYPE_ID)) {
            if (!(id & SI_WIRELESS_FIX_ID)) {
                id = type & SI_WIRELESS_TYPE_ID;
                id |= SI_WIRELESS_FIX_ID;
                OSSetWirelessID(chan, (u16)((id >> 8) & 0xFFFF));
            }

            cmdFixDevice[chan] = 0x4E << 24 | id;
            Type[chan] = SI_ERROR_BUSY;
            SITransfer(chan, &cmdFixDevice[chan], 3, &Type[chan], 3,
                       GetTypeCallback, 0);
            return;
        }
    } else if (type & SI_WIRELESS_RECEIVED) {
        id = type & SI_WIRELESS_TYPE_ID;
        id |= SI_WIRELESS_FIX_ID;
        OSSetWirelessID(chan, (u16)((id >> 8) & 0xFFFF));

        cmdFixDevice[chan] = 0x4E << 24 | id;
        Type[chan] = SI_ERROR_BUSY;
        SITransfer(chan, &cmdFixDevice[chan], 3, &Type[chan], 3,
                   GetTypeCallback, 0);
        return;
    } else {
        OSSetWirelessID(chan, 0);
    }

    CallTypeAndStatusCallback(chan, Type[chan]);
}

u32 SIGetType(s32 chan) {
    static u32 cmdTypeAndStatus;
    BOOL enabled;
    u32 type;
    s64 diff;

    enabled = OSDisableInterrupts();

    type = Type[chan];
    diff = __OSGetSystemTime() - TypeTime[chan];

    if (Si.poll & (0x80 >> chan)) {
        if (type != SI_ERROR_NOREP) {
            TypeTime[chan] = __OSGetSystemTime();
            OSRestoreInterrupts(enabled);
            return type;
        } else {
            type = Type[chan] = SI_ERROR_BUSY;
        }
    } else if (diff <= OS_MSEC_TO_TICKS(50) && type != SI_ERROR_NOREP) {
        OSRestoreInterrupts(enabled);
        return type;
    } else if (diff <= OS_MSEC_TO_TICKS(75)) {
        Type[chan] = SI_ERROR_BUSY;
    } else {
        type = Type[chan] = SI_ERROR_BUSY;
    }

    TypeTime[chan] = __OSGetSystemTime();

    SITransfer(chan, &cmdTypeAndStatus, 1, &Type[chan], 3, GetTypeCallback,
               OS_USEC_TO_TICKS(65));

    OSRestoreInterrupts(enabled);
    return type;
}
