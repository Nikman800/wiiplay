#ifndef RP_PARTY_COW_COW_MODEL_H
#define RP_PARTY_COW_COW_MODEL_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpModelCallback.h>

#include <egg/math.h>
#include <nw4r/math.h>

//! @addtogroup rp_cow
//! @{

// Forward declarations
class RPGrpModel;
class RPGrpModelAnm;
class RPSysKokeshi;

/**
 * @brief The knitted cow ridden by the player's Mii, with both animation sets
 * @details vtable 8035DAE8. Size 0x1F4, verified (Cow_Player_create). The constructor creates
 * the rider avatar (an RPSysKokeshi, 0xB4 bytes) for this player. LoadModels builds the cow
 * (cow_knit_1P/2P, eight animations) and binds ten cow_avatar_anim animations to the avatar.
 * States (mState): 0 run, 1 intro, 2 jump, 3 land, 4 goal [M].
 * @note Name is a project convention (ledger Cow_CowModel).
 */
class RPCowCowModel : public IRPGrpModelCallback {
    friend class RPCowPlayerEffects;
    friend class RPCowPlayer;

public:
    /**
     * @brief Angle and angular speed of one bent joint [L]
     */
    struct Spring {
        f32 mAngle; // at 0x0
        f32 mSpeed; // at 0x4

        void Reset() {
            mAngle = mSpeed = 0.0f;
        }
    };

    explicit RPCowCowModel(s32 player);
    virtual ~RPCowCowModel(); // at 0x8

    /**
     * @brief IRPGrpModelCallback override (ledger calcJoint): bends the cow's root and neck
     * joints [M]
     */
    virtual void EditJointLocalMtx(RPGrpModel* pModel, u16 idx,
                                   nw4r::math::MTX34* pMtx,
                                   nw4r::math::VEC3* pScale); // at 0xC

    /**
     * @brief Ledger init: creates the models and binds the animations [H]
     */
    void LoadModels();
    RPGrpModel* GetModel() const {
        return mpModel;
    }
    // Set every frame by RPCowPlayer::CalcProgress (clamped) [M]
    void SetSpeedBlend(f32 blend) {
        unk19C = blend < 0.0f ? 0.0f : (blend > 1.0f ? 1.0f : blend);
    }
    void SetTailPush(f32 push) {
        unk1A0 = push;
    }
    void SetUnk1A4(f32 value) {
        unk1A4 = value < -1.0f ? -1.0f : (value > 1.0f ? 1.0f : value);
    }
    void SetUnk1A8(f32 value) {
        unk1A8 = value < -1.0f ? -1.0f : (value > 1.0f ? 1.0f : value);
    }
    /**
     * @brief Ledger reset: clears the joint springs and starts every animation at the same
     * random frame [H]
     */
    void Reset();
    /**
     * @brief Ledger Cow_Player_calcFootEffects: steps the joint springs, blends and advances
     * both animation sets, then caches the root, head and foot matrices and foot positions [M]
     */
    void CalcAnim();
    void ClearLandState();
    void PlayGoalAnim();
    bool PlayLandAnim();
    bool PlayJumpAnim();
    bool SetRunState();
    bool SetIntroState();

private:
    // Named constants defined after the code (loaded, not folded) [M]
    static const f32 sModelAnmRateMax;  // 1.0
    static const f32 sAvatarAnmRateMax; // 1.0
    static const f32 sJumpAnmRate;      // 1.0
    static const f32 sGoalAnmRate;      // 1.0
    static const f32 sRootPitchMax;     // 0 (.sbss2)

    s32 mPlayer;                  // at 0x4
    RPGrpModel* mpModel;          // at 0x8 cow_knit_1P / cow_knit_2P
    RPGrpModelAnm* mpModelAnm;    // at 0xC
    RPSysKokeshi* mpAvatar;       // at 0x10 rider Mii
    RPGrpModel* mpAvatarModel;    // at 0x14 avatar body model
    RPGrpModelAnm* mpAvatarAnm;   // at 0x18
    nw4r::math::MTX34 mRootMtx;    // at 0x1C cow_root world matrix
    nw4r::math::MTX34 mHeadMtx;    // at 0x4C j_head world matrix
    nw4r::math::MTX34 mBodyMtx;    // at 0x7C [L] (j_body?)
    nw4r::math::MTX34 mFootMtx[4]; // at 0xAC j_L_front, j_R_front, j_L_back, j_R_back
    EGG::Vector3f mFootPos[4];     // at 0x16C foot positions lowered along the root's up axis
    f32 unk19C;                   // at 0x19C [M] speed blend (walk/run animation weight)
    f32 unk1A0;                   // at 0x1A0 [M] pushes the tail spring
    f32 unk1A4;                   // at 0x1A4 [L] cleared by Reset
    f32 unk1A8;                   // at 0x1A8 [L] cleared by Reset
    Spring mSprings[8];           // at 0x1AC [L] 1 (0x1CC) root pitch, 3/4 neck yaw/pitch
    s32 mState;                   // at 0x1EC
    f32 mLandTimer;               // at 0x1F0 [M] 1.0 on intro
};

//! @}

#endif
