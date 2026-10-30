// Shooting Range target base (RPDucTarget): physics steps, hit bookkeeping and debug drawing.
// Unit 0x8023D908-0x8023E440 (dtk/game_units.tsv). Functions in reverse address order.
// AsAvatar and CanShotPassThrough are inline (their weak copies are emitted by RPDucBalloon.cpp).
#include <Pack/RPParty/RPDucScene/RPDucTarget.h>

#include <Pack/RPParty/RPDucScene/RPDucCamera.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPParty/RPDucScene/RPDucHud.h>
#include <Pack/RPParty/RPDucScene/RPDucShot.h>
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>

#include <egg/gfxe.h>

// These constants remain in the original shared small-data pool.
extern const f32 lbl_8043FE08;
extern const f32 lbl_8043FE0C;
extern const f32 lbl_8043FE10;
extern const f32 lbl_8043FE14;

RPDucTarget::RPDucTarget() {
    RPDucTarget::ClearForces();
    mVelocity = RP_vecZero;
    RPDucTarget::ClearForces();
    unk84 = RP_vecZero;
    mHitRadius = lbl_8043FE14;
    mHitOffset = RP_vecZero;
    mHitScale = lbl_8043FE14;
    mAge = 0;
    mIndex = 0;
    mpMgr = NULL;
    mpSndActor = NULL;
}

RPDucTarget::~RPDucTarget() {}

#pragma auto_inline off
void RPDucTarget::ResetState() {
    mPos = RP_vecZero;
    mScale = RP_vecOne;
    mRot = RP_mtx33Identity;
    mVelocity = RP_vecZero;
    RPDucTarget::ClearForces();
    unk84 = RP_vecZero;
    mHitRadius = lbl_8043FE08;
    mHitOffset = RP_vecZero;
    mHitScale = lbl_8043FE08;
    ClearHitRecord();
    unkE8 = 0;
    mAge = -1;
    SetUnk40(0);
}
#pragma auto_inline reset

void RPDucTarget::ClearForces() {
    mPosForces[0].x = lbl_8043FE14;
    mPosForces[0].y = lbl_8043FE14;
    mPosForces[0].z = lbl_8043FE14;
    mPosForces[1].x = lbl_8043FE14;
    mPosForces[1].y = lbl_8043FE14;
    mPosForces[1].z = lbl_8043FE14;
    mVelForces[0].x = lbl_8043FE14;
    mVelForces[0].y = lbl_8043FE14;
    mVelForces[0].z = lbl_8043FE14;
    mVelForces[1].x = lbl_8043FE14;
    mVelForces[1].y = lbl_8043FE14;
    mVelForces[1].z = lbl_8043FE14;
}

void RPDucTarget::ApplyForces() {
    mPos.x += mPosForces[0].x + mPosForces[1].x;
    mPos.y += mPosForces[0].y + mPosForces[1].y;
    mPos.z += mPosForces[0].z + mPosForces[1].z;
    mVelocity.x += mVelForces[0].x + mVelForces[1].x;
    mVelocity.y += mVelForces[0].y + mVelForces[1].y;
    mVelocity.z += mVelForces[0].z + mVelForces[1].z;
}

void RPDucTarget::ResetParams() {}

void RPDucTarget::Launch(u32 pattern) {}

void RPDucTarget::SpawnHitEffect() {}

void RPDucTarget::PlaySpawnSound() {
    mpSndActor->startSound(27U);
}

void RPDucTarget::PlayHitSound(s32 player) {
    mpSndActor->startSound(11U);
}

void RPDucTarget::ShowScorePopup(s32 points, u32 arg1, s32 player) {
    nw4r::math::VEC2 screenPos;
    RPDucCamera::sInstance->ProjectToScreen(&screenPos, mPos);
    RPDucHud::sInstance->SpawnScorePopup(screenPos, points,
                                         RPDucGame::sInstance->GetMode() == 2 ? 0 : arg1, player,
                                         true);
}

void RPDucTarget::Create() {
    mpSndActor = new RPDucSndActor();
}

void RPDucTarget::Load() {}

void RPDucTarget::Reset() {
    ResetState();
}

void RPDucTarget::ApplyVelocity() {
    mPos.x += mVelocity.x;
    mPos.y += mVelocity.y;
    mPos.z += mVelocity.z;
}

void RPDucTarget::ClearForcesHook() {
    ClearForces();
}

void RPDucTarget::VF_0x80() {}

void RPDucTarget::ApplyForcesHook() {
    ApplyForces();
}

void RPDucTarget::Calculate() {}

void RPDucTarget::DebugDrawSphere() {
    nw4r::math::VEC3 pos;
    RPVec3Add(&pos, &mPos, &mHitOffset);
    nw4r::math::VEC3 scale(mHitScale, mHitScale, mHitScale);
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, mRot, scale, pos);
    EGG::DrawGX::BeginDrawSphere();
    EGG::DrawGX::DrawDLWorld(EGG::DrawGX::DL_SPHERE_8X16, mtx, EGG::DrawGX::ORANGE);
}

void RPDucTarget::DebugDrawBox() {
    nw4r::math::VEC3 pos = mPos;
    nw4r::math::VEC3 scale;
    f32 height = lbl_8043FE08 + mPos.y;
    RPAxes rot = RP_mtx33Identity;
    f32 size;
    size = lbl_8043FE0C * mHitRadius;
    scale.x = size;
    pos.y -= lbl_8043FE10 * height;
    scale.y = height;
    scale.z = size;
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, rot, scale, pos);
    EGG::DrawGX::SetVtxState(EGG::DrawGX::VTXTYPE_CYLINDER);
    EGG::DrawGX::DrawCylinderY(EGG::DrawGX::CIRCLE_32, mtx, EGG::DrawGX::WHITE);
}

void RPDucTarget::Activate(const nw4r::math::VEC3& rPos, u32 arg) {
    ResetState();
    mPos = rPos;
    unkA4 = true;
    ResetParams();
    Launch(arg);
    mAge = 0;
    PlaySpawnSound();
    SetUnk40(1);
}

void RPDucTarget::Remove() {
    mAge = -999;
    mpMgr->mActiveNum--;
    SetUnk40(0);
}

void RPDucTarget::VF_0x98() {}

void RPDucTarget::VF_0x9C(RPDucAvatar* pAvatar) {}

s32 RPDucTarget::GetHittingPlayer() {
    s32 hitter = -1;
    s32 hitterOrder = -1;
    s32 i = 0;
    s32 num = RPDucShotMgr::sInstance->GetActiveNum();
    do {
        if (mHitOrder[i] > -1) {
            if (mHitOrder[i] > hitterOrder) {
                if (hitter != -1) {
                    RPDucScoreMgr::sInstance->AwardStreakBonus(hitter);
                    RPDucScoreMgr::sInstance->ResetStreak(hitter);
                }
                hitterOrder = mHitOrder[i];
                hitter = i;
            } else {
                RPDucScoreMgr::sInstance->AwardStreakBonus(i);
                RPDucScoreMgr::sInstance->ResetStreak(i);
            }
        }
    } while (++i < num);

    return hitter;
}

f32 RPDucTarget::GetDistanceToCamera() {
    nw4r::math::VEC3 camPos = RPDucCamera::sInstance->GetLookPos();
    nw4r::math::VEC3 diff;
    diff.x = mPos.x - camPos.x;
    diff.y = mPos.y - camPos.y;
    diff.z = mPos.z - camPos.z;
    return nw4r::math::VEC3Len(&diff);
}

