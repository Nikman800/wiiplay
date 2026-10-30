// Shooting Range tin can (RPDucCan): tossed up, knocked up again and spun by each hit; the
// model shows the dents; also the range's jingle handle.
// Unit 0x8024BA14-0x8024D30C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucCan.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPDucScene/RPDucCamera.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPSystem.h>

#include <egg/core.h>
#include <egg/gfxe.h>

extern "C" void RP_orthonormalizeBasis(RPAxes* pAxes, f32 arg);
extern "C" void RP_makeRotMtxFromTo(nw4r::math::MTX34* pOut,
                                   const nw4r::math::VEC3* pFrom,
                                   const nw4r::math::VEC3* pTo);

static nw4r::snd::SoundHandle sJingleHandle;

inline u32 CanRandomU32() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    return RP_randSeedX ^ RP_randSeedY;
}

inline f32 CanRandomF32() {
    return 1.192093e-7f * (f32)(CanRandomU32() & 0x7FFFFF);
}

inline f32 CanRandomRange(f32 lo, f32 hi) {
    if (lo == hi) {
        return lo;
    }
    return (hi - lo) * CanRandomF32() + lo;
}

inline void RPDucCan::AdjustPatterns() {
    mParam.mCooldown *= 3;
    s32 i = 0;
    do {
        mParam.mPatterns[i].mBounceFrames[0] *= 3;
        mParam.mPatterns[i].mBounceFrames[1] *= 3;
        mParam.mPatterns[i].mBounceFrames[2] *= 3;
        mParam.mPatterns[i].mBounceFrames[3] *= 3;
        if (EGG::Screen::GetTVMode() == 0) {
            mParam.mPatterns[i].mSpinMax *= 0.85f;
            mParam.mPatterns[i].mSpinMin *= 0.85f;
        }
    } while (++i < sPatternNum);
}

inline void RPDucCan::LoadCanModels(EGG::Archive* pArchive, RPGrpModelResManager* pResMgr,
                                   const char** names, const char* pFileName) {
    RPGrpHandle file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, pFileName));
    for (s32 j = 0; j < 5; j++) {
        mpModels[j] = RPGrpModel::Construct(
            RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResMdl, file, names[j]), 1, 0, 0);
        mpModels[j]->SetDrawGroup(0);
        mpModels[j]->SetDrawScene(0);
        mpModels[j]->SetUnk4(0);
    }
}

void RPDucCan::ApplyVelocity() {
    mPrevPos = mPos;
    mPos.x += mVelocity.x;
    mPos.y += mVelocity.y;
    mPos.z += mVelocity.z;
    mRot.x.x += mRot.y.x * mSpin[0] + mRot.z.x * mSpin[1];
    mRot.x.y += mRot.y.y * mSpin[0] + mRot.z.y * mSpin[1];
    mRot.x.z += mRot.y.z * mSpin[0] + mRot.z.z * mSpin[1];
    mRot.y.x += mRot.x.x * mSpin[2] + mRot.z.x * mSpin[3];
    mRot.y.y += mRot.x.y * mSpin[2] + mRot.z.y * mSpin[3];
    mRot.y.z += mRot.x.z * mSpin[2] + mRot.z.z * mSpin[3];
    mRot.z.x += mRot.x.x * mSpin[4] + mRot.y.x * mSpin[5];
    mRot.z.y += mRot.x.y * mSpin[4] + mRot.y.y * mSpin[5];
    mRot.z.z += mRot.x.z * mSpin[4] + mRot.y.z * mSpin[5];
    RP_orthonormalizeBasis(&mRot, 2.9999f);
}

void RPDucCan::Calculate() {
    (this->*mStateFuncs[mState])();
    mpSndActor->set3DPosition(mPos);
}

void RPDucCan::CalcModelMtx() {
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, mRot, mScale, mPos);
    s32 idx = mHitNum;
    if (idx >= 5) {
        idx = 4;
    }
    RPGrpModel* pModel = mpModels[idx];
    nw4r::g3d::ScnObj* pScnObj = pModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    pModel->Calc();
    pModel->Entry();
    pModel->UpdateFrame();
}

