#ifndef EGG_GFXE_DRAW_PATH_DOF_H
#define EGG_GFXE_DRAW_PATH_DOF_H
#include <egg/types_egg.h>

#include <egg/gfxe/eggDrawPathBase.h>


namespace EGG {

// Forward declarations
class PostEffectBlur;
class PostEffectBlurSimple;
class PostEffectMask;
class PostEffectMaskDOF;
class TextureBuffer;

class DrawPathDOF : public DrawPathBase {
    friend class ScnRenderer;

public:
    DrawPathDOF();
    virtual ~DrawPathDOF();         // at 0x8
    virtual u16 getNumStep() const; // at 0x1C
    virtual void internalCalc();    // at 0x24
    virtual void internalDraw(u16); // at 0x28

private:
    enum Step {
        cStep_0,
        cStep_1,
        cStep_2,

        cStep_Max
    };

private:
    void internalDrawStandard(u16 step);
    void internalDrawLite(u16 step);

private:
    u8 UNK_0x7A[0x7C - 0x7A];
    u16 mFlags;    // at 0x7C
    u8 mAlpha[2];  // at 0x7E

public:
    //! Wii Play: cleared by RPGrpDrawPathDOF's constructor [M]
    int unk80; // at 0x80

private:
    f32 mFocus;                         // at 0x84
    f32 mFocusRange;                    // at 0x88
    f32 unk8C;                          // at 0x8C
    f32 mBlurRadius;                    // at 0x90
    char UNK_0x94[0x9C - 0x94];
    int mBlurType;                      // at 0x9C
    PostEffectBlurSimple* mpBlurSimple; // at 0xA0
    PostEffectBlur* mpBlur;             // at 0xA4
    PostEffectMaskDOF* mpDOF;           // at 0xA8
    TextureBuffer* mpTexBuffer;         // at 0xAC
    PostEffectMask* mpMask;             // at 0xB0
};

} // namespace EGG

#endif
