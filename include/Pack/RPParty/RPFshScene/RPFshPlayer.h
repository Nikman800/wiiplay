#ifndef RP_PARTY_FSH_PLAYER_H
#define RP_PARTY_FSH_PLAYER_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshObject.h>
#include <Pack/RPParty/RPFshScene/RPFshSound.h>

#include <egg/core.h>

#include <nw4r/math.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPCls8034F6B0;
class RPFshRod;

/**
 * @brief One caught fish in a player's record
 * @details Size 0xC. Filled by RPFshPlayer::AddCaughtFish.
 * @note Name is a project convention.
 */
struct RPFshCaughtFish {
    s32 mType;  // at 0x0 RPFshFish::EType
    f32 mSize;  // at 0x4 the fish's RPActorBase scale
    f32 mBonus; // at 0x8 bonus multiplier (RPFshBonusFish) when it was the bonus type, else 1
};

/**
 * @brief A fishing player: Wii Remote input, the rod and the caught-fish record (2 made by
 * RPFshPlayerMgr)
 * @details vtable 80358B50. Size 0x17DC, verified (Fsh_PlayerMgr_create). The constructor
 * builds the RPFshRod, the hand avatar (RPCls8034F6B0) and the sound actor. The state runs
 * inline (no RPStateMachine) from sStateFuncs: when mIsChanging, mState moves to mPrevState and
 * mNextState to mState, mStateTimer is cleared and the new enter is called; then the calc.
 * Calculate also records the remote's core status every frame in a 40-entry ring buffer.
 */
// Wii Play: direct actor base; no intermediate RPFshActor destructor.
class RPFshPlayer : public RPActorBase {
public:
    enum EState {
        STATE_NONE,
        STATE_AIM,
        STATE_WAIT,   // hook in the water
        STATE_FIGHT,  // fish hooked
        STATE_LAND,   // this player caught the fish
        STATE_RETURN, // another player caught it
        STATE_IDLE,

        STATE_NUM
    };

    enum {
        STATUS_HISTORY_NUM = 40,
        CAUGHT_FISH_MAX = 50
    };

public:
    explicit RPFshPlayer(s32 playerIndex);
    virtual ~RPFshPlayer(); // at 0x8

    /**
     * @brief Loads the scene light and the fsh_hand_R_grip animation for the avatar [M]
     */
    virtual void Create();    // at 0x78
    virtual void Reset();     // at 0x7C
    virtual void Calculate(); // at 0x80
    virtual void Exit();      // at 0x84

    /**
     * @brief IRPGrpDrawObject::UserDraw override (shadow) [M]
     */
    virtual void UserDraw(); // at 0x88

    void Init(u32 arg, s32 controllerNo);
    void AddCaughtFish(s32 fishIdx);
    bool IsHookWaiting() const;
    bool IsRemoteSteady();
    void Rumble(f32 strength);
    void UpdateRod();
    /**
     * @brief Where the pointer meets the water plane, clamped to the aim range; with adjust
     * the remote's distance change since the cast moves it (ledger Fsh_Player_calcAimPos) [M]
     */
    EGG::Vector3f CalcAimPos(bool adjust);

