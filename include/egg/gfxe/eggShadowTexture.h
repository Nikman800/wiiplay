#ifndef EGG_GFXE_SHADOW_TEXTURE_H
#define EGG_GFXE_SHADOW_TEXTURE_H
#include "eggModelSnapshot.h"
#include "eggPostEffectBlur.h"
#include "eggPostEffectBlurSimple.h"
#include "types_egg.h"

#include <nw4r/g3d.h>
#include <nw4r/math.h>

namespace EGG {
class ShadowTexture : public ModelSnapshot {
public:
    // Wii Play: TEV and blend setups of BeginDrawQuad (lbl_8030E8E8: DrawGX::BLEND_MUL,
    // BLEND_SUB, BLEND_NORMAL; RPCowPlayer passes 0) [L names]
    enum Blend {
        BLEND_MUL,
        BLEND_SUB,
        BLEND_COLOR,
    };

    enum {
        EFFECT_MTX_MAX = 32,
    };

    // Wii Play: all three parameters are u16 (__ct__Q23EGG13ShadowTextureFUsUsUs)
    ShadowTexture(u16 size, u16 format, u16 mdlMax);

    // Wii Play: overrides ModelSnapshot's slot 0x18 (0x8009AAB8; Wii Sports' symbols call it
    // ResetModel, but an override keeps the base's name)
    virtual void InitList(); // at 0x18

    void Calc();
    void Draw(Screen*, bool, bool, bool);

    void Replace(EGG::ModelEx* pModelEx, const char* pName);
    // Wii Play: 0x8009AC88 (no symbol) [L name]
    void ResetEffectMtx();

    static void BeginDrawQuad(Blend blend);
    void DrawQuad(const nw4r::math::MTX34& rMtx, const GXColor& rColor) const;

    // Temporary names, needed for regalloc
    void SetCapTexture0(CapTexture* pCapTexture) {
        POST_EFF_0xE4->setCapTexture(pCapTexture);
    }
    void SetCapTexture1(CapTexture* pCapTexture) {
        POST_EFF_0xE8->setCapTexture(pCapTexture);
    }

private:
    struct BlurParam {
        f32 radius; // at 0x0
        u8 alpha;   // at 0x4
    };

    struct EffectMtxSet {
        nw4r::g3d::ResTexSrt texSrt; // at 0x0
        u8 texMapID;                 // at 0x4
    };

    void CalcShadow();
    void CalcProjectionMatrix();
    void DrawShape();
    void SetUpTextureAndColor(const nw4r::math::MTX34& rMtx,
                              const GXColor& rColor) const;
    static void SetMaterial(Blend blend);

public:
    u16 SHORT_0x70;
    // Wii Play: set by RPPnpGame::Load (0x50, 1.15, light direction (0, -1, 0)) [L names]
    u8 BYTE_0x72;
    f32 FLOAT_0x74;
    nw4r::math::VEC3 VEC_0x78;
    nw4r::math::MTX34 mLightMtx;      // at 0x84
    nw4r::math::MTX34 mProjectionMtx; // at 0xB4
    PostEffectBlur* POST_EFF_0xE4;
    PostEffectBlurSimple* POST_EFF_0xE8;
    BlurParam mBlurParam[2];                    // at 0xEC
    u8 mEffectMtxNum;                           // at 0xFC
    EffectMtxSet mEffectMtxSet[EFFECT_MTX_MAX]; // at 0x100

    // Wii Play: lbl_8043BBD0, never set in the game (bit 0: alpha compare, bit 1: alpha draw)
    static u16 sFlags;
};
} // namespace EGG

#endif
