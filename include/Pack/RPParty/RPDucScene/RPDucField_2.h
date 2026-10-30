#ifndef RP_PARTY_DUC_FIELD_2_H
#define RP_PARTY_DUC_FIELD_2_H

#include <Pack/RPParty/RPDucScene/RPDucField.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>

inline void RPDucFieldSetPos(nw4r::math::VEC3& pos, f32 x, f32 y, f32 z) {
    pos.x = x;
    pos.y = y;
    pos.z = z;
}

inline u32 RPDucFieldRandU32() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    return RP_randSeedX ^ RP_randSeedY;
}

#endif
