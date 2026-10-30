#ifndef RP_PARTY_COMMON_ACTOR_SCENE_H
#define RP_PARTY_COMMON_ACTOR_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPSystem/RPSysScene.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPActorList;

/**
 * @brief Scene that runs one RPActorList (the Cow minigame scene derives from it)
 * @details vtable 803575D8. Size 0x44, verified (Cow_Scene, vtable 8035CD38, allocates 0x44
 * with this constructor): no members of its own, all state is static. The constructor and
 * destructor allocate/free the debug buffers (RPDebugGraph) and Reset clears the rumble
 * patterns.
 * @note Wii Play-only framework class (ledger ActorScene); the name is a project convention.
 */
class RPActorScene : public RPSysScene {
public:
    RPActorScene();
    virtual ~RPActorScene(); // at 0x8

    /**
     * @brief Stores the current scene pointer, clears sUnk688 and calls CreateActorList [H]
     */
    virtual void Configure(); // at 0x44
    /**
     * @brief RPActorList::LoadResource, then RPActor::LoadResource on every actor [H]
     */
    virtual void LoadResource(); // at 0x48
    virtual void Reset();        // at 0x4C
    /**
     * @brief Unless paused (sFlags & 8): frame++, list Calculate, sort/activate, rumble update,
     * list CalculatePost, CalcActive [H]
     */
    virtual void Calculate(); // at 0x50
    /**
     * @brief Deletes spActorList [H]
     */
    virtual void Exit(); // at 0x54

    /**
     * @brief Creates the game's RPActorList (Cow: the Cow game manager) [M]
     */
    virtual void CreateActorList() = 0; // at 0x5C

    /**
     * @brief IRPGrpDrawObject::UserDraw override: RPActorList::Draw unless paused [H]
     */
    virtual void UserDraw(); // at 0x60
    /**
     * @brief IRPGrpDrawObject::DebugDraw override: when (sFlags & 1) and draw mode 3, passes the
     * current draw pass to every active actor and to the list [H]
     */
    virtual void DebugDraw(); // at 0x64

    static RPActorList* GetActorList() {
        return spActorList;
    }

protected:
    static RPActorList* spActorList; // 8043C680
    static u32 sUnk684;              // 8043C684 [L] copied from the scene manager
    static u32 sUnk688;              // 8043C688 [L]
    static u32 sFlags;               // 8043C68C bit 0 draw passes, bit 3 paused
    static u32 sFrame;               // 8043C690 frame counter
    static u32 sUnk694;              // 8043C694 [L]
    static u32 sUnk698;              // 8043C698 [L]
};

//! @}

#endif
