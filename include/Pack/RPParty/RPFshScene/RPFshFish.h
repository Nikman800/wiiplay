#ifndef RP_PARTY_FSH_FISH_H
#define RP_PARTY_FSH_FISH_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshObject.h>
#include <Pack/RPParty/RPFshScene/RPFshSound.h>

#include <nw4r/math.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPFshRod;
class RPJointMtxBuffer;
class RPRange;
class RPSysEffect;

/**
 * @brief Per-type fish parameters (6 entries at 80320330, Fsh_getFishTypeParams)
 * @details Size 0x38. Field meanings are from the readers [L] unless noted.
 * @note Name is a project convention.
 */
struct RPFshFishTypeParam {
    f32 mMaxSize;    // at 0x0 [M] spawn size is random in [mMaxSize / 2, mMaxSize]
    f32 mDamping;    // at 0x4 0.98 .. 0.95
    s32 unk8;        // at 0x8
    f32 unkC;        // at 0xC
    s32 unk10;       // at 0x10
    s32 unk14;       // at 0x14
    f32 mPullSpeed;  // at 0x18 [M] RPFshFish::GetPullSpeed
    f32 mLength;     // at 0x1C [M] model length at scale 1 (scale = mScale * mLength / A's)
    f32 unk20;       // at 0x20
    f32 unk24;       // at 0x24
    s32 mPoints;     // at 0x28 [M] 200, 120, 60, -50, 350, 250
    s32 unk2C;       // at 0x2C
    s32 mMinFrame;   // at 0x30 [L] -1 when unused
    s32 mMaxFrame;   // at 0x34 [L] -1 when unused
};

/**
 * @brief A fish in the pond (30 made by RPFshFishMgr, type = index % 6)
 * @details vtable 803587E0. Size 0x118, verified (Fsh_FishMgr_create). Types: 0 fsh_fish_A,
 * 1 B, 2 C, 3 sukaA (the "miss" fish), 4 rareA, 5 rareB. The state runs inline (no
 * RPStateMachine): Calculate copies sStateFuncs to the stack, and when mIsChanging it calls the
 * old state's exit, moves mNextState to mState, clears mStateTimer and calls the new enter, then
 * calls the state's calc.
 * Derives from RPActorBase and IRPGrpModelCallback directly (same slot layout as
 * RPFshModelActor): its inline destructor has two null checks (0x6C at 80205974), one more
 * class level gives three (0x70).
 */
class RPFshFish : public RPActorBase, public IRPGrpModelCallback {
public:
    enum EType {
        TYPE_A,
        TYPE_B,
        TYPE_C,
        TYPE_SUKA_A,
        TYPE_RARE_A,
        TYPE_RARE_B,

        TYPE_NUM
    };

    enum EState {
        STATE_NONE,        // inactive
        STATE_SWIM,
        STATE_ESCAPE,
        STATE_VANISH,
        STATE_APPROACH,    // swimming to the hook
        STATE_HOOKED,
        STATE_JUMP,        // pulled out of the water
        STATE_CATCH_VIEW,
        STATE_SHOW_POINTS,
        STATE_FADE_OUT,

        STATE_NUM
    };

public:
    RPFshFish(s32 index, s32 type);
    // Wii Play: inline (emitted after the TRPSndObject<0> copies at the end of the file)
    virtual ~RPFshFish() {} // at 0x8

    /**
     * @brief RPActorBase::CalcModelMtx override: scales the model by mScale and the type's
     * length and updates it (ledger Fsh_Fish_updateModel) [M]
     */
    virtual void CalcModelMtx(); // at 0x74

    /**
     * @brief Constructs the model for mType, its three animations (appear, swim, pull), the
     * joint buffer and the mouth_loc offset [H]
     */
    virtual void Create();    // at 0x94
    virtual void Reset();     // at 0x98
    virtual void Calculate(); // at 0x9C
    virtual void Exit();      // at 0xA0 empty, out of line

    /**
     * @brief IRPGrpDrawObject::UserDraw override (empty shadow draw) [M]
     */
    virtual void UserDraw(); // at 0xA4

    void Spawn();

    s32 GetType() const {
        return mType;
    }
    s32 GetState() const {
        return mState;
    }
    s32 GetNextState() const {
        return mNextState;
    }
    bool IsChanging() const {
        return mIsChanging;
    }
    void ChangeState(s32 state) {
        mNextState = state;
        mIsChanging = true;
    }
    bool IsActive() const {
        return mState != STATE_NONE || (mState == STATE_NONE && mIsChanging);
    }
    /**
     * @brief Not in the pond and not about to be (free for a spawn) [H]
     */
    bool IsFree() const {
        return (mState == STATE_NONE && !mIsChanging) || (mNextState == STATE_NONE && mIsChanging);
    }
    bool IsBiting() const {
        return mIsBiting;
    }
    bool IsHooked() const {
        return mIsHooked;
    }
    const EGG::Vector3f& GetMouthPos() const {
        return mMouthPos;
    }
    nw4r::math::VEC3 CalcModelScale() const; // ledger Fsh_Fish_getScale
    f32 GetPullSpeed() const;
    bool IsNearHook(s32 player) const;
    bool IsMouthNearPoint(f32 radius, const nw4r::math::VEC3& rPoint) const;
    bool IsNearPoint(f32 radius, const nw4r::math::VEC3& rPoint) const;
    bool ResolveCollision();
    void UpdateGetEffects();

