#ifndef RP_PARTY_DUC_SOUND_H
#define RP_PARTY_DUC_SOUND_H
#include <Pack/types_pack.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPAudio/RPSndObject.h>
#include <Pack/RPAudio/RPSndSceneSound.h>

//! @addtogroup rp_duc
//! @{

/**
 * @brief Constructs an RPSndObject (TRPSndObject<4>); never called
 * @details Wii Play: Shooting Range files emitted the weak TRPSndObject<4> vtables
 * (TRPSndObject<4> 0xDC, TRPSndObjectBase<4> 0x44, TRPSndAudioActorBaseWithHandles<4,4> 0x38,
 * TRPSndAudioHandles<8> 0x10). RPDucGame keeps their 0x168 bytes as a zero-filled duplicate
 * before __vt__9RPDucGame; that needs an earlier copy (RPDucScene, where it is trailing data the
 * linker drops). Files that define RP_DUC_SOUND_INSTANTIATE_SND_OBJECT before including this
 * header instantiate it; the copies are dead-stripped. The block is all zeros, so the original N
 * is unknown: N=1 is a stand-in. It must not be 4: compiled Duc units would then emit the weak
 * @16@/@76@ TRPSndObject<4> thunks first and take them from RPWlySound, which owns them.
 */
template <int N> void RPDucSndObjectInstantiate() {
    new TRPSndObject<N>(RPSndAudioMgr::instance()->getPlayer(),
                        *static_cast<nw4r::snd::Sound3DManager*>(NULL));
}
#ifdef RP_DUC_SOUND_INSTANTIATE_SND_OBJECT
template void RPDucSndObjectInstantiate<1>();
#endif

/**
 * @brief Scene sound object (no ledger class; vtable 80359A98, built inline in
 * RPDucScene::Configure). Size 0x18, verified.
 */
class RPDucSceneSound : public RPSndSceneSound {
public:
    virtual ~RPDucSceneSound(); // at 0x8

    /**
     * @brief With 0, sets the four players' 3D sound parameters in RPSndAudioMgr (mode 0x20,
     * 2400 / 80 / 1200) [M]
     */
    virtual void initSound3D(s32 arg); // at 0xC
};

/**
 * @brief Sound actor of the game, the shots, the grass and every target (ledger
 * Duc_SndActor, vtable 80359B00, built inline in their init functions)
 * @details TRPSndObject<0> overriding only the destructor. Size 0x74, verified.
 */
class RPDucSndActor : public TRPSndObject<0> {
public:
    RPDucSndActor()
        : TRPSndObject<0>(RPSndAudioMgr::instance()->getPlayer(),
                          *static_cast<nw4r::snd::Sound3DManager*>(NULL)) {}
    virtual ~RPDucSndActor(); // at 0x40

    void SetRemoteFlag(u32 flag) {
        mRemoteFlag = flag;
    }
};

//! @}

#endif
