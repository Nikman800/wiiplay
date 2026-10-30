#ifndef RP_PARTY_BIL_BALL_MGR_H
#define RP_PARTY_BIL_BALL_MGR_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPBilScene/RPBilObject.h>

#include <egg/math.h>

#include <nw4r/math.h>

//! @addtogroup rp_bil
//! @{

// Forward declarations
class RPBilAimMarker;
class RPBilBall;
class RPBilBallIcon;
class RPBilCueGhost;
class RPSysKokeshiIcon;
struct RPBilSegment;

/**
 * @brief One table contact of a ball, input of RPPhysSphere::CollideA
 * @details No vtable (empty constructor and destructor, ledger Bil_ContactInput). Size 0x20:
 * RPBilBallMgr::CalcTableCollisions builds an array of 0x40 on the stack from the
 * table.kcl hits. Name is a project convention.
 */
struct RPBilContactInput {
    // Weak copies in RPBilBallMgr.cpp, after CalcTableCollisions (which builds the arrays)
    RPBilContactInput() {}
    ~RPBilContactInput() {}

    nw4r::math::VEC3 unk0;   // at 0x0 [L]
    nw4r::math::VEC3 mPoint; // at 0xC [M] contact point (the kcl hit)
    f32 unk18;               // at 0x18 [L]
    f32 unk1C;               // at 0x1C [L]
};

/**
 * @brief One resolved contact, output of RPPhysSphere::CollideA
 * @details No vtable (ledger Bil_ContactResult). Size 0x14 (array of 0x40 on the stack of
 * RPBilBallMgr::CalcTableCollisions). Name is a project convention.
 */
struct RPBilContactResult {
    // Weak copies in RPBilBallMgr.cpp, after CalcTableCollisions (which builds the arrays)
    RPBilContactResult() {}
    ~RPBilContactResult() {}

    bool mHit;  // at 0x0 [M]
    u32 unk4;   // at 0x4 [L]
    u32 unk8;   // at 0x8 [L]
    u32 unkC;   // at 0xC [L]
    f32 mSpeed; // at 0x10 [M] impact speed (RPBilBall::PlayKclHitSound)
};

/**
 * @brief The ten balls, their aim markers and pocketed-ball icons, the cue ghost and the
 * physics step (singleton RPBilBallMgr::sInstance)
 * @details vtable 8035DEF8. Size 0x104, verified (Bil_BallMgr_create). The constructor builds
 * the balls, aim markers, cue ghost and ball icons, two Mii icons (RPSysKokeshiIcon, 0xC0 px)
 * for the cue-ball faces and the 0x60-byte sound actor (pocket voices). Calculate steps the
 * balls in substeps (CalcTableCollisions against table.kcl and the pockets, CalcBallCollisions
 * between balls) until all are at rest. mRackMode cycles through the racks (ball counts read
 * from a table, normally 9 balls + the cue ball).
 */
class RPBilBallMgr : public RPActorBase {
public:
    static RPBilBallMgr* sInstance; // 8043C9F8

    static const int BALL_MAX = 10;
    static const int KOKESHI_ICON_NUM = 2;
    static const int RACK_MODE_NUM = 2;

public:
    RPBilBallMgr();
    virtual ~RPBilBallMgr(); // at 0x8

    virtual void Load();      // at 0x78
    virtual void Reset();     // at 0x7C
    virtual void Calculate(); // at 0x80
    virtual void Exit();      // at 0x84

    virtual void UserDraw(); // at 0x88 empty

