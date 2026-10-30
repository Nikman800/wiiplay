#include "eggPostEffectBlur.h"

#include "eggCapTexture.h"
#include "eggDrawGX.h"
#include "eggGXUtility.h"

#include <revolution/GX.h>

namespace EGG {
using namespace nw4r;

PostEffectBlur::PostEffectBlur() {}

void PostEffectBlur::reset() {
    BlurData* p = mBlurData;
    for (int i = 0; i < 4; i++, p++) {
        p->reset();
    }

    mNumBlurData = 1;
    mColorScale = 1.0f;
    WORD_0x24 = 0;
}

void PostEffectBlur::draw(f32 width, f32 height) {
    if (isVisible()) {
        loadTextureInternal();
        setMatColorChannel();
        setMatInd();
        setMatPE();
        setVtxState();

        for (u8 i = 0; i < mNumBlurData; i++) {
            int end = (mBlurData[i].BYTE_0x0 - 1) / 8;
            for (u8 j = 0; j <= end; j++) {
                drawBlurPass(i, j);
                drawScreenInternal(mOffsetX, mOffsetY, width * mScaleX,
                                   height * mScaleY);
                DrawGX::SetBlendMode(DrawGX::BLEND_ADD);
            }
        }
    }
}

// https://decomp.me/scratch/pbk8u
void PostEffectBlur::drawBlurPass(u8 idx, u8 pass) {
    BlurData& blurData = mBlurData[idx];

    u8 num = blurData.BYTE_0x0 - (u8)(pass * 8);
    if (num > 8) {
        num = 8;
    }

    CapTexture* capTexture = getCapTexture();
    f32 invWidth = 1.0f / capTexture->getWidth();
    f32 invHeight = 1.0f / capTexture->getHeight();

    GXSetNumTexGens(num);

    int angle = pass * 8;
    for (int i = 0; i < num; angle++, i++) {
        u32 texMtxID = GXUtility::getTexMtxID(i);
        GXSetTexCoordGen2(static_cast<GXTexCoordID>(i), GX_TG_MTX2x4,
                          GX_TG_TEX0, texMtxID, FALSE, GX_PTIDENTITY);

        f32 sin;
        f32 cos;
        math::SinCosFIdx(
            &sin, &cos,
            40.743664f *
                (6.2831855f * (blurData.FLOAT_0x8 +
                               static_cast<f32>(angle) / blurData.BYTE_0x0)));

        f32 offsetX = invWidth * (cos * blurData.mRadius);
        f32 offsetY = invHeight * (sin * blurData.mRadius);

        f32 scale = 1.0f;
        f32 rotate = 0.0f;
        math::VEC3 center(0.0f, 0.0f, 0.0f);
        f32 rotSin;
        f32 rotCos;
        math::SinCosFIdx(&rotSin, &rotCos, 40.743664f * rotate);

        f32 a = scale * rotCos;
        f32 b = scale * rotSin;
        f32 mtx[2][4] = {
            {a, -b, offsetX + (center.x + (-a * center.x + b * center.y)), 0.0f},
            {b, a, offsetY + (center.y + (-b * center.x - a * center.y)), 0.0f},
        };
        GXLoadTexMtxImm(mtx, texMtxID, GX_MTX_2x4);
    }

    GXColor color;
    scaleRGBA(&color, blurData.mColor, mColorScale, true);
    GXSetTevKColor(GX_KCOLOR0, color);

    GXTevColorArg colorArg = WORD_0x24 == 0 ? GX_CC_ZERO : GX_CC_ONE;
    GXTevAlphaArg alphaArg = WORD_0x24 == 0 ? GX_CA_ZERO : GX_CA_KONST;

    GXSetNumTevStages(num);
    for (int stage = 0; stage < num; stage++) {
        GXSetTevDirect(static_cast<GXTevStageID>(stage));
        GXSetTevSwapMode(static_cast<GXTevStageID>(stage), GX_TEV_SWAP0,
                         GX_TEV_SWAP0);

        GXTevColorArg stageColorArg = stage > 0 ? GX_CC_CPREV : colorArg;
        GXTevAlphaArg stageAlphaArg = stage > 0 ? GX_CA_APREV : alphaArg;

        GXSetTevKColorSel(static_cast<GXTevStageID>(stage), GX_TEV_KCSEL_K0);
        GXSetTevKAlphaSel(static_cast<GXTevStageID>(stage),
                          GX_TEV_KASEL_K0_A);
        GXSetTevOrder(static_cast<GXTevStageID>(stage),
                      static_cast<GXTexCoordID>(stage),
                      getCapTexture()->getLoadMap(), GX_COLOR_NULL);

        if (WORD_0x24 == 0) {
            GXSetTevColorIn(static_cast<GXTevStageID>(stage), GX_CC_ZERO,
                            GX_CC_TEXC, GX_CC_KONST, stageColorArg);
            GXSetTevAlphaIn(static_cast<GXTevStageID>(stage), GX_CA_ZERO,
                            GX_CA_TEXA, GX_CA_KONST, stageAlphaArg);
        } else {
            GXSetTevColorIn(static_cast<GXTevStageID>(stage), GX_CC_KONST,
                            GX_CC_ZERO, GX_CC_TEXC, stageColorArg);
            GXSetTevAlphaIn(static_cast<GXTevStageID>(stage), GX_CA_KONST,
                            GX_CA_ZERO, GX_CA_TEXA, stageAlphaArg);
        }

        GXSetTevColorOp(static_cast<GXTevStageID>(stage),
                        static_cast<GXTevOp>(WORD_0x24), GX_TB_ZERO,
                        static_cast<GXTevScale>(blurData.WORD_0x10), TRUE,
                        GX_TEVPREV);
        GXSetTevAlphaOp(static_cast<GXTevStageID>(stage),
                        static_cast<GXTevOp>(WORD_0x24), GX_TB_ZERO,
                        static_cast<GXTevScale>(blurData.WORD_0x10), TRUE,
                        GX_TEVPREV);
    }
}
} // namespace EGG
