// Shooting Range balloon (RPDucBalloon): rises and sways on a string, scores double for the
// same colour twice in a row.
// Unit 0x80237BF0-0x80238D48 (the range ends before RPDucClayMgr::SpawnTarget, the last function
// of the clay manager file that follows). Functions in reverse address order.
// The weak RPDucTarget::AsAvatar/CanShotPassThrough copies the binary keeps come from this file.
#include <Pack/RPParty/RPDucScene/RPDucBalloon.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPDucScene/RPDucCamera.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPParty/RPPartyCommon/RPCurve.h>
#include <Pack/RPSystem.h>

#include <egg/core.h>

inline u32 BalloonRandU32() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    return RP_randSeedX ^ RP_randSeedY;
}

inline f32 BalloonRandRange(f32 lo, f32 hi) {
    if (lo == hi) {
        return lo;
    }
    f32 r = 1.192093e-7f * (f32)(BalloonRandU32() & 0x7FFFFF);
    return __fmadds(hi - lo, r, lo);
}

RPDucBalloon::RPDucBalloon() {
    mRiseDir = RP_vecEy;
    mRise = 0.0f;
    mSwayIdx = 0;
    mSwayAngle = 0.0f;
    mStateFuncs[0] = &RPDucBalloon::StateCheckHit;
    mStateFuncs[1] = &RPDucBalloon::StateFloat;
}

RPDucBalloon::~RPDucBalloon() {}

void RPDucBalloon::ResetParams() {
    f32 radius = mParam.mPatterns[0].mHitRadius;
    mHitRadius = radius;
    mHitScale = mParam.mPatterns[0].mHitScale;
    f32 scale = radius / mParam.mBaseRadius;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    mRot = RP_mtx33Identity;
    mSwayIdx = BalloonRandU32() & 0xFFFF;
    unkA4 = false;
    mColor = 0;
    mState = 1;
}

// The string top starts as a copy of mPos (z, y, x) and gets the offset added; the copy
// order, the stores in between and `down` declared before `top` decide registers and slots.
void RPDucBalloon::Launch(u32 pattern) {
    unkE8 = pattern & 7;
    f32 radius = mParam.mPatterns[unkE8].mHitRadius;
    f32 scale = radius / mParam.mBaseRadius;
    nw4r::math::VEC3 down;
    nw4r::math::VEC3 top;
    top.z = mPos.z;
    top.y = mPos.y;
    top.x = mPos.x;
    mHitRadius = radius;
    mHitScale = mParam.mPatterns[unkE8].mHitScale;
    down.x = 0.0f;
    down.y = -1.0f;
    down.z = 0.0f;
    top.x += scale * (mRot.y.x * mParam.mStringLength);
    top.y += scale * (mRot.y.y * mParam.mStringLength);
    top.z += scale * (mRot.y.z * mParam.mStringLength);
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    RPCurveLine line = {top, down};

    f32 segs[5];
    s32 i = 0;
    do {
        segs[i] = mScale.x * mParam.mStringSegs[i];
    } while (++i < 5);
    RPCurveSegLengths segLengths = {segs};
    mpStringCurve->SamplePoints(line, &segLengths);

    mRiseDir = RP_vecEy;
    if (mPos.x > 0.0f) {
        mRiseDir.x = BalloonRandRange(0.0f, mParam.mPatterns[unkE8].mDriftX);
    } else {
        mRiseDir.x = BalloonRandRange(-mParam.mPatterns[unkE8].mDriftX, 0.0f);
    }
    mRiseDir.y = nw4r::math::FSqrt(1.0f - mRiseDir.x * mRiseDir.x);
    mRise = mParam.mPatterns[unkE8].mRise;
    mColor = (pattern >> 4) & 3;
    if (pattern & 0x80000000) {
        mState = 0;
    }
}

// STAND-IN: dead-stripped; puts 1.0f (lbl_8043FD58) right after the u32->f32 double and ahead
// of Launch's -1.0f and FLT_EPSILON (8043FD5C/60). Without it 1.0f lands after FLT_EPSILON.
extern "C" f32 Duc_balloonPoolOrderOne() {
    return 1.0f;
}

