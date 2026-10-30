// Shooting Range player Mii (RPDucAvatar): walks on the range in the final round, is carried off
// by the UFOs, and lines up for the end.
// Unit 0x802527D8-0x80255064 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucAvatar.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPDucScene/RPDucCamera.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPParty/RPDucScene/RPDucHud.h>
#include <Pack/RPParty/RPDucScene/RPDucSpawner.h>
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPSystem.h>

#include <egg/core.h>

static nw4r::math::VEC3 sFacePoint(0.0f, 0.0f, -150.0f);

static inline u32 AvatarRandU32() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    return RP_randSeedX ^ RP_randSeedY;
}

static inline f32 AvatarRandRange(f32 lo, f32 hi) {
    if (lo == hi) {
        return lo;
    }
    f32 r = 0.0000001192093f * (f32)(AvatarRandU32() & 0x7FFFFF);
    return __fmadds(hi - lo, r, lo);
}

/**
 * The last wave of the final round is over: the avatars line up
 */
static inline bool IsLineUpTime() {
    if (RPDucGame::sInstance->GetWaves() != 1) {
        return false;
    }
    if (RPDucSpawner::sInstance->IsActive()) {
        return false;
    }
    return RPDucTargetRegistry::sInstance->CountActiveExclDucksAvatars() == 0;
}

RPDucAvatar* RPDucAvatar::AsAvatar() {
    return this;
}

RPDucAvatar::RPDucAvatar() {
    mpKokeshi[0] = NULL;
    mpKokeshi[1] = NULL;
    mStateFuncs[0] = &RPDucAvatar::StateShootable;
    mStateFuncs[1] = &RPDucAvatar::StateRiseUp;
    mStateFuncs[2] = &RPDucAvatar::StateWalkToX;
    mStateFuncs[3] = &RPDucAvatar::StateIdle;
    mStateFuncs[4] = &RPDucAvatar::StateWander;
    mStateFuncs[5] = &RPDucAvatar::StateCarried;
    mStateFuncs[6] = &RPDucAvatar::StateDropped;
    mStateFuncs[7] = &RPDucAvatar::StateEnter;
    mStateFuncs[8] = &RPDucAvatar::StateRunToPoint;
    mStateFuncs[9] = &RPDucAvatar::StateWalkToLine;
    mStateFuncs[10] = &RPDucAvatar::StateLinedUp;
    mStateFuncs[11] = &RPDucAvatar::StateCelebrate;
    mStateFuncs[12] = &RPDucAvatar::StateRunAway;
}

RPDucAvatar::~RPDucAvatar() {}

void RPDucAvatar::ResetParams() {
    mHitRadius = 8.0f;
    mHitScale = 8.0f;
    f32 scale = GetAvatarHitRadius() / 5.0f;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    mRot = RP_mtx33Identity;
    {
        RPSysEffect effect("Sht_grass_hit", 0, 0);
        effect.create();
        nw4r::math::VEC3 effectScale;
        nw4r::math::VEC3 pos(mPos.x, 10.0f, mPos.z);
        effect.setPos(pos);
        effectScale.x = 1.0f;
        effectScale.y = 1.0f;
        effectScale.z = 1.0f;
        effect.setScale(effectScale);
        effect.update();
    }
    mDir = 1;
    mWalkToX = 0.0f;
    mCarryPos = RP_vecZero;
    mCarryValue = scale;
    mLineTimer = 0;
    mAllGone = false;
    mCelebrateTimer = 0;
    mAnim = 0;
    mTargeted = false;
    mBoundZMax = 0.0f;
    mBoundZMin = -80.0f;
    RPDucCamera* pCamera = RPDucCamera::sInstance;
    mBoundXMin = 10.0f + pCamera->GetScreenLeftAtDepth(mBoundZMax);
    mBoundXMax = pCamera->GetScreenRightAtDepth(mBoundZMax) - 10.0f;
    PickWanderCircle();
}

