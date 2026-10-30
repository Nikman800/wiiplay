#ifndef RP_PARTY_TNK_AI_H
#define RP_PARTY_TNK_AI_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>
#include <Pack/RPParty/RPTnkScene/RPTnkActor.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPTnkTank;
struct RPAxes;

/**
 * @brief Invisible actor swept along a straight path to predict shots and movement
 * @details vtable 8035C100. Size 0x140, verified (RPTnkAiMove / RPTnkAiAttack constructors).
 * Sweep steps the probe from mStartPos along mDir (mStepSize per step, up to mMaxSteps)
 * through the field collision; OnHitWall records the first wall hit.
 */
class RPTnkAiProbe : public RPTnkActor {
    friend class RPTnkAiMove;
    friend class RPTnkAiAttack;
public:
    /**
     * @brief Sweep result (copied out by Sweep) [M]
     */
    struct Result {
        nw4r::math::VEC3 mHitPos;    // at 0x0
        nw4r::math::VEC3 mHitNormal; // at 0xC
        nw4r::math::VEC3 unk18;      // at 0x18 [L] zero
        nw4r::math::VEC3 mEndPos;    // at 0x24
        nw4r::math::VEC3 mEndVel;    // at 0x30
        s32 mSteps;                  // at 0x3C
        f32 mTime;                  // at 0x40 Wii Play: distance, stored with stfs
        u32 mFlags;                  // at 0x44 1 wall hit, 8 ... [L]
    };

public:
    RPTnkAiProbe();
    virtual ~RPTnkAiProbe(); // at 0x8

    virtual void Reset(); // at 0x14
    virtual void OnHitWall(f32 depth, const nw4r::math::VEC3* pNormal, s32 wall,
                           const nw4r::math::VEC3& rVel); // at 0x8C

    void Sweep(Result* pResult);
    BOOL Step();

    void SetStartPos(nw4r::math::VEC3 pos) {
        mStartPos.x = pos.x;
        mStartPos.y = pos.y;
        mStartPos.z = pos.z;
    }
    void SetDirection(nw4r::math::VEC3 dir) {
        mDir.x = dir.x;
        mDir.y = dir.y;
        mDir.z = dir.z;
    }

private:
    nw4r::math::VEC3 mStartPos; // at 0xB0
    nw4r::math::VEC3 mDir;      // at 0xBC
    f32 mSpeed;                 // at 0xC8
    f32 unkCC;                  // at 0xCC [L]
    s32 mStepSize;              // at 0xD0
    s32 mMaxSteps;              // at 0xD4 250 for shot prediction
    s32 mMode;                  // at 0xD8 1: record the end position
    u32 mStopFlags;             // at 0xDC [L]
    Result mResult;             // at 0xE0
    s32 mStep;                  // at 0x128
    bool mHit;                  // at 0x12C
    nw4r::math::VEC3 mHitPos;   // at 0x130
    bool mHitOther;             // at 0x13C [L]
};

/**
 * @brief Enemy tank movement AI
 * @details vtable 8035C0DC (no overrides but the destructor). Size 0x230, verified
 * (RPTnkAiController constructor). States: 0 approach, 1 evade, 2 keep direction. Incoming
 * shells and nearby mines are kept as up to four threats each, sorted by distance.
 * Parameters come from the enemy's entry (0xA8 bytes) in TnkGameParam.bin.
 */
class RPTnkAiMove : public RPSceneObj {
public:
    typedef void (RPTnkAiMove::*StateFunc)();

    /**
     * @brief Incoming shell (size 0x2C) [L]
     */
    struct ShellThreat {
        nw4r::math::VEC3 mPos;  // at 0x0
        nw4r::math::VEC3 mDir;  // at 0xC
        f32 mDist;              // at 0x18 sort key
        f32 mPerpendicularDist; // at 0x1C
        nw4r::math::VEC3 mPerpendicularDir; // at 0x20
    };

    /**
     * @brief Nearby mine (size 0x1C) [L]
     */
    struct MineThreat {
        nw4r::math::VEC3 mPos;  // at 0x0
        f32 mDist;              // at 0xC sort key
        nw4r::math::VEC3 mDir;  // at 0x10 away from the mine
    };

