#ifndef RP_PARTY_PNP_BGM_CTRL_H
#define RP_PARTY_PNP_BGM_CTRL_H
#include <Pack/types_pack.h>

//! @addtogroup rp_pnp
//! @{

/**
 * @brief Table Tennis rally music (owned by RPPnpGame at 0x10)
 * @details No vtable; allocated with 0x2C bytes in RPPnpGame::Load (constructor: ledger
 * Duc_80235A04, misattributed to Duc). Size 0x2C, verified. Starts the rally BGM (0x6B, or 0x6C
 * with two players) on the serve, raises the tempo with the rally count (0.96 to 1.14) and fades
 * extra tracks in and out.
 */
class RPPnpBgmCtrl {
public:
    RPPnpBgmCtrl();

    void Stop();
    void StartRallyBgm(BOOL multiPlayer);
    /**
     * @brief Single player: tempo from the rally count and the level table's ball speed
     */
    void UpdateRally(f32 ballSpeed, s32 rally);
    /**
     * @brief Two players: track mutes from the higher score
     */
    void SetCount(u32 pointState, u32 count);

private:
    /**
     * @brief Brings the extra tracks in when flag turns on, fades them out afterwards
     */
    void UpdateTracks(u32 flag);

private:
    s32 mRally;         // at 0x0
    s32 mRallyMod10;    // at 0x4
    s32 mFadeStep;      // at 0x8 counts sequence variable 0 returning to 0
    u32 mTrackMask;     // at 0xC 0xCB8F
    u32 mFadeTrackMask; // at 0x10 0x8B80
    s16 mPrevSeqVar;    // at 0x14
    s16 mSeqVar;        // at 0x16 sequence variable 0
    u8 mLastFlag;       // at 0x18
    bool mFading;       // at 0x19
    f32 mTempo;         // at 0x1C
    u32 mCount;         // at 0x20
    bool mStarted;      // at 0x24
    f32 mFadeVolume;    // at 0x28
};

//! @}

#endif
