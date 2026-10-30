#ifndef RP_PARTY_WLY_SOUND_H
#define RP_PARTY_WLY_SOUND_H
#include <Pack/types_pack.h>

#include <Pack/RPAudio/RPSndObject.h>
#include <Pack/RPAudio/RPSndObject2D.h>
#include <Pack/RPAudio/RPSndSceneSound.h>

//! @addtogroup rp_wly
//! @{

// Forward declarations
class RPWlyTimerSe;

/**
 * @brief Scene sound object (ledger Wly_SoundHolder, vtable 8035C560, built inline in
 * RPWlyScene::Configure on a fresh run and kept in RPWlyGameConfig::mpSceneSound between
 * stages). Size 0x18, verified.
 */
class RPWlySoundHolder : public RPSndSceneSound {
public:
    virtual ~RPWlySoundHolder();       // at 0x8
    virtual void initSound3D(s32 arg); // at 0xC empty
};

/**
 * @brief BGM sound object owned by RPWlyGameMgr (ledger Wly_SoundObj, vtable 8035CB48,
 * allocated in Wly_GameMgr_ctor)
 * @details RPSndObject2D (the four-handle sound object shared with Billiards' RPBilSoundActor;
 * WORD_0x14 from the constructor argument) plus the BGM ducking state. Overrides calc (slot
 * 0x38) with the BGM ducking; PlayBgm starts RPWlySoundObj::sBgmIds[RPWlySoundActor::sFieldIndex]. Size 0x68, verified.
 */
class RPWlySoundObj : public RPSndObject2D {
public:
    virtual ~RPWlySoundObj(); // at 0x8
    virtual void calc();      // at 0x38 ducks the BGM, restored after 60 frames

    bool PlayBgm(s32 stage); // stage is unused
    bool StopBgm();
    bool DuckBgm();

private:
    nw4r::snd::SoundHandle* mpBgmHandle; // at 0x5C [M] NULL from the constructor
    bool mRestoreRequest;                // at 0x60 duck-restore request
    bool mIsDucking;                     // at 0x61
    u16 mDuckFrame;                      // at 0x62 frame counter
    RPWlyTimerSe* mpTimerSe;             // at 0x64

public:
    static const u32 sBgmIds[10]; // per field (RPWlySoundActor::sFieldIndex)
};

/**
 * @brief Countdown jingle and track control
 * @details No vtable; allocated with 0x1C bytes (Wly_GameMgr_ctor, Wly_Avatar_ctor) and
 * constructed by Init. Reads sequence variable 0 of the round BGM each second and switches
 * track masks (RPWlyTimerSe::sTrackMasks) as the time runs out; plays the countdown sound 0x7A
 * (tempo 0.83 when slowed) and the found jingle 0x79.
 * Size 0x1C, verified.
 */
class RPWlyTimerSe {
public:
    RPWlyTimerSe();
    void Reset();
    /**
     * @brief Per frame from RPWlyTimerLyt::Calculate with the seconds left [H]
     */
    void Update(s32 seconds);
    void StartRoundSound();
    void ResetAndPlayFound();

private:
    // Clears the countdown state (30 seconds, fading from 10)
    void ClearState();

    f32 mOnVolume;     // at 0x0 1.0, volume of the tracks in the mask
    f32 mOffVolume;    // at 0x4 0.0, volume of the other tracks
    s32 mSeconds;      // at 0x8 30 after Init
    s32 mLastSeconds;  // at 0xC -1 after Reset
    s16 mPrevSeqVar;   // at 0x10
    s16 mSeqVar;       // at 0x12 sequence variable 0
    s32 mMaskIdx;      // at 0x14 index into RPWlyTimerSe::sTrackMasks
    s32 mFadeSecond;   // at 0x18 10 after Init: second at which the BGM fades

public:
    static const u32 sTrackMasks[10];
    static bool sRoundSoundActive;
};

/**
 * @brief Wii Play: unnamed intermediate level between RPSndObject and RPWlySoundActor (name is a
 * project convention)
 * @details No members or virtuals of its own; its implicit inline destructor is the third null
 * check in ~RPWlySoundActor. Its vtable is never kept.
 */
class RPWlySoundActorBase : public RPSndObject {
public:
    /**
     * @brief Inlined into RPWlyAvatar's constructor (RPWlySound.cpp defines it)
     */
    inline RPWlySoundActorBase(nw4r::snd::SoundArchivePlayer& rPlayer);
};

/**
 * @brief The avatar's 3D sound actor (ledger Wly_SoundActor, vtable 8035C5E0)
 * @details RPSndObject (TRPSndObject<4>, vtable 8035C6BC, also used by Cow and Bom) plus the
 * avatar voice timer. Size 0x88, verified (allocated in Wly_Avatar_ctor). Overrides only the
 * destructor. The ledger's Wly_SoundActor_* methods (slots 0x8-0xD8) are RPSndObject's.
 */
class RPWlySoundActor : public RPWlySoundActorBase {
public:
    /**
     * @brief Inlined into RPWlyAvatar's constructor (RPWlySound.cpp defines it)
     */
    inline RPWlySoundActor();
    virtual ~RPWlySoundActor(); // at 0x40

    /**
     * @brief Plays voice kind 0 wrong / 1 last found / 2 found / 3 hover / 4+ fallback for
     * player 1 or 2 (RPWlySoundActor::sVoiceIds / RPWlySoundActor::sVoiceLabels by RPWlySoundActor::sFieldIndex) [M]
     * (ledger Wly_playAvatarVoice)
     */
    bool PlayVoice(s32 kind, s32 player);

private:
    RPWlyTimerSe* mpTimerSe; // at 0x84

public:
    static s32 sFieldIndex; // BGM / voice table index 0-9, set by RPWlyField::Create
    static const u32 sVoiceLabels[4][10];
    static const u32 sVoiceIds[4][10];
};

//! @}

#endif
