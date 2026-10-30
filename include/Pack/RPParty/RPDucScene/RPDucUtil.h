#ifndef RP_PARTY_DUC_UTIL_H
#define RP_PARTY_DUC_UTIL_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_duc
//! @{

/**
 * @name Shared party-game constants and random generator
 * @details Globals of the Wii Play party framework (owners not decompiled yet; names from the
 * ledger). RP_vecZero.. (.bss 80439110) are set by a static initializer; the random seeds
 * (.sbss 8043C5F8) belong to the RPUtlDohMath unit's .sbss.
 */
//! @{
extern nw4r::math::VEC3 RP_vecZero;
extern nw4r::math::VEC3 RP_vecEx;
extern nw4r::math::VEC3 RP_vecEy;
extern nw4r::math::VEC3 RP_vecEz;
extern nw4r::math::VEC3 RP_vecOne;
extern RPAxes RP_mtx33Identity;

extern u32 RP_randSeedX;
extern u32 RP_randSeedY;

/**
 * @brief Linear congruential step on X combined with a Galois LFSR step on Y (inline in every
 * caller)
 */
inline u32 RPRandU32() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    u32 y = RP_randSeedY;
    if (y & 1) {
        y ^= 0x11020;
    }
    RP_randSeedY = y >> 1;
    return RP_randSeedX ^ RP_randSeedY;
}

/**
 * @brief Uniform float in [lo, hi) from the low 23 bits of RPRandU32; lo itself when the range
 * is empty (inline in the targets' launch code) [M]
 */
inline f32 RPRandF32() {
    return 1.1920929e-7f * (f32)(RPRandU32() & 0x7FFFFF);
}
inline f32 RPRandF32Range(f32 lo, f32 hi) {
    if (lo == hi) {
        return lo;
    }
    f32 r = RPRandF32();
    return (hi - lo) * r + lo;
}

/**
 * @brief lo <= value <= hi (inline in the targets' screen bound checks) [M]
 */
inline bool RPDucInRange(f32 value, f32 lo, f32 hi) {
    return lo <= value && hi >= value;
}

/**
 * @brief Scalar vector helpers (component by component through pointers, so each result is
 * stored before the next component is read) [M]
 */
inline void RPVec3Add(nw4r::math::VEC3* pOut, const nw4r::math::VEC3* pA,
                      const nw4r::math::VEC3* pB) {
    pOut->x = pA->x + pB->x;
    pOut->y = pA->y + pB->y;
    pOut->z = pA->z + pB->z;
}
inline void RPVec3Sub(nw4r::math::VEC3* pOut, const nw4r::math::VEC3* pA,
                      const nw4r::math::VEC3* pB) {
    pOut->x = pA->x - pB->x;
    pOut->y = pA->y - pB->y;
    pOut->z = pA->z - pB->z;
}
inline nw4r::math::VEC3 RPVec3Diff(const nw4r::math::VEC3& rA, const nw4r::math::VEC3& rB) {
    nw4r::math::VEC3 out;
    out.x = rA.x - rB.x;
    out.y = rA.y - rB.y;
    out.z = rA.z - rB.z;
    return out;
}

/**
 * @brief Creates a resource of the type from the named entry of a loaded file [M]
 */
inline RPGrpHandle RPDucCreateRes(RPGrpModelResManager* pResMgr, RPGrpModelResManager::Type type,
                                  RPGrpHandle file, const char* pName) {
    return pResMgr->CreateData(type, pResMgr->GetPtr(type, file, pName));
}

/**
 * @brief Model matrix whose columns are the rows of rRot scaled by rScale, translated to rPos
 * (inline in the debug draws; the same layout RPModelObj::CalcModelMtx builds)
 */
inline void RPDucMakeMtx(nw4r::math::MTX34* pMtx, const RPAxes& rRot,
                         const nw4r::math::VEC3& rScale, const nw4r::math::VEC3& rPos) {
    pMtx->_00 = rScale.x * rRot.x.x;
    pMtx->_01 = rScale.y * rRot.y.x;
    pMtx->_02 = rScale.z * rRot.z.x;
    pMtx->_03 = rPos.x;
    pMtx->_10 = rScale.x * rRot.x.y;
    pMtx->_11 = rScale.y * rRot.y.y;
    pMtx->_12 = rScale.z * rRot.z.y;
    pMtx->_13 = rPos.y;
    pMtx->_20 = rScale.x * rRot.x.z;
    pMtx->_21 = rScale.y * rRot.y.z;
    pMtx->_22 = rScale.z * rRot.z.z;
    pMtx->_23 = rPos.z;
}

/**
 * @brief Intersection of the line through pOrigin along pDir with the plane through pPlanePoint
 * with normal pPlaneNormal (ledger RP_intersectLinePlane, start of the RPUtlDohMath unit) [M]
 */
bool RPIntersectLinePlane(nw4r::math::VEC3* pOut, const nw4r::math::VEC3* pOrigin,
                          const nw4r::math::VEC3* pDir, const nw4r::math::VEC3* pPlanePoint,
                          const nw4r::math::VEC3* pPlaneNormal);
/**
 * @brief Rotation taking one direction to another (ledger RP_makeRotMtxFromTo, RPUtlDohMath
 * unit) [L]
 */
void RPMakeRotMtxFromTo(nw4r::math::MTX34* pOut, const nw4r::math::VEC3* pFrom,
                        const nw4r::math::VEC3* pTo);
/**
 * @brief Re-orthonormalises three axes (ledger RP_orthonormalizeBasis, RPUtlDohMath unit) [M]
 */
void RPOrthonormalizeBasis(RPAxes* pAxes, f32 arg);
/**
 * @brief Component of a point perpendicular to a ray (ledger RP_calcPerpendicularDir,
 * RPUtlDohMath unit) [L]
 */
void RPCalcPerpendicularDir(nw4r::math::VEC3* pOut, const nw4r::math::VEC3* pPoint,
                            const nw4r::math::VEC3* pOrigin, const nw4r::math::VEC3* pDir);
/**
 * @brief Picks whichever of two vectors is more aligned with a reference (ledger
 * RP_selectMoreAlignedVec, RPUtlDohMath unit) [L]
 */
void RPSelectMoreAlignedVec(nw4r::math::VEC3* pOut, const nw4r::math::VEC3* pA,
                            const nw4r::math::VEC3* pB, const nw4r::math::VEC3* pRef);

/**
 * @brief The Shooting Range jingle (a static sound handle of RPDucCan.cpp, used by RPDucGame) [M]
 */
bool RPDucStopJingle();
bool RPDucStartJingle();
//! @}

//! @}

#endif
