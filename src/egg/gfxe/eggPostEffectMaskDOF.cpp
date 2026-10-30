#include <egg/gfxe/eggPostEffectMaskDOF.h>

#include <egg/gfxe/eggCapTexture.h>
#include <egg/gfxe/eggCpuTexture.h>

#include <revolution/GX.h>
#include <revolution/MTX.h>

namespace EGG {

static const GXColor sCenterColor = {0x00, 0xFF, 0x00, 0xFF};
static const GXColor sFarColor = {0xFF, 0x00, 0x00, 0x00};
static const GXColor sNearColor = {0x00, 0x00, 0x00, 0x00};

PostEffectMaskDOF::PostEffectMaskDOF() {
    unk20 = 8;
    unk24 = 0;
    mCenter = 0.5f;
    mStart = 0.0f;
    mEnd = 1.0f;
    unk40 = 0.0f;
    unk44 = 0.0f;
    unk48 = 0.1f;
    mMode = cMode_3;

    mpGradationTexture = new CpuTexture(64, 64, GX_TF_RGBA8);
    mpGradationTexture->configure();
    mpGradationTexture->alloc();

    mpTexture30 = new CpuTexture(256, 4, GX_TF_RGBA8);
    mpTexture30->configure();
    mpTexture30->alloc();

    mpGradationTexture->setWrapS(GX_CLAMP);
    mpGradationTexture->setWrapT(GX_CLAMP);
    mpTexture30->setWrapS(GX_CLAMP);
    mpTexture30->setWrapT(GX_CLAMP);

    setUpGradation();
}

void PostEffectMaskDOF::setUpGradation() {
    mpGradationTexture->fillGradationTable(
        CpuTexture::GRADATION_QUADRATIC_EASEOUT, 0x73, 0,
        mpGradationTexture->getWidth() / 2, getCenterColor(), getNearColor(),
        false, false);

    mpGradationTexture->fillGradationTable(
        CpuTexture::GRADATION_QUADRATIC, 0x73,
        mpGradationTexture->getWidth() / 2, mpGradationTexture->getWidth(),
        getNearColor(), getFarColor(), false, false);
}

void PostEffectMaskDOF::reset() {
    unk24 = 0;
    mBlendMode = cBlendMode_None;
}

void PostEffectMaskDOF::setUpDepthTable() {
    int width = mpTexture30->getWidth();
    f32 start = mStart - mStart;
    f32 center = mCenter - mStart;
    f32 end = mEnd - mStart;

    for (int i = 0; i < width; i++) {
        f32 t = static_cast<f32>(i) / static_cast<f32>(width - 1);
        f32 ratio = 0.0f;
        GXColor c0, c1;

        if (t >= end) {
            c0 = c1 = getFarColor();
        } else if (t >= center) {
            ratio = (t - center) / (end - center);
            ratio *= ratio;
            ratio *= ratio;
            c0 = getNearColor();
            c1 = getFarColor();
        } else if (t >= start) {
            ratio = 1.0f - (t - start) / (center - start);
            ratio *= ratio;
            ratio *= ratio;
            c0 = getNearColor();
            c1 = getCenterColor();
        } else {
            c0 = c1 = getCenterColor();
        }

        GXColor color;
        color.r = (1.0f - ratio) * c0.r + ratio * c1.r;
        color.g = (1.0f - ratio) * c0.g + ratio * c1.g;
        color.b = (1.0f - ratio) * c0.b + ratio * c1.b;
        color.a = (1.0f - ratio) * c0.a + ratio * c1.a;

        mpTexture30->setColor(i, 0, color);
        mpTexture30->setColor(i, 1, color);
        mpTexture30->setColor(i, 2, color);
        mpTexture30->setColor(i, 3, color);
    }

    mpTexture30->flush();
}

void PostEffectMaskDOF::draw(f32 w, f32 h) {
    if (isVisible()) {
        if (unk20 & 4) {
            setUpDepthTable();
        }

        setMaterialInternal();
        setVtxState();

        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);
        GXSetVtxDesc(GX_VA_TEX1, GX_DIRECT);

        Mtx m;
        PSMTXScale(m, w * mScaleX, h * mScaleY, 1.0f);
        PSMTXTransApply(m, m, mOffsetX, mOffsetY, 0.0f);
        GXLoadPosMtxImm(m, GX_PNMTX0);

        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        {
            GXPosition1x8(0);
            GXTexCoord1x8(0);
            GXTexCoord2f32(unk40, unk44);

            GXPosition1x8(1);
            GXTexCoord1x8(1);
            GXTexCoord2f32(unk40, unk44);

            GXPosition1x8(2);
            GXTexCoord1x8(2);
            GXTexCoord2f32(unk40, unk44);

            GXPosition1x8(3);
            GXTexCoord1x8(3);
            GXTexCoord2f32(unk40, unk44);
        }
        GXEnd();

        GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_FALSE, 64, 0);
        GXSetTexCoordCylWrap(GX_TEXCOORD1, GX_FALSE, GX_FALSE);
    }
}

// STAND-IN (invented, dead-stripped at link; user-approved 2026-10-28): the original's string
// pool holds an unreferenced "m_pDstScreen" assert, so this unit had a function the linker
// stripped. This stand-in only reproduces that string; replace it if the real function is found.
class DOFStringDemo : public PostEffectBase {
public:
    void f(void* m_pDstScreen);
};
void DOFStringDemo::f(void* m_pDstScreen) {
    EGG_ASSERT(getCapTexture());
    EGG_ASSERT(m_pDstScreen);
}

