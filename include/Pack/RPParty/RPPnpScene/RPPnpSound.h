#ifndef RP_PARTY_PNP_SOUND_H
#define RP_PARTY_PNP_SOUND_H
#include <Pack/types_pack.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPAudio/RPSndObject.h>
#include <Pack/RPAudio/RPSndSceneSound.h>

//! @addtogroup rp_pnp
//! @{

/**
 * @brief Scene sound object (ledger Pnp_SceneSound, vtable 80359550, built inline in
 * RPPnpScene::Configure). Size 0x18, verified.
 */
class RPPnpSceneSound : public RPSndSceneSound {
public:
    virtual ~RPPnpSceneSound();        // at 0x8
    virtual void initSound3D(s32 arg); // at 0xC empty
};

/**
 * @brief The racket's sound actor (ledger Pnp_RacketTrail, vtable 80359714, built inline in
 * RPPnpPlayer::Create; remote flag set to 1 or 2 by player)
 * @details TRPSndObject<0> overriding only the destructor (Wii Play: its destructor null-checks
 * twice, not three times like RPPnpBallTrail's, so there is no TRPSndGameObject level). Size
 * 0x74, verified.
 */
class RPPnpRacketTrail : public TRPSndObject<0> {
public:
    // Wii Play: inlined into RPPnpPlayer::Create
    RPPnpRacketTrail()
        : TRPSndObject<0>(RPSndAudioMgr::instance()->getPlayer(),
                          *static_cast<nw4r::snd::Sound3DManager*>(NULL)) {}
    virtual ~RPPnpRacketTrail(); // at 0x40

    // Wii Play: RPPnpPlayer::Create (Remote speaker of the player)
    void SetRemoteFlag(u32 flag) {
        mRemoteFlag = flag;
    }
};

/**
 * @brief The ball's sound actor (ledger Pnp_BallTrail, vtable 80359638, built inline in
 * RPPnpBall::Create)
 * @details TRPSndGameObject<0> overriding only the destructor. Size 0x7C, verified.
 */
class RPPnpBallTrail : public TRPSndGameObject<0> {
public:
    // Wii Play: inlined into RPPnpBall::Create
    RPPnpBallTrail()
        : TRPSndGameObject<0>(RPSndAudioMgr::instance()->getPlayer(),
                              *static_cast<nw4r::snd::Sound3DManager*>(NULL)),
          mLastHit(0),
          mPitch(1.0f) {}
    virtual ~RPPnpBallTrail(); // at 0x40

    /**
     * @brief Hit sound kind (0-14) from row of sHitSoundTable: even kinds up to 6 on the first
     * Remote's speaker, odd kinds up to 7 on the second's. With strength > 0 the volume drops
     * 0.08 per step (minimum 0.1); otherwise the pitch rises 1.2% per consecutive hit (to 1.5).
     * False when not started.
     * @note Ledger Pnp_BallTrail_startHitSound; its own unit RPPnpSound.cpp after the serve phase
     */
    bool StartHitSound(s32 hit, s32 strength, s32 kind, s32 row);

    static u32 sHitSoundTable[2][14]; // 803598C8 (Pnp_ballTrailSoundTable)

private:
    s32 mLastHit; // at 0x74 0 from Create
    f32 mPitch;   // at 0x78
};

//! @}

#endif
