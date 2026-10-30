#ifndef RP_PARTY_DUC_AVATAR_H
#define RP_PARTY_DUC_AVATAR_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPDucScene/RPDucTarget.h>

#include <nw4r/math.h>

//! @addtogroup rp_duc
//! @{

// Forward declarations
class RPSysKokeshi;

/**
 * @brief Player Mii walking on the range, carried off by the UFOs (final round)
 * @details vtable 8035B028. Size 0x204, verified. Two Kokeshi (Mii) bodies, one per player
 * Mii. Thirteen states: shootable, rise up, walk to X, idle, wander, carried, dropped, enter,
 * run to point, walk to line, lined up, celebrate, run away.
 * @note Name is a project convention (ledger Duc_Avatar).
 */
class RPDucAvatar : public RPDucTarget {
    friend class RPDucAvatarMgr;
    friend class RPDucUfo;

public:
    typedef void (RPDucAvatar::*StateFunc)();

public:
    RPDucAvatar();
    virtual ~RPDucAvatar(); // at 0x8

    /**
     * @brief Also creates the two RPSysKokeshi bodies [H]
     */
    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    virtual void Calculate(); // at 0x18

    virtual void CalcModelMtx(); // at 0x24

    virtual void ResetParams();       // at 0x60
    virtual void Launch(u32 pattern); // at 0x64
    virtual void SpawnHitEffect();    // at 0x68
    virtual void PlaySpawnSound();    // at 0x6C
    virtual void PlayHitSound(s32 player); // at 0x70
    virtual void ApplyVelocity();     // at 0x78
    /**
     * @brief Returns this [M]
     */
    virtual RPDucAvatar* AsAvatar(); // at 0xA0

    void SetAnim(u16 anim);
    void TurnTowardPoint();
    void PickWanderCircle();
    void StateShootable();
    void StateRiseUp();
    void StateWalkToX();
    void StateIdle();
    void StateWander();
    void StateCarried();
    void StateDropped();
    void StateEnter();
    void StateRunToPoint();
    void StateWalkToLine();
    void StateLinedUp();
    void StateCelebrate();
    void StateRunAway();

    f32 GetCarryValue();
    void Kill();
    void Release();
    void StartAbduct();

    /**
     * @brief Moves the avatar along with the UFO carrying it (inline in RPDucUfo) [M]
     */
    void SetCarry(nw4r::math::VEC3 pos, f32 value) {
        mCarryPos = pos;
        mCarryValue = value;
    }
    void SetTargeted(bool targeted) {
        mTargeted = targeted;
    }
    /**
     * @brief Removed or running away: no longer a UFO target (inline in RPDucUfo) [M]
     */
    bool IsGone() const {
        return mAge < 0 || mState == 5;
    }

private:
    bool IsWithinBounds(const nw4r::math::VEC3& pos) const {
        if (!RPDucInRange(pos.x, mBoundXMin, mBoundXMax)) {
            return false;
        }
        return RPDucInRange(pos.z, mBoundZMin, mBoundZMax);
    }
    void SetAvatarScale(f32 value) {
        mScale.x = value;
        mScale.y = value;
        mScale.z = value;
    }
    f32 GetAvatarScale() const {
        return mScale.x;
    }
    static f32 GetScaleStep() {
        return 0.05f;
    }
    static f32 GetFullScale() {
        return 1.6f;
    }
    f32 GetAvatarHitRadius() const {
        return mHitRadius;
    }

    s32 mDir;                      // at 0xFC +1 / -1 along x
    f32 mWalkToX;                  // at 0x100
    nw4r::math::VEC3 mTargetPos;   // at 0x104 [M]
    f32 mBoundXMin;                // at 0x110 walking area
    f32 mBoundXMax;                // at 0x114
    f32 mBoundZMax;                // at 0x118
    f32 mBoundZMin;                // at 0x11C
    f32 mWanderRadius;             // at 0x120
    nw4r::math::VEC3 mWanderCenter; // at 0x124
    u16 mWanderAngle;              // at 0x130
    s16 mWanderStep;               // at 0x132
    s32 mWanderTimer;              // at 0x134
    nw4r::math::VEC3 mCarryPos;    // at 0x138 [M] set by the carrying UFO
    f32 mCarryValue;               // at 0x144 [L] scale while carried (GetCarryValue)
    s32 mLineTimer;                // at 0x148
    bool mAllGone;                 // at 0x14C
    s32 mCelebrateTimer;           // at 0x150
    s32 mAnim;                     // at 0x154 [M] player (Mii and side) of the avatar
    bool mTargeted;                // at 0x158 [M] a UFO is after this avatar
    s32 mState;                    // at 0x15C
    RPSysKokeshi* mpKokeshi[2];    // at 0x160 per player Mii [M]
    StateFunc mStateFuncs[13];     // at 0x168
};

//! @}

#endif
