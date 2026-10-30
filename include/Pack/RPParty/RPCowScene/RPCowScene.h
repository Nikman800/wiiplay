#ifndef RP_PARTY_COW_SCENE_H
#define RP_PARTY_COW_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPActorScene.h>

//! @addtogroup rp_cow
//! @{

/**
 * @brief Party Pack Charge! scene
 * @details vtable 8035CD38. Size 0x44, verified (Sys_SceneCreator_createGameScene allocates 0x44
 * with the RPActorScene constructor, then stores this vtable): no members of its own. Everything
 * else is inherited from RPActorScene; CreateActorList creates the RPCowGame actor list
 * (0x7C bytes), which RPActorScene keeps in RPActorScene::spActorList (8043C680).
 * @note Name is a project convention (ledger Cow_Scene).
 */
class RPCowScene : public RPActorScene {
public:
    RPCowScene();
    virtual ~RPCowScene(); // at 0x8

    /**
     * @brief Creates the RPCowGame (ledger Cow_Scene_createCowMgr) [H]
     */
    virtual void CreateActorList(); // at 0x5C
};

//! @}

#endif
