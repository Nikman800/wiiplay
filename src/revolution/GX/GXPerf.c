#include <revolution/GX.h>

extern volatile void* __cpReg;

#define GX_CP_REG_WRITE_U16(i, x) (((volatile u16*)__cpReg)[(i)] = (u16)(x))

typedef enum _GXPerf0 {
    GX_PERF0_VERTICES,
    GX_PERF0_CLIP_VTX,
    GX_PERF0_CLIP_CLKS,
    GX_PERF0_XF_WAIT_IN,
    GX_PERF0_XF_WAIT_OUT,
    GX_PERF0_XF_XFRM_CLKS,
    GX_PERF0_XF_LIT_CLKS,
    GX_PERF0_XF_BOT_CLKS,
    GX_PERF0_XF_REGLD_CLKS,
    GX_PERF0_XF_REGRD_CLKS,
    GX_PERF0_CLIP_RATIO,
    GX_PERF0_TRIANGLES,
    GX_PERF0_TRIANGLES_CULLED,
    GX_PERF0_TRIANGLES_PASSED,
    GX_PERF0_TRIANGLES_SCISSORED,
    GX_PERF0_TRIANGLES_0TEX,
    GX_PERF0_TRIANGLES_1TEX,
    GX_PERF0_TRIANGLES_2TEX,
    GX_PERF0_TRIANGLES_3TEX,
    GX_PERF0_TRIANGLES_4TEX,
    GX_PERF0_TRIANGLES_5TEX,
    GX_PERF0_TRIANGLES_6TEX,
    GX_PERF0_TRIANGLES_7TEX,
    GX_PERF0_TRIANGLES_8TEX,
    GX_PERF0_TRIANGLES_0CLR,
    GX_PERF0_TRIANGLES_1CLR,
    GX_PERF0_TRIANGLES_2CLR,
    GX_PERF0_QUAD_0CVG,
    GX_PERF0_QUAD_NON0CVG,
    GX_PERF0_QUAD_1CVG,
    GX_PERF0_QUAD_2CVG,
    GX_PERF0_QUAD_3CVG,
    GX_PERF0_QUAD_4CVG,
    GX_PERF0_AVG_QUAD_CNT,
    GX_PERF0_CLOCKS,
    GX_PERF0_NONE,
} GXPerf0;

typedef enum _GXPerf1 {
    GX_PERF1_TEXELS,
    GX_PERF1_TX_IDLE,
    GX_PERF1_TX_REGS,
    GX_PERF1_TX_MEMSTALL,
    GX_PERF1_TC_CHECK1_2,
    GX_PERF1_TC_CHECK3_4,
    GX_PERF1_TC_CHECK5_6,
    GX_PERF1_TC_CHECK7_8,
    GX_PERF1_TC_MISS,
    GX_PERF1_VC_ELEMQ_FULL,
    GX_PERF1_VC_MISSQ_FULL,
    GX_PERF1_VC_MEMREQ_FULL,
    GX_PERF1_VC_STATUS7,
    GX_PERF1_VC_MISSREP_FULL,
    GX_PERF1_VC_STREAMBUF_LOW,
    GX_PERF1_VC_ALL_STALLS,
    GX_PERF1_VERTICES,
    GX_PERF1_FIFO_REQ,
    GX_PERF1_CALL_REQ,
    GX_PERF1_VC_MISS_REQ,
    GX_PERF1_CP_ALL_REQ,
    GX_PERF1_CLOCKS,
    GX_PERF1_NONE,
} GXPerf1;

