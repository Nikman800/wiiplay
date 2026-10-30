#ifndef RP_PARTY_HKY_OBJECT_H
#define RP_PARTY_HKY_OBJECT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

//! @addtogroup rp_hky
//! @{

/**
 * @brief Laser Hockey world object driven by RPHkyGame
 * @details Abstract: no vtable of its own exists in the binary, but every Laser Hockey world
 * object (puck, field, paddles) adds these four virtuals in this order, and RPHkyGame calls them
 * through these slots. Name is a project convention.
 */
class RPHkyObject : public RPSimpleModel {
public:
    virtual void Create() = 0; // at 0x1C
    virtual void Load() = 0;   // at 0x20
    virtual void Reset() = 0;  // at 0x24
    virtual void Exit() = 0;   // at 0x28
};

//! @}

#endif
