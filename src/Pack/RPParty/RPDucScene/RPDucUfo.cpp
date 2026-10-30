// Shooting Range UFO (RPDucUfo): wanders, circles, or abducts a player Mii in the final round.
// Unit 0x8023ED50-0x802420CC, ending before RPDucCamera.cpp. Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucUfo.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPDucScene/RPDucAvatar.h>
#include <Pack/RPParty/RPDucScene/RPDucCamera.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPSystem.h>

#include <egg/core.h>

static inline u32 UfoRandU32() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    return RP_randSeedX ^ RP_randSeedY;
}

static inline f32 UfoRandF32() {
    return 0.0000001192093f * (f32)(UfoRandU32() & 0x7FFFFF);
}

static inline f32 UfoRandF32Range(f32 lo, f32 hi) {
    if (lo == hi) {
        return lo;
    }
    f32 r = UfoRandF32();
    return __fmadds(hi - lo, r, lo);
}

static inline u32 UfoRandRange(u32 range) {
    return (UfoRandU32() & 0x7FFF) % range;
}

#define RPRandU32 UfoRandU32
#define RPRandF32Range UfoRandF32Range

static RPDucUfo::StateFunc sStateCheckHit = &RPDucUfo::StateCheckHit;
static RPDucUfo::StateFunc sStateLeave = &RPDucUfo::StateLeave;
static RPDucUfo::StateFunc sStateWait2 = &RPDucUfo::StateWait2;
static RPDucUfo::StateFunc sStateWanderMove = &RPDucUfo::StateWanderMove;
static RPDucUfo::StateFunc sStateWait = &RPDucUfo::StateWait;
static RPDucUfo::StateFunc sStateMoveToCircleStart = &RPDucUfo::StateMoveToCircleStart;
static RPDucUfo::StateFunc sStateCircle = &RPDucUfo::StateCircle;
static RPDucUfo::StateFunc sStateMoveToCircle = &RPDucUfo::StateMoveToCircle;
static RPDucUfo::StateFunc sStateCircleBeforeAbduct = &RPDucUfo::StateCircleBeforeAbduct;
static RPDucUfo::StateFunc sStateApproachAvatar = &RPDucUfo::StateApproachAvatar;
static RPDucUfo::StateFunc sStateLift = &RPDucUfo::StateLift;
static RPDucUfo::StateFunc sStateHold = &RPDucUfo::StateHold;
static RPDucUfo::StateFunc sStateFlyAway = &RPDucUfo::StateFlyAway;

RPDucUfo::RPDucUfo() : mIsTypeB(false) {
    mStateFuncs[0] = sStateCheckHit;
    mStateFuncs[1] = sStateLeave;
    mStateFuncs[2] = sStateWait2;
    mStateFuncs[3] = sStateWanderMove;
    mStateFuncs[4] = sStateWait;
    mStateFuncs[5] = sStateMoveToCircleStart;
    mStateFuncs[6] = sStateCircle;
    mStateFuncs[7] = sStateMoveToCircle;
    mStateFuncs[8] = sStateCircleBeforeAbduct;
    mStateFuncs[9] = sStateApproachAvatar;
    mStateFuncs[10] = sStateLift;
    mStateFuncs[11] = sStateHold;
    mStateFuncs[12] = sStateFlyAway;
}

RPDucUfo::~RPDucUfo() {}

#pragma auto_inline off
void RPDucUfo::LoadParams() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    u32 size = 0;
    mParam = *static_cast<RPDucUfoParam*>(
        RPSysResourceManager::GetFileFromArchive(pArchive, "param/DucUfoParam.bin", &size));

    s32 i = 0;
    do {
        mParam.mPatterns[i].mWait2Frames *= 3;
        mParam.mPatterns[i].mWaitFrames *= 3;
        mParam.mPatterns[i].mWanderFrames *= 3;
        mParam.mPatterns[i].mCircleTimeMin *= 3;
        mParam.mPatterns[i].mCircleTimeMax *= 3;
        mParam.mPatterns[i].mCircleFrames *= 3;
        mParam.mPatterns[i].mAbductFrames *= 3;
        mParam.mPatterns[i].mLiftFrames[0] *= 3;
        mParam.mPatterns[i].mLiftFrames[1] *= 3;
        mParam.mPatterns[i].mHoldFrames *= 3;
    } while (++i < sPatternNum);
}
#pragma auto_inline reset

