#ifndef RP_PARTY_HKY_SCENE_H
#define RP_PARTY_HKY_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPAudio/RPSndSceneSound.h>
#include <Pack/RPSystem.h>

// Forward declarations
class RPHkyGame;
class RPHkySceneSound;

//! @addtogroup rp_hky
//! @{

/**
 * @brief Party Pack Laser Hockey scene
 * @details vtable 80359140. Size 0x4C, verified against Wii Play's scene creator. Forwards the
 * lifecycle hooks to the RPHkyGame singleton.
 */
class RPHkyScene : public RPSysScene {
public:
    RPHkyScene();
    virtual ~RPHkyScene(); // at 0x8

    virtual void Configure();    // at 0x44
    virtual void LoadResource(); // at 0x48
    virtual void Reset();        // at 0x4C
    virtual void Calculate();    // at 0x50
    virtual void Exit();         // at 0x54

    /**
     * @brief IRPGrpDrawObject::UserDraw override
     * @details Overriding a virtual of the secondary base appends a slot to the primary
     * vtable (at 0x5C) and a this-adjusting thunk in the IRPGrpDrawObject part (slot 11).
     */
    virtual void UserDraw(); // at 0x5C

private:
    RPHkyGame* mpGame;            // at 0x44 Hky_gameInstance, created in Configure
    RPHkySceneSound* mpSound;     // at 0x48
};

/**
 * @brief Scene sound object (ledger Hky_SceneSound, vtable 80359130, built inline in
 * RPHkyScene::Configure). Size 0x18, verified.
 */
class RPHkySceneSound : public RPSndSceneSound {
public:
    virtual ~RPHkySceneSound();        // at 0x8
    virtual void initSound3D(s32 arg); // at 0xC empty
};

//! @}

#endif