void RPDucAvatar::Launch(u32 pattern) {
    if (pattern & 0x80000000) {
        mState = 0;
    }
    mAnim = pattern & 3;
    if (mPos.x < 0.0f) {
        mDir = 1;
    } else {
        mDir = -1;
    }
    if (pattern & 0x100) {
        mScale.x = 0.016f;
        mScale.y = 0.016f;
        mScale.z = 0.016f;
        mTargetPos.x = AvatarRandRange(mBoundXMin + mWanderRadius, mBoundXMax - mWanderRadius);
        mTargetPos.y = 0.0f;
        mTargetPos.z = AvatarRandRange(mBoundZMin + mWanderRadius, mBoundZMax - mWanderRadius);
        mState = 7;
        SetAnim(1);
        mpSndActor->startSound(49U);
    } else {
        SetAnim(0);
        u32 flag;
        if (RPDucGame::sInstance->GetMode() != 1 && !RPDucGame::sInstance->IsP2Joined()) {
            flag = 1;
        } else {
            flag = 3;
        }
        mpSndActor->startSoundWithRemotePlayer(52U, flag, 0);
        mState = 1;
    }
}

void RPDucAvatar::SetAnim(u16 anim) {
    RPGrpModel* pBody = mpKokeshi[mAnim]->GetBodyModel();
    pBody->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, anim, 0.0f);
    if (anim == 3) {
        pBody->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, anim, 1.2f);
    } else if (anim == 2) {
        pBody->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, anim, 1.5f);
    } else {
        pBody->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, anim, 1.0f);
    }
    RPGrpModel* pHead = mpKokeshi[mAnim]->GetNigaoeModel();
    switch (anim) {
    case 0:
    case 2:
        RPSysKokeshiManager::ChangeExpression(pHead, (RFLExpression)0);
        break;
    case 1:
        RPSysKokeshiManager::ChangeExpression(pHead, (RFLExpression)4);
        break;
    case 3:
        RPSysKokeshiManager::ChangeExpression(pHead, (RFLExpression)1);
        break;
    }
}

void RPDucAvatar::TurnTowardPoint() {
    if (RPDucUfoMgr::sInstance->GetActiveNum() != 0) {
        nw4r::math::VEC3 dir;
        dir.x = sFacePoint.x - mPos.x;
        dir.y = 0.0f;
        dir.z = sFacePoint.z - mPos.z;
        PSVECNormalize(dir, dir);
        nw4r::math::VEC3 front = mRot.z;
        nw4r::math::VEC3 side = mRot.x;
        if (nw4r::math::VEC3Dot(&side, &dir) < 0.0f) {
            side.x = -side.x;
            side.y = 0.0f;
            side.z = -side.z;
        }
        mRot.z.x = 0.04f * side.x + mRot.z.x;
        mRot.z.y = 0.0f;
        mRot.z.z = 0.04f * side.z + mRot.z.z;
        PSVECNormalize(mRot.z, mRot.z);
        mRot.y = RP_vecEy;
        PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
        RPSelectMoreAlignedVec(&mRot.z, &front, &mRot.z, &dir);
    }
}

DECL_WEAK f32 AvatarWanderDepth() {
    return -80.0f;
}

void RPDucAvatar::PickWanderCircle() {
    mWanderRadius = AvatarRandRange(10.0f, 16.0f);
    if (IsWithinBounds(mPos)) {
        mWanderAngle = AvatarRandU32();
        f32 shrink = 1.0f;
        do {
            u16 angle = mWanderAngle;
            do {
                nw4r::math::VEC3 center;
                center.x = mWanderRadius * nw4r::math::CosIdx(angle) + mPos.x;
                center.y = 0.0f;
                center.z = mWanderRadius * nw4r::math::SinIdx(angle) + mPos.z;
                f32 xmin = mBoundXMin + mWanderRadius * shrink;
                f32 xmax = mBoundXMax - mWanderRadius * shrink;
                f32 zmin = mBoundZMin + mWanderRadius * shrink;
                f32 zmax = mBoundZMax - mWanderRadius * shrink;
                if (RPDucInRange(center.x, xmin, xmax) &&
                    RPDucInRange(center.z, zmin, zmax)) {
                    mWanderCenter.x = center.x;
                    mWanderCenter.y = center.y;
                    mWanderCenter.z = center.z;
                    mWanderAngle = angle + 0x8000;
                    goto found;
                }
                angle += 0x40;
            } while (angle != mWanderAngle);
            shrink *= 0.9f;
            mWanderAngle = AvatarRandU32();
        } while (shrink > 0.4f);
    found:
        mTargetPos = mPos;
        mTargetPos.y = 0.0f;
    } else {
        mWanderRadius = AvatarRandRange(10.0f, 16.0f);
        mWanderCenter.x = AvatarRandRange(mBoundXMin + mWanderRadius, mBoundXMax - mWanderRadius);
        mWanderCenter.y = 0.0f;
        mWanderCenter.z = AvatarRandRange(mBoundZMin + mWanderRadius, mBoundZMax - mWanderRadius);
        mWanderAngle = AvatarRandU32();
        mTargetPos.x = mWanderRadius * nw4r::math::CosIdx(mWanderAngle) + mWanderCenter.x;
        mTargetPos.y = 0.0f;
        mTargetPos.z = mWanderRadius * nw4r::math::SinIdx(mWanderAngle) + mWanderCenter.z;
    }
    mWanderStep = 0x240;
    if (AvatarRandU32() & 0x10) {
        mWanderStep = -mWanderStep;
    }
}