// Emit the first functions' literal pool before sPatternNum.
#pragma defer_codegen off
const s32 RPDucCan::sPatternNum = 8;
#pragma defer_codegen on

bool RPDucStopJingle() {
    sJingleHandle.Stop(0);
    return RPSndAudioMgr::instance()->getPlayer().StartSound(&sJingleHandle, 0U);
}

bool RPDucStartJingle() {
    if (sJingleHandle.IsAttachedSound()) {
        sJingleHandle.Stop(0);
        return true;
    }
    return false;
}

RPDucCan::RPDucCan() {
    mStateFuncs[0] = &RPDucCan::StateCheckHit;
    mStateFuncs[1] = &RPDucCan::StateToss;
    mStateFuncs[2] = &RPDucCan::StateKnocked;
    mHitNum = 0;
    mCooldown = 0;
    mBroken = false;
}

RPDucCan::~RPDucCan() {}

void RPDucCan::ResetParams() {
    f32 radius = mParam.mPatterns[0].mHitRadius;
    mHitRadius = radius;
    mHitScale = mParam.mPatterns[0].mHitScale;
    f32 scale = radius / mParam.mBaseRadius;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    mRot = RP_mtx33Identity;
    mPrevPos.x = mPos.x;
    mPrevPos.y = mPos.y;
    mPrevPos.z = mPos.z;
    mGravityUp = 0.0f;
    mGravityDown = 0.0f;
    mSpin[0] = 0.0f;
    mSpin[1] = 0.0f;
    mSpin[2] = 0.0f;
    mSpin[3] = 0.0f;
    mSpin[4] = 0.0f;
    mSpin[5] = 0.0f;
    mBounceVel = RP_vecZero;
    mSpinAdd[0] = 0.0f;
    mSpinAdd[1] = 0.0f;
    mSpinAdd[2] = 0.0f;
    mSpinAdd[3] = 0.0f;
    mSpinAdd[4] = 0.0f;
    mSpinAdd[5] = 0.0f;
    mHitNum = 0;
    mCooldown = 0;
    mBroken = false;
    mState = 0;
}

void RPDucCan::Launch(u32 pattern) {
    if (pattern & 0x80000000) {
        mState = 0;
        return;
    }
    unkE8 = pattern & 7;
    SetupLaunch();
}

void RPDucCan::SetupLaunch() {
    nw4r::math::VEC3 aim;
    aim.x = 0.0f;
    aim.y = mParam.mPatterns[unkE8].mAimY;
    aim.z = mParam.mPatterns[unkE8].mAimZ;
    aim.x += CanRandomRange(-mParam.mPatterns[unkE8].mAimRangeX, mParam.mPatterns[unkE8].mAimRangeX);
    aim.y += CanRandomRange(-mParam.mPatterns[unkE8].mAimRangeY, mParam.mPatterns[unkE8].mAimRangeY);
    mVelocity.x = aim.x - mPos.x;
    mVelocity.y = aim.y - mPos.y;
    mVelocity.z = aim.z - mPos.z;
    PSVECNormalize(mVelocity, mVelocity);
    mVelocity.x *= mParam.mPatterns[unkE8].mSpeed;
    mVelocity.y *= mParam.mPatterns[unkE8].mSpeed;
    mVelocity.z *= mParam.mPatterns[unkE8].mSpeed;

    mRot.y.x = nw4r::math::CosDeg(CanRandomRange(30.0f, 150.0f));
    mRot.y.y = nw4r::math::FSqrt(1.0f - mRot.y.x * mRot.y.x);
    mRot.y.z = 0.0f;
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    mState = 1;
}