void RPDucUfo::ResetParams() {
    mHitRadius = mParam.mPatterns[0].mHitRadius;
    mHitScale = mParam.mPatterns[0].mHitScale;
    RPDucCamera* pCamera = RPDucCamera::sInstance;
    mBoundLeft = pCamera->GetScreenLeftAtDepth(0.0f);
    mBoundRight = pCamera->GetScreenRightAtDepth(0.0f);
    mBoundTop = pCamera->GetScreenTopAtDepth(0.0f);
    mBoundBottom = 5.0f;
    f32 scale = mHitRadius / mParam.mBaseRadius;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    mDir = RP_vecEx;
    mSpeed = 0.0f;
    mCircleRadius = 0.0f;
    mCircleCenter = RP_vecZero;
    mCircleAngle = 0;
    mCircleStep = 0;
    mWaitTimer = 0;
    mMoveCount = 0;
    mTotalTimer = 0;
    mHoldTimer = 0;
    mGoal = RP_vecZero;
    mCarrying = false;
    mLost = false;
    unk608 = -1;
    mBeamPos.x = mPos.x;
    mBeamPos.y = mPos.y;
    mBeamPos.z = mPos.z;
    mBeamScale = 1.0f;
    mMode = 0;
    mState = 0;
}

void RPDucUfo::Launch(u32 pattern) {
    u32 index = pattern & 7;
    unkE8 = index;
    mHitRadius = mParam.mPatterns[index].mHitRadius;
    mHitScale = mParam.mPatterns[index].mHitScale;
    RPDucCamera* pCamera = RPDucCamera::sInstance;
    mBoundLeft = pCamera->GetScreenLeftAtDepth(0.0f) + mHitRadius;
    mBoundRight = pCamera->GetScreenRightAtDepth(0.0f) - mHitRadius;
    f32 top = pCamera->GetScreenTopAtDepth(0.0f);
    mBoundBottom = mHitRadius;
    mBoundTop = top - mHitRadius;
    if (pattern & 8) {
        mIsTypeB = true;
    } else {
        mIsTypeB = false;
    }

    switch ((pattern >> 4) & 3) {
    case 0:
        mGoal.x = RPRandF32Range(mBoundLeft, mBoundRight);
        mGoal.y = RPRandF32Range(mBoundBottom, mBoundTop);
        mGoal.z = mPos.z;
        mDir.x = mGoal.x - mPos.x;
        mDir.y = mGoal.y - mPos.y;
        mDir.z = 0.0f;
        PSVECNormalize(mDir, mDir);
        mSpeed = mParam.mPatterns[unkE8].mWanderSpeed;
        mWaitTimer = 0;
        mMoveCount = mParam.mPatterns[unkE8].mMoveCount;
        mTotalTimer = mParam.mPatterns[unkE8].mWanderFrames;
        mMode = 1;
        mState = 3;
        break;
    case 1:
        SetupCircle();
        break;
    case 2:
        SetupAbduct();
        break;
    }

    if (mIsTypeB) {
        mpModelB->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
        mpModelB->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexSrt, 0);
        mpModelB->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexSrt, 0, 0.0f);
    } else {
        mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
        mpModel->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexSrt, 0);
        mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexSrt, 0, 0.0f);
    }
}

void RPDucUfo::SetupCircle() {
    mCircleRadius = mParam.mCircleRadius;
    mCircleCenter.x = RPRandF32Range(mBoundLeft + mCircleRadius, mBoundRight - mCircleRadius);
    mCircleCenter.y = RPRandF32Range(mBoundBottom + mCircleRadius, mBoundTop - mCircleRadius);
    mCircleCenter.z = mPos.z;
    mCircleAngle = RPRandU32();
    mGoal.x = mCircleRadius * nw4r::math::CosIdx(mCircleAngle) + mCircleCenter.x;
    mGoal.y = mCircleRadius * nw4r::math::SinIdx(mCircleAngle) + mCircleCenter.y;
    mGoal.z = mPos.z;
    if (mIsTypeB) {
        mCircleStep = mParam.mPatterns[unkE8].mCircleStep[0];
    } else {
        mCircleStep = mParam.mPatterns[unkE8].mCircleStep[1];
    }
    mCircleStep = mCircleStep & 0x7FFE;
    if (RPRandU32() & 0x10) {
        mCircleStep = -mCircleStep;
    }
    mDir.x = mGoal.x - mPos.x;
    mDir.y = mGoal.y - mPos.y;
    mDir.z = 0.0f;
    PSVECNormalize(mDir, mDir);
    mSpeed = mParam.mPatterns[unkE8].mCircleSpeed;
    mVelocity.x = mSpeed * mDir.x;
    mVelocity.y = mSpeed * mDir.y;
    mVelocity.z = 0.0f;
    mTotalTimer = mParam.mPatterns[unkE8].mCircleFrames;
    mMode = 2;
    mState = 5;
}

