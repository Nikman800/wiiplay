#ifndef RP_PARTY_FSH_ROD_H
#define RP_PARTY_FSH_ROD_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshObject.h>
#include <Pack/RPParty/RPFshScene/RPFshSound.h>

#include <egg/math.h>
#include <nw4r/math.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPCurve;
class RPFshPlayer;
class RPGrpModel;
class RPJointMtxBuffer;
class RPSysEffect;

/**
 * @brief A player's fishing rod, line (rope) and hook
 * @details vtable 80358C10. Size 0x160, verified (Fsh_Player_ctor). Even players get
 * fsh_rod_B, odd players fsh_rod_R; the line is the fsh_rope model whose joints follow an
 * RPCurve (8 points) through EditWorldMtx.
 */
// Wii Play: direct bases avoid an extra intermediate destructor and vtable.
class RPFshRod : public RPActorBase, public IRPGrpModelCallback {
public:
    enum {
        ROPE_JOINT_NUM = 8,
        EFFECT_NUM = 6
    };

public:
    explicit RPFshRod(RPFshPlayer* pOwner);
    virtual ~RPFshRod(); // at 0x8

    /**
     * @brief RPActorBase::CalcModelMtx override: places the rod and rope models [M]
     */
    virtual void CalcModelMtx(); // at 0x74

    virtual void Create();    // at 0x94
    virtual void Reset();     // at 0x98
    virtual void Calculate(); // at 0x9C
    virtual void Exit();      // at 0xA0 empty, out of line (among the normal functions)

    /**
     * @brief IRPGrpDrawObject::UserDraw override: hook shadow [M]
     */
    virtual void UserDraw(); // at 0xA4

    /**
     * @brief IRPGrpModelCallback::EditWorldMtx override: rope joints along the curve (ledger
     * Fsh_Rod_calcRopeJoints) [M]
     */
    virtual void EditWorldMtx(RPGrpModel* pModel); // at 0xA8

    f32 GetBendRatio() const;

    const EGG::Vector3f& GetFrontDir() const {
        return mFrontDir;
    }
    const EGG::Vector3f& GetUpDir() const {
        return mUpDir;
    }
    void SetFollowRest(bool follow) {
        mFollowRest = follow;
    }
    void SetUseKcl(bool use) {
        mUseKcl = use;
    }
    f32 GetLengthScale() const {
        return mLengthScale;
    }
    void SetLengthScale(f32 scale) {
        mLengthScale = scale;
    }

    RPCurve* GetRope() const {
        return mpRope;
    }
    f32 GetHookRadius() const {
        return unkD4;
    }
    bool IsHookInWater() const {
        return mIsHookInWater;
    }
    void SetFrontDir(const nw4r::math::VEC3& rDir);
    EGG::Vector3f CalcRestDir() const;
    void SetRopeTargetPoint(s32 idx, const nw4r::math::VEC3& rPos);
    void SetAllRopeTargets(const nw4r::math::VEC3& rPos);
    const nw4r::math::VEC3& GetRopePoint(s32 idx) const;
    void Translate(const nw4r::math::VEC3& rOffset);
    void SetPosFromGrip(const EGG::Vector3f& rGripPos);
    EGG::Vector3f GetGripPos() const;
    EGG::Vector3f GetTipPos() const;
    void CalcHookWater();

private:
    void UpdateAxes() {
        mUpDir = EGG::Vector3f(0.0f, 1.0f, 0.0f);
        mSideDir = mUpDir.cross(mFrontDir);
        mSideDir.normalise();
        mUpDir = mFrontDir.cross(mSideDir);
    }

    EGG::Vector3f mGripPos;            // at 0x64
    EGG::Vector3f mBasePos;            // at 0x70 [M] rod model position
    // Wii Play: components along (mSideDir, mUpDir, mFrontDir)
    EGG::Vector3f mGripOffset;         // at 0x7C grip joint (scaled)
    EGG::Vector3f mTipOffset;          // at 0x88 rod_end joint (scaled)
    EGG::Vector3f mFrontDir;           // at 0x94
    EGG::Vector3f mUpDir;              // at 0xA0
    EGG::Vector3f mSideDir;            // at 0xAC
    bool mFollowRest;                  // at 0xB8 [M] Calculate turns mFrontDir toward CalcRestDir
    bool mIsHookInWater;               // at 0xB9 [L]
    EGG::Vector3f mHookPos;            // at 0xBC [M] where the line crosses the water surface
    EGG::Vector3f mPrevHookPos;        // at 0xC8 [M]
    f32 unkD4;                         // at 0xD4 [L] hook shadow size
    bool mUseKcl;                      // at 0xD8 [M] the line collides with the stage
    EGG::Vector3f mPrevHookTarget;     // at 0xDC [M] rope target 7 before the step
    nw4r::math::VEC3 mHookAxes[3];     // at 0xE8 trivial vectors; no array destructor
    RPFshPlayer* mpOwner;              // at 0x10C
    f32 mBend;                         // at 0x110 [M] GetBendRatio = mBend / 6
    RPCurve* mpRope;                   // at 0x114 8-point line curve
    RPGrpModel* mpRodModel;            // at 0x118
    RPGrpModel* mpRopeModel;           // at 0x11C
    u16 mTipJoint;                     // at 0x120 "rod_end"
    u16 mGripJoint;                    // at 0x122 "grip"
    u16 mRopeJoints[ROPE_JOINT_NUM];   // at 0x124 rope_joint_01..06, rope_joint_end, hook
    RPJointMtxBuffer* mpRodJointMtx;   // at 0x134
    RPJointMtxBuffer* mpRopeJointMtx;  // at 0x138
    f32 mLengthScale;                  // at 0x13C [M] stretches the rod along mFrontDir, 1 after Reset
    s32 mDropFrame;                    // at 0x140 [M] frames of Fsh_hook_drop out of the water
    /**
     * @brief Fsh_hook_drop, Fsh_hook_tail_blue/red, Fsh_hook_bubble, Fsh_rope_ripple/trail
     */
    RPSysEffect* mpEffects[EFFECT_NUM]; // at 0x144
    RPFshRodSnd* mpSnd;                 // at 0x15C 0x74-byte sound actor (vtable 80358CBC)

    static f32 sModelScale;            // 8043B560 1
    static f32 sSpacing;               // 8043B564 200 between the players' grips
    static f32 sRopeGravity;           // 8043B568 -2.3
    static f32 sRopeDamping;           // 8043B56C 0.5
    static f32 sRopeStiffness;         // 8043B570 1
    static f32 sRopeLength;            // 8043B574 10
    static f32 sRestRate;              // 8043B578 0.4
    static f32 sFollowRate;            // 8043B57C 0.1
    static f32 sMaxBend;               // 8043B580 6
    static f32 sHookShadowHeight;      // 8043B584 300
    static const char* sTipJointName;  // 8043B588 "rod_end"
    static const char* sGripJointName; // 8043B58C "grip"
    static const char* sRopeJointNames[ROPE_JOINT_NUM]; // 80358BF0
    static EGG::Vector3f sGripPos;     // 8043A574 (0, 40, 40)
    static EGG::Vector3f sRestDir;     // 8043A58C (0, 1, -1)
    static f32 sSplashBend;            // 8043C748 0.1 * sMaxBend
    static const f32 sDropFrameMax;    // 8043F470 70: frames before Fsh_hook_drop fades
};

//! @}

#endif
