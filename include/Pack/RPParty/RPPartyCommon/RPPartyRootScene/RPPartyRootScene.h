#ifndef RP_PARTY_COMMON_ROOT_SCENE_H
#define RP_PARTY_COMMON_ROOT_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPSystem.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Party Pack root scene
 */
class RPPartyRootScene : public RPSysScene {
public:
    // Wii Play: the constructor is inline (RPSysSceneCreator::createPartyScene; vtable 80358188)
    virtual ~RPPartyRootScene(); // at 0x8

    virtual void Configure(); // at 0x44
    virtual void Reset();     // at 0x4C
    virtual void Exit();      // at 0x54
};

//! @}

#endif