void PostEffectMaskDOF::setMaterialInternal() {
#line 398
    EGG_ASSERT(getCapTexture());

    loadTextureInternal();
    setMatColorChannel();

    GXSetNumTexGens(3);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
                      GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY,
                      GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
                      GX_FALSE, GX_PTIDENTITY);

    GXSetNumIndStages(1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0,
                     getCapTexture()->getLoadMap());
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);

    CpuTexture* pTexture = mpGradationTexture;

    switch (mMode) {
    case cMode_0: {
        GXSetNumTevStages(2);
        GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE,
                              GX_CH_ALPHA);
        GXSetTevDirect(GX_TEVSTAGE0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);

        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP2,
                      GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                        GX_CC_TEXC);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                        GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                        GX_CA_TEXA);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                        GX_TRUE, GX_TEVPREV);
        GXSetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE,
                         GX_ITM_0, GX_ITW_0, GX_ITW_0, GX_TRUE, GX_FALSE,
                         GX_ITBA_OFF);

        f32 range = mEnd - mStart;
        f32 scale = 1.0f / range;
        f32 size = 64.0f;
        int scale_exp = -2;

        while (scale > 0.99902344f) {
            scale *= 0.5f;
            scale_exp++;
            size *= 2.0f;
        }

        while (0.0f < scale && scale < 0.5f) {
            scale *= 2.0f;
            scale_exp--;
            size *= 0.5f;
        }

        if (unk20 & 8) {
            scale = static_cast<int>(scale * 128.0f) / 128.0f;
        }

#line 484
        EGG_ASSERT(-17 <= scale_exp && scale_exp <= 46);

        f32 mtx[2][3];
        mtx[0][0] = scale / 256.0f;
        mtx[0][1] = scale;
        mtx[0][2] = 0.0f;
        mtx[1][2] = 0.0f;
        mtx[1][1] = 0.0f;
        mtx[1][0] = 0.0f;
        GXSetIndTexMtx(GX_ITM_0, mtx, scale_exp);

        unk40 = 32.0f / size + -(mCenter * scale);
        unk44 = 0.0f;

        GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_TRUE, size, 0);
        GXSetTexCoordCylWrap(GX_TEXCOORD1, GX_FALSE, GX_FALSE);
        break;
    }
    case cMode_2: {
        pTexture = mpTexture30;

        GXSetNumTevStages(2);
        GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE,
                              GX_CH_ALPHA);
        GXSetTevDirect(GX_TEVSTAGE0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);

        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP2,
                      GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                        GX_CC_TEXC);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                        GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                        GX_CA_TEXA);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                        GX_TRUE, GX_TEVPREV);
        GXSetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE,
                         GX_ITM_0, GX_ITW_0, GX_ITW_0, GX_TRUE, GX_FALSE,
                         GX_ITBA_OFF);

        f32 mtx[2][3] = {
            {1.0f / 512.0f, 0.5f, 0.0f},
            {0.0f, 0.0f, 0.0f}
        };
        GXSetIndTexMtx(GX_ITM_0, mtx, 1);

        unk40 = -mStart;
        unk44 = 0.0f;
        break;
    }
    case cMode_3: {
        GXSetNumTevStages(1);
        GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE,
                              GX_CH_ALPHA);

        GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP2,
                      GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                        GX_CC_TEXC);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                        GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                        GX_CA_TEXA);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                        GX_TRUE, GX_TEVPREV);
        GXSetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE,
                         GX_ITM_0, GX_ITW_0, GX_ITW_0, GX_FALSE, GX_FALSE,
                         GX_ITBA_OFF);

        f32 mtx[2][3] = {
            {0.0f, 0.5f, 0.0f},
            {0.0f, 0.0f, 0.0f}
        };
        GXSetIndTexMtx(GX_ITM_0, mtx, -1);

        unk40 = 0.0f;
        unk44 = 0.0f;
        break;
    }
    case cMode_1: {
        GXSetNumTevStages(2);
        GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE,
                              GX_CH_ALPHA);
        GXSetTevDirect(GX_TEVSTAGE0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);

        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP2,
                      GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                        GX_CC_TEXC);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                        GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                        GX_CA_TEXA);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                        GX_TRUE, GX_TEVPREV);
        GXSetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE,
                         GX_ITM_0, GX_ITW_0, GX_ITW_0, GX_TRUE, GX_FALSE,
                         GX_ITBA_OFF);

        f32 range = unk48;
        f32 scale = 1.0f / range;
        f32 size = 64.0f;
        int scale_exp = -2;

        while (scale > 0.99902344f) {
            scale *= 0.5f;
            scale_exp++;
            size *= 2.0f;
        }

        while (0.0f < scale && scale < 0.5f) {
            scale *= 2.0f;
            scale_exp--;
            size *= 0.5f;
        }

        if (unk20 & 8) {
            scale = static_cast<int>(scale * 128.0f) / 128.0f;
        }

#line 661
        EGG_ASSERT(-17 <= scale_exp && scale_exp <= 46);

        f32 mtx[2][3];
        mtx[0][0] = scale / 256.0f;
        mtx[0][1] = scale;
        mtx[0][2] = 0.0f;
        mtx[1][0] = scale / 256.0f;
        mtx[1][1] = scale;
        mtx[1][2] = 0.0f;
        GXSetIndTexMtx(GX_ITM_0, mtx, scale_exp);

        unk40 = -mCenter;
        unk44 = -mCenter - unk48;
        break;
    }
    }

    pTexture->load(GX_TEXMAP2);
    setMatPE();
}

const GXColor& PostEffectMaskDOF::getCenterColor() {
    return sCenterColor;
}

const GXColor& PostEffectMaskDOF::getFarColor() {
    return sFarColor;
}

const GXColor& PostEffectMaskDOF::getNearColor() {
    return sNearColor;
}

} // namespace EGG
