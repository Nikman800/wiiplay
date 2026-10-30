// Shooting Range duck (RPDucDuck): jumps out of the grass, approaches and bounces off the screen
// edges, arcs across the screen or flies away; three colours with their own models.
// Unit 0x8023A010-0x8023D1D0 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucDuck.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPDucScene/RPDucCamera.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPSystem.h>

#include <egg/core.h>
#include <egg/gfxe.h>

u32 RPDucDuck::sApproachSide;

extern const f32 lbl_8043FE04;

static inline f32 DuckHitScale() {
    return 1.0f;
}

static inline u32 DuckRandU32() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    return RP_randSeedX ^ RP_randSeedY;
}

static inline f32 DuckRandRange(f32 lo, f32 hi) {
    if (lo == hi) {
        return lo;
    }
    f32 r = 1.192093e-7f * (f32)(DuckRandU32() & 0x7FFFFF);
    return __fmadds(hi - lo, r, lo);
}

static inline u32 DuckRemoteFlags() {
    if (RPDucGame::sInstance->GetMode() != 1 && !RPDucGame::sInstance->IsP2Joined()) {
        return 1;
    }
    return 3;
}

static inline void SpawnGrassHit(const nw4r::math::VEC3& rPos) {
    RPSysEffect effect("Sht_grass_hit", 0, 0);
    effect.create();
    effect.setPos(nw4r::math::VEC3(rPos.x, rPos.y, rPos.z));
    effect.setScale(nw4r::math::VEC3(1.0f, 1.0f, 1.0f));
    effect.update();
}

inline f32 RPDucClamp(f32 value, f32 lo, f32 hi) {
    f32 v = lo > value ? lo : value;
    return hi < v ? hi : v;
}

RPDucDuck::RPDucDuck() {
    mAimPos = RP_vecZero;
    mState = 0;
    mStateFuncs[0] = &RPDucDuck::StateFly;
    mStateFuncs[1] = &RPDucDuck::StateJumpOut;
    mStateFuncs[2] = &RPDucDuck::StateApproach;
    mStateFuncs[3] = &RPDucDuck::StateBounce;
    mStateFuncs[4] = &RPDucDuck::StateBounceExit;
    mStateFuncs[5] = &RPDucDuck::StateArc;
    mStateFuncs[6] = &RPDucDuck::StateFlyForward;
    mStateFuncs[7] = &RPDucDuck::StateDamaged;
    mStateFuncs[8] = &RPDucDuck::StateFallDead;
}

RPDucDuck::~RPDucDuck() {}

#pragma auto_inline off
void RPDucDuck::LoadParams() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    u32 size = 0;
    mParam = *static_cast<RPDucDuckParam*>(
        RPSysResourceManager::GetFileFromArchive(pArchive, "param/DucDuckParam.bin", &size));

    s32 i = 0;
    do {
        mParam.mPatterns[i].mBounceFrames *= 3;
    } while (++i < sPatternNum);
    mParam.mDamageFrames *= 3;

    if (EGG::Screen::GetTVMode() == 0) {
        i = 0;
        do {
            mParam.mPatterns[i].mJumpRangeX *= lbl_8043FE04;
        } while (++i < sPatternNum);
        mParam.mForwardRangeX *= lbl_8043FE04;
    }
}
#pragma auto_inline reset

void RPDucDuck::ResetParams() {
    f32 radius = mParam.mPatterns[0].mHitRadius;
    mHitRadius = radius;
    mHitScale = mParam.mPatterns[0].mHitScale;
    f32 scale = radius / mParam.mBaseRadius;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    mRot = RP_mtx33Identity;
    mAimPos = RP_vecZero;
    mBounceTimer = 0;
    mDamageTimer = 0;
    mAnmPaused = 0;
    mColor = 0;
    mSplashed = 0;
    mpModel->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexPat, 0);
    mpModel->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexPat, 1);
    mpModel->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexPat, 2);
    mpModelB->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexPat, 0);
    mpModelB->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexPat, 1);
    mpModelB->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexPat, 2);
    mpModelC->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexPat, 0);
    mpModelC->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexPat, 1);
    mpModelC->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexPat, 2);
    mState = 0;
}

