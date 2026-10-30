#ifndef RP_PARTY_BOM_SOUND_H
#define RP_PARTY_BOM_SOUND_H
#include <Pack/types_pack.h>

#include <Pack/RPAudio/RPSndObject.h>
#include <Pack/RPAudio/RPSndSceneSound.h>

//! @addtogroup rp_bom
//! @{

/**
 * @brief Scene sound object (no ledger class; vtable 8035E718, built inline in
 * RPBomScene::Configure). Size 0x18, verified.
 * @note Name is a project convention.
 */
class RPBomSoundHolder : public RPSndSceneSound {
public:
    virtual ~RPBomSoundHolder();       // at 0x8
    virtual void initSound3D(s32 arg); // at 0xC empty
};

/**
 * @brief 3D sound actor created by RPBomPoolObj::Create (unused by the bubbles, which override
 * Create)
 * @details RPSndObject with only the destructor overridden (key function in RPBomPlayer.cpp),
 * vtable 8035E9AC (parts at 0x10 / 0x4C: 8035E9B4 / 8035E9D8). Size 0x84, verified (allocated
 * inline in Bom_PoolObj_create, remote flag 0xF). Base of RPBomPlayerSound: the weak
 * ~RPBomPlayerSound kept from RPBomPlayer.cpp has one more inlined destructor level (an extra
 * null check) than ~RPBomSoundActorBase.
 * @note Name is a project convention (ledger Bom_SoundActorBase).
 */
class RPBomSoundActorBase : public RPSndObject {
public:
    /**
     * @brief Inlined into RPBomPoolObj::Create
     */
    RPBomSoundActorBase(nw4r::snd::SoundArchivePlayer& rPlayer, u32 remoteFlag = FLAG_REMOTE_ALL)
        : RPSndObject(rPlayer, *static_cast<nw4r::snd::Sound3DManager*>(NULL), remoteFlag) {}

    virtual ~RPBomSoundActorBase(); // at 0x40
};

/**
 * @brief 3D sound actor of a player (and of each bubble, RPBomPanel::mpSound)
 * @details RPBomSoundActorBase (RPSndObject = TRPSndObject<4>, vtable 8035C6BC, also used by
 * Wly and Cow) with only the destructor overridden: vtable 8035E8D0 (parts at 0x10 / 0x4C:
 * 8035E8D8 / 8035E8FC). Size 0x84, verified (allocated inline in Bom_Player_create and Bom_Panel_create). The remote
 * flag is 1 or 2 by player for the players and 0xF for the bubbles. The ledger's
 * Wly_SoundActor_* slots are RPSndObject's.
 * @note Name is a project convention (ledger Bom_PlayerSound).
 */
class RPBomPlayerSound : public RPBomSoundActorBase {
public:
    /**
     * @brief Inlined into RPBomPanel::Create (remote flag 0xF)
     */
    RPBomPlayerSound(nw4r::snd::SoundArchivePlayer& rPlayer, u32 remoteFlag = FLAG_REMOTE_ALL)
        : RPBomSoundActorBase(rPlayer, remoteFlag) {}

    // Inline: a weak copy (vtable 8035E8D0) is kept from RPBomPlayer.cpp; RPBomPanel_2.cpp
    // has a zero-filled duplicate
    virtual ~RPBomPlayerSound() {} // at 0x40

    // RPBomPlayer::Create (1 or 2 by player)
    void SetRemoteFlag(u32 flag) {
        mRemoteFlag = flag;
    }
};

/**
 * @brief BGM track control of the game manager (RPBomGameManager::mpBgm)
 * @details No vtable. Size 0x14, verified (Bom_GameManager_create). Tracks (mTrack): 0 play
 * BGM (sound 0x21, slowed to 0.83 tempo on some setups), 1 special-bubble jingle (no change),
 * 2 wait for bubbles, 3 level clear (0x25), 4 / 5 first and second miss (0x1D / 0x1E), 6 / 7
 * game over (0x1F / 0x20), 8 / 9 resume after a miss (0x22), 10 all clear (0x26), 11 stopped.
 * Update mutes all tracks but mTrackMask (1, 3 or 0x7F by track) and, when the sequence's
 * global variable 0 reaches 4, moves the miss tracks back to 8 / 9.
 * @note Name is a project convention (ledger Bom_Bgm).
 */
class RPBomBgm {
public:
    RPBomBgm();

    BOOL HoldJingle(u32 id);
    void Update(s32 activeBubbles, s32 level);
    void ApplyTrack();
    void RequestTrack(s32 track);
    void Stop();

public:
    s32 mTrack;        // at 0x0
    bool mChanged;     // at 0x4 ApplyTrack pending
    s16 mSeqVar;       // at 0x6 sequence global variable 0
    s16 unk8;          // at 0x8 [L] cleared only
    bool mLoopWait;    // at 0xA [M] false while waiting for the sequence variable
    u32 unkC;          // at 0xC [L] cleared only
    u32 mTrackMask;    // at 0x10 unmuted tracks
};

//! @}

#endif