    /**
     * @brief Movement parameters, loaded from the enemy's entry in TnkGameParam.bin (size 0x40)
     * [L]
     */
    struct Params {
        f32 mAccel;              // at 0x0 speed gain per frame while facing the direction
        f32 mDecel;              // at 0x4 speed factor while turning
        f32 mSpeed;              // at 0x8 top speed (also the probe speed)
        f32 mTurnAngle;          // at 0xC accelerates only within this angle of mDir
        f32 mTurnStep;           // at 0x10 side component of one rotation step
        f32 mMaxAngle;           // at 0x14 random deviation in StateApproach
        f32 mTargetWeight;       // at 0x18 pull toward the target in StateApproach
        s32 mTimerMin;           // at 0x1C state timer range
        s32 mTimerMax;           // at 0x20
        s32 mProbeSteps;         // at 0x24 probe length in StateApproach
        f32 mShellRangeEnemy;    // at 0x28 RPTnkShellPool::DetectIncomingShells
        f32 mShellRangePlayer;   // at 0x2C
        f32 mMineRadiusEnemy;    // at 0x30 RPTnkMinePool::AddTankAvoidance
        f32 mMineRadiusPlayer;   // at 0x34
        s32 mSetFlag2;           // at 0x38 sets controller flag 2 while evading
        s32 mHistoryNum;         // at 0x3C directions averaged
    };

public:
    RPTnkAiMove(RPTnkTank* pTank);
    virtual ~RPTnkAiMove(); // at 0x8

    void LoadParams(s32 type);
    void ResetMovement(); // Wii Play: preserves RPSceneObj::Reset in the vtable.
    void Update(RPAxes* pRot, nw4r::math::VEC3* pVel, const nw4r::math::VEC3& rPos);
    void SetDirection(nw4r::math::VEC3 dir);
    void AddShellThreat(const ShellThreat& rThreat);
    void AddMineThreat(const MineThreat& rThreat);
    BOOL FindClearDirection(f32 dist, const nw4r::math::VEC3& rPos);
    void CalcVelocity(RPAxes* pRot, nw4r::math::VEC3* pVel);
    void TurnTowardDirection(RPAxes* pRot, s32* pSign,
                             const nw4r::math::VEC3& rDir);
    void RotateBodyStep(RPAxes* pRot, const nw4r::math::VEC3& rDir);

    void StateApproach();
    void StateEvade();
    void StateKeepDirection();

    f32 GetSpeed() const { return mParams.mSpeed; }
    const Params& GetParams() const { return mParams; }
    const nw4r::math::VEC3& GetPos() const { return mPos; }
    RPTnkTank* GetTank() const { return mpTank; }
    // Wii Play: RPTnkAiAttack::ExecuteActions sets it after laying a mine; while set, the
    // update picks state 2 until no mine threat is within 16 (StateKeepDirection clears it)
    void SetUnk1F9(bool value) { unk1F9 = value; }

private:
    // Probe set up for a move test (radius 16, every second step, stop flags 9)
    void SetupProbe(const nw4r::math::VEC3& rPos, const nw4r::math::VEC3& rDir, s32 steps) {
        mpProbe->SetStartPos(rPos);
        mpProbe->SetDirection(rDir);
        mpProbe->mSpeed = mParams.mSpeed;
        mpProbe->unkCC = 16.0f;
        mpProbe->mStepSize = 2;
        mpProbe->mMaxSteps = steps;
        mpProbe->mMode = 0;
        mpProbe->mStopFlags = 9;
    }

    // Empty the threat lists before the pools refill them (defined in RPTnkAi_2.cpp)
    inline void ClearShellThreats();
    inline void ClearMineThreats();

    void FillDirectionHistory(const nw4r::math::VEC3& rDir) {
        nw4r::math::VEC3 dir = rDir;
        PSVECNormalize((Vec*)&dir, (Vec*)&dir);
        s32 i = 0;
        do {
            mDirHistory[i].x = dir.x;
            mDirHistory[i].y = dir.y;
            mDirHistory[i].z = dir.z;
            i++;
        } while (i < 8);
    }

    // Fill and average in one body (FindClearDirection): dir sits below sum on the stack,
    // which two nested inlines (FillDirectionHistory + AverageDirectionHistory) reverse
    void ResetDirectionHistory(const nw4r::math::VEC3& rDir) {
        nw4r::math::VEC3 dir = rDir;
        PSVECNormalize((Vec*)&dir, (Vec*)&dir);
        s32 i = 0;
        do {
            mDirHistory[i].x = dir.x;
            mDirHistory[i].y = dir.y;
            mDirHistory[i].z = dir.z;
            i++;
        } while (i < 8);
        s32 count = mParams.mHistoryNum;
        s32 j = 0;
        nw4r::math::VEC3 sum(0.0f, 0.0f, 0.0f);
        do {
            sum.x += mDirHistory[j].x;
            sum.y += mDirHistory[j].y;
            sum.z += mDirHistory[j].z;
            j++;
        } while (j < count);
        if (PSVECMag((Vec*)&sum) != 0.0f) {
            PSVECNormalize((Vec*)&sum, (Vec*)&sum);
            mDir.x = sum.x;
            mDir.y = sum.y;
            mDir.z = sum.z;
        } else {
            mDir.x = mDirHistory[0].x;
            mDir.y = mDirHistory[0].y;
            mDir.z = mDirHistory[0].z;
        }
    }