// STAND-IN: dead-stripped; puts the u32->f32 magic double (lbl_8043FD50) after 1.2f and ahead of
// 1.0f, -1.0f, FLT_EPSILON. Without it the double lands after FLT_EPSILON, before 127.0f.
extern "C" f32 Duc_balloonPoolOrderU32(u32 x) {
    return (f32)x;
}

bool RPDucBalloon::CheckShotHit() {
    s32 player = GetHittingPlayer();
    if (player != -1) {
        SpawnHitEffect();
        PlayHitSound(player);
        EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
            0x0FFFFFFF, 8, 0x20);

        s32 points;
        bool same = false;
        s32 idx = player;
        if (RPDucGame::sInstance->GetMode() == 2) {
            idx = 0;
        }
        if (mColor == RPDucScoreMgr::sInstance->mLastColors[idx]) {
            same = true;
            points = mParam.mPoints * 2;
        } else {
            points = mParam.mPoints;
        }
        RPDucScoreMgr::sInstance->AddScore(player, points);
        RPDucScoreMgr::sInstance->AddHits(player, 1);
        RPDucScoreMgr::sInstance->AddStreak(player, 1);
        ShowScorePopup(points, player, same);
        RPDucScoreMgr::sInstance->SetLastColor(player, mColor);
        Remove();
        return true;
    }
    return false;
}

void RPDucBalloon::SpawnHitEffect() {
    RPSysEffect effect("Sht_balloon_hit", 0, 0);
    effect.create();
    effect.setPos(nw4r::math::VEC3(mPos.x, mPos.y, mPos.z));
    effect.setScale(nw4r::math::VEC3(mScale.x, mScale.y, mScale.z));
    effect.update();
}

void RPDucBalloon::PlaySpawnSound() {
    mpSndActor->startSound(27U);
}

void RPDucBalloon::PlayHitSound(s32 player) {
    u32 flag = 0;
    if (player == 0) {
        flag = 1;
    } else if (player == 1) {
        flag = 2;
    }
    mpSndActor->startSoundWithRemotePlayer(11U, flag, 0);
}

void RPDucBalloon::StateCheckHit() {
    CheckShotHit();
}

// The anchor locals are declared in register order (oz f1 .. pz f7) and assigned in load order.
void RPDucBalloon::StateFloat() {
    f32 vz, vy, vx, rise;
    f32 dx = mRiseDir.x;
    rise = mRise;
    vx = dx * rise + mVelocity.x;
    vy = mRiseDir.y * rise + mVelocity.y;
    vz = mRiseDir.z * rise + mVelocity.z;
    mVelocity.x = vx * mParam.mDrag;
    mVelocity.y = vy * mParam.mDrag;
    mVelocity.z = vz * mParam.mDrag;

    u32 idx = mSwayIdx + mParam.mSwayStep;
    mSwayIdx = idx & 0xFFFF;
    mSwayAngle = mParam.mSwayAmp * nw4r::math::CosIdx(idx);
    mRot.x.x = nw4r::math::CosDeg(mSwayAngle);
    mRot.x.y = nw4r::math::SinDeg(mSwayAngle);
    mRot.x.z = 0.0f;
    PSVECCrossProduct(mRot.z, mRot.x, mRot.y);

    f32 oz, oy, rx, ox, len, s, px, py, pz;
    rx = mRot.y.x;
    len = mParam.mStringLength;
    ox = rx * len;
    oy = mRot.y.y * len;
    oz = mRot.y.z * len;
    pz = mPos.z;
    s = mScale.x;
    py = mPos.y;
    px = mPos.x;
    nw4r::math::VEC3 top(s * ox + px, s * oy + py, s * oz + pz);
    nw4r::math::VEC3 zero(0.0f, 0.0f, 0.0f);
    RPCurveAnchor anchor = {top, zero, 0};
    mpStringCurve->Build(anchor);

    if (!unkA4 && mPos.y > 0.0f) {
        unkA4 = true;
    }

    if (!CheckShotHit()) {
        nw4r::math::VEC3 pos = mPos;
        pos.y = mPos.y + mParam.mStringLength - mParam.unk14;
        nw4r::math::VEC2 screenPos;
        RPDucCamera::sInstance->ProjectToScreen(&screenPos, pos);
        if (screenPos.y > 1.2f) {
            Remove();
        }
    }
}