DECL_WEAK f32 AvatarUnitScale() {
    return 1.0f;
}

DECL_WEAK f32 AvatarWanderMin() {
    return 10.0f;
}

void RPDucAvatar::SpawnHitEffect() {}

void RPDucAvatar::PlaySpawnSound() {}

void RPDucAvatar::PlayHitSound(s32 player) {
    mpSndActor->startSound(19U);
}

void RPDucAvatar::StateShootable() {
    s32 player = GetHittingPlayer();
    if (player != -1) {
        SpawnHitEffect();
        PlayHitSound(player);
        EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
            0x0FFFFFFF, 8, 0x20);
        ShowScorePopup(10, player, false);
        RPDucScoreMgr::sInstance->AddScore(player, 10);
        RPDucScoreMgr::sInstance->AddHits(player, 1);
        RPDucScoreMgr::sInstance->AddStreak(player, 1);
        Remove();
    }
}

void RPDucAvatar::StateRiseUp() {
    f32 y = 0.4f + mPos.y;
    if (y > 0.0f) {
        mState = 3;
        mVelocity.y = 0.4f - y;
    } else {
        mVelocity.y = 0.4f;
    }
}

void RPDucAvatar::StateWalkToX() {
    mRot.z.x = mDir;
    mRot.z.y = 0.0f;
    mRot.z.z = 0.0f;
    mRot.y = RP_vecEy;
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    f32 dist = __fabsf(mPos.x - mWalkToX);
    if (dist < 0.5f) {
        mVelocity.x = dist * mDir;
        mVelocity.y = 0.0f;
        mVelocity.z = 0.0f;
        mState = 3;
    } else {
        mVelocity.x = 0.5f * mDir;
        mVelocity.y = 0.0f;
        mVelocity.z = 0.0f;
    }
}

void RPDucAvatar::StateIdle() {
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    if (IsLineUpTime()) {
        mTargetPos = RPDucAvatarMgr::sInstance->CalcLinePosition(this);
        SetAnim(2);
        mState = 9;
    } else if (RPDucAvatarMgr::sFinalRoundFlag) {
        mWanderTimer = (s16)((AvatarRandU32() & 0x7FFF) % 12) + 24;
        PickWanderCircle();
        SetAnim(2);
        mState = 4;
    } else {
        TurnTowardPoint();
    }
}