    s32 GetState() const {
        return mState;
    }
    bool IsChanging() const {
        return mIsChanging;
    }
    s32 GetPlayerIndex() const {
        return mPlayerIndex;
    }
    s32 GetHookedFish() const {
        return mHookedFish;
    }
    s32 GetControllerNo() const {
        return mControllerNo;
    }
    void ChangeState(s32 state) {
        mNextState = state;
        mIsChanging = true;
    }
    RPFshRod* GetRod() const {
        return mpRod;
    }
    f32 GetRemoteDist() const {
        return mPointer.z;
    }
    s32 GetCaughtNum() const {
        return mCaughtNum;
    }
    const RPFshCaughtFish& GetCaughtFish(s32 idx) const {
        return mCaughtFish[idx];
    }

private:
    void AimEnter();
    void AimCalc();
    void WaitEnter();
    void WaitCalc();
    void FightEnter();
    void FightCalc();
    void LandEnter();
    void LandCalc();
    void ReturnEnter();
    void ReturnCalc();
    void IdleEnter();
    void IdleCalc();

private:
    s32 mPlayerIndex;                // at 0x60
    u32 unk64;                       // at 0x64 [L] set by Init
    s32 mControllerNo;               // at 0x68
    s32 mState;                      // at 0x6C EState
    s32 mNextState;                  // at 0x70
    s32 mPrevState;                  // at 0x74
    bool mIsChanging;                // at 0x78
    EGG::Vector3f mAimPos;           // at 0x7C [M]
    bool mIsLanding;                 // at 0x88 [M] LandCalc pulls the line to mLandPos
    EGG::Vector3f mLandPos;          // at 0x8C [M] written by LandCalc
    RPFshRod* mpRod;                 // at 0x98
    s32 mHookedFish;                 // at 0x9C [M] RPFshFishMgr index, -1 when none
    s32 mStateTimer;                 // at 0xA0
    u32 unkA4;                       // at 0xA4
    nw4r::math::VEC2 mBasePos;       // at 0xA8 [M] pointer position after Reset
    /**
     * @brief Pointer x, y (scaled to the screen) and the remote distance
     */
    nw4r::math::VEC3 mPointer;       // at 0xB0 [M]
    nw4r::math::VEC3 mPrevPointer;   // at 0xBC [M]
    f32 mCastDist;                   // at 0xC8 [M] remote distance at the cast
    s32 mStatusHead;                 // at 0xCC ring buffer write index
    s32 mStatusNum;                  // at 0xD0 entries in the ring buffer (<= 40)
    EGG::CoreStatus mStatusHistory[STATUS_HISTORY_NUM]; // at 0xD4
    s32 mCaughtNum;                  // at 0x1574
    RPFshCaughtFish mCaughtFish[CAUGHT_FISH_MAX]; // at 0x1578
    RPFshFishSnd* mpSnd;             // at 0x17D0 0x78-byte sound actor (vtable 80358888)
    RPCls8034F6B0* mpAvatar;         // at 0x17D4 hand avatar
    bool mIsAiming;                  // at 0x17D8 [M] set by AimEnter, cleared by WaitEnter

    // The state functions are a local table in Calculate (8031F320, .rodata). The IDLE enter
    // entry points at calcIdle+0x64 (IdleEnter, empty, which the split merged into IdleCalc).

    static f32 sSteadyAccelMin;   // 8043B508 -1.4
    static f32 sSteadyAccelMax;   // 8043B50C -0.5
    static f32 sCastDist;         // 8043B510 30 screen distance that casts
    static f32 sMoveRate;         // 8043B514 0.1
    static f32 sAimBaseY;         // 8043B518 -80
    static f32 sAimDistScale;     // 8043B51C 2500
    static f32 sAimBaseZ;         // 8043B520 100
    static f32 sAimRangeZ;        // 8043B524 110
    static f32 sSpacing;          // 8043B528 200
    static f32 sLandRate;         // 8043B52C 0.5
    static f32 sWaitRate;         // 8043B530 0.4
    static f32 sAimMinY;          // 8043B534 -50
    static f32 sPosPlayRadius;    // 8043B538 0.005
    static f32 sDistRadiusMin;    // 8043B53C 0.05
    static f32 sDistRadiusMax;    // 8043B540 0.2
    static f32 sRemoteDistNear;   // 8043B544 1
    static f32 sRemoteDistFar;    // 8043B548 2.5
    static f32 sIdleMaxX;         // 8043B54C 500
    static s32 sLandFrames;       // 8043B550 100
    static f32 sAimRangeMin;      // 8043C730 sAimBaseZ - sAimRangeZ
    static f32 sAimRangeMax;      // 8043C734 sAimBaseZ + sAimRangeZ
    static EGG::Vector3f sAimBasePos; // 8043A4F4 (0, sAimBaseY, sAimBaseZ)
    static EGG::Vector2f sBasePos;    // 8043C738 (0, -80)
    static EGG::Vector3f sLandOffset; // 8043A518 (0, -6, 5)
    static EGG::Vector3f sLandScale;  // 8043A530 (10, 10, 3)
    static EGG::Vector2f sUnusedPosL; // 8043A558 (-0.25, -0.4) [L] unreferenced
    static EGG::Vector2f sUnusedPosR; // 8043A560 (0.25, -0.4) [L] unreferenced
};

//! @}

#endif
