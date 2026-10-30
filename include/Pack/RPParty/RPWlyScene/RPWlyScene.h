#ifndef RP_PARTY_WLY_SCENE_H
#define RP_PARTY_WLY_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPSystem.h>

//! @addtogroup rp_wly
//! @{

// Forward declarations
class RPWlyGameMgr;
// Defined in RPWlySound.h, which RPWlyScene.cpp includes after this header: the vtables come out
// in reverse class declaration order (RPWlySoundHolder's first)
class RPWlySoundHolder;

/**
 * @brief Party Pack Find Mii scene
 * @details vtable 8035C570. Size 0x4C, verified against Wii Play's scene creator. Configure
 * attaches the minigame sound (first stage only), creates the RPWlyGameMgr singleton
 * (RPWlyGameMgr::sInstance) and calls its Init; LoadResource loads rp_Wly_sound.brsar and the
 * basic_wanted effects, then RPWlyGameMgr::Create. Reset/Calculate/Exit/UserDraw forward to
 * the game manager.
 */
class RPWlyScene : public RPSysScene {
public:
    RPWlyScene();
    virtual ~RPWlyScene(); // at 0x8

    /**
     * @brief Pauses the cursors' effects and the avatars (RPWlyAvatarMgr::SetPaused) [H]
     */
    virtual void pauseCallBack(bool enter); // at 0x40
    virtual void Configure();               // at 0x44
    virtual void LoadResource();            // at 0x48
    virtual void Reset();                   // at 0x4C
    virtual void Calculate();               // at 0x50
    virtual void Exit();                    // at 0x54

    /**
     * @brief IRPGrpDrawObject::UserDraw override (RPWlyGameMgr::Draw)
     * @details Overriding a virtual of the secondary base appends a slot to the primary
     * vtable (at 0x5C) and a this-adjusting thunk in the IRPGrpDrawObject part (slot 11).
     */
    virtual void UserDraw(); // at 0x5C

private:
    RPWlyGameMgr* mpGameMgr;     // at 0x44 RPWlyGameMgr::sInstance, created in Configure
    RPWlySoundHolder* mpSound;   // at 0x48
};

//! @}

#endif