void RPDucUfo::SetupAbduct() {
    RPDucCamera* pCamera = RPDucCamera::sInstance;
    mBoundLeft = pCamera->GetScreenLeftAtDepth(mPos.z) + mHitRadius;
    mBoundRight = pCamera->GetScreenRightAtDepth(mPos.z) - mHitRadius;
    mBoundTop = pCamera->GetScreenTopAtDepth(mPos.z) - mHitRadius;
    mCircleRadius = mParam.mAbductRadius;
    mBoundBottom = mParam.mAbductBottom + mHitRadius;
    mCircleCenter.x = RPRandF32Range(mBoundLeft + mCircleRadius, mBoundRight - mCircleRadius);
    mCircleCenter.y = RPRandF32Range(mBoundBottom + mCircleRadius, mBoundTop - mCircleRadius);
    mCircleCenter.z = mPos.z;
    mCircleAngle = RPRandU32();
    mGoal.x = mCircleRadius * nw4r::math::CosIdx(mCircleAngle) + mCircleCenter.x;
    mGoal.y = mCircleRadius * nw4r::math::SinIdx(mCircleAngle) + mCircleCenter.y;
    mGoal.z = mPos.z;
    if (mIsTypeB) {
        mCircleStep = mParam.mPatterns[unkE8].mAbductStep[0];
    } else {
        mCircleStep = mParam.mPatterns[unkE8].mAbductStep[1];
    }
    mCircleStep = mCircleStep & 0x7FFE;
    if (RPRandU32() & 0x10) {
        mCircleStep = -mCircleStep;
    }
    mDir.x = mGoal.x - mPos.x;
    mDir.y = mGoal.y - mPos.y;
    mDir.z = 0.0f;
    PSVECNormalize(mDir, mDir);
    mSpeed = mParam.mPatterns[unkE8].mAbductSpeed;
    mVelocity.x = mSpeed * mDir.x;
    mVelocity.y = mSpeed * mDir.y;
    mVelocity.z = 0.0f;
    mHoldTimer = mParam.mPatterns[unkE8].mAbductFrames;
    mMode = 3;
    mState = 7;
}

void RPDucUfo::PickWanderTarget() {
    mDir.x = RPRandF32Range(-1.0f, 1.0f);
    mDir.y = RPRandF32Range(-1.0f, 1.0f);
    mDir.z = 0.0f;
    f32 len = PSVECMag(mDir);
    if (0.0f == len) {
        mDir.y = 0.0f;
        mDir.x = 1.0f;
    }
    f32 inv = 1.0f / len;
    mDir.x *= inv;
    mDir.y *= inv;

    f32 dist = RPRandF32Range(mParam.mWanderDistMin, mParam.mWanderDistMax);
    f32 turn = (RPRandU32() & 0x100) ? 0.1f : -0.1f;
    while (true) {
        mGoal.x = mDir.x * dist + mPos.x;
        mGoal.y = mDir.y * dist + mPos.y;
        mGoal.z = mPos.z;
        if (RPDucInRange(mGoal.x, mBoundLeft, mBoundRight) &&
            RPDucInRange(mGoal.y, mBoundBottom, mBoundTop)) {
            break;
        }
        dist *= 0.995f;
        dist = mParam.mWanderDistMin <= dist ? dist : mParam.mWanderDistMin;
        nw4r::math::VEC3 side(0.0f, 0.0f, 0.0f);
        PSVECCrossProduct(RP_vecEz, mDir, side);
        mDir.x = side.x * turn + mDir.x;
        mDir.y = side.y * turn + mDir.y;
        PSVECNormalize(mDir, mDir);
    }
}