void RPDucAvatar::StateWander() {
    u16 angle = mWanderAngle + mWanderStep;
    f32 x = mWanderRadius * nw4r::math::CosIdx(angle) + mWanderCenter.x;
    f32 z = mWanderRadius * nw4r::math::SinIdx(angle) + mWanderCenter.z;
    mVelocity.x = x - mPos.x;
    mVelocity.y = 0.0f;
    mVelocity.z = z - mPos.z;
    mWanderAngle = angle;
    mRot.z.x = mVelocity.x;
    mRot.z.y = 0.0f;
    mRot.z.z = mVelocity.z;
    PSVECNormalize(mRot.z, mRot.z);
    mRot.y = RP_vecEy;
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    if (--mWanderTimer <= 0) {
        nw4r::math::VEC3 center;
        center.x = 2.0f * mWanderRadius * nw4r::math::CosIdx(mWanderAngle) + mWanderCenter.x;
        center.y = 0.0f;
        center.z = 2.0f * mWanderRadius * nw4r::math::SinIdx(mWanderAngle) + mWanderCenter.z;
        if (RPDucInRange(center.x, mBoundXMin + mWanderRadius, mBoundXMax - mWanderRadius) &&
            RPDucInRange(center.z, mBoundZMin + mWanderRadius, mBoundZMax - mWanderRadius)) {
            mWanderCenter.x = center.x;
            mWanderCenter.y = center.y;
            mWanderCenter.z = center.z;
            mWanderAngle += 0x8000;
            mWanderStep = -mWanderStep;
            mpSndActor->startSound(51U);
        }
        mWanderTimer = (s16)((AvatarRandU32() & 0x7FFF) % 12) + 24;
    }
    if (IsLineUpTime()) {
        mTargetPos = RPDucAvatarMgr::sInstance->CalcLinePosition(this);
        SetAnim(2);
        mState = 9;
    }
}

void RPDucAvatar::StateCarried() {
    mVelocity = RP_vecZero;
    mPos.x = mCarryPos.x;
    mPos.y = mCarryPos.y;
    mPos.z = mCarryPos.z;
    mScale.x = mCarryValue;
    mScale.y = mCarryValue;
    mScale.z = mCarryValue;
    mRot = RP_mtx33Identity;
}

void RPDucAvatar::StateDropped() {
    mVelocity.x = 0.0f;
    mVelocity.y -= 0.05f;
    mVelocity.y *= 0.998f;
    mVelocity.z = 0.0f;
    if (mPos.y < 0.0f) {
        mPos.y = 0.0f;
        mVelocity.y = 0.0f;
    }
    f32 step = mScale.x + GetScaleStep();
    SetAvatarScale(step);
    if (step > 1.6f) {
        mScale.x = 1.6f;
        mScale.y = 1.6f;
        mScale.z = 1.6f;
    }
    if (0.0f == mPos.y) {
        f32 size = GetAvatarScale();
        if (size == GetFullScale() && mScale.y == size && mScale.z == size) {
            if (IsWithinBounds(mPos)) {
                mpSndActor->startSound(51U);
                SetAnim(0);
                mState = 3;
            } else {
                mTargetPos.x =
                    AvatarRandRange(mBoundXMin + mWanderRadius, mBoundXMax - mWanderRadius);
                mTargetPos.y = 0.0f;
                mTargetPos.z =
                    AvatarRandRange(mBoundZMin + mWanderRadius, mBoundZMax - mWanderRadius);
                SetAnim(2);
                mState = 8;
            }
        }
    }
}

void RPDucAvatar::StateEnter() {
    nw4r::math::VEC3 dir;
    dir.x = mTargetPos.x - mPos.x;
    dir.y = 0.0f;
    dir.z = mTargetPos.z - mPos.z;
    f32 dist = PSVECMag(dir);
    if (0.0f != dist) {
        PSVECNormalize(dir, dir);
        mRot.z.x = dir.x;
        mRot.z.y = dir.y;
        mRot.z.z = dir.z;
        mRot.y = RP_vecEy;
        PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    }
    if (dist < 2.0f) {
        mVelocity.x = dir.x;
        mVelocity.z = dir.z;
    } else {
        mVelocity.x = 2.0f * dir.x;
        mVelocity.z = 2.0f * dir.z;
    }
    mVelocity.y -= 0.05f;
    mVelocity.y *= 0.998f;
    if (mPos.y < 0.0f) {
        mPos.y = 0.0f;
        mVelocity.y = 0.0f;
    }
    f32 step = mScale.x + GetScaleStep();
    SetAvatarScale(step);
    if (step > 1.6f) {
        mScale.x = 1.6f;
        mScale.y = 1.6f;
        mScale.z = 1.6f;
    }
    if (0.0f == mPos.y) {
        f32 size = GetAvatarScale();
        if (size == GetFullScale() && mScale.y == size && mScale.z == size) {
            if (IsWithinBounds(mPos)) {
                mpSndActor->startSound(51U);
                SetAnim(0);
                mState = 3;
            } else {
                mTargetPos.x =
                    AvatarRandRange(mBoundXMin + mWanderRadius, mBoundXMax - mWanderRadius);
                mTargetPos.y = 0.0f;
                mTargetPos.z =
                    AvatarRandRange(mBoundZMin + mWanderRadius, mBoundZMax - mWanderRadius);
                SetAnim(2);
                mState = 8;
            }
        }
    }
}

