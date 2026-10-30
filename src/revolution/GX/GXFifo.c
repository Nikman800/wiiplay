#include <revolution/BASE/PPCArch.h>
#include <revolution/GX.h>
#include <revolution/OS.h>
#include <string.h>

extern volatile void* __cpReg;
extern volatile void* __piReg;

#define GX_CP_REG_READ_U16(i) (((volatile u16*)__cpReg)[(i)])
#define GX_CP_REG_WRITE_U16(i, x) (((volatile u16*)__cpReg)[(i)] = (u16)(x))
#define GX_PI_REG_READ_U32(i) (((volatile u32*)__piReg)[(i)])
#define GX_PI_REG_WRITE_U32(i, x) (((volatile u32*)__piReg)[(i)] = (u32)(x))

#define CP_STATUS 0
#define CP_ENABLE 1
#define CP_CLR 2
#define CP_FIFO_BASEL 16
#define CP_FIFO_BASEH 17
#define CP_FIFO_TOPL 18
#define CP_FIFO_TOPH 19
#define CP_FIFO_HICNTL 20
#define CP_FIFO_HICNTH 21
#define CP_FIFO_LOCNTL 22
#define CP_FIFO_LOCNTH 23
#define CP_FIFO_COUNTL 24
#define CP_FIFO_COUNTH 25
#define CP_FIFO_WPTRL 26
#define CP_FIFO_WPTRH 27
#define CP_FIFO_RPTRL 28
#define CP_FIFO_RPTRH 29
#define CP_FIFO_BRKL 30
#define CP_FIFO_BRKH 31

#define PI_REG_CPBAS 3
#define PI_REG_CPTOP 4
#define PI_REG_CPWRT 5

#define TOPHYSICAL(a) (((u32)(a)) & 0x3FFFFFFF)


typedef void (*GXBreakPtCallback)(void);

typedef struct __GXFifoObj {
    void* base;        // at 0x0
    void* top;         // at 0x4
    u32 size;          // at 0x8
    u32 hiWatermark;   // at 0xC
    u32 loWatermark;   // at 0x10
    void* rdPtr;       // at 0x14
    void* wrPtr;       // at 0x18
    s32 count;         // at 0x1C
    GXBool wrap;       // at 0x20
    GXBool bind_cpu;   // at 0x21
    GXBool bind_gp;    // at 0x22
} __GXFifoObj;

static __GXFifoObj CPUFifo;
static __GXFifoObj GPFifo;
static GXBool CPUFifoReady = FALSE;
static GXBool GPFifoReady = FALSE;
static GXBool CPGPLinked;
static OSThread* __GXCurrentThread;
static volatile BOOL GXOverflowSuspendInProgress;
static GXBreakPtCallback BreakPointCB;
volatile void* __GXCurrentBP;
static volatile u32 __GXOverflowCount;

void GXFlush(void);
void __GXSaveFifo(void);

static void __GXWriteFifoIntEnable(GXBool hi, GXBool lo) {
    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 29, 1, hi);
    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 28, 1, lo);
    GX_CP_REG_WRITE_U16(CP_ENABLE, gxdt->cpCtrlReg);
}

static void __GXWriteFifoIntReset(GXBool hi, GXBool lo) {
    gxdt->cpClrReg = GX_BITSET(gxdt->cpClrReg, 31, 1, hi);
    gxdt->cpClrReg = GX_BITSET(gxdt->cpClrReg, 30, 1, lo);
    GX_CP_REG_WRITE_U16(CP_CLR, gxdt->cpClrReg);
}

static void __GXFifoReadEnable(void) {
    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 31, 1, 1);
    GX_CP_REG_WRITE_U16(CP_ENABLE, gxdt->cpCtrlReg);
}

static void __GXFifoReadDisable(void) {
    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 31, 1, 0);
    GX_CP_REG_WRITE_U16(CP_ENABLE, gxdt->cpCtrlReg);
}

static void __GXFifoLink(GXBool en) {
    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 27, 1, en ? 1 : 0);
    GX_CP_REG_WRITE_U16(CP_ENABLE, gxdt->cpCtrlReg);
}

static void GXOverflowHandler(s16 interrupt, OSContext* context) {
    __GXOverflowCount++;
    __GXWriteFifoIntEnable(GX_FALSE, GX_TRUE);
    __GXWriteFifoIntReset(GX_TRUE, GX_FALSE);
    GXOverflowSuspendInProgress = TRUE;
    OSSuspendThread(__GXCurrentThread);
}

