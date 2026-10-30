#ifndef RP_AUDIO_SND_SCENE_SOUND_H
#define RP_AUDIO_SND_SCENE_SOUND_H
#include <Pack/types_pack.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>

#include <egg/core.h>

//! @addtogroup rp_audio
//! @{

/**
 * @brief Abstract per-scene sound setup object of the Party Pack minigames
 * @details Wii Play only. vtable 803587D0 (destructor RP_dtor_801FEEF4, slot 0xC pure). Each
 * minigame scene builds a 0x18-byte derived object inline in Configure (Fishing 803587C0,
 * Hockey 80359130, Table Tennis 80359550, Shooting 80359A98, Tanks 8035B0F0, Find Mii
 * 8035C560, Charge! 8035CE08, Billiards 8035DC38, Pose Mii 8035E718) and calls slot 0xC with 0
 * right after construction. The derived classes add no members and override only the
 * destructor (which calls EGG::Disposer's directly) and slot 0xC: empty in most games, the
 * Shooting and Charge! versions set the 3D sound parameters of the four players' Sound3D
 * listeners in RPSndAudioMgr when the argument is 0. Size 0x18, verified.
 * @note Name is a project convention (ledger *_SceneSound / Wly_SoundHolder /
 * Bil_SceneSoundRef).
 */
class RPSndSceneSound : public EGG::Disposer {
public:
    RPSndSceneSound() {
        mpAudioMgr = RPSndAudioMgr::instance();
        unk10 = -1;
    }
    virtual ~RPSndSceneSound() {} // at 0x8

    /**
     * @brief Called with 0 by the scene right after construction [M]
     */
    virtual void initSound3D(s32 arg) = 0; // at 0xC

protected:
    s32 unk10;                 // at 0x10 [L] -1, never read
    RPSndAudioMgr* mpAudioMgr; // at 0x14
};

//! @}

#endif
