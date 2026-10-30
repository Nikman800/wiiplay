#ifndef RP_PARTY_PNP_PLAYER_H
#define RP_PARTY_PNP_PLAYER_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>
#include <Pack/RPParty/RPPnpScene/RPPnpSound.h>

#include <nw4r/math.h>

//! @addtogroup rp_pnp
//! @{

// Forward declarations
class RPCls8034F6B0;

/**
 * @brief A Table Tennis player: the racket model plus the Mii hand
 * @details vtable 80359830. Size 0x2C8, verified (RPPnpGame constructor: player 0 human, player 1
 * human or CPU). The human racket follows the Remote's motion; the last 10 motion samples are
 * kept in a ring buffer to judge swings.
 */
class RPPnpPlayer : public RPSimpleModel {
    // Wii Play: the ball reads the swing and sets the CPU level directly
    friend class RPPnpBall;

public:
    RPPnpPlayer(s32 side, BOOL isCpu);
    virtual ~RPPnpPlayer();   // at 0x8
    virtual void Calculate(); // at 0x14

    void Create();
    void Load();
    void Reset();
    void Exit();

    void UpdateInput();
    void UpdateHuman();
    void UpdateCpu();
    /**
     * @brief Racket against the ball: sets the return velocity and reports event 7 to the ball;
     * true on a hit
     */
    bool CheckBallHit();
    bool CheckServeHit();
    /**
     * @brief CPU return: velocity towards PickCpuTargetX on the opponent's half
     */
    nw4r::math::VEC3 CalcCpuReturnVelocity();
    void PickCpuServeX();
    /**
     * @brief Picks the left / centre / right third of the table by the CPU level's weights
     */
    f32 PickCpuTargetX(f32 ballX);
    void RecordSwingSample(nw4r::math::VEC3 pos, bool valid, bool swing, bool spin);
    void DrawDebug();
    nw4r::math::VEC3 GetSwing() const;

    nw4r::math::VEC3 GetPos() const {
        return mPos;
    }
    BOOL IsCpu() const {
        return mIsCpu;
    }
    bool IsHidden() const {
        return mHidden;
    }
    // Wii Play: copied into RPPnpBalloonLyt's out requests
    bool IsBalloonOut() const {
        return unk98;
    }

private:
    /**
     * @brief Remote motion sample (0x28 bytes); velocity and acceleration are differences to the
     * previous sample
     */
    struct SwingSample {
        nw4r::math::VEC3 mPos;   // at 0x0
        nw4r::math::VEC3 mVel;   // at 0xC
        nw4r::math::VEC3 mAccel; // at 0x18
        bool mValid;             // at 0x24
        u8 unk25;                // at 0x25 [L]
        bool unk26;              // at 0x26 [L]
    };

    RPCls8034F6B0* mpHand;              // at 0x44 Mii hand (RPSysKokeshiGenInfo of this player)
    RPPnpRacketTrail* mpTrail;          // at 0x48 sound actor
    RPSysEffect* mpRacketEffect;        // at 0x4C "Pnp_racket_tail_r" / "_b"
    nw4r::math::MTX34 mEffectMtx;       // at 0x50 racket effect, from the racket model
    u8 unk80;                           // at 0x80 [L]
    u16 mDebugFlags;                    // at 0x82 bit 0/1/2: DrawDebug parts
    f32 unk84;                          // at 0x84 [L] 64
    nw4r::math::VEC3 mTarget;           // at 0x88 [M] racket target position
    s32 unk94;                          // at 0x94 [L]
    bool unk98;                         // at 0x98 [L] set for the CPU in the rally game
    nw4r::math::MTX34 mRacketMtx;       // at 0x9C [M] rotation rows set per side in Reset
    bool mHidden;                       // at 0xCC [M] arrow layout hidden for this player
    bool unkCD;                         // at 0xCD [L]
    bool unkCE;                         // at 0xCE [L] pointer on the screen (human), always set for the CPU
    bool unkCF;                         // at 0xCF [L]
    f32 mHitRadius;                     // at 0xD0 [L] 25
    nw4r::math::VEC3 mHitBoxes[2];      // at 0xD4 [L] copied from Pnp_racketParams
    s32 mSide;                          // at 0xEC 0 near (human), 1 far
    BOOL mIsCpu;                        // at 0xF0
    s32 mCpuLevel;                      // at 0xF4 from RPPnpLevelTable (rally game)
    f32 mCpuServeX;                     // at 0xF8 [M]
    s32 unkFC;                          // at 0xFC [L] hand animation: 0 wait, 1 swing, 2 toss
    f32 unk100;                         // at 0x100 [L] 0 from the constructor
    s32 mSampleCount;                   // at 0x104 ring index = mSampleCount % 10
    SwingSample mSamples[10];           // at 0x108
    nw4r::math::VEC3 mSwingVecs[4];     // at 0x298 [L] written by RecordSwingSample, drawn by
                                        // DrawDebug

public:
    static s32 sHitWindowFrames;       // 8043B6C8 (Pnp_hitWindowFrames) 6, rumble frames
    static f32 sRacketScale;           // 8043B6CC (Pnp_racketHitRadius) 25
    static f32 sPlayerHeight;          // 8043B6D0 (Pnp_playerHeight) 40
    static f32 sCpuReturnSpeed;        // 8043B6D4 (Pnp_cpuReturnSpeed) 1.65
    static f32 sTossOffset;            // 8043B6D8 (Pnp_inputScale) 20
    static f32 sServeHitHeight;        // 8043B6DC (Pnp_serveHitHeight) 10
    static f32 sReturnSpinScale;       // 8043B6E0 (Pnp_returnSpinScale) 0.7
    static f32 sSwingSpeedScale;       // 8043B6E4 (Pnp_swingSpeedScale) 4
    static f32 sSlowSwingSpeed;        // 8043B6E8 (Pnp_hitParamA) 1
    static f32 sHumanFollowRate;       // 8043B6EC (Pnp_humanFollowRate) 0.35
    static f32 sHumanMaxX;             // 8043B6F0 (Pnp_humanMaxX) 170
    static f32 sHumanMinX;             // 8043B6F4 (Pnp_humanMinX) -155
    static f32 sHumanMoveSpeed;        // 8043B6F8 (Pnp_humanParamA) 0.5
    static f32 sSmashSpeedScale;       // 8043B6FC (Pnp_hitParamB) 1.5
    static f32 sSmashGravityScale;     // 8043B700 (Pnp_hitParamC) 2
    static f32 sSmashWindow;           // 8043B704 (Pnp_hitParamD) 0.9
    static f32 sCpuLandingOffset;      // 8043B708 (Pnp_cpuParamA) 60
    static nw4r::math::VEC3 sServeBox;    // 8043A710 (Pnp_racketParams) racket box at the serve
    static nw4r::math::VEC3 sHumanBox;    // 8043A71C racket box, human
    static nw4r::math::VEC3 sCpuBox;      // 8043A728 racket box, CPU
    static nw4r::math::VEC3 sSpinFactors; // 8043A734 (Pnp_spinFactors)
    static bool sSwingHintShown;       // 8043C800 (Pnp_cpuSwingHintShown)
    static RPGrpHandle sHandAnmHandles[3]; // 8043C804 (Pnp_handAnmHandles)
};

//! @}

#endif
