#ifndef RP_PARTY_HKY_SOUND_H
#define RP_PARTY_HKY_SOUND_H
#include <Pack/types_pack.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPAudio/RPSndObject.h>
#include <Pack/RPAudio/RPSndSceneSound.h>

//! @addtogroup rp_hky
//! @{

// Wii Play: RPHkySceneSound is declared in RPHkyScene.h, after RPHkyScene (the order the
// original emits their vtables in)

/**
 * @brief The puck's sound actor (ledger Hky_PuckTrail, vtable 803591C8, built inline in
 * RPHkyPuck::Create; goal / hit sounds)
 * @details TRPSndGameObject<0> overriding only the destructor. Size 0x7C, verified.
 */
class RPHkyPuckTrail : public TRPSndGameObject<0> {
public:
    // Wii Play: inlined into RPHkyPuck::Create
    RPHkyPuckTrail()
        : TRPSndGameObject<0>(RPSndAudioMgr::instance()->getPlayer(),
                              *static_cast<nw4r::snd::Sound3DManager*>(NULL)),
          unk74(0),
          unk78(1.0f) {}
    virtual ~RPHkyPuckTrail(); // at 0x40

    /**
     * @brief Starts sound id with volume 1.5 * speed clamped to [0.5, 1]; false if not started
     */
    bool StartSoundBySpeed(u32 id, f32 speed);

private:
    u32 unk74; // at 0x74 [L] 0 from Create
    f32 unk78; // at 0x78 [L]
};

/**
 * @brief The paddle's sound actor (ledger Hky_PaddleTrail, vtable 80359328, built inline in
 * RPHkyPaddle::Create; swing sound started above a speed threshold)
 * @details TRPSndGameObject<0> overriding only the destructor. Size 0x7C, verified.
 */
class RPHkyPaddleTrail : public TRPSndGameObject<0> {
public:
    // Wii Play: inlined into RPHkyPaddle::Create
    RPHkyPaddleTrail()
        : TRPSndGameObject<0>(RPSndAudioMgr::instance()->getPlayer(),
                              *static_cast<nw4r::snd::Sound3DManager*>(NULL)),
          mPrevSpeed(0.0f),
          mSwinging(false) {}
    virtual ~RPHkyPaddleTrail(); // at 0x40

    /**
     * @brief Holds sound id while the swing is on: on once the speed rises above 50, off below 20
     */
    bool HoldSoundBySpeed(u32 id, f32 speed);

private:
    f32 mPrevSpeed; // at 0x74
    bool mSwinging; // at 0x78 false from Create
};

//! @}

#endif
