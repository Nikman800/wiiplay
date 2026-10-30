#ifndef RP_PARTY_TNK_SOUND_H
#define RP_PARTY_TNK_SOUND_H
#include <Pack/types_pack.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPAudio/RPSndObject.h>
#include <Pack/RPAudio/RPSndSceneSound.h>

//! @addtogroup rp_tnk
//! @{

/**
 * @brief Scene sound object (ledger Tnk_SceneSound, vtable 8035B0F0, built inline in
 * RPTnkScene::Configure). Size 0x18, verified.
 */
class RPTnkSceneSound : public RPSndSceneSound {
public:
    virtual ~RPTnkSceneSound();        // at 0x8
    virtual void initSound3D(s32 arg); // at 0xC empty
};

/**
 * @brief Sound emitter of every Tanks! actor (ledger Tnk_ActorSound, vtable 8035B354, built
 * inline in RPTnkActor::Create)
 * @details TRPSndObject<0> overriding only the destructor. Size 0x74, verified.
 */
class RPTnkActorSound : public TRPSndObject<0> {
public:
    RPTnkActorSound()
        : TRPSndObject<0>(RPSndAudioMgr::instance()->getPlayer(),
                          *static_cast<nw4r::snd::Sound3DManager*>(NULL)) {}
    // Wii Play: inline, the weak copy and vtable come out in RPTnkActor.cpp
    virtual ~RPTnkActorSound() {} // at 0x40
};

/**
 * @brief The mine's own sound actor (ledger Tnk_MineSound, vtable 8035BB18, built inline in
 * RPTnkMine::Create; explosion sound 0x16)
 * @details TRPSndObject<0> overriding only the destructor. Size 0x74, verified.
 */
class RPTnkMineSound : public TRPSndObject<0> {
public:
    virtual ~RPTnkMineSound(); // at 0x40
};

/**
 * @brief The tank's engine/tread sound actor (ledger Tnk_TankSound, vtable 8035BF60;
 * Tnk_TankSound_create is its constructor)
 * @details TRPSndObject<0> overriding only the destructor, plus speed smoothing parameters and
 * a tread-sound counter. Size 0x9C, verified (operator new before Tnk_TankSound_create).
 */
class RPTnkTankSound : public TRPSndGameObject<0> {
public:
    RPTnkTankSound();
    virtual ~RPTnkTankSound(); // at 0x40

    /**
     * @brief Moves the smoothed speed towards the tank's speed and plays the tread sound from
     * Tnk_treadSoundIds at an interval that shortens with the speed [M]
     * (ledger Tnk_TankSound_updateTread)
     */
    void UpdateTread(f32 speed, s32 unused);

    /**
     * @brief Moves mSpeed towards the target speed: proportional steps, a fixed step near the
     * top speed; clamped to [0, mMaxSpeed] (ledger Tnk_TankSound_approachSpeed)
     */
    f32 ApproachSpeed(f32 target);

    // Wii Play: RPTnkTank (Reset, Kill)
    void ClearTread() {
        mSpeed = 0.0f;
        mTarget = 0.0f;
        mStep = 0.0f;
        mTreadTimer = 0;
    }
    // Wii Play: RPTnkTank::SetSeVariant
    void SetTreadIndex(s32 idx) { mTreadIndex = idx; }
    // Wii Play: RPTnkTank::SetSePlayerId
    void SetRemoteFlag(u32 flag) { mRemoteFlag = flag; }

private:
    f32 mMaxSpeed;     // at 0x74 1.0
    f32 mAccelRate;    // at 0x78 1/14 of the difference per frame when speeding up
    f32 mDecelRate;    // at 0x7C 1/45 when slowing down
    f32 mTopStep;      // at 0x80 step above 70% of mMaxSpeed
    f32 mSpeed;        // at 0x84 smoothed speed
    f32 mTarget;       // at 0x88 last target speed
    f32 mStep;         // at 0x8C current step
    f32 unk90;         // at 0x90 [L] mMaxSpeed / mMaxSpeed, unused here
    u16 mTreadTimer;   // at 0x94 frame counter of the tread sound
    s32 mTreadIndex;   // at 0x98 index into Tnk_treadSoundIds
};

//! @}

#endif