void RPDucCan::ApplyHitSpin(const nw4r::math::MTX34* pRotMtx) {
    nw4r::math::VEC3 x;
    PSMTXMultVec(*pRotMtx, mRot.x, x);
    f32 inv = 1.0f / nw4r::math::VEC3Dot(&x, &mRot.x);
    x.x *= inv;
    x.y *= inv;
    x.z *= inv;
    mSpinAdd[0] = nw4r::math::VEC3Dot(&x, &mRot.y);
    mSpinAdd[1] = nw4r::math::VEC3Dot(&x, &mRot.z);

    nw4r::math::VEC3 y;
    PSMTXMultVec(*pRotMtx, mRot.y, y);
    inv = 1.0f / nw4r::math::VEC3Dot(&y, &mRot.y);
    y.x *= inv;
    y.y *= inv;
    y.z *= inv;
    mSpinAdd[2] = nw4r::math::VEC3Dot(&y, &mRot.x);
    mSpinAdd[3] = nw4r::math::VEC3Dot(&y, &mRot.z);

    nw4r::math::VEC3 z;
    PSMTXMultVec(*pRotMtx, mRot.z, z);
    inv = 1.0f / nw4r::math::VEC3Dot(&z, &mRot.z);
    z.x *= inv;
    z.y *= inv;
    z.z *= inv;
    mSpinAdd[4] = nw4r::math::VEC3Dot(&z, &mRot.x);
    mSpinAdd[5] = nw4r::math::VEC3Dot(&z, &mRot.y);
}

bool RPDucCan::CheckShotHit() {
    s32 player = GetHittingPlayer();
    if (player != -1) {
        if (++mHitNum == mParam.mPatterns[unkE8].mMaxHits) {
            mBroken = true;
        }
        PlayHitSound(player);
        EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
            0x0FFFFFFF, 8, 0x20);
        RPDucScoreMgr::sInstance->AddScore(player, mHitNum);
        RPDucScoreMgr::sInstance->AddHits(player, 1);
        RPDucScoreMgr::sInstance->AddStreak(player, 1);
        ShowScorePopup(mHitNum, player, mBroken);
        if (mBroken) {
            SpawnHitEffect();
            Remove();
        } else {
            nw4r::math::VEC3 hitPos = mHitPos[player];
            {
                RPSysEffect effect("Sht_can_hit", 0, 0);
                effect.create();
                effect.setPos(nw4r::math::VEC3(hitPos.x, hitPos.y, hitPos.z));
                f32 scale = mHitRadius / mParam.mBaseRadius;
                nw4r::math::VEC3 scaleVec(scale, scale, scale);
                effect.setScale(scaleVec);
                effect.update();
            }
            CalcHitBounce(player);
            mCooldown = mParam.mCooldown;
        }
        return true;
    }
    return false;
}