void RPDucDuck::Launch(u32 pattern) {
    f32 angle;
    f32 speed;
    f32 rate = 1.0f;
    unkE8 = pattern & 7;
    mColor = (pattern >> 4) & 3;
    if (mColor == 1) {
        rate = 1.25f;
    }
    if (mColor == 2) {
        rate = 1.5f;
    }
    StartAnm(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
    SetAnmUpdateRate(RPGrpModelAnm::Anm_Chr, 0, rate);
    CancelAnm(RPGrpModelAnm::Anm_TexPat, 1);
    CancelAnm(RPGrpModelAnm::Anm_TexPat, 2);
    StartAnm(RPGrpModelAnm::Anm_TexPat, 0, 0.0f);
    SetAnmUpdateRate(RPGrpModelAnm::Anm_TexPat, 0, rate);

    switch ((pattern >> 8) & 3) {
    case 0:
        SetupJumpOut();
        break;
    case 1:
        SetupApproach();
        break;
    case 2: {
        angle = mParam.mPatterns[unkE8].mArcAngle;
        speed = mParam.mPatterns[unkE8].mArcSpeed;
        mVelocity.x = speed * nw4r::math::CosDeg(angle);
        if (mPos.x > 0.0f) {
            mVelocity.x = -mVelocity.x;
        }
        mVelocity.y = speed * nw4r::math::SinDeg(angle);
        mVelocity.z = 0.0f;
        nw4r::math::VEC3 dir;
        dir.x = mVelocity.x;
        dir.y = 0.5f * mVelocity.y;
        dir.z = mVelocity.z;
        PSVECNormalize(dir, dir);
        PSVECNormalize(dir, mRot.z);
        PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
        PSVECNormalize(mRot.x, mRot.x);
        PSVECCrossProduct(mRot.z, mRot.x, mRot.y);
        unkA4 = true;
        mMotion = 2;
        mState = 5;
        break;
    }
    case 3:
        SetupFlyForward();
        break;
    }
}

void RPDucDuck::SetupJumpOut() {
    mAimPos.x = 0.0f;
    mAimPos.y = mParam.mPatterns[unkE8].mJumpAimY;
    mAimPos.z = mParam.mPatterns[unkE8].mJumpAimZ;
    mAimPos.x += DuckRandRange(-mParam.mPatterns[unkE8].mJumpRangeX,
                                mParam.mPatterns[unkE8].mJumpRangeX);
    mAimPos.y += DuckRandRange(-mParam.mPatterns[unkE8].mJumpRangeY,
                                mParam.mPatterns[unkE8].mJumpRangeY);

    nw4r::math::VEC3 dir;
    dir.x = mAimPos.x - mPos.x;
    dir.y = mAimPos.y - mPos.y;
    dir.z = mAimPos.z - mPos.z;
    PSVECNormalize(dir, dir);
    mRot = RP_mtx33Identity;
    mRot.z.x = dir.x;
    mRot.z.y = dir.y;
    mRot.z.z = dir.z;
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    PSVECNormalize(mRot.x, mRot.x);
    PSVECCrossProduct(mRot.z, mRot.x, mRot.y);

    f32 speed = mParam.mPatterns[unkE8].mJumpSpeed;
    if (mColor == 0) {
        mVelocity.x = dir.x * speed;
        mVelocity.y = dir.y * speed;
        mVelocity.z = dir.z * speed;
    } else if (mColor == 1) {
        mVelocity.x = 1.5f * (dir.x * speed);
        mVelocity.y = 1.5f * (dir.y * speed);
        mVelocity.z = 1.5f * (dir.z * speed);
    } else if (mColor == 2) {
        mVelocity.x = 2.0f * (dir.x * speed);
        mVelocity.y = 2.0f * (dir.y * speed);
        mVelocity.z = 2.0f * (dir.z * speed);
    }
    unkA4 = false;
    mMotion = 0;
    mState = 1;
}

void RPDucDuck::SetupApproach() {
    RPDucCamera* pCamera = RPDucCamera::sInstance;
    mBoundLeft = pCamera->GetScreenLeftAtDepth(mPos.z);
    mBoundRight = pCamera->GetScreenRightAtDepth(mPos.z);
    mBoundTop = pCamera->GetScreenTopAtDepth(mPos.z);
    mBoundBottom = 5.0f;

    f32 offset = DuckRandRange(mParam.mApproachRangeMin, mParam.mApproachRangeMax);
    f32 angle;
    if (sApproachSide == 1) {
        sApproachSide = 0;
        angle = 90.0f + offset;
    } else {
        sApproachSide = 1;
        angle = 90.0f - offset;
    }

    f32 radius = mParam.mApproachRadius;
    do {
        mAimPos.x = radius * nw4r::math::CosDeg(angle) + mPos.x;
        mAimPos.y = radius * nw4r::math::SinDeg(angle) + mPos.y;
        mAimPos.z = mPos.z;
        if (RPDucInRange(mAimPos.x, mBoundLeft + mHitRadius, mBoundRight - mHitRadius) &&
            RPDucInRange(mAimPos.y, mBoundBottom + mHitRadius, mBoundTop - mHitRadius)) {
            goto found;
        }
        radius -= 2.0f;
    } while (!(radius < 10.0f));

    mAimPos.x = mParam.mApproachRadius * nw4r::math::CosDeg(angle) + mPos.x;
    mAimPos.y = mParam.mApproachRadius * nw4r::math::SinDeg(angle) + mPos.y;
    mAimPos.z = mPos.z;
    RPDucClamp(mAimPos.x, mBoundLeft + mHitRadius, mBoundRight - mHitRadius);
    RPDucClamp(mAimPos.y, mBoundBottom + mHitRadius, mBoundTop - mHitRadius);

found:
    nw4r::math::VEC3 dir;
    dir.x = mAimPos.x - mPos.x;
    dir.y = mAimPos.y - mPos.y;
    dir.z = mAimPos.z - mPos.z;
    PSVECNormalize(dir, mDir);
    f32 speed = mParam.mPatterns[unkE8].mApproachSpeed;
    mSpeed = speed;
    mVelocity.x = mDir.x * speed;
    mVelocity.y = mDir.y * speed;
    mVelocity.z = 0.0f;
    mRot = RP_mtx33Identity;
    UpdateOrientation();
    unkA4 = false;
    mMotion = 1;
    mState = 2;
}

void RPDucDuck::SetupFlyForward() {
    if (mPos.x > 20.0f) {
        mAimPos.x = mPos.x;
        mAimPos.x += DuckRandRange(mParam.mForwardXMin, mParam.mForwardRangeX);
    } else if (mPos.x < -20.0f) {
        mAimPos.x = mPos.x;
        mAimPos.x += DuckRandRange(-mParam.mForwardRangeX, -mParam.mForwardXMin);
    } else {
        mAimPos.x = mPos.x;
        mAimPos.x += DuckRandRange(-mParam.mForwardRangeX, mParam.mForwardRangeX);
    }
    mAimPos.y = mParam.mForwardY;
    mAimPos.z = mPos.z;
    mAimPos.z -= DuckRandRange(mParam.mForwardZMin, mParam.mForwardZMax);

    nw4r::math::VEC3 dir;
    dir.x = mAimPos.x - mPos.x;
    dir.y = mAimPos.y - mPos.y;
    dir.z = mAimPos.z - mPos.z;
    PSVECNormalize(dir, mDir);
    f32 speed = mParam.mForwardSpeed;
    mSpeed = speed;
    mVelocity.x = mDir.x * speed;
    mVelocity.y = mDir.y * speed;
    mVelocity.z = mDir.z * speed;
    mRot = RP_mtx33Identity;

    nw4r::math::VEC3 front;
    front.x = mVelocity.x;
    front.y = 0.0f;
    front.z = mVelocity.z;
    PSVECNormalize(front, front);
    front.y = 0.2f;
    PSVECNormalize(front, mRot.z);
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    PSVECNormalize(mRot.x, mRot.x);
    PSVECCrossProduct(mRot.z, mRot.x, mRot.y);
    unkA4 = false;

    u32 flag = DuckRemoteFlags();
    RPSndAudioMgr::instance()->startSystemSeWithRemote(34U, flag);
    mMotion = 3;
    mState = 6;
}

bool RPDucDuck::CheckShotHit() {
    s32 player = GetHittingPlayer();
    if (player != -1) {
        SpawnHitEffect();
        PlayHitSound(player);
        if (mColor == 0) {
            EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
                0x0FFFFFFF, 8, 0x20);
        } else {
            EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
                0xFFFFFFFF, 20, 0x20);
        }

        s32 points = 0;
        bool far = false;
        if (mMotion == 0) {
            nw4r::math::VEC2 screenPos;
            RPDucCamera::sInstance->ProjectToScreen(&screenPos, mPos);
            if (screenPos.y > mParam.mFarScreenY) {
                points = mParam.mFarPoints;
                far = true;
            } else {
                points = mParam.mPoints;
            }
            if (mColor == 1) {
                points *= 2;
            } else if (mColor == 2) {
                points *= 3;
            }
        } else if (mMotion == 1) {
            points = mParam.mApproachPoints;
        } else if (mMotion == 2) {
            points = mParam.mArcPoints;
            if (mColor == 1) {
                points = points * 3 / 2;
            } else if (mColor == 2) {
                points *= 2;
            }
        }

        RPDucScoreMgr::sInstance->AddScore(player, points);
        RPDucScoreMgr::sInstance->AddHits(player, 1);
        RPDucScoreMgr::sInstance->AddStreak(player, 1);
        ShowScorePopup(points, player, far);

        mVelocity.x = 0.0f;
        mVelocity.y = mParam.mDamageLift;
        mVelocity.z = 0.0f;
        StartAnm(RPGrpModelAnm::Anm_Chr, 1, 0.0f);
        SetAnmUpdateRate(RPGrpModelAnm::Anm_Chr, 1, 1.6f);
        CancelAnm(RPGrpModelAnm::Anm_TexPat, 0);
        StartAnm(RPGrpModelAnm::Anm_TexPat, 1, 0.0f);
        SetAnmUpdateRate(RPGrpModelAnm::Anm_TexPat, 1, 1.6f);
        mState = 7;
        mDamageTimer = mParam.mDamageFrames;
        unkA4 = false;
        return true;
    }
    return false;
}

