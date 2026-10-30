#ifndef RP_PARTY_COMMON_AVATAR_SCENE_H
#define RP_PARTY_COMMON_AVATAR_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPSystem/RPSysScene.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Mii selection scene with the avatar animation brres, scene lights and menu background
 * @details vtable 80358128. Adds no members to RPSysScene (the destructor only runs
 * EGG::Scene's); no allocation of it was found, so the size (0x44) is not verified.
 * Configure creates model scenes 0 and 1, the resource list, the menu background, a camera,
 * a screen and the 2D/3D views, then the select scene.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPAvatarScene : public RPSysScene {
public:
    virtual ~RPAvatarScene(); // at 0x8

    virtual void Configure();    // at 0x44
    virtual void LoadResource(); // at 0x48
    /**
     * @brief Lyt_AvatarAnim_resetSelection [M]
     */
    virtual void Reset(); // at 0x4C
    /**
     * @brief Sys_AvatarAnimScene_calculate [M]
     */
    virtual void Calculate(); // at 0x50
    /**
     * @brief Destroys the menu background and the select scene [H]
     */
    virtual void Exit(); // at 0x54

    /**
     * @brief Empty override (IRPGrpDrawObject part points here through a thunk) [H]
     */
    virtual void UserDraw(); // at 0x5C
};

//! @}

#endif