void RPDucCan::CalcHitBounce(s32 player) {
    RPDucCamera* pCamera = RPDucCamera::sInstance;
    nw4r::math::VEC3 top = mPos;
    top.x = top.x + mHitScale;
    nw4r::math::VEC2 topScreen;
    pCamera->ProjectToScreen(&topScreen, top);
    nw4r::math::VEC2 baseScreen;
    pCamera->ProjectToScreen(&baseScreen, mPrevPos);
    nw4r::math::VEC2 hitScreen;
    pCamera->ProjectToScreen(&hitScreen, mHitPos[player]);

    f32 spinMin;
    f32 spinMax;
    f32 t;
    t = (hitScreen.x - baseScreen.x) / (topScreen.x - baseScreen.x);
    spinMax = mParam.mPatterns[unkE8].mSpinMax;
    spinMin = mParam.mPatterns[unkE8].mSpinMin;
    f32 spinRange = spinMax - spinMin;
    if (t >= 0.0f) {
        mBounceVel.x = -spinRange * t - spinMin;
    } else {
        mBounceVel.x = -spinRange * t + spinMin;
    }
    if (baseScreen.x > mParam.mEdgeScreenY) {
        if (mBounceVel.x > 0.0f) {
            mBounceVel.x = -mBounceVel.x;
        }
    } else if (baseScreen.x < -mParam.mEdgeScreenY) {
        if (mBounceVel.x < 0.0f) {
            mBounceVel.x = -mBounceVel.x;
        }
    }

    f32 height;
    f32 frames;
    if (mHitNum == 1) {
        height = mParam.mPatterns[unkE8].mBounceHeight[0];
        frames = mParam.mPatterns[unkE8].mBounceFrames[0];
    } else if (mHitNum == 2) {
        height = mParam.mPatterns[unkE8].mBounceHeight[1];
        frames = mParam.mPatterns[unkE8].mBounceFrames[1];
    } else if (mHitNum == 3) {
        height = mParam.mPatterns[unkE8].mBounceHeight[2];
        frames = mParam.mPatterns[unkE8].mBounceFrames[2];
    } else {
        height = mParam.mPatterns[unkE8].mBounceHeight[3];
        frames = mParam.mPatterns[unkE8].mBounceFrames[3];
    }
    f32 gravity = 2.0f * height / (frames * frames);
    mGravityUp = gravity;
    mGravityDown = gravity;
    f32 ratio = mParam.mPatterns[unkE8].mGravityUp /
                mParam.mPatterns[unkE8].mGravityDown;
    mBounceVel.y = frames * gravity;
    mGravityDown = gravity * ratio;
    mBounceVel.z = -mParam.mPatterns[unkE8].mBounceVelZ;

    nw4r::math::VEC3 arm;
    arm.x = mPrevPos.x - mHitPos[player].x;
    arm.y = mPrevPos.y - mHitPos[player].y;
    arm.z = mPrevPos.z - mHitPos[player].z;
    nw4r::math::VEC3 camPos = pCamera->GetLookPos();
    nw4r::math::VEC3 dir;
    dir.x = mHitPos[player].x - camPos.x;
    dir.y = mHitPos[player].y - camPos.y;
    dir.z = mHitPos[player].z - camPos.z;
    PSVECNormalize(dir, dir);
    nw4r::math::VEC3 push;
    push.x = (0.2f * mHitNum + 1.0f) * (dir.x * mParam.mHitPush);
    push.y = (0.2f * mHitNum + 1.0f) * (dir.y * mParam.mHitPush);
    push.z = (0.2f * mHitNum + 1.0f) * (dir.z * mParam.mHitPush);
    PSVECNormalize(arm, arm);

    nw4r::math::VEC3 axis;
    axis.x = dir.x;
    axis.y = dir.y;
    axis.z = dir.z;
    PSVECCrossProduct(axis, arm, axis);
    PSVECNormalize(axis, axis);
    PSVECCrossProduct(arm, axis, axis);
    f32 len = PSVECMag(axis);
    if (0.0f != len) {
        f32 inv = 1.0f / len;
        axis.x *= inv;
        axis.y *= inv;
        axis.z *= inv;
    }
    f32 amount = nw4r::math::VEC3Dot(&push, &axis) / mHitScale;
    axis.x *= amount;
    axis.y *= amount;
    axis.z *= amount;
    nw4r::math::VEC3 from;
    from.x = -arm.x;
    from.y = -arm.y;
    from.z = -arm.z;
    nw4r::math::MTX34 rot;
    RP_makeRotMtxFromTo(&rot, &from, &axis);
    ApplyHitSpin(&rot);
}

void RPDucCan::SpawnHitEffect() {
    RPSysEffect effect("Sht_can_break", 0, 0);
    effect.create();
    effect.setPos(nw4r::math::VEC3(mPos.x, mPos.y, mPos.z));
    f32 scale = mHitRadius / mParam.mBaseRadius;
    effect.setScale(nw4r::math::VEC3(scale, scale, scale));
    effect.update();
}

void RPDucCan::PlaySpawnSound() {
    mpSndActor->startSound(33U);
}

void RPDucCan::PlayHitSound(s32 player) {
    u32 flag = 0;
    if (player == 0) {
        flag = 1;
    } else if (player == 1) {
        flag = 2;
    }
    if (mBroken) {
        mpSndActor->startSoundWithRemotePlayer(21U, flag, 0);
    } else {
        mpSndActor->startSoundWithRemotePlayer(20U, flag, 0);
    }
}

void RPDucCan::StateCheckHit() {
    CheckShotHit();
}

// Unreferenced (dead-stripped) helpers; compiling them here creates the unsigned int->float
// constant and 1.0f in the original's .sdata2 order (before CalcHitBounce's literals).
f32 RPDucCanPoolOne(f32 x) {
    return 1.0f / x;
}

f32 RPDucCanPoolU32ToF32(u32 x) {
    return x;
}