void RPDucUfo::SetupFlee() {
    mGoal.x = 0.0f;
    mGoal.y = mParam.mPatterns[unkE8].mFleeAimY;
    mGoal.z = mParam.mPatterns[unkE8].mFleeAimZ;
    mGoal.x += RPRandF32Range(-mParam.mPatterns[unkE8].mFleeRangeX,
                              mParam.mPatterns[unkE8].mFleeRangeX);
    mGoal.y += RPRandF32Range(-mParam.mPatterns[unkE8].mFleeRangeY,
                              mParam.mPatterns[unkE8].mFleeRangeY);
    nw4r::math::VEC3 dir;
    dir.x = mGoal.x - mPos.x;
    dir.y = mGoal.y - mPos.y;
    dir.z = mGoal.z - mPos.z;
    PSVECNormalize(dir, dir);
    f32 speed;
    if (RPDucGame::sInstance->GetMode() != 1) {
        speed = mParam.mPatterns[unkE8].mFleeSpeed[0];
    } else {
        speed = mParam.mPatterns[unkE8].mFleeSpeed[1];
    }
    mVelocity.x = dir.x * speed;
    mVelocity.y = dir.y * speed;
    mVelocity.z = dir.z * speed;
    if (mVelocity.z > 0.0f) {
        mVelocity.z = 0.0f;
    }
    mState = 12;
}

bool RPDucUfo::CheckShotHit() {
    bool versus = false;
    if (RPDucGame::sInstance->GetMode() == 1) {
        versus = true;
    }
    s32 player = GetHittingPlayer();
    if (player != -1) {
        if (mMode == 3) {
            mpEffects[0]->forceKill();
            mpEffects[1]->forceKill();
            mpEffects[2]->forceKill();
            mpTargetAvatar->mTargeted = false;
            if (mCarrying) {
                mpTargetAvatar->Release();
            }
        }
        SpawnHitEffect();
        PlayHitSound(player);
        if (!mIsTypeB) {
            EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
                0xFFFFFFFF, 8, 0x20);
        } else {
            EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
                0xFFFFFFFF, 20, 0x20);
        }

        s32 points;
        if (mIsTypeB) {
            points = versus ? mParam.mPointsVersusB : mParam.mPointsB;
        } else {
            points = versus ? mParam.mPointsVersusA : mParam.mPointsA;
        }
        RPDucScoreMgr::sInstance->AddScore(player, points);
        RPDucScoreMgr::sInstance->AddHits(player, 1);
        RPDucScoreMgr::sInstance->AddStreak(player, 1);
        ShowScorePopup(points, player, mIsTypeB);
        Remove();
        return true;
    }
    return false;
}

#pragma auto_inline off
void RPDucUfo::UpdateBeamEffects() {
    nw4r::math::VEC3 pos(mBeamPos.x, mBeamPos.y, mBeamPos.z);
    f32 s = mBeamScale * (mHitRadius / mParam.mBaseRadius);
    nw4r::math::VEC3 scale(s, s, s);
    mpEffects[0]->setPos(pos);
    mpEffects[0]->setScale(scale);
    mpEffects[0]->update();
    mpEffects[1]->setPos(pos);
    mpEffects[1]->setScale(scale);
    mpEffects[1]->update();
    mpEffects[2]->setPos(pos);
    mpEffects[2]->setScale(scale);
    mpEffects[2]->update();
}
#pragma auto_inline reset

void RPDucUfo::SpawnHitEffect() {
    nw4r::math::VEC3 pos(mPos.x, mPos.y, mPos.z);
    f32 s = mParam.mPatterns[unkE8].mHitRadius / mParam.mBaseRadius;
    nw4r::math::VEC3 scale(s, s, s);
    if (mIsTypeB) {
        RPSysEffect effect("Sht_ufo_b_hit", 0, 0);
        effect.create();
        effect.setPos(pos);
        effect.setScale(scale);
        effect.update();
    } else {
        RPSysEffect effect("Sht_ufo_a_hit", 0, 0);
        effect.create();
        effect.setPos(pos);
        effect.setScale(scale);
        effect.update();
    }
}

void RPDucUfo::PlaySpawnSound() {
    mpSndActor->startSound(31U);
}

void RPDucUfo::PlayHitSound(s32 player) {
    u32 flag = 0;
    if (player == 0) {
        flag = 1;
    } else if (player == 1) {
        flag = 2;
    }
    if (!mIsTypeB) {
        mpSndActor->startSoundWithRemotePlayer(18U, flag, 0);
    } else {
        mpSndActor->startSoundWithRemotePlayer(25U, flag, 0);
    }
}

void RPDucUfo::StateCheckHit() {
    CheckShotHit();
}

