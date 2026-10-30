#include <revolution/BASE.h>
#include <revolution/GX.h>
#include <revolution/OS.h>

extern volatile void* __peReg;
extern volatile void* __memReg;

GXBool __GXIsGPFifoReady(void);
void __GXCleanGPFifo(void);
void __GXInitRevisionBits(void);

static GXDrawSyncCallback TokenCB;
static GXDrawDoneCallback DrawDoneCB;
static volatile GXBool DrawDone;
static OSThreadQueue FinishQueue;

#define GX_PE_REG_READ_U16(i) (((volatile u16*)__peReg)[(i)])
#define GX_PE_REG_WRITE_U16(i, x) (((volatile u16*)__peReg)[(i)] = (u16)(x))
#define GX_MEM_REG_READ_U16(i) (((volatile u16*)__memReg)[(i)])

void GXSetMisc(UNKWORD token, UNKWORD val) {
    switch (token) {
    case 0:
        break;
    case 1:
        gxdt->SHORT_0x4 = val;
        gxdt->SHORT_0x0 = !gxdt->SHORT_0x4;
        gxdt->lastWriteWasXF = TRUE;
        if (gxdt->SHORT_0x4 > 0) {
            gxdt->gxDirtyFlags |= 8;
        }
        break;
    case 2:
        gxdt->dlistSave = (val != 0);
        break;
    case 3:
        gxdt->BYTE_0x5FA = (val != 0);
        break;
    }
}

void GXFlush(void) {
    if (gxdt->gxDirtyFlags) {
        __GXSetDirtyState();
    }

    WGPIPE.i = 0;
    WGPIPE.i = 0;
    WGPIPE.i = 0;
    WGPIPE.i = 0;
    WGPIPE.i = 0;
    WGPIPE.i = 0;
    WGPIPE.i = 0;
    WGPIPE.i = 0;
    PPCSync();
}

void GXResetWriteGatherPipe(void) {
    while (PPCMfwpar() & 1) {
    }
    PPCMtwpar(0x0C008000);
}

static inline void __GXAbortWait(u32 clocks) {
    s64 time0, time1;

    time0 = OSGetTime();
    do {
        time1 = OSGetTime();
    } while (time1 - time0 <= clocks / 4);
}

static inline u32 __GXReadMEMCounterU32(u32 regAddrL, u32 regAddrH) {
    u32 ctrH0, ctrH1, ctrL;

    ctrH0 = GX_MEM_REG_READ_U16(regAddrH);
    do {
        ctrH1 = ctrH0;
        ctrL = GX_MEM_REG_READ_U16(regAddrL);
        ctrH0 = GX_MEM_REG_READ_U16(regAddrH);
    } while (ctrH0 != ctrH1);

    return (ctrH0 << 16) | ctrL;
}

static inline void __GXAbortWaitPECopyDone(void) {
    u32 peCnt0, peCnt1;

    peCnt0 = __GXReadMEMCounterU32(0x28, 0x27);
    do {
        peCnt1 = peCnt0;
        __GXAbortWait(32);
        peCnt0 = __GXReadMEMCounterU32(0x28, 0x27);
    } while (peCnt0 != peCnt1);
}

void __GXAbort(void) {
    if (gxdt->BYTE_0x5FA && __GXIsGPFifoReady()) {
        __GXAbortWaitPECopyDone();
    }

    *(volatile u32*)0xCC003018 = 1;
    __GXAbortWait(200);
    *(volatile u32*)0xCC003018 = 0;
    __GXAbortWait(20);
}

void GXAbortFrame(void) {
    __GXAbort();

    if (__GXIsGPFifoReady()) {
        __GXCleanGPFifo();
        __GXInitRevisionBits();
        gxdt->gxDirtyFlags = 0;
        GXFlush();
    }
}

void GXSetDrawSync(u16 token) {
    BOOL enabled;
    u32 reg = 0;

    enabled = OSDisableInterrupts();

    reg = token | 0x48 << 24;
    GX_BP_LOAD_REG(reg);
    reg = GX_BITSET(reg, 16, 16, token);
    reg = GX_BITSET(reg, 0, 8, 0x47);
    GX_BP_LOAD_REG(reg);
    GXFlush();

    OSRestoreInterrupts(enabled);
    gxdt->lastWriteWasXF = FALSE;
}

static inline void GXSetDrawDone(void) {
    u32 reg;
    BOOL enabled = OSDisableInterrupts();

    reg = 2 | 0x45 << 24;
    GX_BP_LOAD_REG(reg);
    GXFlush();
    DrawDone = FALSE;

    OSRestoreInterrupts(enabled);
}

static inline void GXWaitDrawDone(void) {
    BOOL enabled = OSDisableInterrupts();

    while (!DrawDone) {
        OSSleepThread(&FinishQueue);
    }

    OSRestoreInterrupts(enabled);
}

void GXDrawDone(void) {
    GXSetDrawDone();
    GXWaitDrawDone();
}

void GXPixModeSync(void) {
    GX_BP_LOAD_REG(gxdt->zControl);
    gxdt->lastWriteWasXF = FALSE;
}

void GXPokeAlphaMode(GXCompare func, u8 threshold) {
    u32 reg = func << 8 | threshold;
    GX_PE_REG_WRITE_U16(3, reg);
}

