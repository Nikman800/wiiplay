// Circle-against-wall test and the floor-quad basis (Tanks).
// Unit 0x80258868-0x80258ADC (+ .ctors 0x802E9DF0, .bss 0x8043A870-0x8043A8B8,
// .sdata2 0x804402B0-0x804402C8). It follows RPTnkField_2.cpp, whose thunks end that file.
#include <Pack/RPParty/RPTnkScene/RPTnkField.h>

#include <Pack/RPParty/RPDucScene/RPDucUtil.h>

// Floor-quad basis (x, -z, y); RPTnkTank::DrawTreadMark reads it too. The three axis
// statics and the basis share one pooled .bss block.
static nw4r::math::VEC3 sBasisX(1.0f, 0.0f, 0.0f);
static nw4r::math::VEC3 sBasisNegZ(0.0f, 0.0f, -1.0f);
static nw4r::math::VEC3 sBasisY(0.0f, 1.0f, 0.0f);
RPAxes sFloorQuadBasis = {sBasisX, sBasisNegZ, sBasisY};

extern const f32 sWallMargin;
const f32 sWallMargin = 8.0f;

// The wall is non-const: a const RPTnkWall* changes the scheduling of the XZ difference.
BOOL RPTnkTestCircleVsWall(f32 radius, nw4r::math::VEC3* pNormal, f32* pDepth, u32* pWallId,
                           const nw4r::math::VEC3* pPos, RPTnkWall* pWall) {
    nw4r::math::VEC3 diff;
    diff.x = pPos->x - pWall->mPos.x;
    diff.y = 0.0f;
    diff.z = pPos->z - pWall->mPos.z;
    nw4r::math::VEC3 tangent;
    PSVECCrossProduct(RP_vecEy, pWall->mNormal, tangent);

    f32 distances[3];
    f32 along = distances[1] = diff.x * tangent.x + diff.z * tangent.z;
    if (along > radius) return FALSE;
    f32 beyond = distances[2] = -along - pWall->mLength;
    if (beyond > radius) return FALSE;
    f32 dist = distances[0] = diff.x * pWall->mNormal.x + diff.z * pWall->mNormal.z;
    f32 depth = radius - dist;
    if (depth < 0.0f) return FALSE;
    if (-dist > 8.0f) return FALSE;

    f32 sq;
    if (along > 0.0f) {
        if (along > dist) return FALSE;
        sq = along * along;
    } else if (beyond > 0.0f) {
        if (beyond > dist) return FALSE;
        sq = beyond * beyond;
    } else {
        goto hit;
    }
    radius = radius * radius - sq;
    depth = nw4r::math::FSqrt(radius) - distances[0];
    if (depth < 0.0f) return FALSE;
hit:
    *pNormal = pWall->mNormal;
    *pDepth = depth;
    *pWallId = pWall->unk1C;
    return TRUE;
}

// Stand-in for an unused (dead-stripped) function compiled before RPTnkTestCircleVsWall:
// its 1.0f / 0.0f come first in the pool (sWallMargin, 1, 0, 8, -1).
extern "C" void Tnk_WallTest_unusedHelper(nw4r::math::VEC3* p) {
    p->x = 1.0f;
    p->y = 0.0f;
}
