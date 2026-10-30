#include <Pack/RPParty/RPPartyCommon/RPCollisionModel.h>

#include <Pack/RPParty/RPPartyCommon/RPBBox.h>

// Out of line in other framework units (ledger names, used by name in linked units)
extern "C" void RP_BBox_reset2(RPBBox* pBox);
extern "C" EGG::Vector3f RP_mtx33MultVec(const EGG::Matrix34f& rMtx, const EGG::Vector3f& rVec);

EGG::Vector3f& RPCollisionModel::GetPos() {
    return mPos;
}

EGG::Vector3f& RPCollisionModel::GetRot() {
    return mRot;
}

f32 RPCollisionModel::GetRadius() const {
    return 0.0f;
}

RPCollisionModel::RPCollisionModel() {
    Reset(NULL);
}

void RPCollisionModel::Reset(void* pKclFile) {
    SetResource(pKclFile);
    mPos.x = mPos.y = mPos.z = 0.0f;
    mRot.x = mRot.y = mRot.z = 0.0f;
    mMtx.makeIdentity();
    mScaleA = mScaleB = 1.0f;
}

bool RPCollisionModel::CheckSphere(f32 radius, const EGG::Vector3f& rPos,
                                   EGG::Vector3f* pPosOfs, EGG::Vector3f* pRotOfs,
                                   u16* pFlags) {
    static RPKclHit sHits[0x40];

    // max and pHit declared first: they take r31/r30 ahead of hit and flags
    s32 max;
    RPKclHit* pHit;
    bool hit = false;
    RPBBox posBox;
    RP_BBox_reset2(&posBox);
    RPBBox rotBox;
    RP_BBox_reset2(&rotBox);

    f32 thickness = mpHeader->thickness;
    mpHeader->thickness = thickness * mScaleB;
    u16 flags = 0;
    s32 num = SearchSphereOctree(sHits, 0x40, rPos, radius);
    mpHeader->thickness = thickness;

    if (num > 0) {
        max = 0x40;
        if (num < 0x40) {
            max = num;
        }
        pHit = sHits;
        for (s32 i = 0; i < max; pHit++, i++) {
            if (pFlags != NULL && *pFlags != 0 && pHit->attribute != 0 && !(*pFlags & pHit->attribute)) {
                continue;
            }
            if (pPosOfs != NULL) {
                nw4r::math::VEC3 ofs;
                nw4r::math::VEC3Scale(&ofs, &pHit->normal, pHit->distance);
                posBox.Extend(EGG::Vector3f(ofs.x, ofs.y, ofs.z));
            }
            if (pRotOfs != NULL) {
                rotBox.Extend(EGG::Vector3f(pHit->normal.x, pHit->normal.y, pHit->normal.z));
            }
            if (pFlags != NULL) {
                flags |= pHit->attribute;
            }
            hit = true;
        }
    }

    if (hit) {
        if (pPosOfs != NULL) {
            *pPosOfs = posBox.GetSum();
        }
        if (pRotOfs != NULL) {
            *pRotOfs = rotBox.GetSum();
        }
        if (pFlags != NULL) {
            *pFlags = flags;
        }
        return true;
    }
    return false;
}

// rMtx's 3x3 part transposed times rVec (into the model's space)
inline EGG::Vector3f RPCollisionModelRotateInv(const EGG::Matrix34f& rMtx,
                                               const EGG::Vector3f& rVec) {
    f32 x = rMtx._00 * rVec.x + rMtx._10 * rVec.y + rMtx._20 * rVec.z;
    f32 y = rMtx._01 * rVec.x + rMtx._11 * rVec.y + rMtx._21 * rVec.z;
    f32 z = rMtx._02 * rVec.x + rMtx._12 * rVec.y + rMtx._22 * rVec.z;
    return EGG::Vector3f(x, y, z);
}

bool RPCollisionModel::CheckHit(f32 radius, const EGG::Vector3f& rPos,
                                const EGG::Vector3f& rPrevPos, EGG::Vector3f* pPosOfs,
                                EGG::Vector3f* pRotOfs, u16* pFlags) {
    f32 r = radius * mScaleB;
    EGG::Vector3f move = RPCollisionModelRotateInv(mMtx, rPrevPos - mRot) * mScaleB;
    EGG::Vector3f pos = RPCollisionModelRotateInv(mMtx, rPos - mPos) * mScaleB - move;

    int steps = 1;
    EGG::Vector3f step(0.0f, 0.0f, 0.0f);
    f32 len = move.length();
    if (len > 0.0f) {
        f32 thick = mpHeader->thickness;
        f32 unit = mScaleB * thick;
        steps = (int)(len / unit) + 1;
        step = move * unit / len;
    }

    for (int i = 0; i < steps; i++) {
        pos += step;
        move -= step;
        u16 flags = pFlags != NULL ? *pFlags : 0;
        if (CheckSphere(r, pos, pPosOfs, pRotOfs, pFlags != NULL ? &flags : NULL)) {
            if (pPosOfs != NULL) {
                *pPosOfs = RP_mtx33MultVec(mMtx, (*pPosOfs - move) * mScaleA);
            }
            if (pRotOfs != NULL) {
                *pRotOfs = RP_mtx33MultVec(mMtx, *pRotOfs);
            }
            if (pFlags != NULL) {
                *pFlags = flags;
            }
            return true;
        }
    }
    return false;
}
