#include <egg/gfxe.h>

#include <revolution/GX.h>

namespace EGG {

static inline void SetKColorWhite(GXTevStageID stage) {
    GXColor whiteColor = {1, 1, 1, 0xFF};
    GXSetTevKColorSel(stage, GX_TEV_KCSEL_K1);
    GXSetTevKColor(GX_KCOLOR1, whiteColor);
}

static inline void SetMaskStage(GXTevStageID stage, GXTexMapID map, GXColor color) {
    GXSetTevOrder(stage, GX_TEXCOORD0, map, GX_COLOR_NULL);
    GXSetTevKColorSel(stage, GX_TEV_KCSEL_K0);
    GXSetTevKAlphaSel(stage, GX_TEV_KASEL_K0_A);
    GXSetTevKColor(GX_KCOLOR0, color);
    GXSetTevColorIn(stage, GX_CC_KONST, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(stage, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(stage, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(stage, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

PostEffectMask::PostEffectMask() {}

void PostEffectMask::reset() {
    mColor = DrawGX::WHITE;
    mIntensity = 0.0f;
    mpDstScreen = NULL;
    unk30 = 1.0f;
    unk20 = 0;
}

void PostEffectMask::setMaterialInternal() {
    u32 hasScreen = mpDstScreen != NULL;
    loadTextureInternal();

    if (hasScreen) {
#line 112
        EGG_ASSERT(getCapTexture()->getLoadMap() != mpDstScreen->getLoadMap());
        mpDstScreen->load(mpDstScreen->getLoadMap());
    }

    GXColor maskColor;
    scaleRGBA(&maskColor, mColor, mIntensity, false);

    setMatColorChannel();

    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    setMatInd();

    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE,
                          GX_CH_ALPHA);

    switch (unk20) {
    case 0: {
        u8 stageNum = static_cast<u8>(hasScreen + 1);
        GXSetNumTevStages(stageNum);

        for (int i = 0; i < stageNum; i++) {
            GXSetTevDirect(static_cast<GXTevStageID>(i));
            GXSetTevSwapMode(static_cast<GXTevStageID>(i), GX_TEV_SWAP0,
                             GX_TEV_SWAP0);

            if (i == 0) {
                SetMaskStage(static_cast<GXTevStageID>(i), getCapTexture()->getLoadMap(), maskColor);
            } else {
                GXSetTevOrder(static_cast<GXTevStageID>(i), GX_TEXCOORD0,
                              mpDstScreen->getLoadMap(), GX_COLOR_NULL);
                SetKColorWhite(static_cast<GXTevStageID>(i));
                GXSetTevColorIn(static_cast<GXTevStageID>(i), GX_CC_ZERO,
                                GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
                GXSetTevColorOp(static_cast<GXTevStageID>(i), GX_TEV_ADD,
                                GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                                GX_TEVPREV);
                GXSetTevAlphaIn(static_cast<GXTevStageID>(i), GX_CA_ZERO,
                                GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
                GXSetTevAlphaOp(static_cast<GXTevStageID>(i), GX_TEV_ADD,
                                GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                                GX_TEVPREV);
            }
        }

        if (!hasScreen) {
            setBlendMode(cBlendMode_Mul);
        } else {
            setBlendMode(cBlendMode_None);
        }
        break;
    }
    case 1: {
        GXSetNumTevStages(2);

        for (int i = 0; i < 2; i++) {
            GXSetTevDirect(static_cast<GXTevStageID>(i));
            GXSetTevSwapMode(static_cast<GXTevStageID>(i), GX_TEV_SWAP0,
                             GX_TEV_SWAP0);

            if (i == 0) {
                SetMaskStage(static_cast<GXTevStageID>(i), getCapTexture()->getLoadMap(), maskColor);
            } else {
                GXSetTevOrder(static_cast<GXTevStageID>(i), GX_TEXCOORD0,
                              getCapTexture()->getLoadMap(), GX_COLOR_NULL);
                GXSetTevColorIn(static_cast<GXTevStageID>(i), GX_CC_ZERO,
                                GX_CC_CPREV, GX_CC_TEXA, GX_CC_ZERO);
                GXSetTevColorOp(static_cast<GXTevStageID>(i), GX_TEV_ADD,
                                GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE,
                                GX_TEVPREV);
                GXSetTevAlphaIn(static_cast<GXTevStageID>(i), GX_CA_ZERO,
                                GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
                GXSetTevAlphaOp(static_cast<GXTevStageID>(i), GX_TEV_ADD,
                                GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                                GX_TEVPREV);
            }
        }

        setBlendMode(cBlendMode_Mul);
        break;
    }
    }

    setMatPE();
}

} // namespace EGG
