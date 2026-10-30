#ifndef RP_PARTY_PNP_BALL_H
#define RP_PARTY_PNP_BALL_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

#include <nw4r/math.h>

class RPPnpBallTrail;

//! @addtogroup rp_pnp
//! @{

/**
 * @brief Table Tennis ball
 * @details vtable 803597F0. Size 0xE0, verified. mState tracks the rally: OnEvent is called by
 * the floor, table and rackets and advances it (states 9-13 end the point; RPPnpGame moves to
 * the point state when it sees one). The three tail effects are oriented along the velocity
 * through mEffectMtx.
 */
class RPPnpBall : public RPSimpleModel {
    // Wii Play: the rackets set the velocity, gravity and smash flags directly
    friend class RPPnpPlayer;

public:
    virtual ~RPPnpBall();     // at 0x8
    virtual void Calculate(); // at 0x14

    void Create();
    void Load();
    void Reset();
    void Exit();

    /**
     * @brief pos += velocity, velocity += mGravity, velocity scaled by the air drag
     */
    void Integrate();
    /**
     * @brief Rally state machine: something (floor, table side, racket) touched the ball
     */
    void OnEvent(s32 event, s32 side);
    void OnEventState4(s32 event, s32 side);
    void OnEventState7(s32 event, s32 side);
    void OnEventState8(s32 event, s32 side);
    /**
     * @brief Simulates the trajectory to the table height; true if it lands on the table
     */
    bool PredictTableLanding(nw4r::math::VEC3* pLandingPos);
    void PrepareServe();
    /**
     * @brief Serve toss from rPos (state 1 -> 2): updates mRallyBonus, starts the rally
     */
    void TossServe(const nw4r::math::VEC3& rPos);
    void ApplyReceiverSpin();
    void DrawShadow();

    s32 GetState() const {
        return mState;
    }
    s32 GetHitCount() const {
        return mHitCount;
    }
    nw4r::math::VEC3 GetPos() const {
        return mPos;
    }
    void SetPos(nw4r::math::VEC3 pos) {
        mPos = pos;
    }
    nw4r::math::VEC3 GetPrevPos() const {
        return mPrevPos;
    }
    nw4r::math::VEC3 GetVelocity() const {
        return mVelocity;
    }
    void SetVelocity(nw4r::math::VEC3 velocity) {
        mVelocity = velocity;
    }
    // Wii Play: RPPnpPlayer, after a racket hit
    void SetGravity(nw4r::math::VEC3 gravity) {
        mGravity = gravity;
    }
    f32 GetRadius() const {
        return mShadowScale * mRadius;
    }
    bool IsHidden() const {
        return mHidden;
    }
    bool WillLandOnTable() const {
        return mWillLandOnTable;
    }
    nw4r::math::VEC3 GetLandingPos() const {
        return mLandingPos;
    }
    // Wii Play: RPPnpAudience::Calculate copies the landing point member by member
    const nw4r::math::VEC3& GetLandingPosRef() const {
        return mLandingPos;
    }

    static nw4r::math::VEC3 sStartPos;      // 8043A6D8 (Pnp_ballStartPos) (60, 126, -130)
    static nw4r::math::VEC3 sServeVelocity; // 8043A6E4 (Pnp_ballServeVelocity) (0, 15, 0)
    static nw4r::math::VEC3 sGravity;       // 8043A6F0 (Pnp_ballGravity) (0, -0.98, 0)
    static f32 sDrag;                       // 8043B6B8 (Pnp_ballDrag) 1.1, scales gravity's y
    static s32 sDebugStartRally;            // 8043C7F8 (Pnp_debugStartRally) hit count / 2 at serve

private:
    RPPnpBallTrail* mpTrail;          // at 0x44 sound actor
    RPSysEffect* mpTailEffect;        // at 0x48 "Pnp_balltail"
    RPSysEffect* mpTailEffect2;       // at 0x4C "Pnp_balltail02"
    RPSysEffect* mpTailGlowEffect;    // at 0x50 "Pnp_balltail02_glow"
    nw4r::math::MTX34 mEffectMtx;     // at 0x54 axes from the velocity, translation = mPos
    s32 mHitCount;                    // at 0x84 racket hits; rallies = mHitCount / 2
    s32 mRallyBonus;                  // at 0x88 [L] added to the rally count for the level lookup
    s32 unk8C;                        // at 0x8C [L]
    f32 mSpeedScale;                  // at 0x90 from RPPnpLevelTable for the current rally
    f32 mRadius;                      // at 0x94 [M] 3.0
    f32 mShadowScale;                 // at 0x98 [L] 2.2
    nw4r::math::VEC3 mPrevPos;        // at 0x9C
    nw4r::math::VEC3 mVelocity;       // at 0xA8
    nw4r::math::VEC3 mGravity;        // at 0xB4 [M] (0, -0.98, 0) while in play
    s32 mState;                       // at 0xC0
    bool mWillLandOnTable;            // at 0xC4
    nw4r::math::VEC3 mLandingPos;     // at 0xC8 PredictTableLanding result
    s32 mHitterSide;                  // at 0xD4 last side to hit the ball, 2 = none
    s32 mTargetSide;                  // at 0xD8 side the ball must bounce on next
    bool mVisible;                    // at 0xDC [M]
    bool mHidden;                     // at 0xDD [L] hides the tail effects
};

//! @}

// Wii Play: RPPnpBall is declared before the sound actors (vtable order in RPPnpBall.cpp)
#include <Pack/RPParty/RPPnpScene/RPPnpSound.h>

#endif