static void GXUnderflowHandler(s16 interrupt, OSContext* context) {
    OSResumeThread(__GXCurrentThread);
    GXOverflowSuspendInProgress = FALSE;
    __GXWriteFifoIntReset(GX_TRUE, GX_TRUE);
    __GXWriteFifoIntEnable(GX_TRUE, GX_FALSE);
}

static void GXBreakPointHandler(s16 interrupt, OSContext* context) {
    OSContext exceptionContext;

    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 26, 1, 0);
    GX_CP_REG_WRITE_U16(CP_ENABLE, gxdt->cpCtrlReg);

    if (BreakPointCB) {
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(&exceptionContext);
        BreakPointCB();
        OSClearContext(&exceptionContext);
        OSSetCurrentContext(context);
    }
}

void GXCPInterruptHandler(s16 interrupt, OSContext* context) {
    gxdt->cpStatReg = GX_CP_REG_READ_U16(CP_STATUS);

    if (GX_BITGET(gxdt->cpCtrlReg, 28, 1) &&
        GX_BITGET(gxdt->cpStatReg, 30, 1)) {
        GXUnderflowHandler(interrupt, context);
    }

    if (GX_BITGET(gxdt->cpCtrlReg, 29, 1) &&
        GX_BITGET(gxdt->cpStatReg, 31, 1)) {
        GXOverflowHandler(interrupt, context);
    }

    if (GX_BITGET(gxdt->cpCtrlReg, 26, 1) &&
        GX_BITGET(gxdt->cpStatReg, 27, 1)) {
        GXBreakPointHandler(interrupt, context);
    }
}

static void GXInitFifoPtrs(GXFifoObj* fifo, void* readPtr, void* writePtr) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;
    BOOL en = OSDisableInterrupts();

    realFifo->rdPtr = readPtr;
    realFifo->wrPtr = writePtr;
    realFifo->count = (u8*)writePtr - (u8*)readPtr;
    if (realFifo->count < 0) {
        realFifo->count += realFifo->size;
    }

    OSRestoreInterrupts(en);
}

static void GXInitFifoLimits(GXFifoObj* fifo, u32 hi, u32 lo) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;

    realFifo->hiWatermark = hi;
    realFifo->loWatermark = lo;
}

void GXInitFifoBase(GXFifoObj* fifo, void* base, u32 size) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;

    realFifo->base = base;
    realFifo->top = (void*)((u32)base + size - 4);
    realFifo->size = size;
    realFifo->count = 0;
    GXInitFifoLimits(fifo, size - (16 * 1024), (size / 2) & ~0x1F);
    GXInitFifoPtrs(fifo, base, base);
}

GXBool CPGPLinkCheck(void) {
    u32 check = 0;
    s32 range1, range2;
    BOOL overlap;

    if (!CPUFifoReady || !GPFifoReady) {
        return GX_FALSE;
    }

    if (CPUFifo.base == GPFifo.base) {
        check++;
    }

    if (CPUFifo.top == GPFifo.top) {
        check++;
    }

    if (check == 2) {
        return GX_TRUE;
    }

    range1 = (s32)((u8*)CPUFifo.top - (u8*)GPFifo.base);
    range2 = (s32)((u8*)GPFifo.top - (u8*)CPUFifo.base);
    overlap = (range1 > 0 && range2 > 0) || (range1 < 0 && range2 < 0);

    if (overlap) {
        OSReport("CPUFifo: %08X - %08X\n", (u32)CPUFifo.base, (u32)CPUFifo.top);
        OSReport("GP Fifo: %08X - %08X\n", (u32)GPFifo.base, (u32)GPFifo.top);
    }

    return GX_FALSE;
}