void RPDucDuck::UpdateOrientation() {
    f32 rotX[4] = {15.0f, -7.0f, 15.0f, -7.0f};
    f32 rotY[4] = {-110.0f, -110.0f, 110.0f, 110.0f};
    f32 rotZ[4] = {-10.0f, -10.0f, 10.0f, 10.0f};
    nw4r::math::MTX34 mtx;
    if (mVelocity.x <= 0.0f) {
        if (mVelocity.y <= 0.0f) {
            f32 x = rotX[0];
            f32 factor = 256.0f / 360.0f;
            nw4r::math::MTX34RotXYZFIdx(&mtx, factor * x, factor * rotY[0], factor * rotZ[0]);
        } else {
            nw4r::math::MTX34RotXYZDeg(&mtx, rotX[1], rotY[1], rotZ[1]);
        }
    } else if (mVelocity.y <= 0.0f) {
        nw4r::math::MTX34RotXYZDeg(&mtx, rotX[2], rotY[2], rotZ[2]);
    } else {
        nw4r::math::MTX34RotXYZDeg(&mtx, rotX[3], rotY[3], rotZ[3]);
    }
    mRot.x.x = mtx._00;
    mRot.x.y = mtx._10;
    mRot.x.z = mtx._20;
    mRot.y.x = mtx._01;
    mRot.y.y = mtx._11;
    mRot.y.z = mtx._21;
    mRot.z.x = mtx._02;
    mRot.z.y = mtx._12;
    mRot.z.z = mtx._22;
}

