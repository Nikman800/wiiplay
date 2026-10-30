#ifndef RP_PARTY_BIL_SOUND_H
#define RP_PARTY_BIL_SOUND_H
#include <Pack/types_pack.h>

#include <Pack/RPAudio/RPSndObject.h>
#include <Pack/RPAudio/RPSndObject2D.h>
#include <Pack/RPAudio/RPSndSceneSound.h>

//! @addtogroup rp_bil
//! @{

/**
 * @brief Scene sound object (ledger Bil_SceneSoundRef, vtable 8035DC38, built inline in
 * RPBilScene::Configure). Size 0x18, verified.
 */
class RPBilSceneSound : public RPSndSceneSound {
public:
    virtual ~RPBilSceneSound();        // at 0x8
    virtual void initSound3D(s32 arg); // at 0xC empty
};

/**
 * @brief Wii Play: empty level between TRPSndObject<8> and RPBilSound3D (name is a project
 * convention)
 * @details RPBilSound3D's destructor null-checks three times before TRPSndObject's body
 * (802C7C40), like the TRPSndGameObject sound actors, but the unit has none of
 * RPSndGameObjectHelper's data, so the level is not TRPSndGameObject. Its vtable is never
 * stored.
 */
class RPBilSound3DBase : public TRPSndObject<8> {
public:
    RPBilSound3DBase(nw4r::snd::SoundArchivePlayer& rPlayer, nw4r::snd::Sound3DManager& rManager)
        : TRPSndObject<8>(rPlayer, rManager) {}
};

/**
 * @brief Billiards per-object 3D sound player (ledger Bil_Sound3D, vtable 8035DF98; owned by
 * RPBilBall and RPBilPlayer)
 * @details RPBilSound3DBase (12 handles, TRPSndObject<8> vtable 8035E074) overriding only the
 * destructor (three null checks, 802C7C40), plus four linear volume/pitch maps (slope,
 * intercept) set by the constructor. Size 0xB4, verified.
 */
class RPBilSound3D : public RPBilSound3DBase {
public:
    RPBilSound3D();
    virtual ~RPBilSound3D(); // at 0x40

    // Wii Play: RPBilBall::PlayRollSound passes only the speed
    bool playRoll(f32 speed);
    bool playRollingLoop(f32 linearSpeed, f32 angularSpeed, s32 material);
    bool playImpact(f32 speed, s32 material);
    bool playCueHit(f32 speed, s32 player, s32 remoteFlag);

private:
    f32 mSpinSlope;      // at 0x94 rolling-loop spin volume: 0 at 0.2, 1 at 0.8
    f32 mSpinIntercept;  // at 0x98
    f32 mSlideSlope;     // at 0x9C rolling-loop slide volume: 0 at 0.15, 1 at 0.5
    f32 mSlideIntercept; // at 0xA0
    f32 mCueSlope;       // at 0xA4 cue hit volume: 0.1 at 20, 1 at 64 (end point inferred)
    f32 mCueIntercept;   // at 0xA8
    f32 mRollSlope;      // at 0xAC roll pitch: 1.3 at 3, 1.9 at 4.8 (end point inferred from the slope)
    f32 mRollIntercept;  // at 0xB0
};

/**
 * @brief Billiards 2D sound actor (ledger Bil_SoundActor / Bil_SoundActorPart, vtable
 * 8035DD60, built inline by RPBilCamera and RPBilBallMgr; camera tick and pocket voice sounds)
 * @details RPSndObject2D (WORD_0x14 = 1) overriding only the destructor, plus the camera-tick
 * counter. Size 0x60, verified.
 */
class RPBilSoundActor : public RPSndObject2D {
public:
    RPBilSoundActor() : RPSndObject2D(1) {}
    virtual ~RPBilSoundActor(); // at 0x8

    /**
     * @brief Plays a camera tick while orbiting, more often the faster the angle changes [M]
     */
    bool PlayCameraTick(f32 delta);
    /**
     * @brief Starts a pocket voice on handle 3 [M]
     */
    bool StartPocketSound(u32 id);

private:
    u16 mTickCount; // at 0x5C [M] camera tick counter (not set by the constructor)
};

//! @}

#endif
