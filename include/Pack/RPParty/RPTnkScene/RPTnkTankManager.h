#ifndef RP_PARTY_TNK_TANK_MANAGER_H
#define RP_PARTY_TNK_TANK_MANAGER_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPTnkActor;
class RPTnkCheckMarkPool;
class RPTnkFootmarkTex;
class RPTnkTank;

/**
 * @brief Owns and steps every tank (singleton Tnk_tankManagerInstance)
 * @details vtable 8035B598. Size 0x3C, verified (Tnk_TankManager_createInstance). Player tanks
 * are in mpPlayers, enemies in a pool-like table (mppEnemyBegin..mppEnemyLast, round robin from
 * mppEnemyNext). Also creates the check-mark pool and the tread-mark textures.
 */
class RPTnkTankManager : public RPSceneObj {
public:
    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkTankManager(); // at 0x8

    virtual void Load();  // at 0x10
    virtual void Reset(); // at 0x14
    virtual void Exit();  // at 0x1C also destroys the check-mark pool and tread textures

    void CreateTanks(s32 playerNum, s32 enemyNum);
    void SpawnPlayer(const nw4r::math::VEC3& rPos, s32 player, f32 angle, BOOL show);
    // Assembly passes both spawn arguments in general-purpose registers.
    void SpawnPlayer(const nw4r::math::VEC3& rPos, s32 player, s32 param, BOOL show);
    void SpawnNextEnemy(const nw4r::math::VEC3& rPos, s32 type, s32 idx, BOOL show);
    void ActivatePlayers();
    void ActivateEnemies();
    void DeactivateAll();

    void ApplyVelocities();
    void ClearHitInfo();
    void ResolveCollisions();
    void UpdateTreadDistances();
    void CalcTanks();
    void IncrementTimers();
    void CalcModelMtxAll();
    void DrawTrackMarks();
    void LimitAiFlag110();
    void LimitAiFlag118();

    BOOL FindShellHit(RPTnkActor* pShell, void* pResult);
    BOOL FindMineHit(RPTnkActor* pMine, void* pResult);
    void RecordShellHit(BOOL enemy, s32 idx, const void* pHit);
    void RecordMineHit(BOOL enemy, s32 idx, const void* pHit);
    BOOL TestMineHitTank(RPTnkActor* pMine);
    BOOL TestMineHitTank(RPTnkActor* pMine, s32 idx, BOOL enemy);
    void SeparateTank(s32 idx, bool enemy);
    f32 GetNearestEnemyDistance(const nw4r::math::VEC3& rPos) const;
    f32 GetNearestEnemyDistance(RPTnkActor* pActor);
    s32 GetMaxActiveEnemyType() const;
    u32 GetPlayersDeadState(s32 playerNum) const;
    s32 GetEnemyAliveNum() const { return mEnemyAliveNum; }
    // Wii Play: RPTnkTank::Kill
    void DecEnemyAliveNum() { mEnemyAliveNum--; }
    RPTnkTank* GetPlayerTank(s32 player) const { return mpPlayers[player]; }
    // Wii Play: RPTnkAiAttack::PathHitsAlly walks every enemy tank
    RPTnkTank** GetEnemyBegin() const { return mppEnemyBegin; }
    RPTnkTank** GetEnemyLast() const { return mppEnemyLast; }

    static RPTnkTankManager* sInstance; // Tnk_tankManagerInstance

private:
    s32 mPlayerTankNum;              // at 0x4 2
    s32 mEnemyCapacity;              // at 0x8 8
    s32 mActivePlayerNum;            // at 0xC 1 or 2 by mode
    s32 mEnemyLeft;                  // at 0x10 [M] reset to mEnemyCapacity
    s32 mEnemyAliveNum;              // at 0x14
public:
    RPTnkTank* mpPlayers[4];         // at 0x18
private:
    RPTnkTank** mppEnemyBegin;       // at 0x28
    RPTnkTank** mppEnemyLast;        // at 0x2C
    RPTnkTank** mppEnemyNext;        // at 0x30
    RPTnkCheckMarkPool* mpCheckMarkPool; // at 0x34
    RPTnkFootmarkTex* mpFootmarkTex; // at 0x38
};

//! @}

#endif