void RPDucCan::StateToss() {
    if (mVelocity.y > 0.0f) {
        mVelocity.y -= mParam.mPatterns[unkE8].mGravityDown;
    } else {
        mVelocity.y -= mParam.mPatterns[unkE8].mGravityUp;
    }
    mVelocity.x *= mParam.mDrag;
    mVelocity.y *= mParam.mDrag;
    mVelocity.z *= mParam.mDrag;
    if (mPos.z < 0.0f) {
        mVelocity.z *= 0.96f;
    }
    mSpin[0] *= mParam.mSpinDecay;
    mSpin[1] *= mParam.mSpinDecay;
    mSpin[2] *= mParam.mSpinDecay;
    mSpin[3] *= mParam.mSpinDecay;
    mSpin[4] *= mParam.mSpinDecay;
    mSpin[5] *= mParam.mSpinDecay;

    if (!CheckShotHit()) {
        if (mPos.y < 0.0f) {
            mpSndActor->startSound(1U);
            Remove();
            return;
        }
    } else {
        mVelocity.x = mBounceVel.x;
        mVelocity.y = mBounceVel.y;
        mSpin[0] += mSpinAdd[0];
        mSpin[1] += mSpinAdd[1];
        mSpin[2] += mSpinAdd[2];
        mSpin[3] += mSpinAdd[3];
        mSpin[4] += mSpinAdd[4];
        mSpin[5] += mSpinAdd[5];
        mBounceVel = RP_vecZero;
        mSpinAdd[0] = 0.0f;
        mSpinAdd[1] = 0.0f;
        mSpinAdd[2] = 0.0f;
        mSpinAdd[3] = 0.0f;
        mSpinAdd[4] = 0.0f;
        mSpinAdd[5] = 0.0f;
        mState = 2;
    }

    if (--mCooldown < 0) {
        mCooldown = 0;
        unkA4 = true;
    } else {
        unkA4 = false;
    }
}

void RPDucCan::StateKnocked() {
    if (mVelocity.y > 0.0f) {
        mVelocity.y -= mGravityUp;
    } else {
        mVelocity.y -= mGravityDown;
    }
    mVelocity.x *= mParam.mDrag;
    mVelocity.y *= mParam.mDrag;
    mVelocity.z *= mParam.mDrag;
    if (mPos.z < 0.0f) {
        mVelocity.z *= 0.96f;
    }
    mSpin[0] *= mParam.mSpinDecay;
    mSpin[1] *= mParam.mSpinDecay;
    mSpin[2] *= mParam.mSpinDecay;
    mSpin[3] *= mParam.mSpinDecay;
    mSpin[4] *= mParam.mSpinDecay;
    mSpin[5] *= mParam.mSpinDecay;

    if (!CheckShotHit()) {
        if (mPos.y < 0.0f) {
            mpSndActor->startSound(1U);
            Remove();
            return;
        }
    } else {
        mVelocity.x = mBounceVel.x;
        mVelocity.y = mBounceVel.y;
        mSpin[0] += mSpinAdd[0];
        mSpin[1] += mSpinAdd[1];
        mSpin[2] += mSpinAdd[2];
        mSpin[3] += mSpinAdd[3];
        mSpin[4] += mSpinAdd[4];
        mSpin[5] += mSpinAdd[5];
        mBounceVel = RP_vecZero;
        mSpinAdd[0] = 0.0f;
        mSpinAdd[1] = 0.0f;
        mSpinAdd[2] = 0.0f;
        mSpinAdd[3] = 0.0f;
        mSpinAdd[4] = 0.0f;
        mSpinAdd[5] = 0.0f;
    }

    if (--mCooldown < 0) {
        mCooldown = 0;
        unkA4 = true;
    } else {
        unkA4 = false;
    }
}

void RPDucCan::Load() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    u32 size = 0;
    mParam = *static_cast<RPDucCanParam*>(
        RPSysResourceManager::GetFileFromArchive(pArchive, "param/DucCanParam.bin", &size));
    AdjustPatterns();
    const char* names[5] = {"sht_can_00", "sht_can_01", "sht_can_02", "sht_can_03", "sht_can_04"};
    pArchive = RPSysResourceManager::LoadGameCommonArchive();
    LoadCanModels(pArchive, RPGrpModelResManager::GetCurrent(), names, "G3D/sht_can.brres");
}
