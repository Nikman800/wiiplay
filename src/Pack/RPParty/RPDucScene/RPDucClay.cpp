// Shooting Range clay pigeon (RPDucClay): thrown towards a random aim point, rises, slows and
// falls; scores more when hit far away.
// Unit 0x80238EAC-0x80239EAC (the range ends before RPDucDuckMgr::SpawnTarget, the last function
// of the duck manager file that follows). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucClay.h>

#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPDucScene/RPDucCamera.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <egg/core.h>
#include <egg/gfxe/eggScreen.h>

static RPDucClay::StateFunc sCheckHitFunc = &RPDucClay::StateCheckHit;
static RPDucClay::StateFunc sRiseFunc = &RPDucClay::StateRise;
static RPDucClay::StateFunc sSlowFunc = &RPDucClay::StateSlow;
static RPDucClay::StateFunc sFallFunc = &RPDucClay::StateFall;

RPDucClay::RPDucClay() : mSpin(0.0f), mThrust(0.0f) {
    mStateFuncs[0] = sCheckHitFunc;
    mStateFuncs[1] = sRiseFunc;
    mStateFuncs[2] = sSlowFunc;
    mStateFuncs[3] = sFallFunc;
}

RPDucClay::~RPDucClay() {}

void RPDucClay::ResetParams() {
    f32 radius = mParam.mPatterns[0].mHitRadius;
    mHitRadius = radius;
    mHitScale = mParam.mPatterns[0].mHitScale;
    f32 scale = radius / mParam.mBaseRadius;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    mRot = RP_mtx33Identity;
    mSpeedScale = 1.0f;
    mState = 0;
}

void RPDucClay::Launch(u32 pattern) {
    unkE8 = pattern & 7;
    SetupLaunch();
}

void RPDucClay::SetupLaunch() {
    RPDucClayPattern pat = mParam.mPatterns[unkE8];
    mAimPos.x = 0.0f;
    mAimPos.y = pat.mAimY;
    mAimPos.z = pat.mAimZ;

    f32 rangeX = pat.mAimRangeX[0];
    if (EGG::Screen::GetTVMode() == 0) {
        rangeX = pat.mAimRangeX[1];
    }
    mAimPos.x += RPDucClayRandRange(-rangeX, rangeX);
    mAimPos.y += RPDucClayRandRange(-pat.mAimRangeY, pat.mAimRangeY);

    nw4r::math::VEC3 dir;
    dir.x = mAimPos.x - mPos.x;
    dir.y = mAimPos.y - mPos.y;
    dir.z = mAimPos.z - mPos.z;
    PSVECNormalize(dir, dir);
    mVelocity.x = dir.x * pat.mSpeed;
    mVelocity.y = dir.y * pat.mSpeed;
    mVelocity.z = dir.z * pat.mSpeed;

    mRot = RP_mtx33Identity;
    if (0.0f > mVelocity.x) {
        mRot.y.x = RPDucClayRandRange(pat.mTiltRange[1], pat.mTiltRange[0]);
    } else {
        mRot.y.x = RPDucClayRandRange(-pat.mTiltRange[0], -pat.mTiltRange[1]);
    }
    mRot.y.y = 1.0f - mRot.y.x * mRot.y.x;
    mRot.z.x = dir.x;
    mRot.z.y = dir.y;
    mRot.z.z = dir.z;
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    PSVECNormalize(mRot.x, mRot.x);
    PSVECCrossProduct(mRot.z, mRot.x, mRot.y);

    mThrust = pat.mThrust;
    mSpin = mParam.mSpin;
    mState = 1;
}

// Materialize the unsigned conversion pool before SetupLaunch's random scale.
// This unreferenced helper is stripped from the linked executable.
#pragma auto_inline off
f32 RPDucClayUintToFloat(u32 value) {
    return (f32)value;
}
#pragma auto_inline on

bool RPDucClay::CheckShotHit() {
    s32 player = GetHittingPlayer();
    if (player != -1) {
        SpawnHitEffect();
        PlayHitSound(player);
        EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
            0x0FFFFFFF, 8, 0x20);

        s32 points = mParam.mPoints;
        bool bonus = false;
        if (mPos.z > mParam.mBonusZ) {
            points = mParam.mBonusPoints;
            bonus = true;
        }
        RPDucScoreMgr::sInstance->AddScore(player, points);
        RPDucScoreMgr::sInstance->AddHits(player, 1);
        RPDucScoreMgr::sInstance->AddStreak(player, 1);
        ShowScorePopup(points, player, bonus);
        Remove();
        return true;
    }
    return false;
}

void RPDucClay::SpawnHitEffect() {
    RPSysEffect effect("Sht_clay_hit", 0, 0);
    effect.create();

    RPAxes axes = RP_mtx33Identity;
    axes.z = mVelocity;
    axes.z.y = 0.0f;
    if (0.0f != PSVECMag(axes.z)) {
        PSVECNormalize(axes.z, axes.z);
    } else {
        axes.z = RP_vecEz;
    }
    PSVECCrossProduct(axes.y, axes.z, axes.x);
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, axes, RP_vecOne, RP_vecZero);
    effect.setMtx(mtx);
    effect.setPos(nw4r::math::VEC3(mPos.x, mPos.y, mPos.z));
    f32 scale = mParam.mPatterns[unkE8].mHitRadius / mParam.mBaseRadius;
    effect.setScale(nw4r::math::VEC3(scale, scale, scale));
    effect.update();
}

