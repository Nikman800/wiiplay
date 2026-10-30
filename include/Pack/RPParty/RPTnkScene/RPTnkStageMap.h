#ifndef RP_PARTY_TNK_STAGE_MAP_H
#define RP_PARTY_TNK_STAGE_MAP_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPTnkMapData;

/**
 * @brief Reads a stage from RPTnkMapData: blocks, player and enemy spawns (singleton
 * RPTnkStageMap::sInstance)
 * @details vtable 8035B9B0. Size 0xB8, verified (Tnk_StageMap_createInstance). Map cells:
 * 100-107 blocks of kind 0, 200-207 blocks of kind 1 (cell digit = height - 1), 300 player 1,
 * 301 player 2, 400-407 enemy spawn points. Enemy types per stage come from the game
 * parameter file (+0x690 + stage * 0x58); values >= 10 pick a random type in a range.
 */
class RPTnkStageMap : public RPSceneObj {
public:
    static RPTnkStageMap* sInstance; // 8043C918

    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkStageMap(); // at 0x8

    virtual void Create();    // at 0xC
    /**
     * @brief Creates the RPTnkMapData singleton and keeps it [H]
     */
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Exit();      // at 0x1C destroys the RPTnkMapData singleton

    // stage selects enemy parameters; mapStage selects map cells and angles.
    void LoadStage(s32 stage, s32 mapStage, BOOL spawnEnemies, BOOL twoPlayers);
    void ReloadStage(s32 stage, BOOL spawnEnemies, BOOL twoPlayers);
    void SpawnBlocks(s32 stage);
    void ReadSpawnPositions(s32 stage);
    void ReadPlayerAngles(s32 stage);

    // Wii Play: RPTnkTank::Explode clears a destroyed enemy's spawn slot
    void SetEnemyActive(u32 idx, bool active) { mEnemyActive[idx] = active; }

private:
    RPTnkMapData* mpMapData;              // at 0x4
    s32 unk8;                             // at 0x8 [L] reset to 0
    nw4r::math::VEC3 mPlayerPos[2];       // at 0xC map cells 300, 301
    nw4r::math::VEC3 mEnemyPos[8];        // at 0x24 map cells 400-407
    s32 mEnemyTypes[8];                   // at 0x84
    bool mEnemyActive[8];                 // at 0xA4
    bool mSwapPlayers;                    // at 0xAC random in two-player mode
    f32 mPlayerAngles[2];                 // at 0xB0 0/90/180/270 from the map header
};

//! @}

#endif