void RPDucDuck::SpawnHitEffect() {
    nw4r::math::VEC3 pos(mPos.x, mPos.y, mPos.z);
    f32 size = DuckHitScale();
    nw4r::math::VEC3 scale(size, size, size);
    switch (mColor) {
    case 0: {
        RPSysEffect effect("Sht_duck_a_hit", 0, 0);
        effect.create();
        effect.setPos(pos);
        effect.setScale(scale);
        effect.update();
        break;
    }
    case 1: {
        RPSysEffect effect("Sht_duck_b_hit", 0, 0);
        effect.create();
        effect.setPos(pos);
        effect.setScale(scale);
        effect.update();
        break;
    }
    case 2: {
        RPSysEffect effect("Sht_duck_c_hit", 0, 0);
        effect.create();
        effect.setPos(pos);
        effect.setScale(scale);
        effect.update();
        break;
    }
    }
}

void RPDucDuck::PlaySpawnSound() {
    if (mMotion != 2) {
        mpSndActor->startSound(32U);
    }
}

void RPDucDuck::PlayHitSound(s32 player) {
    u32 flag = 0;
    if (player == 0) {
        flag = 1;
    } else if (player == 1) {
        flag = 2;
    }
    if (mColor == 0) {
        mpSndActor->startSoundWithRemotePlayer(19U, flag, 0);
    } else {
        mpSndActor->startSoundWithRemotePlayer(26U, flag, 0);
    }
}

