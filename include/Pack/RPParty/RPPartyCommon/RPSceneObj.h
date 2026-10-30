#ifndef RP_PARTY_COMMON_SCENE_OBJ_H
#define RP_PARTY_COMMON_SCENE_OBJ_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/RP_DEBUG_STUB_1.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Base of the task objects that the Duc, Tnk and Bom game managers own and drive
 * @details vtable 80357180 (instantiated directly). No data members beyond the vtable
 * pointer: size 0x4, verified (Tnk_EmptyTask_createInstance allocates 0x4 with this
 * constructor). Every slot except the destructor is an empty virtual.
 * Slot names from about 80 derived vtables: slot 0x1C holds the *_exit/*_destroy/*_unload
 * overrides (Tnk_Field_destroy, Bom_GameManager_exit), slot 0x20 the *_draw overrides
 * (Bom_Cursor_draw, Bom_CursorManager_draw).
 * @note Wii Play-only framework class (ledger RP_SceneObj); the name is a project convention.
 */
class RPSceneObj : public RP_DEBUG_STUB_1 {
public:
    RPSceneObj();
    virtual ~RPSceneObj(); // at 0x8

    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Exit();      // at 0x1C
    virtual void Draw();      // at 0x20
};

//! @}

#endif