void RPDucAvatar::StateRunToPoint() {
    nw4r::math::VEC3 dir;
    dir.x = mTargetPos.x - mPos.x;
    dir.y = 0.0f;
    dir.z = mTargetPos.z - mPos.z;
    f32 dist = PSVECMag(dir);
    if (0.0f != dist) {
        PSVECNormalize(dir, dir);
        mRot.z.x = dir.x;
        mRot.z.y = dir.y;
        mRot.z.z = dir.z;
        mRot.y = RP_vecEy;
        PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    }
    if (dist < 2.0f) {
        mVelocity.x = dir.x;
        mVelocity.y = dir.y;
        mVelocity.z = dir.z;
        mpSndActor->startSound(51U);
        SetAnim(0);
        mState = 3;
    } else {
        mVelocity.x = 2.0f * dir.x;
        mVelocity.y = 0.0f;
        mVelocity.z = 2.0f * dir.z;
    }
    if (IsLineUpTime()) {
        mpSndActor->startSound(51U);
        mTargetPos = RPDucAvatarMgr::sInstance->CalcLinePosition(this);
        SetAnim(2);
        mState = 9;
    }
}

void RPDucAvatar::StateWalkToLine() {
    nw4r::math::VEC3 dir;
    dir.x = mTargetPos.x - mPos.x;
    dir.y = 0.0f;
    dir.z = mTargetPos.z - mPos.z;
    f32 dist = PSVECMag(dir);
    if (0.0f != dist) {
        PSVECNormalize(dir, dir);
        mRot.z.x = dir.x;
        mRot.z.y = dir.y;
        mRot.z.z = dir.z;
        mRot.y = RP_vecEy;
        PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    }
    if (dist < 1.5f) {
        mVelocity.x = dir.x;
        mVelocity.y = dir.y;
        mVelocity.z = dir.z;
        SetAnim(0);
        mpSndActor->startSound(51U);
        mState = 10;
    } else {
        mVelocity.x = 1.5f * dir.x;
        mVelocity.y = 0.0f;
        mVelocity.z = 1.5f * dir.z;
    }
}

void RPDucAvatar::StateLinedUp() {
    mVelocity = RP_vecZero;
    mRot.z.z += 0.1f;
    PSVECNormalize(mRot.z, mRot.z);
    mRot.y = RP_vecEy;
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    if (!mAllGone && RPDucAvatarMgr::sInstance->AreAllGone()) {
        mAllGone = true;
    }
    if (mAllGone && ++mLineTimer >= 90) {
        nw4r::math::VEC3 pos = mPos;
        pos.y = 30.0f;
        nw4r::math::VEC2 screenPos;
        RPDucCamera::sInstance->ProjectToScreen(&screenPos, pos);
        s32 player = mAnim;
        RPDucHud::sInstance->SpawnScorePopup(screenPos, 10, player, 1, false);
        RPDucScoreMgr::sInstance->AddScore(player, 10);
        if (RPDucAvatarMgr::sInstance->IsLowest(this)) {
            mpSndActor->startSound(50U);
        }
        RPDucSpawner::sInstance->TriggerBonusWave();
        SetAnim(3);
        mState = 11;
    }
}

void RPDucAvatar::StateCelebrate() {
    mVelocity = RP_vecZero;
    mRot = RP_mtx33Identity;
    if (++mCelebrateTimer >= 360) {
        s32 dir = 1;
        if (mAnim == 0) {
            dir = -1;
        }
        mDir = dir;
        SetAnim(2);
        u32 flag;
        if (RPDucGame::sInstance->GetMode() != 1 && !RPDucGame::sInstance->IsP2Joined()) {
            flag = 1;
        } else {
            flag = 3;
        }
        mpSndActor->startSoundWithRemotePlayer(52U, flag, 0);
        mState = 12;
    }
}

