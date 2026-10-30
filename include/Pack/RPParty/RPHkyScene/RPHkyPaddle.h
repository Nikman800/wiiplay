#ifndef RP_PARTY_HKY_PADDLE_H
#define RP_PARTY_HKY_PADDLE_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPHkyScene/RPHkyField.h>
#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

class RPHkyPaddleTrail;

//! @addtogroup rp_hky
//! @{

/**
 * @brief A player's paddle (player 0 or 1, human or CPU)
 * @details vtable 80359404. Size 0x1CC, verified.
 */
class RPHkyPaddle : public RPSimpleModel {
    // Wii Play: RPHkyGame::StepPhysics passes &mScore to RPHkyPuck::CheckGoal
    friend class RPHkyGame;
    friend class RPHkyPuck; // Wii Play: reads mVelocity and the edges

public:
    enum Shape {
        Shape_Round, //!< round paddle (RPHkyPaddleRound model)
        Shape_Rect,  //!< rectangular paddle, rotated by the remote's tilt
    };

    RPHkyPaddle(s32 playerIndex);
    virtual ~RPHkyPaddle(); // at 0x8

    virtual void Calculate(); // at 0x14

    virtual void Create(); // at 0x1C
    virtual void Load();   // at 0x20
    virtual void Reset();  // at 0x24
    virtual void Exit();   // at 0x28

    /**
     * @brief Mean of the position history (the last five frames' movement) per physics sub-step
     */
    static nw4r::math::VEC3 CalcHitVelocity(const RPHkyPaddle* pPaddle);
    // Wii Play: the real one takes a non-const paddle (the const form hoists the history loads
    // above the stores to the result); the declaration above is kept for linked includers
    static nw4r::math::VEC3 CalcHitVelocity(RPHkyPaddle* pPaddle);
    /**
     * @brief Index of the outline edge the edge iterator last returned, or -1
     */
    s32 GetHitEdgeIndex() const;
    void SetShape(s32 shape) DECOMP_DONT_INLINE;
    /**
     * @brief Requests rumble slot idx with the given pattern length (see UpdateRumble)
     */
    void RequestRumble(s32 idx, s32 frames);
    /**
     * @brief Tests the puck (circle) against this paddle; on a hit returns the push normal and
     * penetration depth (rect paddles step the edge iterator, see ResetEdgeIter)
     */
    BOOL TestPuckCollision(f32 radius, const nw4r::math::VEC3& rPos, nw4r::math::VEC3* pNormal,
                           f32* pDepth);
    void ResetEdgeIter();
    /**
     * @brief Pushes this paddle and the other one apart
     */
    void CollidePaddle(RPHkyPaddle* pOther);
    void UpdateRumble();
    /**
     * @brief Velocity (damping, CPU steering, accumulated pushes), speed limit, clamping to
     * the player's half of the rink
     */
    void UpdateMovement();
    void CollideField();
    void ApplyVelocity();
    /**
     * @brief Position history, trail sound, then CPU target or remote pointer and tilt
     */
    void UpdateControl();

    nw4r::math::VEC3 GetPos() const {
        return mPos;
    }
    nw4r::math::MTX33 GetRot() const {
        return mRot;
    }
    // Wii Play: rows of mRot as axes (UpdateControl)
    nw4r::math::VEC3& GetAxisX() {
        return *reinterpret_cast<nw4r::math::VEC3*>(&mRot._00);
    }
    nw4r::math::VEC3& GetAxisY() {
        return *reinterpret_cast<nw4r::math::VEC3*>(&mRot._10);
    }
    nw4r::math::VEC3& GetAxisZ() {
        return *reinterpret_cast<nw4r::math::VEC3*>(&mRot._20);
    }
    s32 GetScore() const {
        return mScore;
    }
    s32 GetShapeType() const {
        return mShapeType;
    }

    static f32 sRoundRadius;        // 8043B638 34
    static f32 sCpuGain;            // 8043B63C 0.03
    static f32 sMaxSpeed;           // 8043B640 10
    static f32 sVelDamping;         // 8043B644 0.5
    static f32 sRectLength;         // 8043B648 105 (91 once the shape is set)
    static f32 sRectWidth;          // 8043B64C 16
    static s32 sRumbleFrames;       // 8043B650 10
    static f32 sAngleScale;         // 8043B654 3
    static f32 sCpuTargetLerp;      // 8043B658 1
    static f32 sCpuMaxSpeed;        // 8043B65C 0.3, recomputed from the score
    static f32 sCpuSpeedBase[2];    // 8043B660 per shape (round, rect)
    static f32 sCpuSpeedMin[2];     // 8043B668
    static f32 sCpuSpeedLead[2];    // 8043B670 per goal of the human player
    static f32 sCpuSpeedTrail[2];   // 8043B678 per goal behind
    static f32 sCpuSpeedMax[2];     // 8043B680
    static s32 sCpuLookAhead[2];    // 8043B688 frames the CPU extrapolates the puck
    static f32 sPointerDepthScale[2]; // 8043B690 1.2

private:
    BOOL IsCpu() const;
    void StartControllerRumble(s32 frames);
    void UpdatePosHistory();
    void UpdateAngleHistory(f32 angle);
    void AddPush(const nw4r::math::VEC3& rPush);
    void AddVelocityPush(const nw4r::math::VEC3& rPush);
    void ResetPushes();
    void SetRectEdges();
    BOOL CollideRectEdges(f32 radius, const nw4r::math::VEC3& rPos, nw4r::math::VEC3* pNormal,
                          f32* pDepth);
    BOOL CollideRound(f32 radius, const nw4r::math::VEC3& rPos, nw4r::math::VEC3* pNormal,
                      f32* pDepth);
    void CollideRectVsPaddle(RPHkyPaddle* pOther) DECOMP_DONT_INLINE;
    void PushApart(RPHkyPaddle* pOther, const nw4r::math::VEC3& rNormal, f32 depth);
    void CollideRoundVsPaddle(RPHkyPaddle* pOther) DECOMP_DONT_INLINE;
    void CollideFieldRect() DECOMP_DONT_INLINE;
    void CollideFieldRound() DECOMP_DONT_INLINE;
    f32 CalcCpuTargetRect();
    f32 CalcCpuTargetRound();
    void UpdateCpuStallTimer();