void GXSetCPUFifo(GXFifoObj* fifo) {
    const __GXFifoObj* realFifo = (const __GXFifoObj*)fifo;
    BOOL en = OSDisableInterrupts();
    u32 reg;

    if (fifo == NULL) {
        CPUFifoReady = GX_FALSE;
        CPGPLinked = GX_FALSE;
        CPUFifo.bind_cpu = CPUFifo.bind_gp = GX_FALSE;
        OSRestoreInterrupts(en);
        return;
    }

    CPUFifo = *realFifo;
    CPUFifoReady = GX_TRUE;
    CPUFifo.bind_cpu = GX_TRUE;

    if (CPGPLinkCheck()) {
        CPGPLinked = GX_TRUE;
        CPUFifo.bind_gp = GX_TRUE;

        reg = 0;
        GX_PI_REG_WRITE_U32(PI_REG_CPBAS, TOPHYSICAL(CPUFifo.base));
        GX_PI_REG_WRITE_U32(PI_REG_CPTOP, TOPHYSICAL(CPUFifo.top));
        reg = GX_BITSET(reg, 3, 24, TOPHYSICAL(CPUFifo.wrPtr) >> 5);
        reg = GX_BITSET(reg, 2, 1, 0);
        GX_PI_REG_WRITE_U32(PI_REG_CPWRT, reg);

        __GXWriteFifoIntReset(GX_TRUE, GX_TRUE);
        __GXWriteFifoIntEnable(GX_TRUE, GX_FALSE);
        __GXFifoLink(GX_TRUE);
    } else {
        CPUFifo.bind_gp = GX_FALSE;

        if (CPGPLinked) {
            __GXFifoLink(GX_FALSE);
            CPGPLinked = GX_FALSE;
        }

        __GXWriteFifoIntEnable(GX_FALSE, GX_FALSE);

        reg = 0;
        GX_PI_REG_WRITE_U32(PI_REG_CPBAS, TOPHYSICAL(CPUFifo.base));
        GX_PI_REG_WRITE_U32(PI_REG_CPTOP, TOPHYSICAL(CPUFifo.top));
        reg = GX_BITSET(reg, 3, 24, TOPHYSICAL(CPUFifo.wrPtr) >> 5);
        reg = GX_BITSET(reg, 2, 1, 0);
        GX_PI_REG_WRITE_U32(PI_REG_CPWRT, reg);
    }

    PPCSync();
    OSRestoreInterrupts(en);
}

void GXSetGPFifo(const GXFifoObj* fifo) {
    const __GXFifoObj* realFifo = (const __GXFifoObj*)fifo;
    BOOL en;
    u32 stbtmp;

    en = OSDisableInterrupts();
    __GXFifoReadDisable();
    __GXWriteFifoIntEnable(GX_FALSE, GX_FALSE);

    if (fifo == NULL) {
        GPFifoReady = GX_FALSE;
        CPGPLinked = GX_FALSE;
        __GXFifoLink(GX_FALSE);
        GPFifo.bind_gp = GPFifo.bind_cpu = GX_FALSE;
        OSRestoreInterrupts(en);
        return;
    }

    GPFifo = *realFifo;
    GPFifoReady = GX_TRUE;
    GPFifo.bind_gp = GX_TRUE;
    GX_CP_REG_WRITE_U16(CP_FIFO_BASEL, TOPHYSICAL(GPFifo.base));
    GX_CP_REG_WRITE_U16(CP_FIFO_TOPL, TOPHYSICAL(GPFifo.top));
    GX_CP_REG_WRITE_U16(CP_FIFO_COUNTL, GPFifo.count);
    GX_CP_REG_WRITE_U16(CP_FIFO_WPTRL, TOPHYSICAL(GPFifo.wrPtr));
    GX_CP_REG_WRITE_U16(CP_FIFO_RPTRL, TOPHYSICAL(GPFifo.rdPtr));
    GX_CP_REG_WRITE_U16(CP_FIFO_HICNTL, GPFifo.hiWatermark);
    GX_CP_REG_WRITE_U16(CP_FIFO_LOCNTL, GPFifo.loWatermark);
    GX_CP_REG_WRITE_U16(CP_FIFO_BASEH, TOPHYSICAL(GPFifo.base) >> 16);
    GX_CP_REG_WRITE_U16(CP_FIFO_TOPH, TOPHYSICAL(GPFifo.top) >> 16);
    GX_CP_REG_WRITE_U16(CP_FIFO_COUNTH, GPFifo.count >> 16);
    GX_CP_REG_WRITE_U16(CP_FIFO_WPTRH, TOPHYSICAL(GPFifo.wrPtr) >> 16);
    GX_CP_REG_WRITE_U16(CP_FIFO_RPTRH, TOPHYSICAL(GPFifo.rdPtr) >> 16);
    GX_CP_REG_WRITE_U16(CP_FIFO_HICNTH, GPFifo.hiWatermark >> 16);
    GX_CP_REG_WRITE_U16(CP_FIFO_LOCNTH, GPFifo.loWatermark >> 16);

    PPCSync();

    if (CPGPLinkCheck()) {
        CPGPLinked = GX_TRUE;
        GPFifo.bind_cpu = GX_TRUE;
        __GXWriteFifoIntEnable(GX_TRUE, GX_FALSE);
        __GXFifoLink(GX_TRUE);
    } else {
        CPGPLinked = GX_FALSE;
        GPFifo.bind_cpu = GX_FALSE;
        __GXWriteFifoIntEnable(GX_FALSE, GX_FALSE);
        __GXFifoLink(GX_FALSE);
    }

    stbtmp = gxdt->cpCtrlReg;
    stbtmp = GX_BITSET(stbtmp, 30, 1, 0);
    stbtmp = GX_BITSET(stbtmp, 26, 1, 0);
    GX_CP_REG_WRITE_U16(CP_ENABLE, stbtmp);
    GX_CP_REG_WRITE_U16(CP_ENABLE, gxdt->cpCtrlReg);
    __GXWriteFifoIntReset(GX_TRUE, GX_TRUE);
    __GXFifoReadEnable();
    OSRestoreInterrupts(en);
}