void RPDucUfo::StateLeave() {
    mVelocity.y += mParam.mPatterns[unkE8].mLeaveAccel;
    if (mPos.z > 0.0f) {
        mPos.z = 0.0f;
        mVelocity.z = 0.0f;
    }
    if (!CheckShotHit()) {
        nw4r::math::VEC2 screenPos;
        RPDucCamera::sInstance->ProjectToScreen(&screenPos, mPos);
        if (screenPos.y > 1.3f) {
            Remove();
        }
    }
}

void RPDucUfo::StateWait2() {
    mVelocity = RP_vecZero;
    if (!CheckShotHit()) {
        if (--mWaitTimer == 0) {
            PickWanderTarget();
            mState = 3;
        }
        if (--mTotalTimer <= 0) {
            mState = 1;
        }
    }
}

void RPDucUfo::StateWanderMove() {
    nw4r::math::VEC3 diff;
    diff.x = mGoal.x - mPos.x;
    diff.y = mGoal.y - mPos.y;
    diff.z = 0.0f;
    f32 dist = PSVECMag(diff);
    if (dist > mSpeed) {
        mVelocity.x = mDir.x * mSpeed; mVelocity.y = mDir.y * mSpeed; mVelocity.z = 0.0f;
    } else {
        SetVelocity2D(mDir.y * dist, mDir.x * dist);
    }
    mVelocity.z = mParam.mPatterns[unkE8].mWanderVelZ;
    if (mPos.z > 0.0f) {
        mPos.z = 0.0f;
    }
    if (mPos.z < mParam.mNearZ) {
        mHitScale = mParam.mPatterns[unkE8].mHitScale *
                    (1.0f + (mParam.mNearZ - mPos.z) / (mParam.mNearZ - mParam.mGrowZ));
    }

    if (!CheckShotHit()) {
        if (dist < mSpeed) {
            if (--mMoveCount == 0) {
                mWaitTimer = mParam.mPatterns[unkE8].mWaitFrames;
                mMoveCount = mParam.mPatterns[unkE8].mMoveCount;
                mState = 4;
            } else {
                mWaitTimer = mParam.mPatterns[unkE8].mWait2Frames;
                mState = 2;
            }
        }
        if (--mTotalTimer <= 0) {
            mVelocity = RP_vecZero;
            mState = 1;
        }
    }
}

void RPDucUfo::StateWait() {
    mVelocity = RP_vecZero;
    if (!CheckShotHit()) {
        if (--mWaitTimer == 0) {
            PickWanderTarget();
            mState = 3;
        }
        if (--mTotalTimer <= 0) {
            mState = 1;
        }
    }
}

void RPDucUfo::StateMoveToCircleStart() {
    nw4r::math::VEC3 diff;
    diff.x = mGoal.x - mPos.x;
    diff.y = mGoal.y - mPos.y;
    diff.z = 0.0f;
    f32 dist = PSVECMag(diff);
    if (dist > mSpeed) {
        mVelocity.x = mDir.x * mSpeed; mVelocity.y = mDir.y * mSpeed; mVelocity.z = 0.0f;
    } else {
        SetVelocity2D(mDir.y * dist, mDir.x * dist);
    }

    if (!CheckShotHit()) {
        if (dist < mSpeed) {
            f32 time = RPRandF32Range(mParam.mPatterns[unkE8].mCircleTimeMin,
                                      mParam.mPatterns[unkE8].mCircleTimeMax);
            mState = 6;
            mMoveCount = time;
        }
        if (--mTotalTimer == 0) {
            mState = 1;
        }
    }
}