void GXSetGPMetric(GXPerf0 perf0, GXPerf1 perf1) {
    u32 reg;

    switch (gxdt->perf0) {
    case GX_PERF0_VERTICES:
    case GX_PERF0_CLIP_VTX:
    case GX_PERF0_CLIP_CLKS:
    case GX_PERF0_XF_WAIT_IN:
    case GX_PERF0_XF_WAIT_OUT:
    case GX_PERF0_XF_XFRM_CLKS:
    case GX_PERF0_XF_LIT_CLKS:
    case GX_PERF0_XF_BOT_CLKS:
    case GX_PERF0_XF_REGLD_CLKS:
    case GX_PERF0_XF_REGRD_CLKS:
    case GX_PERF0_CLOCKS:
    case GX_PERF0_CLIP_RATIO:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0);
        break;
    case GX_PERF0_TRIANGLES:
    case GX_PERF0_TRIANGLES_CULLED:
    case GX_PERF0_TRIANGLES_PASSED:
    case GX_PERF0_TRIANGLES_SCISSORED:
    case GX_PERF0_TRIANGLES_0TEX:
    case GX_PERF0_TRIANGLES_1TEX:
    case GX_PERF0_TRIANGLES_2TEX:
    case GX_PERF0_TRIANGLES_3TEX:
    case GX_PERF0_TRIANGLES_4TEX:
    case GX_PERF0_TRIANGLES_5TEX:
    case GX_PERF0_TRIANGLES_6TEX:
    case GX_PERF0_TRIANGLES_7TEX:
    case GX_PERF0_TRIANGLES_8TEX:
    case GX_PERF0_TRIANGLES_0CLR:
    case GX_PERF0_TRIANGLES_1CLR:
    case GX_PERF0_TRIANGLES_2CLR:
        GX_BP_LOAD_REG(0x23000000);
        break;
    case GX_PERF0_QUAD_0CVG:
    case GX_PERF0_QUAD_NON0CVG:
    case GX_PERF0_QUAD_1CVG:
    case GX_PERF0_QUAD_2CVG:
    case GX_PERF0_QUAD_3CVG:
    case GX_PERF0_QUAD_4CVG:
    case GX_PERF0_AVG_QUAD_CNT:
        GX_BP_LOAD_REG(0x24000000);
        break;
    case GX_PERF0_NONE:
    default:
        break;
    }

    switch (gxdt->perf1) {
    case GX_PERF1_TEXELS:
    case GX_PERF1_TX_IDLE:
    case GX_PERF1_TX_REGS:
    case GX_PERF1_TX_MEMSTALL:
    case GX_PERF1_TC_MISS:
    case GX_PERF1_CLOCKS:
    case GX_PERF1_TC_CHECK1_2:
    case GX_PERF1_TC_CHECK3_4:
    case GX_PERF1_TC_CHECK5_6:
    case GX_PERF1_TC_CHECK7_8:
        GX_BP_LOAD_REG(0x67000000);
        break;
    case GX_PERF1_VC_ELEMQ_FULL:
    case GX_PERF1_VC_MISSQ_FULL:
    case GX_PERF1_VC_MEMREQ_FULL:
    case GX_PERF1_VC_STATUS7:
    case GX_PERF1_VC_MISSREP_FULL:
    case GX_PERF1_VC_STREAMBUF_LOW:
    case GX_PERF1_VC_ALL_STALLS:
    case GX_PERF1_VERTICES:
        gxdt->perfSel = GX_BITSET(gxdt->perfSel, 24, 4, 0);
        GX_CP_LOAD_REG(0x20, gxdt->perfSel);
        break;
    case GX_PERF1_FIFO_REQ:
    case GX_PERF1_CALL_REQ:
    case GX_PERF1_VC_MISS_REQ:
    case GX_PERF1_CP_ALL_REQ:
        reg = 0;
        GX_CP_REG_WRITE_U16(3, reg);
        break;
    case GX_PERF1_NONE:
    default:
        break;
    }

    gxdt->perf0 = perf0;
    switch (gxdt->perf0) {
    case GX_PERF0_VERTICES:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0x273);
        break;
    case GX_PERF0_CLIP_VTX:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0x14A);
        break;
    case GX_PERF0_CLIP_CLKS:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0x16B);
        break;
    case GX_PERF0_XF_WAIT_IN:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0x84);
        break;
    case GX_PERF0_XF_WAIT_OUT:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0xC6);
        break;
    case GX_PERF0_XF_XFRM_CLKS:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0x210);
        break;
    case GX_PERF0_XF_LIT_CLKS:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0x252);
        break;
    case GX_PERF0_XF_BOT_CLKS:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0x231);
        break;
    case GX_PERF0_XF_REGLD_CLKS:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0x1AD);
        break;
    case GX_PERF0_XF_REGRD_CLKS:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0x1CE);
        break;
    case GX_PERF0_CLOCKS:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0x21);
        break;
    case GX_PERF0_CLIP_RATIO:
        GX_XF_LOAD_REG(GX_XF_REG_PERF0, 0x153);
        break;
    case GX_PERF0_TRIANGLES:
        GX_BP_LOAD_REG(0x2300AE7F);
        break;
    case GX_PERF0_TRIANGLES_CULLED:
        GX_BP_LOAD_REG(0x23008E7F);
        break;
    case GX_PERF0_TRIANGLES_PASSED:
        GX_BP_LOAD_REG(0x23009E7F);
        break;
    case GX_PERF0_TRIANGLES_SCISSORED:
        GX_BP_LOAD_REG(0x23001E7F);
        break;
    case GX_PERF0_TRIANGLES_0TEX:
        GX_BP_LOAD_REG(0x2300AC3F);
        break;
    case GX_PERF0_TRIANGLES_1TEX:
        GX_BP_LOAD_REG(0x2300AC7F);
        break;
    case GX_PERF0_TRIANGLES_2TEX:
        GX_BP_LOAD_REG(0x2300ACBF);
        break;
    case GX_PERF0_TRIANGLES_3TEX:
        GX_BP_LOAD_REG(0x2300ACFF);
        break;
    case GX_PERF0_TRIANGLES_4TEX:
        GX_BP_LOAD_REG(0x2300AD3F);
        break;
    case GX_PERF0_TRIANGLES_5TEX:
        GX_BP_LOAD_REG(0x2300AD7F);
        break;
    case GX_PERF0_TRIANGLES_6TEX:
        GX_BP_LOAD_REG(0x2300ADBF);
        break;
    case GX_PERF0_TRIANGLES_7TEX:
        GX_BP_LOAD_REG(0x2300ADFF);
        break;
    case GX_PERF0_TRIANGLES_8TEX:
        GX_BP_LOAD_REG(0x2300AE3F);
        break;
    case GX_PERF0_TRIANGLES_0CLR:
        GX_BP_LOAD_REG(0x2300A27F);
        break;
    case GX_PERF0_TRIANGLES_1CLR:
        GX_BP_LOAD_REG(0x2300A67F);
        break;
    case GX_PERF0_TRIANGLES_2CLR:
        GX_BP_LOAD_REG(0x2300AA7F);
        break;
    case GX_PERF0_QUAD_0CVG:
        GX_BP_LOAD_REG(0x2402C0C6);
        break;
    case GX_PERF0_QUAD_NON0CVG:
        GX_BP_LOAD_REG(0x2402C16B);
        break;
    case GX_PERF0_QUAD_1CVG:
        GX_BP_LOAD_REG(0x2402C0E7);
        break;
    case GX_PERF0_QUAD_2CVG:
        GX_BP_LOAD_REG(0x2402C108);
        break;
    case GX_PERF0_QUAD_3CVG:
        GX_BP_LOAD_REG(0x2402C129);
        break;
    case GX_PERF0_QUAD_4CVG:
        GX_BP_LOAD_REG(0x2402C14A);
        break;
    case GX_PERF0_AVG_QUAD_CNT:
        GX_BP_LOAD_REG(0x2402C1AD);
        break;
    case GX_PERF0_NONE:
        break;
    default:
        break;
    }

    gxdt->perf1 = perf1;
    switch (gxdt->perf1) {
    case GX_PERF1_TEXELS:
        GX_BP_LOAD_REG(0x67000042);
        break;
    case GX_PERF1_TX_IDLE:
        GX_BP_LOAD_REG(0x67000084);
        break;
    case GX_PERF1_TX_REGS:
        GX_BP_LOAD_REG(0x67000063);
        break;
    case GX_PERF1_TX_MEMSTALL:
        GX_BP_LOAD_REG(0x67000129);
        break;
    case GX_PERF1_TC_MISS:
        GX_BP_LOAD_REG(0x67000252);
        break;
    case GX_PERF1_CLOCKS:
        GX_BP_LOAD_REG(0x67000021);
        break;
    case GX_PERF1_TC_CHECK1_2:
        GX_BP_LOAD_REG(0x6700014B);
        break;
    case GX_PERF1_TC_CHECK3_4:
        GX_BP_LOAD_REG(0x6700018D);
        break;
    case GX_PERF1_TC_CHECK5_6:
        GX_BP_LOAD_REG(0x670001CF);
        break;
    case GX_PERF1_TC_CHECK7_8:
        GX_BP_LOAD_REG(0x67000211);
        break;
    case GX_PERF1_VC_ELEMQ_FULL:
        gxdt->perfSel = GX_BITSET(gxdt->perfSel, 24, 4, 2);
        GX_CP_LOAD_REG(0x20, gxdt->perfSel);
        break;
    case GX_PERF1_VC_MISSQ_FULL:
        gxdt->perfSel = GX_BITSET(gxdt->perfSel, 24, 4, 3);
        GX_CP_LOAD_REG(0x20, gxdt->perfSel);
        break;
    case GX_PERF1_VC_MEMREQ_FULL:
        gxdt->perfSel = GX_BITSET(gxdt->perfSel, 24, 4, 4);
        GX_CP_LOAD_REG(0x20, gxdt->perfSel);
        break;
    case GX_PERF1_VC_STATUS7:
        gxdt->perfSel = GX_BITSET(gxdt->perfSel, 24, 4, 5);
        GX_CP_LOAD_REG(0x20, gxdt->perfSel);
        break;
    case GX_PERF1_VC_MISSREP_FULL:
        gxdt->perfSel = GX_BITSET(gxdt->perfSel, 24, 4, 6);
        GX_CP_LOAD_REG(0x20, gxdt->perfSel);
        break;
    case GX_PERF1_VC_STREAMBUF_LOW:
        gxdt->perfSel = GX_BITSET(gxdt->perfSel, 24, 4, 7);
        GX_CP_LOAD_REG(0x20, gxdt->perfSel);
        break;
    case GX_PERF1_VC_ALL_STALLS:
        gxdt->perfSel = GX_BITSET(gxdt->perfSel, 24, 4, 9);
        GX_CP_LOAD_REG(0x20, gxdt->perfSel);
        break;
    case GX_PERF1_VERTICES:
        gxdt->perfSel = GX_BITSET(gxdt->perfSel, 24, 4, 8);
        GX_CP_LOAD_REG(0x20, gxdt->perfSel);
        break;
    case GX_PERF1_FIFO_REQ:
        reg = 2;
        GX_CP_REG_WRITE_U16(3, reg);
        break;
    case GX_PERF1_CALL_REQ:
        reg = 3;
        GX_CP_REG_WRITE_U16(3, reg);
        break;
    case GX_PERF1_VC_MISS_REQ:
        reg = 4;
        GX_CP_REG_WRITE_U16(3, reg);
        break;
    case GX_PERF1_CP_ALL_REQ:
        reg = 5;
        GX_CP_REG_WRITE_U16(3, reg);
        break;
    case GX_PERF1_NONE:
        break;
    default:
        break;
    }

    gxdt->lastWriteWasXF = FALSE;
}

void GXClearGPMetric(void) {
    GX_CP_REG_WRITE_U16(2, 4);
}