static void __GXSaveFifoCPStat(void) {
    u32 temp;

    temp = (u32)GX_CP_REG_READ_U16(CP_FIFO_RPTRH) << 16;
    temp |= (u32)GX_CP_REG_READ_U16(CP_FIFO_RPTRL);
    GPFifo.rdPtr = OSPhysicalToCached(temp);

    temp = (u32)GX_CP_REG_READ_U16(CP_FIFO_COUNTH) << 16;
    temp |= (u32)GX_CP_REG_READ_U16(CP_FIFO_COUNTL);
    GPFifo.count = (s32)temp;
}

static void __GXSaveFifoPIStat(void) {
    u32 reg = GX_PI_REG_READ_U32(PI_REG_CPWRT);

    CPUFifo.wrPtr = OSPhysicalToCached(reg & 0x1FFFFFE0);
    CPUFifo.wrap = (GXBool)((reg & 0x20000000) ? GX_TRUE : GX_FALSE);
}

void __GXSaveFifo(void) {
    BOOL en = OSDisableInterrupts();

    if (CPUFifoReady) {
        __GXSaveFifoPIStat();
    }

    if (GPFifoReady) {
        __GXSaveFifoCPStat();
    }

    if (CPGPLinked) {
        CPUFifo.rdPtr = GPFifo.rdPtr;
        CPUFifo.count = GPFifo.count;
        GPFifo.wrPtr = CPUFifo.wrPtr;
        GPFifo.wrap = CPUFifo.wrap;
    } else if (CPUFifoReady) {
        CPUFifo.count = (s32)CPUFifo.wrPtr - (s32)CPUFifo.rdPtr;
        if (CPUFifo.count < 0) {
            CPUFifo.count += CPUFifo.size;
        }
    }

    OSRestoreInterrupts(en);
}

GXBool __GXIsGPFifoReady(void) {
    return GPFifoReady;
}

void GXGetGPStatus(GXBool* overhi, GXBool* underlow, GXBool* readIdle,
                   GXBool* cmdIdle, GXBool* brkpt) {
    gxdt->cpStatReg = GX_CP_REG_READ_U16(CP_STATUS);
    *overhi = gxdt->cpStatReg & 1;
    *underlow = (gxdt->cpStatReg >> 1) & 1;
    *readIdle = (gxdt->cpStatReg >> 2) & 1;
    *cmdIdle = (gxdt->cpStatReg >> 3) & 1;
    *brkpt = (gxdt->cpStatReg >> 4) & 1;
}

BOOL GXGetCPUFifo(GXFifoObj* fifo) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;

    if (!CPUFifoReady) {
        return GX_FALSE;
    }

    GXFlush();
    __GXSaveFifo();
    *realFifo = CPUFifo;
    return GX_TRUE;
}

BOOL GXGetGPFifo(GXFifoObj* fifo) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;

    if (!GPFifoReady) {
        return GX_FALSE;
    }

    __GXSaveFifo();
    *realFifo = GPFifo;
    return GX_TRUE;
}

void GXGetFifoPtrs(const GXFifoObj* fifo, void** readPtr, void** writePtr) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;

    *readPtr = realFifo->rdPtr;
    *writePtr = realFifo->wrPtr;
}

u32 GXGetFifoCount(GXFifoObj* fifo) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;

    return (u32)realFifo->count;
}

u8 GXGetFifoWrap(GXFifoObj* fifo) {
    __GXFifoObj* realFifo = (__GXFifoObj*)fifo;

    return realFifo->wrap;
}

