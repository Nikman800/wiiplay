#ifndef RP_PARTY_COMMON_MAIN_SCENE_H
#define RP_PARTY_COMMON_MAIN_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPSystem.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Party Pack main menu scene
 */
class RPPartyMainScene : public RPSysScene {
public:
    // Wii Play: the constructor is inline (RPSysSceneCreator::createPartyScene; vtable 80358128)
    virtual ~RPPartyMainScene(); // at 0x8
};

//! @}

#endif
