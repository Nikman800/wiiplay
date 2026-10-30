#ifndef RP_PARTY_TNK_BGM_MIX_H
#define RP_PARTY_TNK_BGM_MIX_H
#include <Pack/types_pack.h>

//! @addtogroup rp_tnk
//! @{

/**
 * @brief Stage music: picks the track mask from the stage, enemies left and strongest enemy
 * @details No vtable; allocated with 0x14 bytes in RPTnkGameManager::Create (constructor =
 * Reset). Size 0x14, verified. The track masks come from Tnk_bgmTrackMasks.
 */
class RPTnkBgmMix {
public:
    RPTnkBgmMix();

    /**
     * @brief Track mask 0x411, everything else 0 [H]
     */
    void Reset();
    void Stop();
    void StartStageBgm(s32 stage, s32 enemyNum, s32 maxEnemyType);
    void PlayStageClearBgm();
    void SetParams(s32 enemyNum, s32 maxEnemyType);
    void ApplyTrackMask();

private:
    u32 mTrackMask;    // at 0x0
    s32 mStage;        // at 0x4
    s32 mEnemyNum;     // at 0x8
    s32 mMaxEnemyType; // at 0xC
    bool mPlaying;     // at 0x10
};

//! @}

#endif