    nw4r::math::VEC3 mPushMin;         // at 0x44 most negative position push this step
    nw4r::math::VEC3 mPushMax;         // at 0x50 most positive position push
    nw4r::math::VEC3 mVelPushMin;      // at 0x5C velocity pushes, same scheme
    nw4r::math::VEC3 mVelPushMax;      // at 0x68
    f32 unk74;                         // at 0x74 [L] zeroed by Create
    f32 unk78;                         // at 0x78 [L]
    f32 unk7C;                         // at 0x7C [L]
    nw4r::math::VEC3 mVelocity;        // at 0x80 XZ used
    nw4r::math::VEC3 mPosHistory[5];   // at 0x8C movement per frame, newest last
    nw4r::math::VEC3 mPrevPos;         // at 0xC8 [M]
    f32 mAngleHistory[4];              // at 0xD4 [M] angle change per frame
    f32 mAngleDelta;                   // at 0xE4 [L] newest angle change
    f32 mPrevAngle;                    // at 0xE8 [L] mAngle * sAngleScale
    f32 mAngle;                        // at 0xEC [M] tilt angle
    RPHkyPaddleTrail* mpTrail;         // at 0xF0 sound actor
    RPSysEffect* mpEffect;             // at 0xF4 "Hky_paddletail_r1" / "_b1"
    s32 mPlayerIndex;                  // at 0xF8 [M]
    s32 mShapeType;                    // at 0xFC [M] Shape
    s32 mHitTimer;                     // at 0x100 [M] frames since the puck last hit
    s32 mRumbleState[4];               // at 0x104 [M] 0 idle, 1 running, 2 requested again
    s32 mRumbleFrames[4];              // at 0x114 [M]
    f32 mCpuAngleOffset;               // at 0x124 [L] extra spin of the CPU's rect paddle
    s32 mCpuStallTimer;                // at 0x128 [M]
    s32 mCpuSpinTimer;                 // at 0x12C [L]
    nw4r::math::VEC3 mTarget;          // at 0x130 [M] CPU target / remote pointer on the rink
    nw4r::math::VEC3 mPrevPos1;        // at 0x13C [L] position one frame ago
    nw4r::math::VEC3 mPrevPos2;        // at 0x148 [L] two frames ago
    s32 mScore;                        // at 0x154 goals scored (HUD, result, first to 7)
    RPHkySegment mEdges[4];            // at 0x158 paddle outline
    RPHkySegment* mpEdgeIter;          // at 0x1C8
};

/**
 * @brief Round paddle model and glow attached to a paddle
 * @details vtable 803594F0. Size 0x50, verified.
 */
class RPHkyPaddleRound : public RPSimpleModel {
public:
    RPHkyPaddleRound(s32 playerIndex, RPHkyPaddle* pOwner);
    virtual ~RPHkyPaddleRound(); // at 0x8

    virtual void Calculate(); // at 0x14

    virtual void Create(); // at 0x1C
    virtual void Load();   // at 0x20
    virtual void Reset();  // at 0x24
    virtual void Exit();   // at 0x28

    /**
     * @brief Restarts the hit flash (clr animation 0 from frame 0)
     */
    void RestartHitAnim();

    RPSysEffect* GetEffect() const {
        return mpEffect;
    }

private:
    void SetRot(nw4r::math::MTX33 rot) {
        mRot = rot;
    }
    void SetPos(nw4r::math::VEC3 pos) {
        mPos = pos;
    }
    void SetScale(nw4r::math::VEC3 scale) {
        mScale = scale;
    }


    s32 mPlayerIndex;      // at 0x44 [M]
    RPHkyPaddle* mpOwner;  // at 0x48
    RPSysEffect* mpEffect; // at 0x4C "Hky_paddletail_rc_glow" / "_bc_glow"
};

//! @}

// Wii Play: declare the trail after the paddle so their vtables retain the original order.
#include <Pack/RPParty/RPHkyScene/RPHkySound.h>

#endif