void RPDucClay::PlaySpawnSound() {
    u32 flag;
    if (RPDucGame::sInstance->GetMode() != 1 && !RPDucGame::sInstance->IsP2Joined()) {
        flag = 1;
    } else {
        flag = 3;
    }
    mpSndActor->startSound(29U);
    mpSndActor->startSoundWithRemotePlayer(30U, flag, 0);
}

void RPDucClay::PlayHitSound(s32 player) {
    nw4r::math::VEC3 camPos = RPDucCamera::sInstance->GetLookPos();
    nw4r::math::VEC3 diff;
    diff.x = mPos.x - camPos.x;
    diff.y = mPos.y - camPos.y;
    diff.z = mPos.z - camPos.z;
    f32 dist = PSVECMag(diff);

    u32 flag = 0;
    if (player == 0) {
        flag = 1;
    } else if (player == 1) {
        flag = 2;
    }

    if (dist < 600.0f) {
        mpSndActor->startSoundWithRemotePlayer(15U, flag, 0);
    } else if (dist < 1200.0f) {
        mpSndActor->startSoundWithRemotePlayer(16U, flag, 0);
    } else {
        mpSndActor->startSoundWithRemotePlayer(17U, flag, 0);
    }
}

void RPDucClay::StateCheckHit() {
    CheckShotHit();
}

void RPDucClay::StateRise() {
    mThrust = mParam.mPatterns[unkE8].mThrust;
    mVelocity.x += mRot.y.x * mThrust;
    mVelocity.y += mRot.y.y * mThrust;
    mVelocity.y -= mParam.mPatterns[unkE8].mThrust;
    mVelocity.x *= mParam.mDrag;
    mVelocity.y *= mParam.mDrag;
    mVelocity.z *= mParam.mDrag;
    mSpin *= mParam.mSpinDecay;

    if (!CheckShotHit() && mAimPos.z > mPos.z) {
        mState = 2;
    }
}

void RPDucClay::StateSlow() {
    mThrust *= mParam.mPatterns[unkE8].mThrustDecay;
    mVelocity.x += mRot.y.x * mThrust;
    mVelocity.y += mRot.y.y * mThrust;
    mVelocity.y += -mParam.mPatterns[unkE8].mThrust;
    mVelocity.x *= mParam.mDrag;
    mVelocity.y *= mParam.mDrag;
    mVelocity.z *= mParam.mDrag;
    mSpin *= mParam.mSpinDecay;

    if (mPos.z < mParam.mFarZ) {
        mHitScale = mParam.mPatterns[unkE8].mHitScale *
                    (1.0f + (mParam.mFarZ - mPos.z) / (mParam.mFarZ - mParam.mNearZ));
    }

    if (!CheckShotHit() && 0.0f > mVelocity.y) {
        mState = 3;
    }
}

void RPDucClay::StateFall() {
    f32 prev = mSpeedScale;
    mSpeedScale += mParam.mPatterns[unkE8].mFallScaleAdd;
    if (mSpeedScale > mParam.mPatterns[unkE8].mFallScaleMax) {
        mSpeedScale = mParam.mPatterns[unkE8].mFallScaleMax;
    }
    f32 delta = mSpeedScale - prev;
    mThrust *= mParam.mPatterns[unkE8].mThrustDecay;
    mVelocity.x += mSpeedScale * (mRot.y.x * mThrust);
    mVelocity.y += mSpeedScale * (mRot.y.y * mThrust);
    mVelocity.y += mSpeedScale * -mParam.mPatterns[unkE8].mThrust;
    mVelocity.z = 0.3f * (mVelocity.z * delta) + mVelocity.z;
    mVelocity.x *= mParam.mDrag;
    mVelocity.y *= mParam.mDrag;
    mVelocity.z *= mParam.mDrag;
    mSpin *= mParam.mSpinDecay;

    if (mPos.z < mParam.mFarZ) {
        mHitScale = mParam.mPatterns[unkE8].mHitScale *
                    (1.0f + (mParam.mFarZ - mPos.z) / (mParam.mFarZ - mParam.mNearZ));
    }

    if (!CheckShotHit() && 0.0f > mPos.y) {
        Remove();
    }
}

void RPDucClay::Load() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    u32 size = 0;
    mParam = *static_cast<RPDucClayParam*>(
        RPSysResourceManager::GetFileFromArchive(pArchive, "param/DucClayParam.bin", &size));

    pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_clay.brres"));
    mpModel = RPGrpModel::Construct(
        pResMgr->CreateData(RPGrpModelResManager::Type_ResMdl,
                            pResMgr->GetPtr(RPGrpModelResManager::Type_ResMdl, file, 0)),
        1, 0, 0);
    mpModel->SetDrawGroup(0);
    mpModel->SetDrawScene(0);
}

void RPDucClay::ApplyVelocity() {
    mPos.x += mVelocity.x;
    mPos.y += mVelocity.y;
    mPos.z += mVelocity.z;
    mRot.z.x = mSpin * mRot.x.x + mRot.z.x;
    mRot.z.y = mSpin * mRot.x.y + mRot.z.y;
    mRot.z.z = mSpin * mRot.x.z + mRot.z.z;
    PSVECNormalize(mRot.z, mRot.z);
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
}

void RPDucClay::Calculate() {
    (this->*mStateFuncs[mState])();
    mpSndActor->set3DPosition(mPos);
}


