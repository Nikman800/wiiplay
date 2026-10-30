#ifndef EGG_GFXE_POST_EFFECT_BLUR_SIMPLE_H
#define EGG_GFXE_POST_EFFECT_BLUR_SIMPLE_H
#include "eggPostEffectBase.h"
#include "types_egg.h"

namespace EGG {

class PostEffectBlurSimple : public PostEffectBase {
public:
    PostEffectBlurSimple();
    virtual ~PostEffectBlurSimple() {}  // at 0x8
    virtual void reset();               // at 0x14
    virtual void setMaterialInternal(); // at 0x18

    //! Wii Play: set by DrawPathBloom (RPGrpView writes these bytes through unk20)
    void setRadius(f32 radius) {
        *reinterpret_cast<f32*>(&unk20[0x20 - 0x20]) = radius;
    }
    void setIntensity(u8 intensity) {
        *reinterpret_cast<u8*>(&unk20[0x28 - 0x20]) = intensity;
    }
    void setUnk2C(u32 value) {
        *reinterpret_cast<u32*>(&unk20[0x2C - 0x20]) = value;
    }
    void setUnk30(u8 value) {
        *reinterpret_cast<u8*>(&unk20[0x30 - 0x20]) = value;
    }

public:
    char unk20[0x34 - 0x20];
};

} // namespace EGG

#endif