void RPDucDuck::StateFly() {
    UpdateOrientation();
    CheckShotHit();
}

void RPDucDuck::StateJumpOut() {
    if (mColor == 0) {
        mVelocity.y += mParam.mPatterns[unkE8].mJumpLift;
    } else if (mColor == 1) {
        mVelocity.y = 2.0f * mParam.mPatterns[unkE8].mJumpLift + mVelocity.y;
    } else if (mColor == 2) {
        mVelocity.y = 3.0f * mParam.mPatterns[unkE8].mJumpLift + mVelocity.y;
    }
    mVelocity.x *= mParam.mJumpDrag;
    mVelocity.y *= mParam.mJumpDrag;
    mVelocity.z *= mParam.mJumpDrag;

    nw4r::math::VEC3 front;
    front.x = mVelocity.x;
    front.y = 0.0f;
    front.z = mVelocity.z;
    PSVECNormalize(front, front);
    front.y = 0.2f;
    PSVECNormalize(front, mRot.z);
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    PSVECNormalize(mRot.x, mRot.x);
    PSVECCrossProduct(mRot.z, mRot.x, mRot.y);

    if (!unkA4 && mPos.y > 0.0f) {
        unkA4 = true;
    }

    if (!CheckShotHit()) {
        nw4r::math::VEC2 screenPos;
        nw4r::math::VEC3 pos = mPos;
        pos.y = pos.y - mHitRadius;
        RPDucCamera::sInstance->ProjectToScreen(&screenPos, pos);
        if (screenPos.y > 1.1f) {
            Remove();
        }

        if (!mSplashed && mPos.y > 5.0f) {
            SpawnGrassHit(mPos);
            mSplashed = 1;
        }
    }
}

// The discarded helper keeps the unsigned conversion literal ahead of the jump-state constants.
f32 RPDucDuckUnsignedToFloat(u32 value) {
    return value;
}

void RPDucDuck::StateApproach() {
    nw4r::math::VEC3 diff;
    diff.x = mAimPos.x - mPos.x;
    diff.y = mAimPos.y - mPos.y;
    diff.z = 0.0f;
    f32 dist = PSVECMag(diff);
    UpdateOrientation();

    if (!unkA4 && mPos.y > 0.0f) {
        unkA4 = true;
    }

    if (!CheckShotHit()) {
        if (dist < mSpeed) {
            mBounceTimer = (s16)mParam.mPatterns[unkE8].mBounceFrames;
            mState = 3;
        }

        if (!mSplashed && mPos.y > 5.0f) {
            SpawnGrassHit(mPos);
            mSplashed = 1;
        }
    }
}

void RPDucDuck::StateBounce() {
    if (mPos.x <= mBoundLeft + mHitRadius) {
        mPos.x = mBoundLeft + mHitRadius;
        mVelocity.x = -mVelocity.x;
    } else if (mPos.x >= mBoundRight - mHitRadius) {
        mPos.x = mBoundRight - mHitRadius;
        mVelocity.x = -mVelocity.x;
    } else if (mPos.y <= mBoundBottom + mHitRadius) {
        mPos.y = mBoundBottom + mHitRadius;
        mVelocity.y = -mVelocity.y;
    } else if (mPos.y >= mBoundTop - mHitRadius) {
        mPos.y = mBoundTop - mHitRadius;
        mVelocity.y = -mVelocity.y;
    }
    UpdateOrientation();

    if (!CheckShotHit()) {
        if (--mBounceTimer <= 0 && mVelocity.y > 0.0f) {
            mState = 4;
        }
    }
}

