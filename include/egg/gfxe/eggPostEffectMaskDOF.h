#ifndef EGG_GFXE_POST_EFFECT_MASK_DOF_H
#define EGG_GFXE_POST_EFFECT_MASK_DOF_H
#include <egg/types_egg.h>

#include <egg/gfxe/eggPostEffectBase.h>
#include <egg/prim.h>

#include <revolution/GX.h>

namespace EGG {

// Forward declarations
class CpuTexture;

class PostEffectMaskDOF : public PostEffectBase {
public:
    enum Mode {
        cMode_0,
        cMode_1,
        cMode_2,
        cMode_3,
    };

public:
    PostEffectMaskDOF();
    virtual void draw(f32 w, f32 h);    // at 0x10
    virtual void reset();               // at 0x14
    virtual void setMaterialInternal(); // at 0x18

    void setUpGradation();
    void setUpDepthTable();

    void setStart(f32 start) {
#line 69
        EGG_ASSERT(start >= 0.f && start <= 1.f);
        mStart = start;
    }
    void setEnd(f32 end) {
#line 71
        EGG_ASSERT(end >= 0.f && end <= 1.f);
        mEnd = end;
    }

    void setCenter(f32 center) {
#line 73
        EGG_ASSERT(center >= 0.f && center <= 1.f);
        mCenter = center;
    }

    void setMode(Mode mode) {
        mMode = mode;
    }

private:
    static const GXColor& getCenterColor();
    static const GXColor& getFarColor();
    static const GXColor& getNearColor();

private:
    u8 unk20;                       // at 0x20
    u32 unk24;                      // at 0x24
    CpuTexture* mpGradationTexture; // at 0x28
    u32 unk2C;                      // at 0x2C
    CpuTexture* mpTexture30;        // at 0x30
    f32 mCenter;                    // at 0x34
    f32 mStart;                     // at 0x38
    f32 mEnd;                       // at 0x3C
    f32 unk40;                      // at 0x40
    f32 unk44;                      // at 0x44
    f32 unk48;                      // at 0x48
    Mode mMode;                     // at 0x4C
};

} // namespace EGG

#endif
