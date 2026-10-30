#include <Pack/RPUtility/RPUtlDohMath.h>

#include <nw4r/math.h>

#include <revolution/MTX.h>
#include <revolution/OS.h>

struct RPAxes {
    nw4r::math::VEC3 x; // at 0x0
    nw4r::math::VEC3 y; // at 0xC
    nw4r::math::VEC3 z; // at 0x18
};

// Wii Play: shared vector constants and random seeds of the party games, defined in
// RPUtlDohMath_2.cpp. Linked units refer to them (and to the RP_* functions below) by these names.
extern nw4r::math::VEC2 RP_vec2Zero;
extern nw4r::math::VEC3 RP_vecZero;
extern nw4r::math::VEC3 RP_vecEx;
extern nw4r::math::VEC3 RP_vecEy;
extern nw4r::math::VEC3 RP_vecEz;
extern nw4r::math::VEC3 RP_vecOne;
extern RPAxes RP_mtx33Identity;

extern u32 RP_randSeedX;
extern u32 RP_randSeedY;
extern u32 RP_randSeedZ;
extern u32 RP_randSeedW;

extern "C" void RP_seedRandomTicks() {
    while ((RP_randSeedX = OSGetTick()) == 0) {}
    while ((RP_randSeedY = OSGetTick()) == 0) {}
    while ((RP_randSeedZ = OSGetTick()) == 0) {}
    while ((RP_randSeedW = OSGetTick()) == 0) {}
}

BOOL RPSelectMoreAlignedVec(nw4r::math::VEC3* pOut,
                            const nw4r::math::VEC3* pRef,
                            const nw4r::math::VEC3* pA,
                            const nw4r::math::VEC3* pB) {
    if (nw4r::math::VEC3Dot(pRef, pA) >= nw4r::math::VEC3Dot(pRef, pB)) {
        *pOut = *pA;
        return FALSE;
    }

    *pOut = *pB;
    return TRUE;
}

extern "C" void RP_orthonormalizeBasis(RPAxes* pAxes, f32 tolerance) {
    f32 error;

    do {
        PSVECNormalize(pAxes->x, pAxes->x);
        PSVECNormalize(pAxes->y, pAxes->y);
        PSVECNormalize(pAxes->z, pAxes->z);

        nw4r::math::VEC3 x, y, z;
        PSVECCrossProduct(pAxes->y, pAxes->z, x);
        PSVECCrossProduct(pAxes->z, pAxes->x, y);
        PSVECCrossProduct(pAxes->x, pAxes->y, z);

        f32 len = PSVECMag(x);
        f32 inv = 1.0f / len;
        pAxes->x.x = 0.5f * (pAxes->x.x + inv * x.x);
        pAxes->x.y = 0.5f * (pAxes->x.y + inv * x.y);
        pAxes->x.z = 0.5f * (pAxes->x.z + inv * x.z);
        error = len;

        len = PSVECMag(y);
        inv = 1.0f / len;
        pAxes->y.x = 0.5f * (pAxes->y.x + inv * y.x);
        pAxes->y.y = 0.5f * (pAxes->y.y + inv * y.y);
        pAxes->y.z = 0.5f * (pAxes->y.z + inv * y.z);
        error += len;

        len = PSVECMag(z);
        inv = 1.0f / len;
        pAxes->z.x = 0.5f * (pAxes->z.x + inv * z.x);
        pAxes->z.y = 0.5f * (pAxes->z.y + inv * z.y);
        pAxes->z.z = 0.5f * (pAxes->z.z + inv * z.z);
        error += len;
    } while (error < tolerance);
}

