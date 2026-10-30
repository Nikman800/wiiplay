#ifndef RP_PARTY_BIL_BALL_H
#define RP_PARTY_BIL_BALL_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPBilScene/RPBilObject.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>
#include <Pack/RPParty/RPPartyCommon/RPPhysSphere.h>

#include <egg/math.h>

#include <nw4r/math.h>

//! @addtogroup rp_bil
//! @{

// Forward declarations
class RPGrpModel;

/**
 * @brief Start and end of a moving sphere's path (RPBilBall::IntersectRay) [M]
 * @details Name is a project convention.
 */
struct RPBilSegment {
    EGG::Vector3f mStart; // at 0x0
    EGG::Vector3f mEnd;   // at 0xC
};
class RPPhysSphere;
class RPSysEffect;

// Wii Play: in the original CheckPocket is const and IntersectRay is not (the constness of this
// decides whether the state machine load is scheduled into the prologue). Both functions match
// only with RP_BIL_BALL_ORIGINAL_CONSTNESS 1, which needs the symbols renamed first:
// 802C18F8 CheckPocket__9RPBilBallCFv, 802C19C0
// IntersectRay__9RPBilBallFRC12RPBilSegmentPQ23EGG8Vector3fPff (RPBilBallMgr calls both).
#ifndef RP_BIL_BALL_ORIGINAL_CONSTNESS
#define RP_BIL_BALL_ORIGINAL_CONSTNESS 1
#endif
#if RP_BIL_BALL_ORIGINAL_CONSTNESS
#define RP_BIL_BALL_CHECK_POCKET_CONST const
#define RP_BIL_BALL_INTERSECT_RAY_CONST
#else
#define RP_BIL_BALL_CHECK_POCKET_CONST
#define RP_BIL_BALL_INTERSECT_RAY_CONST const
#endif

/**
 * @brief A billiard ball (ten made by RPBilBallMgr; index 0 is the cue ball)
 * @details vtable 8035DE68. Size 0x84, verified (Bil_BallMgr_ctor). The ball's motion lives in
 * an RPPhysSphere (mass 100, radius 2.85, rest speed 0.0004); the RPActorBase position,
 * velocity, axis and scale accessors are overridden to forward to it. The constructor builds
 * the 4-state machine from sStateFuncs, the 3D sound player and, for the cue ball, the
 * Bil_ball_tail02 effect. Load builds bad_ball (numbered texture) or bad_ball_0_face_2 (cue
 * ball with the Mii faces of RPBilBreakLottery).
 */
// Wii Play: derives from RPActorBase directly (an RPBilActor level adds a null check to the
// destructor); its own virtuals take RPBilActor's slots.
class RPBilBall : public RPActorBase {
public:
    enum EState {
        STATE_NONE,
        STATE_ON_TABLE,
        STATE_POCKETED,
        STATE_OUT, //!< knocked off the table

        STATE_NUM
    };

public:
    explicit RPBilBall(s32 index);
    virtual ~RPBilBall(); // at 0x8

    virtual const nw4r::math::VEC3& GetPos() const;      // at 0x34
    virtual void SetPos(const nw4r::math::VEC3& rPos);   // at 0x38
    /**
     * @brief The phys sphere's velocity [H]
     */
    virtual const nw4r::math::VEC3& GetVec1C() const;    // at 0x3C
    virtual void SetVec1C(const nw4r::math::VEC3& rVec); // at 0x40
    virtual const nw4r::math::VEC3& GetAxisZ() const;    // at 0x4C
    virtual void SetAxisZ(const nw4r::math::VEC3& rVec); // at 0x50
    virtual const nw4r::math::VEC3& GetAxisY() const;    // at 0x54
    virtual void SetAxisY(const nw4r::math::VEC3& rVec); // at 0x58
    virtual const nw4r::math::VEC3& GetAxisX() const;    // at 0x5C
    virtual void SetAxisX(const nw4r::math::VEC3& rVec); // at 0x60
    virtual f32 GetScale() const;                        // at 0x64
    virtual void SetScale(f32 scale);                    // at 0x68

    virtual void Load();      // at 0x78
    virtual void Reset();     // at 0x7C
    virtual void Calculate(); // at 0x80
    virtual void Exit();      // at 0x84

    virtual void UserDraw(); // at 0x88 empty

    void PlayRollSound(f32 speed);
    void PlayPlaceSound();
    /**
     * @brief Cue hit sound for player 0 or 1 (other players: none) [M]
     */
    void PlayCueHitSound(f32 power, s32 player, s32 ballIndex);
    void PlayBallHitSound(f32 speed);
    void PlayKclHitSound(u32 kind, f32 speed);
    void UpdateRollingSound();
    /**
     * @brief Checks the pocket entry and changes to STATE_POCKETED; returns the pocket or
     * -1 [M]
     */
    s32 CheckPocket() RP_BIL_BALL_CHECK_POCKET_CONST;
    /**
     * @brief Hit of a sphere of the given radius moving along rSeg with this ball: position
     * and distance [M]
     */
    bool IntersectRay(const RPBilSegment& rSeg, EGG::Vector3f* pHitPos, f32* pDist,
                      f32 radius) RP_BIL_BALL_INTERSECT_RAY_CONST;
    /**
     * @brief Rack position of ball index for the ball manager's rack mode [M]
     */
    EGG::Vector3f GetRackPosition(s32 index);
    void PlaceInRack();
    /**
     * @brief STATE_OUT, recording the shot number [H]
     */
    bool SetOut(s32 shot);
    /**
     * @brief STATE_POCKETED, recording the shot number [H]
     */
    bool SetPocketed(s32 shot);
    void ResetToRack();
    void UpdateModel();
    void PreCalc();

    void SetUnk70(bool value) {
        unk70 = value;
    }
    RPPhysSphere* GetPhysSphere() const {
        return mpPhysSphere;
    }
    s32 GetState() const {
        return mpStateMachine->GetState();
    }

private:
    void OnTableEnter();
    void OnTableCalc();
    void PocketedEnter();
    void PocketedCalc();
    void OutEnter();
    void OutCalc();

private:
    s32 mIndex;                                // at 0x60 ball number (0 = cue ball)
public:
    RPStateMachine<RPBilBall>* mpStateMachine; // at 0x64
    s32 mPocketedShot;                         // at 0x68 -1 after Reset
    s32 mOutShot;                              // at 0x6C -1 after Reset
    bool unk70;                                // at 0x70 [L] true after Reset
    bool unk71;                                // at 0x71 [L] true after Reset; touching a cushion
    s32 mHitFlags;                             // at 0x74 [L] cleared at the end of each shot;
                                               // counts out-of-table kcl hits (3: out)
private:
    RPBilSound3D* mpSound;                     // at 0x78
public:
    RPPhysSphere* mpPhysSphere;                // at 0x7C
private:
    RPSysEffect* mpTailEffect;                 // at 0x80 cue ball only (Bil_ball_tail02)

public:
    static f32 sMass;
    static f32 sParam8;
    static f32 sRadius;
    static f32 sParam10;
    static f32 sRestSpeed;
    static f32 sParam18;
};

//! Ledger Bil_BallSM (size 0x28)
typedef RPStateMachine<RPBilBall> RPBilBallSeq;

//! @}

#endif
