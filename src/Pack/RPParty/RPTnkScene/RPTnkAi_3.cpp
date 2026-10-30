#include <Pack/RPParty/RPTnkScene/RPTnkAi.h>
#include <Pack/RPParty/RPTnkScene/RPTnkField.h>
#include <Pack/RPParty/RPTnkScene/RPTnkMine.h>

#include <revolution/MTX.h>

using nw4r::math::VEC3;

extern VEC3 RP_vecZero;
extern VEC3 RP_vecEz;

namespace {

inline f32 ProbeMin(f32 a, f32 b) {
    if (a > b) a = b;
    return a;
}

inline f32 ProbeMax(f32 a, f32 b) {
    return a < b ? b : a;
}

inline void AccumulateProbeForce(VEC3* pAccum, f32 scale, const VEC3& rDir) {
    f32 x, y, z;
    x = scale * rDir.x;
    z = scale * rDir.z;
    y = scale * rDir.y;
    pAccum[0].x = ProbeMin(pAccum[0].x, x);
    pAccum[0].y = ProbeMin(pAccum[0].y, y);
    pAccum[0].z = ProbeMin(pAccum[0].z, z);
    pAccum[1].x = ProbeMax(pAccum[1].x, x);
    pAccum[1].y = ProbeMax(pAccum[1].y, y);
    pAccum[1].z = ProbeMax(pAccum[1].z, z);
}

} // namespace

RPTnkAiProbe::RPTnkAiProbe() {}

RPTnkAiProbe::~RPTnkAiProbe() {}

BOOL RPTnkAiProbe::Step() {
    bool hit = false;
    f32 radius = unkCC;
    f32 distance = mSpeed * static_cast<f32>(mStep);
    mPos.x = mDir.x * distance + mStartPos.x;
    mPos.y = mDir.y * distance + mStartPos.y;
    mPos.z = mDir.z * distance + mStartPos.z;
    mVelocity.x = mDir.x * mSpeed;
    mVelocity.y = mDir.y * mSpeed;
    mVelocity.z = mDir.z * mSpeed;
    mHitRadius = radius;

    s32 mode = 0;
    if (mMode == 1) {
        mode = 1;
    }
    if (mStopFlags & 1) {
        hit |= RPTnkField::sInstance->CollideActor(this, mode);
    }
    if (mStopFlags & 8) {
        if (RPTnkMinePool::sInstance->IsNearAnyMine(this)) {
            mHitOther = true;
        }
    }
    return hit;
}

void RPTnkAiProbe::Reset() {
    RPTnkActor::Reset();
    nw4r::math::_VEC3 zero = RP_vecZero;
    mHitOffset.x = zero.x;
    mHitOffset.y = zero.y;
    mHitOffset.z = zero.z;
    mStartPos.x = mPos.x;
    mStartPos.y = mPos.y;
    mStartPos.z = mPos.z;
    mDir.x = RP_vecEz.x;
    mDir.y = RP_vecEz.y;
    mDir.z = RP_vecEz.z;
    mSpeed = 0.0f;
    unkCC = 1.0f;
    mStepSize = 1;
    mMaxSteps = 1;
}

void RPTnkAiProbe::Sweep(Result* pResult) {
    mResult.mHitPos.x = RP_vecEz.x;
    mResult.mHitPos.y = RP_vecEz.y;
    mResult.mHitPos.z = RP_vecEz.z;
    mResult.mHitNormal.x = RP_vecEz.x;
    mResult.mHitNormal.y = RP_vecEz.y;
    mResult.mHitNormal.z = RP_vecEz.z;
    mResult.unk18.x = RP_vecZero.x;
    mResult.unk18.y = RP_vecZero.y;
    mResult.unk18.z = RP_vecZero.z;
    mResult.mSteps = 0;
    mResult.mTime = 0.0f;
    mResult.mFlags = 0;
    mStep = 0;
    mHit = false;
    mHitPos.x = RP_vecZero.x;
    mHitPos.y = RP_vecZero.y;
    mHitPos.z = RP_vecZero.z;
    mHitOther = false;

    switch (mMode) {
    case 0:
        mStep = 1;
        do {
            if (Step()) {
                break;
            }
            mStep += mStepSize;
        } while (mStep < mMaxSteps);
        break;
    case 1:
        mStep = 1;
        do {
            ClearForces();
            if (Step()) {
                Integrate();
                break;
            }
            mStep += mStepSize;
        } while (mStep < mMaxSteps);
        break;
    }

    mResult.mHitPos.x = mDir.x;
    mResult.mHitPos.y = mDir.y;
    mResult.mHitPos.z = mDir.z;
    mResult.mSteps = mStep;
    mResult.mTime = mSpeed * static_cast<f32>(mStep);
    if (mHit) {
        mResult.mHitNormal.x = mHitPos.x;
        mResult.mHitNormal.y = mHitPos.y;
        mResult.mHitNormal.z = mHitPos.z;
        mResult.unk18.x = RP_vecZero.x;
        mResult.unk18.y = RP_vecZero.y;
        mResult.unk18.z = RP_vecZero.z;
        mResult.mFlags |= 1;
    }
    if (mHitOther) {
        mResult.mFlags |= 8;
    }
    switch (mMode) {
    case 0:
        break;
    case 1:
        mResult.mEndPos.x = mPos.x;
        mResult.mEndPos.y = mPos.y;
        mResult.mEndPos.z = mPos.z;
        mResult.mEndVel.x = mVelocity.x;
        mResult.mEndVel.y = mVelocity.y;
        mResult.mEndVel.z = mVelocity.z;
        break;
    }
    *pResult = mResult;
}

void RPTnkAiProbe::OnHitWall(f32 depth, const VEC3* pNormal, s32 wall,
                            const VEC3& rVel) {
    mHit = true;
    if (mMode == 1) {
        f32 dot = nw4r::math::VEC3Dot(&mVelocity, pNormal);
        f32 speed = PSVECMag((const Vec*)&mVelocity);
        if (dot < 0.0f) {
            VEC3 direction;
            direction.x = -mVelocity.x;
            direction.y = 0.0f;
            direction.z = -mVelocity.z;
            PSVECNormalize((const Vec*)&direction, (Vec*)&direction);
            f32 distance = nw4r::math::VEC3Dot(&direction, pNormal);
            if (distance != 0.0f) {
                distance = depth / distance;
            }
            if (distance > speed) {
                AccumulateProbeForce(mPosAccum, depth, *pNormal);
            } else {
                AccumulateProbeForce(mPosAccum, distance, direction);
            }
        } else {
            AccumulateProbeForce(mPosAccum, depth, *pNormal);
        }
        f32 reflection = -2.0f * nw4r::math::VEC3Dot(&mVelocity, pNormal);
        if (reflection > 0.0f) {
            AccumulateProbeForce(mVelAccum, reflection, *pNormal);
        }
    }
    mHitPos.x = mHitPos.x + pNormal->x;
    mHitPos.y = mHitPos.y + pNormal->y;
    mHitPos.z = mHitPos.z + pNormal->z;
    if (PSVECMag((const Vec*)&mHitPos) != 0.0f) {
        PSVECNormalize((const Vec*)&mHitPos, (Vec*)&mHitPos);
    }
}
