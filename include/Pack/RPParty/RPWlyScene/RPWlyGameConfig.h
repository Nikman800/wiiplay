#ifndef RP_PARTY_WLY_GAME_CONFIG_H
#define RP_PARTY_WLY_GAME_CONFIG_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/RPSysKokeshiGenInfo.h>
#include <Pack/RPKernel/RPSysKokeshiOverloadInfo.h>
#include <Pack/RPParty/RPWlyScene/RPWlySound.h>

#include <egg/math.h>

//! @addtogroup rp_wly
//! @{

/**
 * @brief Two-player level remap: one tier per 5 stages (RPWlyLevelTable::GetLevelIndex copies
 * it whole)
 */
struct RPWlyTierOffsets {
    s32 mTier[4];
};

/**
 * @brief Find Mii progress kept between stages in the party scene-info singleton
 * (RP_sceneInfoInstance + 0x30)
 * @details No vtable of its own. Size 0xA0. Read at the start of every stage (Configure, the
 * game manager and level table) through RPWlyMiiEntry::GetConfig and written back on clear
 * through RPWlyMiiEntry::SetConfig.
 */
class RPWlyGameConfig {
public:
    s32 mMode;                                 // at 0x0 copied to RPWlyGameMgr::mMode
    s32 mScores[2];                            // at 0x4 [L] copied to RPWlyGameMgr 0x24/0x28
    s32 mTimer;                                // at 0xC frames left (1800 initially)
    RPSysKokeshiGenInfo mGenInfo;              // at 0x10
    RPSysKokeshiOverloadInfo mOverloadInfo;    // at 0x6C
    RPWlySoundHolder* mpSceneSound;            // at 0x74 kept from the first stage
    RPWlyTierOffsets mTierOffsets;             // at 0x78 two-player level remap per 5 stages
    EGG::Vector2f mCursorPos[2];               // at 0x88 [M] cursors kept between stages
    f32 mCursorAngles[2];                      // at 0x98 [M] (RPWlyClearPhase, RPWlyGameMgr)
};

/**
 * @brief RPWlyGameConfig with a vtable, embedded at 0x30 of the 0xD8-byte scene-info
 * singleton (vtable 803581E8) built by Mii_createMiiListHolder
 * @details vtable 8035CB38 (pointer after mConfig, at 0xA0). Size 0xA4 [H]: the
 * singleton's RPSysMiddleDB* follows at 0xD4 = 0x30 + 0xA4. The ledger's 0xD8 allocations
 * (Mii_createMiiListHolder, RP_Cls_80358188_vf15) are the singleton's. The constructor sets
 * mTimer to 1800, mTierOffsets to -1 and everything else to 0.
 */
class RPWlyMiiEntry {
public:
    // Declared before the first virtual so that the vtable pointer follows it (at 0xA0), and a
    // member rather than a base: the constructor sets the vtable pointer before constructing it.
    RPWlyGameConfig mConfig; // at 0x0

    RPWlyMiiEntry();
    virtual ~RPWlyMiiEntry(); // at 0x8

    // Wii Play: these inlines stay out of line (weak copies after RPWlyScene::Configure). The
    // getter returns a copy and the setter takes one by value: callers build the argument copy
    // inline before the call (ledger Wly_GameConfig_copyCtor and Wly_GameConfig_assign).
    RPWlyGameConfig GetConfig() const {
        return mConfig;
    }
    void SetConfig(RPWlyGameConfig config) {
        mConfig = config;
    }
};

//! @}

#endif
