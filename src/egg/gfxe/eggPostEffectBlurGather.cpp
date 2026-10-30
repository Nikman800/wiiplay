#include "eggPostEffectBlurGather.h"
#include "eggDrawGX.h"
#include "eggCapTexture.h"
#include <revolution/GX.h>

namespace EGG
{
    static const f32 COLOR_SCALE_ONE = 1.0f;
    static const f32 RESET_COLOR_SCALE = 0.5f;
    static const f32 SCALE_255 = 255.0f;
    static const f32 SCALE_ZERO = 0.0f;

    enum {
        BLUR_CC_CPREV = 2,
        BLUR_CC_TEXC = 8,
        BLUR_CC_RASA = 8,
        BLUR_CC_HALF = 14,
        BLUR_CC_KONST = 14,
        BLUR_CC_ZERO = 15,
        BLUR_CA_APREV = 0,
        BLUR_CA_TEXA = 4,
        BLUR_CA_RASA = 4,
        BLUR_CA_KONST = 6,
        BLUR_CA_ZERO = 7,
        BLUR_TEV_ADD = 0,
        BLUR_TEV_SUB = 1,
        BLUR_CS_SCALE_1 = 0,
        BLUR_CS_SCALE_2 = 2,
        BLUR_KCSEL_K2 = 14,
        BLUR_KASEL_K2_A = 30,
        BLUR_COLOR0A0 = 4
    };

    PostEffectBlurGather::PostEffectBlurGather()
    {
    }

    void PostEffectBlurGather::reset()
    {
        BlurGatherData* it = mBlurGatherData;
        for (int i = 0; i < eCnstBlurGather_Max; i++, it++){
            it->reset();
        }

        mNumBlurGatherData = 1;
        mFlags = 0x1;
        FLOAT_0x24 = RESET_COLOR_SCALE;
    }

    // https://decomp.me/scratch/7ue6S
    void PostEffectBlurGather::setMaterialInternal()
    {
        int useGatherTexture = mpCapTexture != NULL && (mFlags & 0x1) == 0;
        int numTevStages = mNumBlurGatherData +
                          (static_cast<u8>(useGatherTexture) << 1);

        BlurGatherData* pData;
        setMatColorChannel();
        u8 numTexGens = mNumBlurGatherData + useGatherTexture;
        GXSetNumTexGens(numTexGens);

        for (int i = 0; i < numTexGens; i++) {
            GXSetTexCoordGen2(static_cast<GXTexCoordID>(i), GX_TG_MTX2x4,
                              GX_TG_TEX0, 60, GX_FALSE, 125);

            if (i < mNumBlurGatherData) {
                mBlurGatherData[i].mpCapTexture->load(mBlurGatherData[i].mpCapTexture->getLoadMap());
            } else {
                loadTextureInternal();
            }
        }

        GXColor scaleColor;
        scaleColor.r = SCALE_255 * FLOAT_0x24;
        scaleColor.g = SCALE_255 * FLOAT_0x24;
        scaleColor.b = SCALE_255 * FLOAT_0x24;
        scaleColor.a = SCALE_255 * FLOAT_0x24;

        setMatInd();
        GXSetNumTevStages(numTevStages);
        pData = mBlurGatherData;

        for (int i = 0; i < numTevStages; i++) {
            GXSetTevDirect(static_cast<GXTevStageID>(i));
            GXSetTevSwapMode(static_cast<GXTevStageID>(i), GX_TEV_SWAP0,
                             GX_TEV_SWAP0);

            if (i < mNumBlurGatherData) {
                GXColor color;
                scaleRGBA(&color, pData->mColor, pData->mColorScale, false);
                GXSetTevKColorSel(static_cast<GXTevStageID>(i),
                                  static_cast<GXTevKColorSel>(GX_TEV_KCSEL_K0 + i));
                GXSetTevKAlphaSel(static_cast<GXTevStageID>(i),
                                  static_cast<GXTevKAlphaSel>(GX_TEV_KASEL_K0_A + i));
                GXSetTevKColor(static_cast<GXTevKColorID>(i), color);
                GXSetTevOrder(static_cast<GXTevStageID>(i),
                              static_cast<GXTexCoordID>(i),
                              pData->mpCapTexture->getLoadMap(), GX_COLOR_NULL);

                GXSetTevColorIn(static_cast<GXTevStageID>(i),
                                GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST,
                                i == 0 ? GX_CC_ZERO : GX_CC_CPREV);
                GXSetTevColorOp(static_cast<GXTevStageID>(i), GX_TEV_ADD,
                                GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                                static_cast<GXTevRegID>(i == mNumBlurGatherData - 1));

                GXSetTevAlphaIn(static_cast<GXTevStageID>(i),
                                GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST,
                                i == 0 ? GX_CA_ZERO : GX_CA_APREV);
                GXSetTevAlphaOp(static_cast<GXTevStageID>(i), GX_TEV_ADD,
                                GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                                static_cast<GXTevRegID>(i == mNumBlurGatherData - 1));
            } else if (i == mNumBlurGatherData) {
                GXSetTevKColorSel(static_cast<GXTevStageID>(i),
                                  GX_TEV_KCSEL_K2);
                GXSetTevKAlphaSel(static_cast<GXTevStageID>(i),
                                  GX_TEV_KASEL_K2_A);
                GXSetTevKColor(GX_KCOLOR2, scaleColor);
                GXSetTevOrder(static_cast<GXTevStageID>(i),
                              static_cast<GXTexCoordID>(i),
                              mpCapTexture->getLoadMap(), GX_COLOR_NULL);
                GXSetTevColorIn(static_cast<GXTevStageID>(i),
                                GX_CC_KONST, GX_CC_ZERO, GX_CC_ZERO,
                                GX_CC_TEXC);
                GXSetTevColorOp(static_cast<GXTevStageID>(i), GX_TEV_SUB,
                                GX_TB_ZERO,
                                static_cast<GXTevScale>(BLUR_CS_SCALE_2),
                                GX_TRUE, GX_TEVPREV);
                GXSetTevAlphaIn(static_cast<GXTevStageID>(i),
                                GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO,
                                static_cast<GXTevAlphaArg>(BLUR_CA_RASA));
                GXSetTevAlphaOp(static_cast<GXTevStageID>(i), GX_TEV_SUB,
                                GX_TB_ZERO,
                                static_cast<GXTevScale>(BLUR_CS_SCALE_2),
                                GX_TRUE, GX_TEVPREV);
            } else {
                GXSetTevOrder(static_cast<GXTevStageID>(i), GX_TEXCOORD_NULL,
                              GX_TEXMAP_NULL, GX_COLOR_NULL);
                GXSetTevColorIn(static_cast<GXTevStageID>(i), GX_CC_C0,
                                GX_CC_ZERO, GX_CC_CPREV, GX_CC_ZERO);
                GXSetTevColorOp(static_cast<GXTevStageID>(i), GX_TEV_ADD,
                                GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
                GXSetTevAlphaIn(static_cast<GXTevStageID>(i), GX_CA_A0,
                                GX_CA_ZERO, GX_CA_APREV, GX_CA_ZERO);
                GXSetTevAlphaOp(static_cast<GXTevStageID>(i), GX_TEV_ADD,
                                GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            }
            pData++;
        }

        setMatPE();
    }
}