    void PushDirectionHistory(const nw4r::math::VEC3& rDir) {
        nw4r::math::VEC3 dir = rDir;
        PSVECNormalize((Vec*)&dir, (Vec*)&dir);
        s32 i = 7;
        do {
            mDirHistory[i].x = mDirHistory[i - 1].x;
            mDirHistory[i].y = mDirHistory[i - 1].y;
            mDirHistory[i].z = mDirHistory[i - 1].z;
            i--;
        } while (i >= 1);
        mDirHistory[0].x = dir.x;
        mDirHistory[0].y = dir.y;
        mDirHistory[0].z = dir.z;
    }

    void AverageDirectionHistory() {
        s32 count = mParams.mHistoryNum;
        s32 i = 0;
        nw4r::math::VEC3 sum(0.0f, 0.0f, 0.0f);
        do {
            sum.x += mDirHistory[i].x;
            sum.y += mDirHistory[i].y;
            sum.z += mDirHistory[i].z;
            i++;
        } while (i < count);
        if (PSVECMag((Vec*)&sum) != 0.0f) {
            PSVECNormalize((Vec*)&sum, (Vec*)&sum);
            mDir.x = sum.x;
            mDir.y = sum.y;
            mDir.z = sum.z;
        } else {
            mDir.x = mDirHistory[0].x;
            mDir.y = mDirHistory[0].y;
            mDir.z = mDirHistory[0].z;
        }
    }

    Params mParams;                 // at 0x4
    ShellThreat mShellThreats[4];   // at 0x44
    s32 mShellThreatNum;            // at 0xF4
    MineThreat mMineThreats[4];     // at 0xF8
    s32 mMineThreatNum;             // at 0x168
    f32 unk16C;                     // at 0x16C [L]
    nw4r::math::VEC3 mPos;          // at 0x170 tank position this frame (Update)
    nw4r::math::VEC3 mDirHistory[8]; // at 0x17C
    nw4r::math::VEC3 mDir;          // at 0x1DC
    nw4r::math::VEC3 mSide;         // at 0x1E8 up x mDir
    s32 unk1F4;                     // at 0x1F4 [L] reset to 1
    bool unk1F8;                    // at 0x1F8 [L]
    bool unk1F9;                    // at 0x1F9 [L]
    s32 mTimer;                     // at 0x1FC [L]
    s32 mState;                     // at 0x200
    RPTnkAiProbe* mpProbe;          // at 0x204
    RPTnkTank* mpTank;              // at 0x208
    StateFunc mStateFuncs[3];       // at 0x20C
};

/**
 * @brief Enemy tank attack AI: aiming, firing and mine laying
 * @details vtable 8035C1C8 (no overrides but the destructor). Size 0x120, verified
 * (RPTnkAiController constructor). PredictShotPath sweeps the probe for up to three path
 * segments (bounces) and fires when the path reaches a player and not the shooter or an ally.
 */
class RPTnkAiAttack : public RPSceneObj {
    // Wii Play: LimitAiFlag110 / LimitAiFlag118 delay all but the first tanks about to fire / lay
    // a mine on the same frame (direct member access: accessor inlines change the loads)
    friend class RPTnkTankManager;

public:
    /**
     * @brief One straight part of a predicted shot path (size 0x20) [M]
     */
    struct PathSegment {
        nw4r::math::VEC3 mPos; // at 0x0
        nw4r::math::VEC3 mDir; // at 0xC
        s32 mStartStep;        // at 0x18
        s32 mEndStep;          // at 0x1C

        void Set(const nw4r::math::VEC3& pos, const nw4r::math::VEC3& dir, s32 step) {
            mPos.x = pos.x;
            mPos.y = pos.y;
            mPos.z = pos.z;
            mDir.x = dir.x;
            mDir.y = dir.y;
            mDir.z = dir.z;
            mStartStep = step;
            mEndStep = step;
        }
    };