void RPDucAvatar::StateRunAway() {
    mVelocity.x = 0.8f * mDir;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    mRot.z.x = 0.1f * mDir + mRot.z.x;
    PSVECNormalize(mRot.z, mRot.z);
    mRot.y = RP_vecEy;
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    nw4r::math::VEC2 screenPos;
    RPDucCamera::sInstance->ProjectToScreen(&screenPos, mPos);
    if (__fabsf(screenPos.x) > 1.1f) {
        Remove();
    }
}

void RPDucAvatar::Create() {
    RPDucTarget::Create();
    RPSysKokeshiGenInfo info;
    info.SetGenType((RPSysKokeshiManager::GenType)0);
    info.SetExpressionFlag(0x13);
    info.SetBodyType(4);
    info.SetPlayerNo(0);
    mpKokeshi[0] = new RPSysKokeshi(&info);
    info.SetPlayerNo(1);
    if (RPSceneInfo::spInstance->GetPlayerNum() == 1) {
        info.SetPlayerNo(0);
    }
    mpKokeshi[1] = new RPSysKokeshi(&info);
}

void RPDucAvatar::Load() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    for (s32 i = 0; i < 2; i++) {
        mpKokeshi[i]->LoadResource();
        mpKokeshi[i]->ApplyLightTexture(0);
        RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
        RPGrpHandle file = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResFile,
            RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_avatar_anim.brres"));
        RPGrpModel* pBody = mpKokeshi[i]->GetBodyModel();
        pBody->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 4);
        pBody->GetModelAnm()->Regist(
            RPGrpModelAnm::Anm_Chr, 0,
            RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "sht_waitR"));
        pBody->GetModelAnm()->Regist(
            RPGrpModelAnm::Anm_Chr, 1,
            RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "sht_batabataR"));
        pBody->GetModelAnm()->Regist(
            RPGrpModelAnm::Anm_Chr, 2,
            RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "sht_runawayR"));
        pBody->GetModelAnm()->Regist(
            RPGrpModelAnm::Anm_Chr, 3,
            RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "sht_happyR"));
    }
}

// The original retains the unsigned conversion bias before the animation literals.
// This unused helper seeds that shared literal; its code is removed by the linker.
DECL_WEAK f32 AvatarUnsignedConversion(u32 value) {
    return value;
}

void RPDucAvatar::ApplyVelocity() {
    mPos.x += mVelocity.x;
    mPos.y += mVelocity.y;
    mPos.z += mVelocity.z;
}

void RPDucAvatar::Calculate() {
    (this->*mStateFuncs[mState])();
    mpSndActor->set3DPosition(mPos);
}

void RPDucAvatar::CalcModelMtx() {
    RPSysKokeshi* pKokeshi = mpKokeshi[mAnim];
    RPGrpModel* pHead = pKokeshi->GetNigaoeModel();
    RPGrpModel* pBody = pKokeshi->GetBodyModel();
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, mRot, RP_vecOne, mPos);
    nw4r::g3d::ScnObj* pScnObj = pBody->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    pBody->SetBaseScale(mScale);
    pBody->Calc();
    EGG::Matrix34f headMtx;
    mpKokeshi[mAnim]->GetHeadJointMtx(&headMtx);
    pScnObj = pHead->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &headMtx);
    }
    pHead->SetBaseScale(mScale);
    pHead->Calc();
    pHead->Entry();
    pBody->Entry();
    pHead->UpdateFrame();
    pBody->UpdateFrame();
}

void RPDucAvatar::StartAbduct() {
    mVelocity = RP_vecZero;
    SetAnim(1);
    mpSndActor->startSound(48U);
    mState = 5;
}

void RPDucAvatar::Release() {
    if (mAge >= 0) {
        mVelocity = RP_vecZero;
        if (mState == 5) {
            mpSndActor->startSound(49U);
        }
        if (mPos.y > 0.0f || mScale.x <= 1.6f) {
            mState = 6;
        } else {
            mState = 3;
        }
    }
}

void RPDucAvatar::Kill() {
    Remove();
}

f32 RPDucAvatar::GetCarryValue() {
    return 1.6f;
}