void RPDucBalloon::Create() {
    RPDucTarget::Create();
}

void RPDucBalloon::Load() {
    const char* names[3] = {"sht_b_red", "sht_b_blue", "sht_b_yellow"};
    s32 i;
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    u32 size = 0;
    mParam = *static_cast<RPDucBalloonParam*>(
        RPSysResourceManager::GetFileFromArchive(pArchive, "param/DucBalloonParam.bin", &size));

    pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_balloon.brres"));
    for (i = 0; i < 3; i++) {
        mpColorModels[i] = RPGrpModel::Construct(
            RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResMdl, file, names[i]), 1, 0, 0);
        mpColorModels[i]->SetDrawGroup(0);
        mpColorModels[i]->SetDrawScene(0);
        mpColorModels[i]->SetUnkCFlags(0x12);
    }

    file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_balloon_himo.brres"));
    mpStringModel = RPGrpModel::Construct(
        RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResMdl, file, "sht_balloon_himo"), 1, 0,
        0);
    mpStringModel->SetDrawGroup(0);
    mpStringModel->SetDrawScene(0);
    mpStringModel->SetUnkCFlags(0x10);

    f32 segs[5];
    i = 0;
    do {
        segs[i] = mParam.mStringSegs[i];
    } while (++i < 5);
    RPCurveDesc desc = {6, mParam.unk10, mParam.unk18, mParam.unk0C, segs, NULL};
    mpStringCurve = new RPCurve(desc);
    mpStringModel->SetCallback(this);
}

void RPDucBalloon::ApplyVelocity() {
    mPos.x += mVelocity.x;
    mPos.y += mVelocity.y;
    mPos.z += mVelocity.z;
}

void RPDucBalloon::Calculate() {
    (this->*mStateFuncs[mState])();
    mpSndActor->set3DPosition(mPos);
}

void RPDucBalloon::CalcModelMtx() {
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, mRot, RP_vecOne, mPos);
    RPGrpModel* pModel = mpColorModels[mColor];
    nw4r::g3d::ScnObj* pScnObj = pModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    pModel->SetBaseScale(mScale);
    pModel->UpdateFrame();
    pModel->Calc();
    pModel->Entry();

    nw4r::math::VEC3 top = mPos;
    top.x = mScale.x * (mRot.y.x * mParam.mStringLength) + top.x;
    top.y = mScale.y * (mRot.y.y * mParam.mStringLength) + top.y;
    top.z = mScale.z * (mRot.y.z * mParam.mStringLength) + top.z;
    RPDucMakeMtx(&mtx, mRot, RP_vecOne, top);
    pScnObj = mpStringModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    mpStringModel->SetBaseScale(mScale);
    mpStringModel->Calc();
    mpStringModel->Entry();
}

void RPDucBalloon::EditWorldMtx(RPGrpModel* pModel) {
    u16 i = 0;
    do {
        nw4r::math::MTX34* pMtx = pModel->ReferWorldMtx(i);
        nw4r::math::VEC3 p0 = mpStringCurve->GetPoint(i);
        nw4r::math::VEC3 p1 = mpStringCurve->GetPoint(i + 1);
        f32 z, y, x;
        x = p0.x;
        y = p0.y;
        z = p0.z;
        f32 dx = p1.x - x;
        f32 dy = p1.y - y;
        f32 dz = p1.z - z;
        nw4r::math::VEC3 pos;
        pos.x = x;
        pos.y = y;
        pos.z = z;
        RPAxes axes = RP_mtx33Identity;
        axes.x.x = dx;
        axes.x.y = dy;
        axes.x.z = dz;
        PSVECNormalize(axes.x, axes.x);
        PSVECCrossProduct(axes.z, axes.x, axes.y);
        RPDucMakeMtx(pMtx, axes, mScale, pos);
    } while (++i < 5);
}