    void NextRackMode();
    /**
     * @brief Mii face of a player on the cue ball (-1: none) [M]
     */
    void SetCueBallFace(s32 side, s32 player);
    bool AreIconsIdle() const;
    void ResetIcons();
    bool AreIconsDone() const;
    void ShowPocketedIcons(f32 y, u32 player);
    void PlaceCueBall(const EGG::Vector3f& rPos);
    /**
     * @brief Ball `ball` fits at rPos: inside the table and clear of the balls on it [M]
     */
    bool CanPlaceBall(s32 ball, const EGG::Vector3f& rPos) const;
    void HideCueGhost();
    void ShowCueGhost(const EGG::Vector3f& rPos, bool valid);
    void HideAimMarkers();
    /**
     * @brief Shows the first aim marker at rPos [M]
     */
    void ShowAimMarker(const nw4r::math::VEC3& rPos);
    /**
     * @brief Lowest-numbered object ball still on the table, -1 when none [H]
     */
    s32 GetLowestBall() const;
    /**
     * @brief Ends the current shot: counts it, clears the per-shot hit data [H]
     */
    void EndShot();
    s32 SumPocketedNumbers() const;
    s32 CountPocketed() const;
    bool IsBallPocketed(s32 ball) const;
    /**
     * @brief Nearest object ball hit by the cue ball moving to rTarget: index and cue ball
     * position at the hit [M]
     */
    bool RaycastObjectBalls(const EGG::Vector3f& rTarget, s32* pBall, EGG::Vector3f* pHitPos) const;
    /**
     * @brief Nearest ball on the table hit by a sphere of the given radius moving along rSeg
     * [M]
     */
    bool RaycastBalls(const RPBilSegment& rSeg, s32* pBall, EGG::Vector3f* pHitPos,
                      f32 radius);
    bool HasObjectBallsOnTable() const;
    bool AreAllBallsStopped() const;
    void CalcTableCollisions();
    void CalcBallCollisions();

    RPBilBall* GetBall(s32 i) const {
        return mpBalls[i];
    }
    s32 GetRackMode() const {
        return mRackMode;
    }
    s32 GetBallNum() const {
        return mBallNum;
    }
    s32 GetFirstHitBall() const {
        return mFirstHitBall;
    }
    s32 GetUnk70() const {
        return unk70;
    }
    void SetUnk78(s32 value) {
        unk78 = value;
    }

private:
    /**
     * @brief The ball went into a pocket during the current shot [M]
     */
    bool IsPocketedThisShot(s32 ball) const;
    /**
     * @brief The ball left the table during the current shot [M]
     */
    bool IsOutThisShot(s32 ball) const;

public:

    static void CreateInstance();
    static void DestroyInstance();

private:
    s32 mRackMode;                                     // at 0x60 1 at construction
    s32 mBallNum;                                      // at 0x64 balls in play (10)
    s32 mShotNo;                                       // at 0x68 [M] incremented by EndShot
    s32 mFirstHitBall;                                 // at 0x6C [M] -1 after each shot
    s32 unk70;                                         // at 0x70 [L] -1 after each shot
    bool unk74;                                        // at 0x74 [L]
    s32 unk78;                                         // at 0x78 [L] -1 after Reset
    RPBilBall* mpBalls[BALL_MAX];                      // at 0x7C
    RPBilAimMarker* mpAimMarkers[BALL_MAX];            // at 0xA4
    RPBilCueGhost* mpCueGhost;                         // at 0xCC
    RPBilBallIcon* mpBallIcons[BALL_MAX];              // at 0xD0
    RPSysKokeshiIcon* mpKokeshiIcons[KOKESHI_ICON_NUM]; // at 0xF8
    RPBilSoundActor* mpSoundActor;                     // at 0x100

    /**
     * @brief Balls in play per rack mode, 80441318
     */
    static const s32 sBallNum[RACK_MODE_NUM];

public:
    static f32 sCushionParam0;
    static f32 sCushionParam1;
    static f32 sCushionParam2;
    static f32 sCushionParam3;
    static f32 sCushionParam4;
    static f32 sCushionParam5;
    static f32 sCushionParam6;
    static f32 sCushionParam7;
    static f32 sCushionParam8;
    static f32 sCushionParam9;
    static f32 sCushionParam10;
    static f32 sCushionParam11;
    static f32 sCushionParam12;
    static f32 sCushionParam13;
    static f32 sBallCollisionParamA;
    static f32 sBallCollisionParamB;
    static f32 sCushionParam14;
};

//! @}

#endif