void GXPokeAlphaRead(u32 mode) {
    u32 reg = 0;

    reg = GX_BITSET(reg, 30, 2, mode);
    reg = GX_BITSET(reg, 29, 1, 1);
    GX_PE_REG_WRITE_U16(4, reg);
}

void GXPokeAlphaUpdate(GXBool update) {
    u32 reg = GX_PE_REG_READ_U16(1);

    reg = GX_BITSET(reg, 27, 1, update);
    GX_PE_REG_WRITE_U16(1, reg);
}

void GXPokeBlendMode(GXBlendMode mode, GXBlendFactor src, GXBlendFactor dst,
                     GXLogicOp op) {
    u32 reg = GX_PE_REG_READ_U16(1);

    reg = GX_BITSET(reg, 31, 1, mode == GX_BM_BLEND || mode == GX_BM_SUBTRACT);
    reg = GX_BITSET(reg, 20, 1, mode == GX_BM_SUBTRACT);
    reg = GX_BITSET(reg, 30, 1, mode == GX_BM_LOGIC);
    reg = GX_BITSET(reg, 16, 4, op);
    reg = GX_BITSET(reg, 21, 3, src);
    reg = GX_BITSET(reg, 24, 3, dst);
    reg = GX_BITSET(reg, 0, 8, 0x41);
    GX_PE_REG_WRITE_U16(1, reg);
}

void GXPokeColorUpdate(GXBool update) {
    u32 reg = GX_PE_REG_READ_U16(1);

    reg = GX_BITSET(reg, 28, 1, update);
    GX_PE_REG_WRITE_U16(1, reg);
}

void GXPokeDstAlpha(GXBool enable, u8 alpha) {
    u32 reg = 0;

    reg = GX_BITSET(reg, 24, 8, alpha);
    reg = GX_BITSET(reg, 23, 1, enable);
    GX_PE_REG_WRITE_U16(2, reg);
}

void GXPokeDither(GXBool dither) {
    u32 reg = GX_PE_REG_READ_U16(1);

    reg = GX_BITSET(reg, 29, 1, dither);
    GX_PE_REG_WRITE_U16(1, reg);
}

void GXPokeZMode(GXBool compare, GXCompare func, GXBool update) {
    u32 reg = 0;

    reg = GX_BITSET(reg, 31, 1, compare);
    reg = GX_BITSET(reg, 28, 3, func);
    reg = GX_BITSET(reg, 27, 1, update);
    GX_PE_REG_WRITE_U16(0, reg);
}

GXDrawSyncCallback GXSetDrawSyncCallback(GXDrawSyncCallback callback) {
    GXDrawSyncCallback old = TokenCB;
    BOOL enabled = OSDisableInterrupts();

    TokenCB = callback;
    OSRestoreInterrupts(enabled);
    return old;
}

static void GXTokenInterruptHandler(s16 intr, OSContext* ctx) {
    u16 token;
    OSContext exceptionCtx;
    u32 reg;

    token = GX_PE_REG_READ_U16(7);

    if (TokenCB) {
        OSClearContext(&exceptionCtx);
        OSSetCurrentContext(&exceptionCtx);
        TokenCB(token);
        OSClearContext(&exceptionCtx);
        OSSetCurrentContext(ctx);
    }

    reg = GX_PE_REG_READ_U16(5);
    reg = GX_BITSET(reg, 29, 1, 1);
    GX_PE_REG_WRITE_U16(5, reg);
}

GXDrawDoneCallback GXSetDrawDoneCallback(GXDrawDoneCallback callback) {
    GXDrawDoneCallback old = DrawDoneCB;
    BOOL enabled = OSDisableInterrupts();

    DrawDoneCB = callback;
    OSRestoreInterrupts(enabled);
    return old;
}

static void GXFinishInterruptHandler(s16 intr, OSContext* ctx) {
    OSContext exceptionCtx;
    u32 reg = 0;

    reg = GX_PE_REG_READ_U16(5);
    reg = GX_BITSET(reg, 28, 1, 1);
    GX_PE_REG_WRITE_U16(5, reg);

    DrawDone = TRUE;

    if (DrawDoneCB) {
        OSClearContext(&exceptionCtx);
        OSSetCurrentContext(&exceptionCtx);
        DrawDoneCB();
        OSClearContext(&exceptionCtx);
        OSSetCurrentContext(ctx);
    }

    OSWakeupThread(&FinishQueue);
}

void __GXPEInit(void) {
    u32 reg;

    __OSSetInterruptHandler(OS_INTR_PI_PE_TOKEN, GXTokenInterruptHandler);
    __OSSetInterruptHandler(OS_INTR_PI_PE_FINISH, GXFinishInterruptHandler);
    OSInitThreadQueue(&FinishQueue);
    __OSUnmaskInterrupts(OS_INTR_MASK(OS_INTR_PI_PE_TOKEN));
    __OSUnmaskInterrupts(OS_INTR_MASK(OS_INTR_PI_PE_FINISH));

    reg = GX_PE_REG_READ_U16(5);
    reg = GX_BITSET(reg, 29, 1, 1);
    reg = GX_BITSET(reg, 28, 1, 1);
    reg = GX_BITSET(reg, 31, 1, 1);
    reg = GX_BITSET(reg, 30, 1, 1);
    GX_PE_REG_WRITE_U16(5, reg);
}