void RPDucUfo::StateCircle() {
    u16 angle = mCircleAngle + mCircleStep;
    f32 x = mCircleRadius * nw4r::math::CosIdx(angle) + mCircleCenter.x;
    f32 y = mCircleRadius * nw4r::math::SinIdx(angle) + mCircleCenter.y;
    mVelocity.x = x - mPos.x;
    mVelocity.y = y - mPos.y;
    mCircleAngle = angle;
    mVelocity.z = 0.0f;
    if (--mMoveCount <= 0) {
        nw4r::math::VEC3 center;
        center.x = 2.0f * mCircleRadius * nw4r::math::CosIdx(angle) + mCircleCenter.x;
        center.y = 2.0f * mCircleRadius * nw4r::math::SinIdx(mCircleAngle) + mCircleCenter.y;
        center.z = mCircleCenter.z;
        if (RPDucInRange(center.x, mBoundLeft + mCircleRadius, mBoundRight - mCircleRadius) &&
            RPDucInRange(center.y, mBoundBottom + mCircleRadius, mBoundTop - mCircleRadius)) {
            mCircleCenter.x = center.x;
            mCircleCenter.y = center.y;
            mCircleCenter.z = center.z;
            mCircleAngle += 0x8000;
            mCircleStep = -mCircleStep;
        }
        mMoveCount = RPRandF32Range(mParam.mPatterns[unkE8].mCircleTimeMin,
                                    mParam.mPatterns[unkE8].mCircleTimeMax);
    }

    mVelocity.z = mParam.mPatterns[unkE8].mCircleVelZ;
    if (mPos.z > 0.0f) {
        mPos.z = 0.0f;
    }
    if (mPos.z < mParam.mNearZ) {
        mHitScale = mParam.mPatterns[unkE8].mHitScale *
                    (1.0f + (mParam.mNearZ - mPos.z) / (mParam.mNearZ - mParam.mGrowZ));
    }

    if (!CheckShotHit()) {
        if (--mTotalTimer == 0) {
            mState = 1;
        }
    }
}

void RPDucUfo::StateMoveToCircle() {
    nw4r::math::VEC3 diff;
    diff.x = mGoal.x - mPos.x;
    diff.y = mGoal.y - mPos.y;
    diff.z = 0.0f;
    f32 dist = PSVECMag(diff);
    if (dist > mSpeed) {
        mVelocity.x = mDir.x * mSpeed; mVelocity.y = mDir.y * mSpeed; mVelocity.z = 0.0f;
    } else {
        SetVelocity2D(mDir.y * dist, mDir.x * dist);
    }

    if (!CheckShotHit() && dist < mSpeed) {
        mMoveCount = UfoRandRange(mParam.mPatterns[unkE8].mAbductTimeMax -
                                 mParam.mPatterns[unkE8].mAbductTimeMin);
        mMoveCount += mParam.mPatterns[unkE8].mAbductTimeMin;
        mHoldTimer = mParam.mPatterns[unkE8].mAbductFrames;
        mState = 8;
    }
}

void RPDucUfo::StateCircleBeforeAbduct() {
    u16 angle = mCircleAngle + mCircleStep;
    f32 x = mCircleRadius * nw4r::math::CosIdx(angle) + mCircleCenter.x;
    f32 y = mCircleRadius * nw4r::math::SinIdx(angle) + mCircleCenter.y;
    mVelocity.x = x - mPos.x;
    mVelocity.y = y - mPos.y;
    mCircleAngle = angle;
    mVelocity.z = 0.0f;
    if (--mMoveCount <= 0) {
        nw4r::math::VEC3 center;
        center.x = 2.0f * mCircleRadius * nw4r::math::CosIdx(angle) + mCircleCenter.x;
        center.y = 2.0f * mCircleRadius * nw4r::math::SinIdx(mCircleAngle) + mCircleCenter.y;
        center.z = mCircleCenter.z;
        if (RPDucInRange(center.x, mBoundLeft + mCircleRadius, mBoundRight - mCircleRadius) &&
            RPDucInRange(center.y, mBoundBottom + mCircleRadius, mBoundTop - mCircleRadius)) {
            mCircleCenter.x = center.x;
            mCircleCenter.y = center.y;
            mCircleCenter.z = center.z;
            mCircleAngle += 0x8000;
            mCircleStep = -mCircleStep;
        }
        mMoveCount = UfoRandRange(mParam.mPatterns[unkE8].mAbductTimeMax -
                                 mParam.mPatterns[unkE8].mAbductTimeMin);
        mMoveCount += mParam.mPatterns[unkE8].mAbductTimeMin;
    }

    if (!CheckShotHit()) {
        if (--mHoldTimer <= 0) {
            nw4r::math::VEC3 pos = mpTargetAvatar->mPos;
            mGoal = pos; mGoal.y = mParam.mAbductY; mGoal.z += 3.0f;
            PSVECNormalize(mVelocity, mDir);
            mSpeed = PSVECMag(mVelocity);
            mState = 9;
        }
    }
}