void RPDucDuck::StateBounceExit() {
    if (mPos.x <= mBoundLeft + mHitRadius) {
        mPos.x = mBoundLeft + mHitRadius;
        mVelocity.x = -mVelocity.x;
    } else if (mPos.x >= mBoundRight - mHitRadius) {
        mPos.x = mBoundRight - mHitRadius;
        mVelocity.x = -mVelocity.x;
    } else if (mPos.y <= mBoundBottom + mHitRadius) {
        mPos.y = mBoundBottom + mHitRadius;
        mVelocity.y = -mVelocity.y;
    }
    UpdateOrientation();

    if (!CheckShotHit()) {
        nw4r::math::VEC2 screenPos;
        nw4r::math::VEC3 pos = mPos;
        pos.y = pos.y - mHitRadius;
        RPDucCamera::sInstance->ProjectToScreen(&screenPos, pos);
        if (screenPos.y > 1.1f) {
            Remove();
        }
    }
}

void RPDucDuck::StateArc() {
    mVelocity.y += mParam.mPatterns[unkE8].mArcGravity;
    nw4r::math::VEC3 dir;
    dir.x = mVelocity.x;
    dir.y = 0.5f * mVelocity.y;
    dir.z = mVelocity.z;
    PSVECNormalize(dir, dir);
    PSVECNormalize(dir, mRot.z);
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    PSVECNormalize(mRot.x, mRot.x);
    PSVECCrossProduct(mRot.z, mRot.x, mRot.y);
    unkA4 = true;

    if (!CheckShotHit()) {
        if (!mSplashed) {
            nw4r::math::VEC2 screenPos;
            nw4r::math::VEC3 pos = mPos;
            pos.y = pos.y - mHitRadius;
            RPDucCamera::sInstance->ProjectToScreen(&screenPos, pos);
            bool entered;
            if (mVelocity.x >= 0.0f && screenPos.x > -1.2f) {
                entered = true;
            } else if (mVelocity.x < 0.0f && screenPos.x < 1.2f) {
                entered = true;
            } else {
                entered = false;
            }
            if (entered) {
                mpSndActor->startSound(32U);
                mSplashed = 1;
            }
        }

        nw4r::math::VEC2 screenPos;
        nw4r::math::VEC3 pos = mPos;
        pos.y = pos.y - mHitRadius;
        RPDucCamera::sInstance->ProjectToScreen(&screenPos, pos);
        bool left;
        if (mVelocity.x >= 0.0f && screenPos.x > 1.2f) {
            left = true;
        } else if (mVelocity.x < 0.0f && screenPos.x < -1.2f) {
            left = true;
        } else {
            left = false;
        }
        if (left) {
            Remove();
        }
    }
}

void RPDucDuck::StateFlyForward() {
    mSpeed *= mParam.mForwardAccel;
    if (mSpeed <= mParam.mForwardSpeedMax) {
        mSpeed = mParam.mForwardSpeedMax;
    }
    mVelocity.x = mDir.x * mSpeed;
    mVelocity.y = mDir.y * mSpeed;
    mVelocity.z = mDir.z * mSpeed;
    f32 rate = 0.9f * (mSpeed / mParam.mForwardSpeedMax);
    SetAnmUpdateRate(RPGrpModelAnm::Anm_Chr, 0, rate);
    SetAnmUpdateRate(RPGrpModelAnm::Anm_TexPat, 0, rate);

    nw4r::math::VEC2 screenPos;
    nw4r::math::VEC3 pos = mPos;
    pos.y = pos.y - mHitRadius;
    RPDucCamera::sInstance->ProjectToScreen(&screenPos, pos);
    if (screenPos.y > 1.1f) {
        Remove();
    }

    if (!mSplashed && mPos.y > 5.0f) {
        SpawnGrassHit(mPos);
        mSplashed = 1;
    }
}

