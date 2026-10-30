#ifndef EGG_GFXE_DRAW_PATH_SHADOW_VOLUME_H
#define EGG_GFXE_DRAW_PATH_SHADOW_VOLUME_H
#include <egg/types_egg.h>

#include <egg/gfxe/eggDrawPathBase.h>

namespace EGG {

class DrawPathShadowVolume : public DrawPathBase {
    friend class ScnRenderer;

public:
    DrawPathShadowVolume();
    virtual ~DrawPathShadowVolume(); // at 0x8
    virtual u16 getNumStep() const;  // at 0x1C
    virtual void internalCalc();     // at 0x24
    virtual void internalDraw(u16);  // at 0x28

    // Wii Play: shadow colour (RPHkyGame's constructor) [M]
    void SetShadowColor(const GXColor& rColor) {
        mShadowColor = rColor;
    }

    // Wii Play: used by RPGrpDrawPathManager [M]
    void OnSVFlag(u16 flag) {
        mSVFlags |= flag;
    }

    void OffSVFlag(u16 flag) {
        mSVFlags &= ~flag;
    }

private:
    enum Step {
        cStep_0,
        cStep_1,
        cStep_2,

        cStep_Max
    };

private:
    char UNK_0x7A[0x7C - 0x7A]; // Wii Play: DrawPathBase ends at 0x7A
    //! Wii Play: flags; RPGrpDrawPathManager sets 0x2 when bloom is on [M]
    u16 mSVFlags;               // at 0x7C
    u8 mClearAlpha;             // at 0x7E
    TextureBuffer* mpFull;      // at 0x80
    TextureBuffer* mpFogZ;      // at 0x84
    GXColor mShadowColor;       // at 0x88 Wii Play [M]
    int mBlendType;             // at 0x8C
    u32 mDrawFlag;              // at 0x90
    s16 mFogIndex;              // at 0x94
};

} // namespace EGG

#endif