void RPDucUfo::StateApproachAvatar() {
    nw4r::math::VEC3 diff;
    bool reached = false;
    if (mSpeed < mParam.mPatterns[unkE8].mApproachSpeed) {
        mSpeed += 0.01f;
        if (mSpeed > mParam.mPatterns[unkE8].mApproachSpeed) {
            mSpeed = mParam.mPatterns[unkE8].mApproachSpeed;
        }
    } else if (mSpeed > mParam.mPatterns[unkE8].mApproachSpeed) {
        mSpeed -= 0.01f;
        if (mSpeed < mParam.mPatterns[unkE8].mApproachSpeed) {
            mSpeed = mParam.mPatterns[unkE8].mApproachSpeed;
        }
    }

    SetAbductGoal(mpTargetAvatar->mPos);
    diff.x = mGoal.x - mPos.x;
    diff.y = mGoal.y - mPos.y;
    diff.z = mGoal.z - mPos.z;
    if (PSVECMag(diff) < mSpeed) {
        reached = true;
    }
    f32 steer = mParam.mPatterns[unkE8].mSteer;
    PSVECNormalize(diff, diff);
    mDir.x = steer * diff.x + mDir.x;
    mDir.y = steer * diff.y + mDir.y;
    mDir.z = steer * diff.z + mDir.z;
    PSVECNormalize(mDir, mDir);
    mVelocity.x = mDir.x * mSpeed;
    mVelocity.y = mDir.y * mSpeed;
    mVelocity.z = mDir.z * mSpeed;

    if (!CheckShotHit()) {
        RPDucAvatar* pAvatar = mpTargetAvatar;
        if (pAvatar->IsGone()) {
            SetupFlee();
        } else if (reached) {
            nw4r::math::VEC3 pos = pAvatar->mPos;
            mCarryOffset = mPos.y - pos.y;
            mpTargetAvatar->SetCarry(pos, mpTargetAvatar->GetCarryValue());
            mpTargetAvatar->StartAbduct();
            mBeamScale = 1.0f;
            mpEffects[0]->create();
            mpEffects[1]->create();
            mpEffects[2]->create();
            if (RPDucGame::sInstance->GetMode() != 1) {
                mHoldTimer = mParam.mPatterns[unkE8].mLiftFrames[0];
            } else {
                mHoldTimer = mParam.mPatterns[unkE8].mLiftFrames[1];
            }
            mCarrying = true;
            mState = 10;
        }
    }
}

void RPDucUfo::StateLift() {
    f32 speed = mParam.mLiftHeight;
    if (RPDucGame::sInstance->GetMode() != 1) {
        speed /= mParam.mPatterns[unkE8].mLiftFrames[0];
    } else {
        speed /= mParam.mPatterns[unkE8].mLiftFrames[1];
    }
    mVelocity.x = 0.0f; mVelocity.y = speed; mVelocity.z = 0.0f;
    nw4r::math::VEC3 pos = mPos; pos.y = mPos.y - mCarryOffset; pos.z = mPos.z - 3.0f;
    mpTargetAvatar->SetCarry(pos, mpTargetAvatar->GetCarryValue());
    mBeamScale = 1.0f;
    if (!CheckShotHit()) {
        if (--mHoldTimer <= 0) {
            mHoldTimer = mParam.mPatterns[unkE8].mHoldFrames;
            mState = 11;
        }
    }
}

void RPDucUfo::StateHold() {
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    // Unused hold progress: only its int->float conversions survive.
    f32 t = (f32)mHoldTimer / mParam.mPatterns[unkE8].mHoldFrames;
    nw4r::math::VEC3 pos = mPos;
    pos.y -= mCarryOffset;
    pos.z -= 3.0f;
    mpTargetAvatar->SetCarry(pos, mpTargetAvatar->GetCarryValue());
    mBeamScale = 1.0f;
    if (!CheckShotHit()) {
        if (--mHoldTimer <= 0) {
            SetupFlee();
        }
    }
}