void RPDucDuck::StateDamaged() {
    if (30.0f <= GetAnmFrame(RPGrpModelAnm::Anm_Chr, 1)) {
        const RPDucDuckMotion& rMotion = mParam.mMotions[mMotion];
        mVelocity.y += rMotion.mGravity;
        mVelocity.x *= rMotion.mDrag;
        mVelocity.y *= rMotion.mDrag;
        mVelocity.z *= rMotion.mDrag;
    } else {
        mVelocity.x *= mParam.mDamageDrag;
        mVelocity.y *= mParam.mDamageDrag;
        mVelocity.z *= mParam.mDamageDrag;
    }

    if (mDamageTimer > 0 && 24.0f <= GetAnmFrame(RPGrpModelAnm::Anm_Chr, 1)) {
        if (--mDamageTimer == 0) {
            mAnmPaused = 0;
            SetAnmUpdateRate(RPGrpModelAnm::Anm_Chr, 1, 1.0f);
            SetAnmUpdateRate(RPGrpModelAnm::Anm_TexPat, 1, 1.0f);
        } else {
            mAnmPaused = 1;
            StartAnm(RPGrpModelAnm::Anm_Chr, 1, 24.0f);
            StartAnm(RPGrpModelAnm::Anm_TexPat, 1, 24.0f);
            if (mDamageTimer == mParam.mDamageFrames - 3) {
                mpFallEffect->create();
                nw4r::math::VEC3 scale;
                nw4r::math::VEC3 pos(mMouthPos.x, mMouthPos.y, mMouthPos.z);
                scale.x = 1.0f;
                scale.y = 1.0f;
                scale.z = 1.0f;
                mpFallEffect->setPos(pos);
                mpFallEffect->setScale(scale);
                mpFallEffect->update();
            }
        }
    }

    f32 frame = GetAnmFrame(RPGrpModelAnm::Anm_Chr, 1);
    if (GetAnmNumFrame(RPGrpModelAnm::Anm_Chr, 1) - 1.0f <= frame) {
        StartAnm(RPGrpModelAnm::Anm_Chr, 2, 0.0f);
        SetAnmUpdateRate(RPGrpModelAnm::Anm_Chr, 2, 1.0f);
        CancelAnm(RPGrpModelAnm::Anm_TexPat, 1);
        StartAnm(RPGrpModelAnm::Anm_TexPat, 2, 0.0f);
        SetAnmUpdateRate(RPGrpModelAnm::Anm_TexPat, 2, 1.0f);
        mState = 8;
    }
}

void RPDucDuck::StateFallDead() {
    const RPDucDuckMotion& rMotion = mParam.mMotions[mMotion];
    mVelocity.y += rMotion.mGravity;
    mVelocity.x *= rMotion.mDrag;
    mVelocity.y *= rMotion.mDrag;
    mVelocity.z *= rMotion.mDrag;
    mRot = RP_mtx33Identity;
    if (mPos.y - mHitRadius <= -10.0f) {
        mpFallEffect->fade();
        Remove();
    }
}

void RPDucDuck::CreateAnmBuffer(RPGrpModelAnm::Anm anm, u16 num) {
    mpModel->GetModelAnm()->CreateBuffer(anm, num);
    mpModelB->GetModelAnm()->CreateBuffer(anm, num);
    mpModelC->GetModelAnm()->CreateBuffer(anm, num);
}

void RPDucDuck::RegistAnm(RPGrpModelAnm::Anm anm, u16 idx, RPGrpHandle handle) {
    mpModel->GetModelAnm()->Regist(anm, idx, handle);
    mpModelB->GetModelAnm()->Regist(anm, idx, handle);
    mpModelC->GetModelAnm()->Regist(anm, idx, handle);
}

bool RPDucDuck::StartAnm(RPGrpModelAnm::Anm anm, u16 idx, f32 frame) {
    return GetColorModel()->GetModelAnm()->Start(anm, idx, frame);
}

bool RPDucDuck::CancelAnm(RPGrpModelAnm::Anm anm, u16 idx) {
    return GetColorModel()->GetModelAnm()->Cancel(anm, idx);
}

void RPDucDuck::SetAnmUpdateRate(RPGrpModelAnm::Anm anm, u16 idx, f32 rate) {
    GetColorModel()->GetModelAnm()->SetUpdateRate(anm, idx, rate);
}

f32 RPDucDuck::GetAnmFrame(RPGrpModelAnm::Anm anm, u16 idx) {
    return GetColorModel()->GetModelAnm()->GetFrame(anm, idx);
}

f32 RPDucDuck::GetAnmNumFrame(RPGrpModelAnm::Anm anm, u16 idx) {
    return GetColorModel()->GetModelAnm()->GetNumFrame(anm, idx);
}

