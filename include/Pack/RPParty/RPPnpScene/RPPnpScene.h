#ifndef RP_PARTY_PNP_SCENE_H
#define RP_PARTY_PNP_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPSystem.h>

//! @addtogroup rp_pnp
//! @{

// Forward declarations
class RPPnpGame;
// Declared in RPPnpSound.h, which the .cpp includes after this header: the vtables come out in
// reverse class declaration order (RPPnpSceneSound first, as in Laser Hockey)
class RPPnpSceneSound;

/**
 * @brief Party Pack Table Tennis scene
 * @details vtable 80359560. Size 0x4C, verified (Sys_SceneCreator_createGameScene allocates
 * 0x4C). Same shape as RPHkyScene: Configure creates the RPPnpGame singleton (Pnp_gameInstance)
 * and the scene sound object; the other hooks forward to the game.
 */
class RPPnpScene : public RPSysScene {
public:
    RPPnpScene();
    virtual ~RPPnpScene(); // at 0x8

    virtual void Configure();    // at 0x44
    virtual void LoadResource(); // at 0x48
    virtual void Reset();        // at 0x4C
    virtual void Calculate();    // at 0x50
    virtual void Exit();         // at 0x54

    /**
     * @brief IRPGrpDrawObject::UserDraw override (primary slot 0x5C, thunk in slot 11)
     */
    virtual void UserDraw(); // at 0x5C

private:
    RPPnpGame* mpGame;        // at 0x44 Pnp_gameInstance
    RPPnpSceneSound* mpSound; // at 0x48
};

//! @}

#endif
