// Wii Play: the second file of the math utilities (its own .sdata2 pool, static init of the
// shared vector constants).
#include <Pack/types_pack.h>

#include <nw4r/math.h>

struct RPAxes {
    nw4r::math::VEC3 x; // at 0x0
    nw4r::math::VEC3 y; // at 0xC
    nw4r::math::VEC3 z; // at 0x18
};

// Shared vector constants and random seeds of the party games. Linked units refer to them by
// these names.
nw4r::math::VEC2 RP_vec2Zero(0.0f, 0.0f);
nw4r::math::VEC3 RP_vecZero(0.0f, 0.0f, 0.0f);
nw4r::math::VEC3 RP_vecEx(1.0f, 0.0f, 0.0f);
nw4r::math::VEC3 RP_vecEy(0.0f, 1.0f, 0.0f);
nw4r::math::VEC3 RP_vecEz(0.0f, 0.0f, 1.0f);
nw4r::math::VEC3 RP_vecOne(1.0f, 1.0f, 1.0f);
RPAxes RP_mtx33Identity = {RP_vecEx, RP_vecEy, RP_vecEz};

u32 RP_randSeedX;
u32 RP_randSeedY;
u32 RP_randSeedZ;
u32 RP_randSeedW;