extern "C" void RP_makeRotMtxFromTo(nw4r::math::MTX34* pOut,
                                    const nw4r::math::VEC3* pFrom,
                                    const nw4r::math::VEC3* pTo) {
    nw4r::math::VEC3 to = *pTo;
    f32 mag = PSVECMag(to);
    if (0.0f == mag) {
        PSMTXIdentity(*pOut);
        return;
    }

    f32 inv = 1.0f / mag;
    to.x *= inv;
    to.y *= inv;
    to.z *= inv;

    nw4r::math::VEC3 axis;
    PSVECCrossProduct(*pFrom, to, axis);

    f32 hx = pFrom->x + pTo->x;
    f32 hy = pFrom->y + pTo->y;
    f32 hz = pFrom->z + pTo->z;
    f32 hlen = nw4r::math::FSqrt(hx * hx + hy * hy + hz * hz);
    f32 hinv = 1.0f / hlen;

    f32 e = hinv * (hx * pFrom->x + hy * pFrom->y + hz * pFrom->z);
    f32 s = hinv * (hx * to.x + hy * to.y + hz * to.z);

    pOut->_00 = axis.x * axis.x + e * (pFrom->x * pFrom->x + to.x * to.x);
    pOut->_11 = axis.y * axis.y + e * (pFrom->y * pFrom->y + to.y * to.y);
    pOut->_22 = axis.z * axis.z + e * (pFrom->z * pFrom->z + to.z * to.z);

    f32 t0 = axis.x * axis.y + e * (pFrom->x * pFrom->y + to.x * to.y);
    f32 u0 = axis.z * s;
    pOut->_10 = t0 + u0;
    pOut->_01 = t0 - u0;

    f32 t1 = axis.y * axis.z + e * (pFrom->y * pFrom->z + to.y * to.z);
    f32 u1 = axis.x * s;
    pOut->_21 = t1 + u1;
    pOut->_12 = t1 - u1;

    f32 t2 = axis.x * axis.z + e * (pFrom->x * pFrom->z + to.x * to.z);
    f32 u2 = axis.y * s;
    pOut->_02 = t2 + u2;
    pOut->_20 = t2 - u2;

    pOut->_03 = 0.0f;
    pOut->_13 = 0.0f;
    pOut->_23 = 0.0f;
}

void RPCalcPerpendicularDir(nw4r::math::VEC3* pOut,
                            const nw4r::math::VEC3* pPoint,
                            const nw4r::math::VEC3* pOrigin,
                            const nw4r::math::VEC3* pDir) {
    nw4r::math::VEC3 diff;
    diff.x = pPoint->x - pOrigin->x;
    diff.y = pPoint->y - pOrigin->y;
    diff.z = pPoint->z - pOrigin->z;

    f32 mag = PSVECMag(diff);
    if (0.0f == mag) {
        *pOut = RP_vecZero;
        return;
    }

    f32 inv = 1.0f / mag;
    diff.x *= inv;
    diff.y *= inv;
    diff.z *= inv;

    nw4r::math::VEC3 dir;
    PSVECNormalize(*pDir, dir);

    nw4r::math::VEC3 side;
    PSVECCrossProduct(dir, diff, side);

    mag = PSVECMag(side);
    if (0.0f == mag) {
        *pOut = RP_vecZero;
        return;
    }

    inv = 1.0f / mag;
    side.x *= inv;
    side.y *= inv;
    side.z *= inv;

    PSVECCrossProduct(dir, side, *pOut);
}

extern "C" f32 RP_calcPerpendicularDistance(const nw4r::math::VEC3* pPoint,
                                            const nw4r::math::VEC3* pOrigin,
                                            const nw4r::math::VEC3* pDir) {
    nw4r::math::VEC3 diff;
    nw4r::math::VEC3 perp;
    diff.x = pOrigin->x - pPoint->x;
    diff.y = pOrigin->y - pPoint->y;
    diff.z = pOrigin->z - pPoint->z;
    RPCalcPerpendicularDir(&perp, pPoint, pOrigin, pDir);
    return nw4r::math::VEC3Dot(&perp, &diff);
}

static inline nw4r::math::VEC3 RPSubVec(const nw4r::math::VEC3* pA,
                                        const nw4r::math::VEC3* pB) {
    nw4r::math::VEC3 out;
    out.x = pA->x - pB->x;
    out.y = pA->y - pB->y;
    out.z = pA->z - pB->z;
    return out;
}

static inline f32 RPDotNormalized(const nw4r::math::VEC3& rVec,
                                  nw4r::math::VEC3 normal) {
    PSVECNormalize(normal, normal);
    return nw4r::math::VEC3Dot(&rVec, &normal);
}

static inline f32 RPNormalizedDot(nw4r::math::VEC3 normal,
                                  const nw4r::math::VEC3& rVec) {
    PSVECNormalize(normal, normal);
    return nw4r::math::VEC3Dot(&normal, &rVec);
}

void RPIntersectLinePlane(nw4r::math::VEC3* pOut,
                          const nw4r::math::VEC3* pStart,
                          const nw4r::math::VEC3* pEnd,
                          const nw4r::math::VEC3* pPlanePoint,
                          const nw4r::math::VEC3* pPlaneNormal) {
    nw4r::math::VEC3 dir;
    dir.x = pEnd->x - pStart->x;
    dir.y = pEnd->y - pStart->y;
    dir.z = pEnd->z - pStart->z;

    f32 denom = RPDotNormalized(dir, *pPlaneNormal);
    f32 t = -RPNormalizedDot(*pPlaneNormal, RPSubVec(pStart, pPlanePoint)) / denom;

    pOut->x = pStart->x + dir.x * t;
    pOut->y = pStart->y + dir.y * t;
    pOut->z = pStart->z + dir.z * t;
}