    /**
     * @brief Parameters of a fish type (Fsh_getFishTypeParams, 80214E78) [H]
     */
    static const RPFshFishTypeParam* GetTypeParam(s32 type);

    /**
     * @brief Random horizontal unit direction (ledger Fsh_randomDirXZ) [M]
     */
    EGG::Vector3f RandomDirXZ();

private:
    void FixAxes();
    void SwimEnter();
    void SwimCalc();
    void EscapeEnter();
    void EscapeCalc();
    void EscapeExit();
    void VanishEnter();
    void VanishCalc();
    void ApproachEnter();
    void ApproachCalc();
    void HookedEnter();
    void HookedCalc();
    void HookedExit();
    void JumpEnter();
    void JumpCalc();
    void CatchViewEnter();
    void CatchViewCalc();
    void ShowPointsEnter();
    void ShowPointsCalc();
    void FadeOutEnter();
    void FadeOutCalc();

private:
    s32 mIndex;                        // at 0x64 0..29
    s32 mType;                         // at 0x68 EType
    s32 mState;                        // at 0x6C EState
    s32 mNextState;                    // at 0x70
    bool mIsChanging;                  // at 0x74
    nw4r::math::VEC3 mMouthOffset;     // at 0x78 mouth_loc joint translation
    EGG::Vector3f mMouthPos;           // at 0x84 [M] world mouth position
    f32 unk90;                         // at 0x90 [L] saved height (jump)
    nw4r::math::VEC3 mCatchFromPos;    // at 0x94 [M] position when the catch view starts
    s32 mStateTimer;                   // at 0xA0 frames in the current state
    s32 unkA4;                         // at 0xA4 [L]
    s32 mFrame;                        // at 0xA8 [M] counts while the start layout is playing
    EGG::Vector3f mJumpAxisZ;          // at 0xAC [M]
    EGG::Vector3f mJumpAxisY;          // at 0xB8 [M]
    s32 unkC4;                         // at 0xC4 [L] signed countdown
    s32 unkC8;                         // at 0xC8 [L]
    u8 unkCC;                          // at 0xCC [L]
    s32 unkD0;                         // at 0xD0 [L] signed bite count
    bool mIsBiting;                    // at 0xD4 [M] set by ApproachCalc; RPFshPlayer hooks it
    bool mIsHooked;                    // at 0xD5 [M] on the line (hooked to the catch view)
    f32 mPopScale;                     // at 0xD8 [M] 1 + ease bump while mPopTimer runs
    s32 mPopTimer;                     // at 0xDC [M] -1 when idle
    s32 unkE0;                         // at 0xE0 [L]
    f32 mScaleMul;                     // at 0xE4 [M]
    RPRange* mpEase;                   // at 0xE8
    RPJointMtxBuffer* mpJointMtx;      // at 0xEC
    /**
     * @brief Fsh_run_a/b/c, Fsh_get_back/glow/kira, Fsh_exmark, Fsh_run02_a/b
     */
    RPSysEffect* mpEffects[9];         // at 0xF0
    RPFshFishSnd* mpSnd;               // at 0x114 0x78-byte sound actor (vtable 80358888)

    /**
     * @brief State functions, 8031F050 (copied to the stack by Calculate)
     */
    // The state functions are a local table in Calculate (8031F050, .rodata)

    static f32 sSwimTurnRate;        // 8043B498 0.2
    static f32 sSwimAccel;           // 8043B49C 0.08
    static f32 sApproachSpeed;       // 8043B4A0 2
    static f32 sApproachRate;        // 8043B4A4 0.8
    static s32 sApproachFrames;      // 8043B4A8 60
    static f32 sMouthRadiusScale;    // 8043B4AC 0.65
    static f32 sMouthRadiusAdd;      // 8043B4B0 25
    static f32 sNearRadiusScale;     // 8043B4B4 1
    static f32 sNearRadiusAdd;       // 8043B4B8 25
    static f32 sHookViewAngle;       // 8043B4BC 2.199
    static f32 sHookViewDist;        // 8043B4C0 100
    static s32 sCatchViewFrames;     // 8043B4C4 12
    static s32 sCatchViewHoldFrames; // 8043B4C8 10
    static s32 sShowPointsFrames;    // 8043B4CC 85
    static s32 sShowPointsEndFrames; // 8043B4D0 120 (bonus fish)
    static f32 sPopScale;            // 8043B4D4 0.2
    static s32 sPopFrames;           // 8043B4D8 20
    static s32 sJumpFrames;          // 8043B4DC 20
    static f32 sJumpHeight;          // 8043B4E0 4
    static s32 sEscapeFrames;        // 8043B4E4 180
    static s32 sEscapeTurnFrames;    // 8043B4E8 40
    static f32 sEscapeSpeed;         // 8043B4EC 4
    static f32 sEscapeDamp;          // 8043B4F0 0.85
    static f32 sEscapeAccel;         // 8043B4F4 1.6
    static f32 sGetEffectOffset;     // 8043B4F8 1
    static f32 sGetEffectHeightA;    // 8043B4FC 7
    static f32 sGetEffectHeightB;    // 8043B500 2.5
    static bool sShowPointsClose;    // 8043B504 true
    static EGG::Vector3f sCatchViewPos; // 8043A4DC (0, 280, 250)

    /**
     * @brief Parameters per type (80320330)
     */
    static const RPFshFishTypeParam sTypeParams[TYPE_NUM];
};

//! @}

#endif
