#ifndef RP_PARTY_HKY_PUCK_H
#define RP_PARTY_HKY_PUCK_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

// Forward declarations
class RPHkyPaddle;
class RPHkyPuckTrail;

//! @addtogroup rp_hky
//! @{

/**
 * @brief Laser Hockey puck
 * @details vtable 803592A4. Size 0xC8, verified. Physics per step: velocity XZ damped by
 * friction, Y += gravity, XZ speed capped; accumulators are added then cleared.
 */
class RPHkyPuck : public RPSimpleModel {
public:
    // Wii Play: contact slots of mHitState / mHitSpeed. The enum type matters: an int ternary
    // between these values compiles branch-free, the original branches
    enum EHitSlot {
        Hit_WallLeft,
        Hit_WallRight,
        Hit_Paddle0,
        Hit_Paddle1,
    };

    // Wii Play: derives from RPSimpleModel directly (its destructor calls RPSimpleModel's);
    // the constructor is inlined into RPHkyGame's
    RPHkyPuck() {}
    virtual ~RPHkyPuck(); // at 0x8

    virtual void Calculate(); // at 0x14

    virtual void Create(); // at 0x1C
    virtual void Load();   // at 0x20
    virtual void Reset();  // at 0x24
    virtual void Exit();   // at 0x28

    /**
     * @brief Hides the model and stops the tail effect (time up)
     */
    void Hide();
    /**
     * @brief Shows the model and starts play: BGM request 0, or 3 at match point in 2P
     */
    void Serve();
    /**
     * @brief Contact state per paddle: on a new contact stores the speed and rumbles the
     * paddle's remote 5..10 frames by the relative speed
     */
    void RegisterHit(s32 idx, RPHkyPaddle* pPaddle);
    /**
     * @brief Puck vs paddle contact, bounce impulse, Hky_paddlehit effect
     */
    void CollidePaddle(RPHkyPaddle* pPaddle);
    /**
     * @brief Goal check: increments the scorer (cap 99), goal effects and sounds, resets the puck
     */
    void CheckGoal(s32* pScore0, s32* pScore1);
    /**
     * @brief Friction, goal-mouth and edge checks, accumulated impulses into velocity, spin clamp
     */
    void UpdatePhysics();
    /**
     * @brief Puck vs rink walls: bounce, Hky_wallhit_* effects and wall animations
     */
    void CollideField();
    /**
     * @brief Clears the impulse accumulators and moves by the velocity
     */
    void ApplyVelocity();
    void SetInPlay();
    void ResetState();

    // Wii Play: read by RPHkyPaddle's CPU control
    const nw4r::math::VEC3& GetPos() const {
        return mPos;
    }
    const nw4r::math::VEC3& GetVelocity() const {
        return mVelocity;
    }

    static f32 sRadius;            // 8043B5D0 16
    static f32 sSpeed;             // 8043B5D4 0.65, set from sSpeedInit
    static f32 sSpeedInit;         // 8043B5D8 0.6 (0.65 in 16:9)
    static f32 sSpeedStep;         // 8043B5DC 0.025 (0.02 from Create)
    static f32 sSpeedMax;          // 8043B5E0 1.5
    static f32 sFriction;          // 8043B5E4 0.9998
    static f32 sWallBounce;        // 8043B5E8 1
    static f32 sPaddleBounce;      // 8043B5EC 1
    static f32 sWallSpinFactor;    // 8043B5F0 0.1
    static f32 sPaddleSpinFactor;  // 8043B5F4 0.1
    static f32 sSpinImpulseScale;  // 8043B5F8 -10
    static f32 sSpinMax;           // 8043B5FC 0.9
    static f32 sSpinDamping;       // 8043B600 0.9998
    static f32 sGravity;           // 8043B604 -0.003
    static f32 sGoalSlope;         // 8043B608 1
    static f32 sGoalDepth;         // 8043B60C -10
    static f32 sSpinFactor;        // 8043B610 -0
    static s32 sHitCooldown;       // 8043B614 9
    static f32 sPaddleHitScale;    // 8043B618 1
    static f32 sPaddlePushScale;   // 8043B61C 1
    static f32 sWallAngleLimit;    // 8043B620 180

    // Debug: hold the puck at (sDebugHoldX, 0, sDebugHoldZ) [M]
    static u16 sDebugHold;  // 8043C7D0
    static f32 sDebugHoldX; // 8043C7D4
    static f32 sDebugHoldZ; // 8043C7D8

private:
    void ResetPushes();
    void AddPush(const nw4r::math::VEC3& rPush);
    void AddVelocityPush(const nw4r::math::VEC3& rPush);
    void AddSpinPush(f32 push);
    static void AddPaddlePush(RPHkyPaddle* pPaddle, const nw4r::math::VEC3& rPush);
    static void AddPaddleVelocityPush(RPHkyPaddle* pPaddle, const nw4r::math::VEC3& rPush);
    void PlayWallHit(const char* pName, const nw4r::math::VEC3& rPos, int anim);

    RPHkyPuckTrail* mpTrail;          // at 0x44 sound actor (goal / hit sounds)
    RPSysEffect* mpTailEffect;        // at 0x48 "Hky_balltail"
    bool mActive;                     // at 0x4C tail effect pending (Serve until it drops)
    char unk4D[0x50 - 0x4D];
    bool mInPlay;                     // at 0x50 simulated this frame
    s32 mHitState[4];                 // at 0x54 contact state per wall side / paddle: 0, 1 new, 2 held
    f32 mHitSpeed[4];                 // at 0x64 speed at the contact
    s32 mHitCooldown[3];              // at 0x74 [M]
    nw4r::math::VEC3 mPushMin;        // at 0x80 most negative position push this step
    nw4r::math::VEC3 mPushMax;        // at 0x8C most positive position push
    nw4r::math::VEC3 mVelPushMin;     // at 0x98 velocity pushes, same scheme
    nw4r::math::VEC3 mVelPushMax;     // at 0xA4
    f32 mSpin;                        // at 0xB0 clamped to sSpinMax
    f32 mSpinPushMin;                 // at 0xB4
    f32 mSpinPushMax;                 // at 0xB8
    nw4r::math::VEC3 mVelocity;       // at 0xBC
};

//! @}

#endif