GXBreakPtCallback GXSetBreakPtCallback(GXBreakPtCallback cb) {
    GXBreakPtCallback oldcb = BreakPointCB;
    BOOL enabled = OSDisableInterrupts();

    BreakPointCB = cb;
    OSRestoreInterrupts(enabled);
    return oldcb;
}

void GXEnableBreakPt(void* break_pt) {
    BOOL enabled = OSDisableInterrupts();

    __GXFifoReadDisable();
    GX_CP_REG_WRITE_U16(CP_FIFO_BRKL, TOPHYSICAL(break_pt));
    GX_CP_REG_WRITE_U16(CP_FIFO_BRKH, TOPHYSICAL(break_pt) >> 16);
    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 30, 1, 0);
    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 26, 1, 0);
    GX_CP_REG_WRITE_U16(CP_ENABLE, gxdt->cpCtrlReg);
    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 30, 1, 1);
    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 26, 1, 1);
    GX_CP_REG_WRITE_U16(CP_ENABLE, gxdt->cpCtrlReg);
    __GXCurrentBP = break_pt;
    __GXFifoReadEnable();
    OSRestoreInterrupts(enabled);
}

void GXDisableBreakPt(void) {
    BOOL enabled = OSDisableInterrupts();

    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 30, 1, 0);
    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 26, 1, 0);
    GX_CP_REG_WRITE_U16(CP_ENABLE, gxdt->cpCtrlReg);
    __GXCurrentBP = NULL;
    OSRestoreInterrupts(enabled);
}

void __GXFifoInit(void) {
    __OSSetInterruptHandler(OS_INTR_PI_CP, GXCPInterruptHandler);
    __OSUnmaskInterrupts(OS_INTR_MASK(OS_INTR_PI_CP));
    __GXCurrentThread = OSGetCurrentThread();
    GXOverflowSuspendInProgress = FALSE;
    memset(&CPUFifo, 0, sizeof(__GXFifoObj));
    memset(&GPFifo, 0, sizeof(__GXFifoObj));
    CPUFifoReady = GX_FALSE;
    GPFifoReady = GX_FALSE;
}

void __GXCleanGPFifo(void) {
    BOOL enabled;

    if (!GPFifoReady) {
        return;
    }

    enabled = OSDisableInterrupts();
    __GXFifoReadDisable();
    __GXWriteFifoIntEnable(GX_FALSE, GX_FALSE);
    GPFifo.rdPtr = GPFifo.wrPtr;
    GPFifo.count = 0;
    GX_CP_REG_WRITE_U16(CP_FIFO_COUNTL, GPFifo.count);
    GX_CP_REG_WRITE_U16(CP_FIFO_WPTRL, TOPHYSICAL(GPFifo.wrPtr));
    GX_CP_REG_WRITE_U16(CP_FIFO_RPTRL, TOPHYSICAL(GPFifo.rdPtr));
    GX_CP_REG_WRITE_U16(CP_FIFO_COUNTH, GPFifo.count >> 16);
    GX_CP_REG_WRITE_U16(CP_FIFO_WPTRH, TOPHYSICAL(GPFifo.wrPtr) >> 16);
    GX_CP_REG_WRITE_U16(CP_FIFO_RPTRH, TOPHYSICAL(GPFifo.rdPtr) >> 16);

    PPCSync();

    if (CPGPLinked) {
        u32 reg = 0;

        CPUFifo.rdPtr = GPFifo.rdPtr;
        CPUFifo.wrPtr = GPFifo.wrPtr;
        CPUFifo.count = GPFifo.count;
        reg = GX_BITSET(reg, 3, 24, TOPHYSICAL(CPUFifo.wrPtr) >> 5);
        reg = GX_BITSET(reg, 2, 1, 0);
        GX_PI_REG_WRITE_U32(PI_REG_CPWRT, reg);
        __GXWriteFifoIntEnable(GX_TRUE, GX_FALSE);
        __GXFifoLink(GX_TRUE);
    }

    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 30, 1, 0);
    gxdt->cpCtrlReg = GX_BITSET(gxdt->cpCtrlReg, 26, 1, 0);
    GX_CP_REG_WRITE_U16(CP_ENABLE, gxdt->cpCtrlReg);
    __GXCurrentBP = NULL;
    __GXWriteFifoIntReset(GX_TRUE, GX_TRUE);
    __GXFifoReadEnable();
    OSRestoreInterrupts(enabled);
}

OSThread* GXGetCurrentGXThread(void) {
    return __GXCurrentThread;
}
