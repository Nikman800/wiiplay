#ifndef RP_PARTY_WLY_CURSOR_H
#define RP_PARTY_WLY_CURSOR_H
#include <Pack/types_pack.h>

#include <egg/math.h>

//! @addtogroup rp_wly
//! @{

// Forward declarations
class RPSysEffect;

/**
 * @brief Player pointer cursor drawn with three effects (cursor, blur, shadow)
 * @details vtable 8035C898 (pointer at 0x48: the members are declared before the first
 * virtual, and there is no virtual destructor). Size 0x4C, verified (Wly_GameMgr_ctor, one per
 * player). Calculate follows the controller's DPD position (halfway per frame, clamped to
 * +-0.965) and angle; in single player it also tracks the pointer distance (mDist*) for the
 * zoom ratio.
 */
class RPWlyCursor {
public:
    RPWlyCursor(s32 player);

    void GetPos(EGG::Vector2f* pPos) const;
    EGG::Vector2f GetPos() const; // out of line in RPWlyAvatarMgr.cpp
    // By-value class parameter: never inlined, a weak copy is emitted (RPWlySound_2)
    void SetPos(EGG::Vector2f pos, f32 angle) {
        mTargetPos = mPos = pos;
        mAngle = angle;
    }
    void SetVisible(bool visible) {
        mVisible = visible;
    }

private:
    bool mVisible;                  // at 0x0 [M] effects shown (also during the result)
    s32 mPlayer;                    // at 0x4
public:
    EGG::Vector2f mTargetPos;       // at 0x8 raw DPD position (y negated)
    EGG::Vector2f mPos;             // at 0x10
    f32 mAngle;                     // at 0x18
private:
    char unk1C[0x28 - 0x1C];        // at 0x1C
    RPSysEffect* mpEffects[3];      // at 0x28 Wtd_cursor_normal_1p/2p, _blur, _sd
public:
    f32 mDistRatio;                 // at 0x34 [M] 0..1
private:
    bool mDistInit;                 // at 0x38
    f32 mDistMin;                   // at 0x3C [M]
    f32 mDistMax;                   // at 0x40 [M]
    f32 mDist;                      // at 0x44 [M] smoothed pointer distance

public:
    /**
     * @brief Creates the three cursor effects (called from RPWlyGameMgr::Init) [H]
     */
    virtual void Create(); // at 0x8
    virtual void Init();   // at 0xC empty
    virtual void Reset();  // at 0x10
    virtual void Calculate(); // at 0x14
    virtual void Draw();      // at 0x18 empty
    virtual void SetEffectsVisible(); // at 0x1C from mVisible (always on in the result state)
};

//! @}

#endif