#pragma auto_inline off
void RPDucDuck::UpdateMouthEffect() {
    RPGrpModel* pModel = GetColorModel();
    const nw4r::math::MTX34* pMtx = pModel->ReferWorldMtx(pModel->GetJointIndex("mouth_1"));
    mMouthPos.x = pMtx->_03;
    mMouthPos.y = pMtx->_13;
    mMouthPos.z = pMtx->_23;
    mpFallEffect->setPos(nw4r::math::VEC3(mMouthPos.x, mMouthPos.y, mMouthPos.z));
    mpFallEffect->update();
}
#pragma auto_inline reset

void RPDucDuck::Create() {
    RPDucTarget::Create();
    mMouthPos.x = mPos.x;
    mMouthPos.y = mPos.y;
    mMouthPos.z = mPos.z;
    mpFallEffect = new RPSysEffect("Sht_duck_fall", 0, 0);
}

void RPDucDuck::Load() {
    LoadParams();
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_duck.brres"));
    RPGrpHandle mdlA = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResMdl, file, "sht_duck_a");
    RPGrpHandle mdlB = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResMdl, file, "sht_duck_b");
    RPGrpHandle mdlC = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResMdl, file, "sht_duck_c");
    mpModel = RPGrpModel::Construct(mdlA, 1, 0, 3);
    mpModelB = RPGrpModel::Construct(mdlB, 1, 0, 3);
    mpModelC = RPGrpModel::Construct(mdlC, 1, 0, 3);

    RPGrpHandle fly = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "sht_duck_fly");
    RPGrpHandle damage =
        RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "sht_duck_damage");
    RPGrpHandle fall = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "sht_duck_fall");
    CreateAnmBuffer(RPGrpModelAnm::Anm_Chr, 3);
    RegistAnm(RPGrpModelAnm::Anm_Chr, 0, fly);
    RegistAnm(RPGrpModelAnm::Anm_Chr, 1, damage);
    RegistAnm(RPGrpModelAnm::Anm_Chr, 2, fall);

    fly = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmTexPat, file, "sht_duck_fly");
    damage = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmTexPat, file, "sht_duck_damage");
    fall = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmTexPat, file, "sht_duck_fall");
    CreateAnmBuffer(RPGrpModelAnm::Anm_TexPat, 3);
    RegistAnm(RPGrpModelAnm::Anm_TexPat, 0, fly);
    RegistAnm(RPGrpModelAnm::Anm_TexPat, 1, damage);
    RegistAnm(RPGrpModelAnm::Anm_TexPat, 2, fall);

    mpModel->SetDrawGroup(0);
    mpModel->SetDrawScene(0);
    mpModelB->SetDrawGroup(0);
    mpModelB->SetDrawScene(0);
    mpModelC->SetDrawGroup(0);
    mpModelC->SetDrawScene(0);
    mpModel->SetCallback(this);
    mpModelB->SetCallback(this);
    mpModelC->SetCallback(this);
}

void RPDucDuck::ApplyVelocity() {
    mPos.x += mVelocity.x;
    mPos.y += mVelocity.y;
    mPos.z += mVelocity.z;
}

void RPDucDuck::Calculate() {
    (this->*mStateFuncs[mState])();
    mpSndActor->set3DPosition(mPos);
}

void RPDucDuck::CalcModelMtx() {
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, mRot, mScale, mPos);
    RPGrpModel* pModel = GetColorModel();
    nw4r::g3d::ScnObj* pScnObj = pModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    pModel->Calc();
    pModel->Entry();
    if (mAnmPaused == 0) {
        pModel->UpdateFrame();
    }
}

void RPDucDuck::EditWorldMtx(RPGrpModel* pModel) {
    UpdateMouthEffect();
}

// Emit the function literal pool before the trailing named constants.
#pragma defer_codegen off
const s32 RPDucDuck::sPatternNum = 8;

extern const f32 lbl_8043FE04 = 0.75f;
extern const f32 lbl_8043FE08 = 1.0f;
extern const f32 lbl_8043FE0C = 2.0f;
extern const f32 lbl_8043FE10 = 0.5f;
extern const f32 lbl_8043FE14 = 0.0f;