void RPDucUfo::StateFlyAway() {
    if (RPDucGame::sInstance->GetMode() != 1) {
        mVelocity.y += mParam.mPatterns[unkE8].mFlyAccel[0];
    } else {
        mVelocity.y += mParam.mPatterns[unkE8].mFlyAccel[1];
    }
    if (mCarrying) {
        nw4r::math::VEC3 pos = mPos;
        pos.y = pos.y - mCarryOffset;
        pos.z = pos.z - 3.0f;
        mpTargetAvatar->SetCarry(pos, mpTargetAvatar->GetCarryValue());
    }
    mVelocity.x *= 0.998f;
    mVelocity.y *= 0.998f;
    mVelocity.z *= 0.998f;

    f32 dist = GetDistanceToCamera();
    if (dist > mParam.mNearDist) {
        f32 n = dist - mParam.mNearDist; mHitScale = mParam.mPatterns[unkE8].mHitScale * (1.0f + n / (mParam.mFarDist - mParam.mNearDist));
    }

    if (!CheckShotHit()) {
        nw4r::math::VEC2 screenPos;
        RPDucCamera::sInstance->ProjectToScreen(&screenPos, mPos);
        if (screenPos.y > 1.3f) {
            if (mCarrying) {
                mpEffects[0]->forceKill();
                mpEffects[1]->forceKill();
                mpEffects[2]->forceKill();
                mpTargetAvatar->Kill();
                mLost = true;
            }
            Remove();
        }
    }
}

void RPDucUfo::Create() {
    RPDucTarget::Create();
    mpEffects[0] = new RPSysEffect("Sht_ufo_abduct", 0, 0);
    mpEffects[1] = new RPSysEffect("Sht_ufo_ptc", 0, 0);
    mpEffects[2] = new RPSysEffect("Sht_ufo_glow", 0, 0);
}

void RPDucUfo::Load() {
    LoadParams();
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_ufo_a.brres"));
    mpModel = RPGrpModel::Construct(
        RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResMdl, file, "sht_ufo_a"), 1, 0, 0x204);
    RPGrpHandle anm = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "sht_ufo_aR");
    CreateAnmBuffer(RPGrpModelAnm::Anm_Chr, 1);
    RegistAnm(RPGrpModelAnm::Anm_Chr, 0, anm);
    anm = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmTexSrt, file, "sht_ufo_aR");
    CreateAnmBuffer(RPGrpModelAnm::Anm_TexSrt, 1);
    RegistAnm(RPGrpModelAnm::Anm_TexSrt, 0, anm);
    mpModel->SetDrawGroup(0);
    mpModel->SetDrawScene(0);
    mpModel->SetUnkCFlags(0x12);
    mpModel->SetCallback(this);

    file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_ufo_b.brres"));
    mpModelB = RPGrpModel::Construct(
        RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResMdl, file, "sht_ufo_b"), 1, 0, 0x204);
    anm = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "sht_ufo_bR");
    mpModelB->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
    mpModelB->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 0, anm);
    anm = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmTexSrt, file, "sht_ufo_bR");
    mpModelB->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_TexSrt, 1);
    mpModelB->GetModelAnm()->Regist(RPGrpModelAnm::Anm_TexSrt, 0, anm);
    mpModelB->SetDrawGroup(0);
    mpModelB->SetDrawScene(0);
    mpModelB->SetUnkCFlags(0x12);
    mpModelB->SetCallback(this);
}

void RPDucUfo::ApplyVelocity() {
    mPos.x += mVelocity.x;
    mPos.y += mVelocity.y;
    mPos.z += mVelocity.z;
}

void RPDucUfo::Calculate() {
    (this->*mStateFuncs[mState])();
    mpSndActor->set3DPosition(mPos);
}

void RPDucUfo::CalcModelMtx() {
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, mRot, mScale, mPos);
    RPGrpModel* pModel = GetCurrentModel();
    nw4r::g3d::ScnObj* pScnObj = pModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    pModel->UpdateFrame();
    pModel->Calc();
    pModel->Entry();
}

void RPDucUfo::EditWorldMtx(RPGrpModel* pModel) {
    RPGrpModel* pCurrent = GetCurrentModel();
    if (pCurrent == pModel) {
        const nw4r::math::MTX34* pMtx =
            pCurrent->ReferWorldMtx(pCurrent->GetJointIndex("ufo_body"));
        nw4r::math::VEC3 pos;
        pos.x = pMtx->_03; mBeamPos.x = pos.x;
        pos.y = pMtx->_13; mBeamPos.y = pos.y;
        pos.z = mBeamPos.z = pMtx->_23;
        UpdateBeamEffects();
    }
}

void RPDucUfo::VF_0x9C(RPDucAvatar* pAvatar) {
    mpTargetAvatar = pAvatar;
    if (mMode == 3) {
        mMode = 3;
        mState = 7;
    }
}

// Emit the function literal pool before the trailing named constant.
#pragma defer_codegen off
const s32 RPDucUfo::sPatternNum = 8;
