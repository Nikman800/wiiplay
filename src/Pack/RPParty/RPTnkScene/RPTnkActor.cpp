// Tanks actor base (RPTnkActor): position, velocity, force accumulators and hit circles.
// Unit 0x80259114-0x80259940 (dtk/game_units.tsv) plus 0x80259940-0x80259A88 at the start of the
// RPTnkTank.cpp unit: the weak RPTnkActorSound destructor, RPTnkActor's inline virtuals and the
// destructor thunk are this file's weak copies. Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkActor.h>

#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPParty/RPTnkScene/RPTnkActorPool.h>

RPTnkActor::RPTnkActor() {
    RPTnkActor::ClearForces();
    mVelocity = RP_vecZero;
    RPTnkActor::ClearForces();
    mHitOffset = RP_vecZero;
    mHitRadius = 0.0f;
    mAge = 0;
    mIndex = 0;
    mpPool = NULL;
}

RPTnkActor::~RPTnkActor() {}

void RPTnkActor::ResetState() {
    mPos = RP_vecZero;
    mScale = RP_vecOne;
    mRot = RP_mtx33Identity;
    mVelocity = RP_vecZero;
    RPTnkActor::ClearForces();
    mHitOffset = RP_vecZero;
    mHitRadius = 1.0f;
    mHitOffsetAlt = RP_vecZero;
    mAge = -1;
    SetUnk40(0);
}

void RPTnkActor::ClearForces() {
    mPosAccum[0].x = 0.0f;
    mPosAccum[0].y = 0.0f;
    mPosAccum[0].z = 0.0f;
    mPosAccum[1].x = 0.0f;
    mPosAccum[1].y = 0.0f;
    mPosAccum[1].z = 0.0f;
    mVelAccum[0].x = 0.0f;
    mVelAccum[0].y = 0.0f;
    mVelAccum[0].z = 0.0f;
    mVelAccum[1].x = 0.0f;
    mVelAccum[1].y = 0.0f;
    mVelAccum[1].z = 0.0f;
}

void RPTnkActor::Integrate() {
    mPos.x += mPosAccum[0].x + mPosAccum[1].x;
    mPos.y += mPosAccum[0].y + mPosAccum[1].y;
    mPos.z += mPosAccum[0].z + mPosAccum[1].z;
    mVelocity.x += mVelAccum[0].x + mVelAccum[1].x;
    mVelocity.y += mVelAccum[0].y + mVelAccum[1].y;
    mVelocity.z += mVelAccum[0].z + mVelAccum[1].z;
}

void RPTnkActor::OnSpawn() {}

void RPTnkActor::ApplySpawnParam(u32 param) {}

void RPTnkActor::Create() {
    mpSound = new RPTnkActorSound();
}

void RPTnkActor::Load() {}

void RPTnkActor::Reset() {
    ResetState();
}

void RPTnkActor::ApplyVelocity() {
    mPos.x = mPos.x + mVelocity.x;
    mPos.y = mPos.y + mVelocity.y;
    mPos.z = mPos.z + mVelocity.z;
}

void RPTnkActor::ClearForcesStep() {
    ClearForces();
}

void RPTnkActor::CollideWithField() {}

void RPTnkActor::IntegrateStep() {
    Integrate();
}

void RPTnkActor::Calculate() {}

void RPTnkActor::Spawn(const nw4r::math::VEC3& rPos, u32 param) {
    ResetState();
    mPos = rPos;
    OnSpawn();
    ApplySpawnParam(param);
    mAge = 0;
    SetUnk40(1);
}

void RPTnkActor::Kill() {
    mAge = -999;
    if (mpPool != NULL) {
        mpPool->DecActiveNum();
    }
    SetUnk40(0);
}

BOOL RPTnkActor::TestHit(RPTnkActor* pOther, f32* pDist) {
    f32 radius = pOther->mHitRadius;
    radius = mHitRadius + radius;
    nw4r::math::VEC3 otherPos = pOther->GetHitCenter();
    f32 dx = otherPos.x - (mPos.x + mHitOffset.x);
    radius *= radius;
    dx *= dx;
    if (dx > radius) {
        return FALSE;
    }

    f32 dz = otherPos.z - (mPos.z + mHitOffset.z);
    dz *= dz;
    if (dz > radius) {
        return FALSE;
    }

    if (dx + dz > radius) {
        return FALSE;
    }

    if (pDist != NULL) {
        *pDist = nw4r::math::FSqrt(dx + dz);
    }
    return TRUE;
}

BOOL RPTnkActor::TestHitAlt(RPTnkActor* pOther, f32* pDist) {
    f32 radius = pOther->mHitRadius;
    radius = mHitRadius + radius;
    nw4r::math::VEC3 otherPos = pOther->GetHitCenter();
    f32 dx = otherPos.x - (mPos.x + mHitOffsetAlt.x);
    radius *= radius;
    dx *= dx;
    if (dx > radius) {
        return FALSE;
    }

    f32 dz = otherPos.z - (mPos.z + mHitOffsetAlt.z);
    dz *= dz;
    if (dz > radius) {
        return FALSE;
    }

    if (dx + dz > radius) {
        return FALSE;
    }

    if (pDist != NULL) {
        *pDist = nw4r::math::FSqrt(dx + dz);
    }
    return TRUE;
}

void RPTnkActor::OnHitWall(f32 depth, const nw4r::math::VEC3* pNormal, s32 wall,
                           const nw4r::math::VEC3& rVel) {}
