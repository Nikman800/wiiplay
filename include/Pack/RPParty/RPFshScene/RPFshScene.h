#ifndef RP_PARTY_FSH_SCENE_H
#define RP_PARTY_FSH_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshSound.h>
#include <Pack/RPSystem.h>

//! @addtogroup rp_fsh
//! @{

/**
 * @brief Party Pack Fishing scene
 * @details vtable 80358760. Size 0x48, verified against Wii Play's scene creator. Configure
 * seeds the random generator, creates the model scenes, views and the RPFshGame singleton;
 * LoadResource registers the fsh_* brres files, the scene light, basic_fishing effects and
 * rp_Fsh_sound.brsar, then has RPFshGame create every object. Reset/Calculate/Exit forward to
 * the RPFshGame singleton (Fsh_gameInstance).
 */
class RPFshScene : public RPSysScene {
public:
    RPFshScene();
    virtual ~RPFshScene(); // at 0x8

    virtual void Configure();    // at 0x44
    virtual void LoadResource(); // at 0x48
    virtual void Reset();        // at 0x4C
    virtual void Calculate();    // at 0x50
    virtual void Exit();         // at 0x54

private:
    RPFshSceneSound* mpSound; // at 0x44 built in Configure, slot 0xC called with 0
};

//! @}

#endif