    /**
     * @brief Attack parameters, loaded from the enemy's entry in TnkGameParam.bin (size 0x4C)
     * [L]
     */
    struct Params {
        s32 mMineUnk78;             // at 0x0 unk78 after a mine is laid [L]
        f32 mAimAngle;              // at 0x4 PickAimTarget: aim deviation range (+-)
        f32 mAimStep;               // at 0x8 side factor of one aim step
        s32 mAimTime;               // at 0xC frames between aim updates
        s32 mFireIntervalMin;       // at 0x10 fire interval range
        s32 mFireIntervalMax;       // at 0x14
        f32 mSelfAngle;             // at 0x18 PathHitsSelf: cone angle (CosDeg)
        f32 mPlayerAngle;           // at 0x1C PathHitsPlayer: cone angle
        f32 mAllyAngle;             // at 0x20 PathHitsAlly: cone angle
        f32 mAllyDist;              // at 0x24 PathHitsAlly: distance to an ally
        s32 mFireUnk74;             // at 0x28 unk74 after firing [L]
        s32 mMineMax;               // at 0x2C mines on the field at most
        s32 mFireUnk78;             // at 0x30 unk78 after firing [L]
        f32 mMineClearDist;         // at 0x34 FindClearDirection distance when laying a mine
        f32 mMineEnemyDist;         // at 0x38 no mine within this distance of an enemy
        s32 mMineIntervalMin;       // at 0x3C mine interval range
        s32 mMineIntervalMax;       // at 0x40
        f32 mMineChance;            // at 0x44 chance (percent) to lay a mine, no block near
        f32 mMineChanceNearBlock;   // at 0x48 chance (percent) to lay a mine next to a block
    };

public:
    RPTnkAiAttack(RPTnkTank* pTank);
    virtual ~RPTnkAiAttack(); // at 0x8

    void LoadParams(s32 type);
    void ResetAttack(); // Wii Play: preserves RPSceneObj::Reset in the vtable.
    void Update(f32 muzzleDist, RPAxes* pTurret, const nw4r::math::VEC3& rPos);
    void SetAimDirection(nw4r::math::VEC3 dir);
    void PickAimTarget();
    void DecideFire();
    // Wii Play: Update forwards the muzzle distance in f1.
    void DecideFire(f32 muzzleDist);
    void DecideLayMine();
    void ExecuteActions();
    // Wii Play: action execution receives the muzzle distance and turret axes.
    void ExecuteActions(f32 muzzleDist, RPAxes* pTurret);
    void PredictShotPath(f32 muzzleDist);
    BOOL PathHitsSelf();
    BOOL PathHitsAlly();
    BOOL PathHitsPlayer();

private:
    f32 GetSegmentLength(const PathSegment& segment) const {
        f32 length = (f32)(segment.mEndStep - segment.mStartStep);
        length *= mShellSpeed;
        return length;
    }

    RPTnkAiProbe* mpProbe;          // at 0x4
    s32 unk8;                       // at 0x8 [L]
    s32 mMaxShells;                 // at 0xC
    s32 mShellBounces;              // at 0x10 path segments = bounces + 1
    f32 mShellSpeed;                // at 0x14 [M]
    Params mParams;                 // at 0x18
    nw4r::math::VEC3 mPos;          // at 0x64 tank position this frame
    s32 unk70;                      // at 0x70 [L]
    s32 unk74;                      // at 0x74 [L]
    s32 unk78;                      // at 0x78 [L]
    s32 mType;                      // at 0x7C enemy type
    nw4r::math::VEC3 mAimTarget;    // at 0x80
    nw4r::math::VEC3 mAimDir;       // at 0x8C
    nw4r::math::VEC3 mAimSide;      // at 0x98
    s32 mSegmentNum;                // at 0xA4
    PathSegment mSegments[3];       // at 0xA8
    bool mFire;                     // at 0x108
    s32 mAimTimer;                  // at 0x10C
    s32 mFireInterval;              // at 0x110 signed (cmpwi in RPTnkTankManager::LimitAiFlag110)
    bool mLayMine;                  // at 0x114
    s32 mMineInterval;              // at 0x118
    RPTnkTank* mpTank;              // at 0x11C
};

/**
 * @brief Enemy tank AI front end owning the move and attack AIs
 * @details vtable 8035C090. Size 0x18, verified (RPTnkTank::Create).
 */
class RPTnkAiController : public RPSceneObj {
public:
    RPTnkAiController(RPTnkTank* pTank);
    virtual ~RPTnkAiController(); // at 0x8

    /**
     * @brief Mode 0, reloads and resets both AIs, clears the flags [H]
     */
    virtual void Reset(); // at 0x14

    void SetMode(s32 mode);
    void SetFlags(u32 flags, BOOL set);
    void SetAimDirection(const nw4r::math::VEC3& rDir);
    void SetMoveDirection(const nw4r::math::VEC3& rDir);
    void UpdateAttack(f32 muzzleDist, RPAxes* pTurret, const nw4r::math::VEC3& rPos);
    void UpdateMove(RPAxes* pRot, nw4r::math::VEC3* pVel,
                    const nw4r::math::VEC3& rPos);

    RPTnkAiAttack* GetAttack() const { return mpAttack; }
    RPTnkAiMove* GetMove() const { return mpMove; }
    u32 GetFlags() const { return mFlags; }

private:
    RPTnkAiAttack* mpAttack; // at 0x4
    RPTnkAiMove* mpMove;     // at 0x8
    s32 mMode;               // at 0xC enemy type
    u32 mFlags;              // at 0x10
    RPTnkTank* mpTank;       // at 0x14
};

//! @}

#endif
