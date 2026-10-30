#ifndef RP_PARTY_COMMON_AXES_H
#define RP_PARTY_COMMON_AXES_H
#include <Pack/types_pack.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Three axis vectors (rows x, y, z); copies go member by member, three 12-byte blocks
 * (RPDucTarget::ResetState) [M]
 */
struct RPAxes {
    nw4r::math::VEC3 x; // at 0x0
    nw4r::math::VEC3 y; // at 0xC
    nw4r::math::VEC3 z; // at 0x18
};

//! @}

#endif
