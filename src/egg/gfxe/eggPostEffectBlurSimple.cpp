#include "eggPostEffectBlurSimple.h"

#include "eggCapTexture.h"
#include "eggGXUtility.h"

#include <nw4r/math.h>

#include <revolution/GX.h>

namespace EGG {

// 2x4 texture matrix: scale (sx, sy), rotation about a center, translation.
// The center goes through a VEC3 in memory (two `frsp` on its zero value),
// and sin/cos live in one array (stack order cos, sin below the center).
static inline void calcTexMtx(f32 mtx[2][4], f32 sx, f32 sy, f32 rot, f32 cx,
                              f32 cy, f32 cz, f32 tx, f32 ty) {
    nw4r::math::VEC3 center(cx, cy, cz);
    f32 cs[2];
    nw4r::math::SinCosRad(&cs[1], &cs[0], rot);
    f32 sc = sx * cs[0];
    f32 ss = sy * cs[1];
    mtx[0][0] = sc;
    mtx[0][1] = -ss;
    mtx[1][0] = ss;
    mtx[1][1] = sc;
    mtx[1][3] = 0.0f;
    mtx[0][3] = 0.0f;
    mtx[0][2] = tx + (center.x + (-sc * center.x + ss * center.y));
    mtx[1][2] = ty + (center.y + (-ss * center.x - sc * center.y));
}

// The header keeps these members as raw bytes (unk20) for now:
// 0x20 f32 radius, 0x24 GXColor, 0x28 u8 intensity, 0x2C GXTevScale,
// 0x30 u8 (use only two texture coordinates).
static inline f32& Radius(PostEffectBlurSimple* p) {
    return *reinterpret_cast<f32*>(&p->unk20[0x20 - 0x20]);
}
static inline GXColor& Color(PostEffectBlurSimple* p) {
    return *reinterpret_cast<GXColor*>(&p->unk20[0x24 - 0x20]);
}
static inline u8& Intensity(PostEffectBlurSimple* p) {
    return *reinterpret_cast<u8*>(&p->unk20[0x28 - 0x20]);
}
static inline u32& Unk2C(PostEffectBlurSimple* p) {
    return *reinterpret_cast<u32*>(&p->unk20[0x2C - 0x20]);
}
static inline u8& Unk30(PostEffectBlurSimple* p) {
    return *reinterpret_cast<u8*>(&p->unk20[0x30 - 0x20]);
}

PostEffectBlurSimple::PostEffectBlurSimple() {}

void PostEffectBlurSimple::reset() {
    setIntensity(0x80);
    setRadius(1.0f);
    setUnk2C(GX_CS_SCALE_4);
    const GXColor white = {255, 255, 255, 255};
    Color(this) = white;
    setUnk30(true);
    mBlendMode = cBlendMode_AddAlpha;
}

void PostEffectBlurSimple::setMaterialInternal() {
    loadTextureInternal();
    setMatColorChannel();
    setMatInd();

    u8 numStages = 2 + (Unk30(this) == 0);
    GXSetNumTexGens(numStages);
    GXSetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    f32 scale = Radius(this) / mpCapTexture->getWidth();

    for (int i = 0; i < 2; i++) {
        // The original keeps the ID unmasked: getTexMtxID returns u32 there.
        u32 texMtxID = GXUtility::getTexMtxID(i);

        f32 offset = scale * (2.0f * i - 1.0f);
        // Variables, not literals: literals fold the whole matrix.
        f32 zero = 0.0f;
        f32 one = 1.0f;
        f32 mtx[2][4];
        calcTexMtx(mtx, one, one, zero, zero, zero, zero, offset, zero);

        GXSetTexCoordGen(static_cast<GXTexCoordID>(i), GX_TG_MTX2x4,
                         GX_TG_TEX0, texMtxID);
        GXLoadTexMtxImm(mtx, texMtxID, GX_MTX_2x4);
    }

    GXColor kcolor = {0, 0, 0, Intensity(this)};
    GXSetNumTevStages(numStages);
    GXSetTevKColor(GX_KCOLOR0, kcolor);

    for (int i = 0; i < numStages; i++) {
        GXTevStageID stage = static_cast<GXTevStageID>(i);
        GXSetTevDirect(stage);
        GXSetTevKColorSel(stage, GX_TEV_KCSEL_K0);
        GXSetTevKAlphaSel(stage, GX_TEV_KASEL_K0_A);

        if (i < 2) {
            GXSetTevOrder(stage, static_cast<GXTexCoordID>(i),
                          mpCapTexture->getLoadMap(), GX_COLOR_NULL);
        } else {
            GXSetTevOrder(stage, GX_TEXCOORD2, mpCapTexture->getLoadMap(),
                          GX_COLOR_NULL);
        }

        GXSetTevSwapMode(stage, GX_TEV_SWAP0, GX_TEV_SWAP0);

        switch (i) {
        case 0:
            GXSetTevColorIn(stage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                            GX_CC_TEXC);
            GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                            GX_CA_ZERO);
            GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE,
                            GX_TEVPREV);
            GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE,
                            GX_TEVPREV);
            break;
        case 1:
            GXSetTevColorIn(stage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_HALF,
                            GX_CC_ZERO);
            GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                            GX_CA_KONST);
            GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO,
                            static_cast<GXTevScale>(Unk2C(this)),
                            TRUE, GX_TEVPREV);
            GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE,
                            GX_TEVPREV);
            break;
        case 2:
            GXSetTevColorIn(stage, GX_CC_TEXC, GX_CC_CPREV, GX_CC_APREV,
                            GX_CC_ZERO);
            GXSetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                            GX_CA_KONST);
            GXSetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE,
                            GX_TEVPREV);
            GXSetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE,
                            GX_TEVPREV);
            break;
        }
    }

    setMatPE();
}

} // namespace EGG
