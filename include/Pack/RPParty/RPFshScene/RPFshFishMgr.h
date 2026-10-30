#ifndef RP_PARTY_FSH_FISH_MGR_H
#define RP_PARTY_FSH_FISH_MGR_H
#include <Pack/types_pack.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPFshFish;

/**
 * @brief Fish spawn pattern (4 entries at 8031F2E0)
 * @details Size 0x10. Reset picks a pattern by a random percentage against the thresholds at
 * 8031F2D0 (40, 60, 90, 100) when there are two players, else pattern 0. [L]
 * @note Name is a project convention.
 */
struct RPFshSpawnPattern {
    s32 mInitialNum;  // at 0x0 [M] 5, 2, 8, 5
    s32 mMaxSameType; // at 0x4 [M] 2, 1, 4, 4 fish of one type at most
    u8 mTypeRates[6]; // at 0x8 [M] per type: 25/25/25/25/0/0 .. 10/10/10/70/0/0
};

/**
 * @brief Owner of the 30 pond fish (singleton Fsh_fishMgr)
 * @details vtable 80358B40 (destructor only). Size 0x8C, verified (Fsh_FishMgr_create allocates
 * 0x8C and constructs RPFshFish(i, i % 6) for i < 30). The fish are created/reset/exited through
 * their virtuals; the manager spawns new fish while fewer than 50 have been spawned.
 */
class RPFshFishMgr {
public:
    enum {
        FISH_NUM = 30,
        SPAWN_MAX = 50
    };

public:
    RPFshFishMgr();
    virtual ~RPFshFishMgr(); // at 0x8

    void CreateAll();
    void Reset();
    void Calculate();
    void ExitAll();

    s32 CountFishOfType(s32 type) const;
    bool SpawnFishOfType(s32 type);
    void SpawnRandomFish();
    void SpawnInitialFish();
    /**
     * @brief Every fish still in the pond swims again; fishIdx goes to the catch view [M]
     */
    void SetFishToSwim(s32 fishIdx);

    s32 GetUnk10() const {
        return unk10;
    }
    void SetUnk10(s32 value) {
        unk10 = value;
    }
    void AddUnkC() {
        unkC++;
    }
    RPFshFish* GetFish(s32 idx) const {
        return mpFish[idx];
    }

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshFishMgr* GetInstance() {
        return sInstance;
    }

private:
    const RPFshSpawnPattern* GetSpawnPattern() const {
        return &sSpawnPatterns[mPattern];
    }

    s32 mPattern;           // at 0x4 index into sSpawnPatterns
    s32 mSpawnNum;          // at 0x8 fish spawned so far (< SPAWN_MAX)
    s32 unkC;               // at 0xC [L]
    s32 unk10;              // at 0x10 [L] -1 after Reset
    RPFshFish* mpFish[FISH_NUM]; // at 0x14

    static const RPFshSpawnPattern sSpawnPatterns[4]; // 8031F2E0
    static s32 sRareSpawnTimes[4];    // 80358B20 seconds left: 10, 30, 50, 90
    static f32 sRareSpawnChances[4];  // 80358B30 percent: 25, 25, 25, 100

    static RPFshFishMgr* sInstance; // 8043C728
};

//! @}

#endif
