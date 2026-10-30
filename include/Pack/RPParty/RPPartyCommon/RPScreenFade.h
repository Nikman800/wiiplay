#ifndef RP_PARTY_COMMON_SCREEN_FADE_H
#define RP_PARTY_COMMON_SCREEN_FADE_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/IRPSysHostIOSocket.h>

#include <revolution/GX.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Screen fade colour stepped toward a target on the minigame result screens
 * (RPCommon + 0x2C)
 * @details vtable 803586E8 (IRPSysHostIOSocket). Size 0x14, verified (Party_Common_ctor).
 * Calculate moves mTimer up while mFadeIn is set (down otherwise), clamped to
 * [0, sFadeFrames], and blends mColor from opaque white toward mTargetColor (alpha from 0).
 * While sReloadColor is set, mTargetColor is re-read from sTargetColor every frame.
 * @note Wii Play-only framework class (ledger Party_ScreenFade); the name is a project
 * convention.
 */
class RPScreenFade : public IRPSysHostIOSocket {
public:
    RPScreenFade();
    virtual ~RPScreenFade(); // at 0x8

    void Reset();
    void Calculate();

    void SetFadeIn(bool fadeIn) {
        mFadeIn = fadeIn;
    }
    const GXColor& GetColor() const {
        return mColor;
    }

private:
    bool mFadeIn;         // at 0x4 set by RPMedalResult when the record screen opens
    s32 mTimer;           // at 0x8 0 .. sFadeFrames
    GXColor mColor;       // at 0xC current colour
    GXColor mTargetColor; // at 0x10 from sTargetColor

    static s32 sFadeFrames;      // 8043B488
    static GXColor sTargetColor; // 8043B48C
    static bool sReloadColor;    // 8043C718 [M] debug switch
};

//! @}

#endif
