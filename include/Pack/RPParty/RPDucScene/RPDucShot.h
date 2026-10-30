#ifndef RP_PARTY_DUC_SHOT_H
#define RP_PARTY_DUC_SHOT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPDucScene/RPDucSound.h>
#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>
#include <Pack/RPParty/RPPartyCommon/RPPosHistory.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_duc
//! @{

// Forward declarations
class RPCursorTrail;
class RPSysEffect;
class RPDucShtGrass;

/**
 * @brief Pointer position history of one controller (100 samples)
 * @details vtable 8035AD70 (at 0x10, RPPosHistory's vtable pointer). Size 0x18, verified
 * (Duc_Shot_init). PullFromController pushes the controller's current core status instead of
 * the buffered ones.
 * @note Name is a project convention (ledger Duc_ShotPointer).
 */
class RPDucShotPointer : public RPPosHistory {
public:
    RPDucShotPointer(s32 channel, s32 capacity);
    virtual ~RPDucShotPointer();          // at 0x8
    virtual void PullFromController();    // at 0x10

private:
    u32 unk14; // at 0x14 [L] never accessed; the allocation is 0x18
};

/**
 * @brief One player's gun: aim from the pointer, cursor effects, shot hit tests
 * @details vtable 8035A450. Size 0xA0, verified. Player 0 or 1 (mPlayer). Position, scale and
 * rotation at 0x4 have the same layout as RPModelObj's but there is no model.
 * Cursor modes (SetCursorMode): 0 main and tail shown, 1 main only, 2 hidden [M].
 * @note Name is a project convention (ledger Duc_Shot).
 */
class RPDucShot : public RPSceneObj {
public:
    RPDucShot();
    virtual ~RPDucShot(); // at 0x8

    /**
     * @brief Creates the cursor trail, the pointer history and mpSndActor [H]
     */
    virtual void Create();    // at 0xC
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Draw();      // at 0x20 empty

    virtual void RestartMainEffect();    // at 0x24
    virtual void RestartTailEffect();    // at 0x28
    virtual void SetCursorMode(s32 mode); // at 0x2C
    virtual void UpdateCursorEffects();  // at 0x30
    virtual void UpdatePointer();        // at 0x34

    /**
     * @brief Creates the main/tail cursor effects of the player [H]
     */
    void CreateCursorEffects(u16 player);
    void ProcessHits();
    void CheckGrassHits();
    void FindHitTargets();
    void PlayShotSound();
    void PlayControllerSound();
    void UpdateAim();
    bool TestRaySphere(const nw4r::math::VEC3& rCenter, f32 radius, nw4r::math::VEC3* pHit,
                       f32* pDist);

private:
    void SetScreenPosition(nw4r::math::VEC2 pos);
    void CheckGrassHit(RPDucShtGrass* pGrass, f32* pDist);

    nw4r::math::VEC3 mPos;          // at 0x4 aim point
    nw4r::math::VEC3 mScale;        // at 0x10
    RPAxes mRot;                    // at 0x1C
    RPCursorTrail* mpCursorTrail;   // at 0x40
    f32 mScreenHalfWidth;           // at 0x44
    nw4r::math::VEC2 mScreenPos;    // at 0x48 [M] reset to (-0.4 or 0.4, 0.15)
    nw4r::math::VEC3 mPrevPos;      // at 0x50
    RPDucShotPointer* mpPointer;    // at 0x5C
    s32 mShotPending;               // at 0x60 [M] hit tests to run
    s32 mShotTimer;                 // at 0x64 [M]
    s32 mFireCooldown;              // at 0x68 [M] frames before the next shot
    nw4r::math::VEC3 mRayDir;       // at 0x6C [M]
    nw4r::math::VEC3 mViewDir;      // at 0x78 [M] camera direction
    s32 mPlayer;                    // at 0x84
    s32 mCursorMode;                // at 0x88
    RPSysEffect* mpMainEffects[2];  // at 0x8C 1P / 2P cursor main
    RPSysEffect* mpTailEffects[2];  // at 0x94 1P / 2P cursor tail
    RPDucSndActor* mpSndActor;      // at 0x9C
};

/**
 * @brief Owns the players' shots (singleton Duc_shotMgrInstance)
 * @details vtable 8035A488. Size 0x1C, verified. Room for four shots; the scene creates two.
 * @note Name is a project convention (ledger Duc_ShotMgr).
 */
class RPDucShotMgr : public RPSceneObj {
public:
    virtual ~RPDucShotMgr(); // at 0x8

    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Exit();      // at 0x1C
    virtual void Draw();      // at 0x20

    static void CreateInstance();
    static void DestroyInstance();

    static RPDucShotMgr* sInstance;

    s32 GetActiveNum() const { return mActiveNum; }

    void CreateShots(s32 num);
    void ProcessHitsAll();
    void SetCursorMode(s32 player, s32 mode);
    void SetCursorModeAll(s32 mode);
    void PlayControllerSoundAll();

private:
    s32 mShotNum;          // at 0x4 created
    s32 mActiveNum;        // at 0x8 1 in single player, else 2
    RPDucShot* mpShots[4]; // at 0xC
};

//! @}

#endif
