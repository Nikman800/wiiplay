#ifndef RP_PARTY_FSH_SOUND_H
#define RP_PARTY_FSH_SOUND_H
#include <Pack/types_pack.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPAudio/RPSndObject.h>
#include <Pack/RPAudio/RPSndSceneSound.h>
#include <Pack/RPAudio/RPSndUtility.h>

//! @addtogroup rp_fsh
//! @{

/**
 * @brief Scene sound object (no ledger class; vtable 803587C0, built inline in
 * RPFshScene::Configure). Size 0x18, verified.
 */
class RPFshSceneSound : public RPSndSceneSound {
public:
    virtual ~RPFshSceneSound();        // at 0x8
    virtual void initSound3D(s32 arg); // at 0xC empty
};

/**
 * @brief Sound actor of a fish and of the player (ledger Fsh_SndActor, vtable 80358888, built
 * inline in the RPFshFish and RPFshPlayer constructors; the player sets the remote flag to 1 or
 * 2)
 * @details TRPSndObject<0> (ledger Fsh_SndActorBase, vtable 80358964) overriding only the
 * destructor. Size 0x78, verified.
 */
// Wii Play: TRPSndGameObject level, from the destructor's three null checks (Fsh_SndActor_dtor)
class RPFshFishSnd : public TRPSndGameObject<0> {
public:
    RPFshFishSnd()
        : TRPSndGameObject<0>(RPSndAudioMgr::instance()->getPlayer(),
                          *static_cast<nw4r::snd::Sound3DManager*>(NULL)),
          mSizeClass(1) {}

    // Wii Play: RPFshPlayer
    void SetRemoteFlag(u32 flag) {
        mRemoteFlag = flag;
    }
    virtual ~RPFshFishSnd(); // at 0x40 (RPFshFish file)

    /**
     * @brief Size class 4 for a rare fish, 0 for the "miss" fish, else 3 / 1 / 2 by size (above
     * 0.7, below 0.3, between) [M]
     */
    void SetSizeClass(u32 isRare, u32 isSuka, f32 size);
    bool StartBite(u32 remoteFlag);
    bool HoldStruggle(u32 remoteFlag);
    bool StartCatch(u32 remoteFlag);

private:
    s32 mSizeClass; // at 0x74 [M] 1 from the constructors
};

/**
 * @brief Sound actor of the rod (ledger Fsh_RodSndActor, vtable 80358CBC, built inline in the
 * RPFshRod constructor)
 * @details TRPSndObject<0> overriding only the destructor. Size 0x74, verified.
 */
// Wii Play: TRPSndGameObject level, from the destructor's extra null check
class RPFshRodSnd : public TRPSndGameObject<0> {
public:
    // Wii Play: the caller passes the player (RPFshRod::RPFshRod); looking it up in here swaps
    // the 3D actor loop counter and player registers
    explicit RPFshRodSnd(nw4r::snd::SoundArchivePlayer& rPlayer)
        : TRPSndGameObject<0>(rPlayer, *static_cast<nw4r::snd::Sound3DManager*>(NULL)) {}
    virtual ~RPFshRodSnd(); // at 0x40 (RPFshRod.cpp)

    /**
     * @brief Holds the line splash sound, louder and higher the more the rod bends [M]
     */
    bool HoldSplash(f32 bend);
};

/**
 * @brief Holds a sound on the scene's global handle (Fsh_holdGlobalSound) [M]
 */
bool RPFshHoldGlobalSound(u32 id);
/**
 * @brief Fade-out frames of the BGM before sound id (2 before 0x68, 10 before 0x71) [M]
 */
s32 RPFshGetBgmFadeFrames(u32 id);
bool RPFshStopAmbientSound();
/**
 * @brief Countdown sound table (Fsh_getStartSoundTable, 0x802136D4; the table is .data at
 * 80359048) [M]
 */
const RPSndRangeTable::Info* RPFshGetStartSoundTable();
void RPFshStartAmbientSound();

//! @}

#endif
