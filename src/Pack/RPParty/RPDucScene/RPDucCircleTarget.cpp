// Shooting Range round target (RPDucCircleTarget): turns in, stands, turns out; the Mii face
// targets topple when their own player hits them.
// Unit 0x802508A8-0x802520F8 (the range ends before RPDucAvatarMgr::SpawnTarget, the last
// function of the avatar manager file that follows). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucCircleTarget.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPMiiIconSet.h>
#include <Pack/RPSystem.h>

#include <egg/core.h>
#include <egg/gfxe.h>

// Separate entries let CodeWarrior pool the table while retaining each source address.
static RPDucCircleTarget::StateFunc sCheckHitFunc = &RPDucCircleTarget::StateCheckHit;
static RPDucCircleTarget::StateFunc sGrowInFunc = &RPDucCircleTarget::StateGrowIn;
static RPDucCircleTarget::StateFunc sStandFunc = &RPDucCircleTarget::StateStand;
static RPDucCircleTarget::StateFunc sShrinkOutFunc = &RPDucCircleTarget::StateShrinkOut;
static RPDucCircleTarget::StateFunc sToppleFunc = &RPDucCircleTarget::StateTopple;
static RPDucCircleTarget::StateFunc sFallFunc = &RPDucCircleTarget::StateFall;

/**
 * A Mii face target that belongs to the player: hitting it topples it and costs the streak
 * (one player: model c; versus: c for player 1, d for player 2)
 */
static inline s32 IsOwnFace(s32 type, s32 player) {
    if (RPDucGame::sInstance->GetMode() != 1) {
        if (type == 2) {
            return 1;
        }
        return 0;
    }
    if ((player == 0 && type == 2) || (player == 1 && type == 3)) {
        return 1;
    }
    return 0;
}

RPDucCircleTarget::RPDucCircleTarget()
    : mRadius(0.0f), mScaleBase(0.0f), mTimer(0), mGrowFrames(0), mStandFrames(0),
      mGridSlot(0) {
    mStateFuncs[0] = sCheckHitFunc;
    mStateFuncs[1] = sGrowInFunc;
    mStateFuncs[2] = sStandFunc;
    mStateFuncs[3] = sShrinkOutFunc;
    mStateFuncs[4] = sToppleFunc;
    mStateFuncs[5] = sFallFunc;
}

RPDucCircleTarget::~RPDucCircleTarget() {}

#pragma auto_inline off
void RPDucCircleTarget::LoadParams() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    u32 size = 0;
    const RPDucCircleTargetParam* param = static_cast<RPDucCircleTargetParam*>(
        RPSysResourceManager::GetFileFromArchive(pArchive, "param/DucCircleMatoParam.bin", &size));
    mParam.mToppleMax = param->mToppleMax;
    mParam.mToppleStep = param->mToppleStep;
    mParam.mFallDrag = param->mFallDrag;
    mParam.mFallGravity = param->mFallGravity;
    mParam.unk10 = param->unk10;
    mParam.mPoints[0] = param->mPoints[0];
    mParam.mPoints[1] = param->mPoints[1];
    mParam.mPoints[2] = param->mPoints[2];
    mParam.mPoints[3] = param->mPoints[3];
    mParam.mPoints[4] = param->mPoints[4];
    mParam.mPoints[5] = param->mPoints[5];
    mParam.mPoints[6] = param->mPoints[6];
    mParam.mPoints[7] = param->mPoints[7];
    mParam.mBaseRadius = param->mBaseRadius;
    mParam.mLayout = param->mLayout;
    s32 i = 0;
    do {
        mParam.mLayout.mPatterns[i].mGrowFrames *= 3;
        mParam.mLayout.mPatterns[i].mStandFrames[0] *= 3;
        mParam.mLayout.mPatterns[i].mStandFrames[1] *= 3;
    } while (++i < sPatternNum);
}
#pragma auto_inline reset

void RPDucCircleTarget::ResetParams() {
    if (EGG::Screen::GetTVMode() == 1) {
        mHitRadius = mParam.mLayout.mPatterns[0].mRadius[0];
        mHitScale = mParam.mLayout.mPatterns[0].mHitScale[0];
    } else {
        mHitRadius = mParam.mLayout.mPatterns[0].mRadius[1];
        mHitScale = mParam.mLayout.mPatterns[0].mHitScale[1];
    }
    mPos.z = mParam.mLayout.mGrid[0][2];
    f32 scale = mHitRadius / mParam.mBaseRadius;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    mRot = RP_mtx33Identity;
    mRot.z.x = nw4r::math::CosDeg(180.0f);
    mRot.z.y = 0.0f;
    mRot.z.z = nw4r::math::SinDeg(180.0f);
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    mTimer = 0;
    mGridSlot = 0;
    unkE8 = 0;
    mpModels[1]->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexSrt, 0);
    mState = 1;
}

void RPDucCircleTarget::Launch(u32 pattern) {
    unkE8 = pattern & 7;
    if (EGG::Screen::GetTVMode() == 1) {
        mRadius = mParam.mLayout.mPatterns[unkE8].mRadius[0];
        mScaleBase = mParam.mLayout.mPatterns[unkE8].mHitScale[0];
    } else {
        mRadius = mParam.mLayout.mPatterns[unkE8].mRadius[1];
        mScaleBase = mParam.mLayout.mPatterns[unkE8].mHitScale[1];
    }
    mGrowFrames = mParam.mLayout.mPatterns[unkE8].mGrowFrames;
    if (RPDucGame::sInstance->GetMode() != 1) {
        mStandFrames = mParam.mLayout.mPatterns[unkE8].mStandFrames[0];
    } else {
        mStandFrames = mParam.mLayout.mPatterns[unkE8].mStandFrames[1];
    }
    mGridSlot = (pattern >> 4) & 0xFF;
    f32 t = (f32)(mTimer + 1) / (f32)(mGrowFrames + 1);
    mHitRadius = mRadius * t;
    mHitScale = mScaleBase * t;
    f32 scale = mHitRadius / mParam.mBaseRadius;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    RPDucCircleTargetMgr::sInstance->SetGridSlotUsed(mGridSlot, true);

    u32 col = (u32)mGridSlot % 6;
    u32 row = (u32)mGridSlot / 6;
    if (EGG::Screen::GetTVMode() == 1) {
        mPos.x = mParam.mLayout.mGrid[0][0] * (s32)col + mParam.mLayout.mGrid[0][3];
        mPos.y = mParam.mLayout.mGrid[0][1] * (s32)row + mParam.mLayout.mGrid[0][4];
        mPos.z = mParam.mLayout.mGrid[0][2];
    } else {
        mPos.x = mParam.mLayout.mGrid[1][0] * (s32)col + mParam.mLayout.mGrid[1][3];
        mPos.y = mParam.mLayout.mGrid[1][1] * (s32)row + mParam.mLayout.mGrid[1][4];
        mPos.z = mParam.mLayout.mGrid[1][2];
    }
    mBase.x = mPos.x;
    mBase.z = mPos.z;
    mBase.y = mPos.y - mRadius;
    mTopple = 0.0f;
    mType = (pattern >> 12) & 3;
    s32 mode = RPDucGame::sInstance->GetMode();
    if (mType == 3 && mode != 1) {
        mType = 2;
    }
}

bool RPDucCircleTarget::CheckShotHit() {
    bool versus = false;
    if (RPDucGame::sInstance->GetMode() == 1) {
        versus = true;
    }
    s32 player = GetHittingPlayer();
    if (player != -1) {
        SpawnHitEffects(player);
        PlayHitSound(player);
        s32 points;
        switch (mType) {
        case 0:
            points = versus ? mParam.mPoints[3] : mParam.mPoints[2];
            RPDucScoreMgr::sInstance->AddScore(player, points);
            RPDucScoreMgr::sInstance->AddHits(player, 1);
            RPDucScoreMgr::sInstance->AddStreak(player, 1);
            RPDucScoreMgr::sInstance->AddCircleHits(player, 1);
            ShowScorePopup(points, player, false);
            EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
                0xFFFFFFFF, 8, 0x20);
            Remove();
            break;
        case 1:
            points = versus ? mParam.mPoints[1] : mParam.mPoints[0];
            RPDucScoreMgr::sInstance->AddScore(player, points);
            RPDucScoreMgr::sInstance->AddHits(player, 1);
            RPDucScoreMgr::sInstance->AddStreak(player, 1);
            RPDucScoreMgr::sInstance->AddCircleHits(player, 1);
            ShowScorePopup(points, player, true);
            EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
                0xFFFFFFFF, 20, 0x20);
            Remove();
            break;
        case 2:
        case 3:
            if (IsOwnFace(mType, player)) {
                points = versus ? mParam.mPoints[7] : mParam.mPoints[6];
                RPDucScoreMgr::sInstance->AddScore(player, points);
                RPDucScoreMgr::sInstance->AwardStreakBonus(player);
                RPDucScoreMgr::sInstance->ResetStreak(player);
                ShowScorePopup(points, player, false);
                EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
                    0xFFFFFFFF, 8, 0x20);
                mState = 4;
            } else {
                points = versus ? mParam.mPoints[5] : mParam.mPoints[4];
                RPDucScoreMgr::sInstance->AddScore(player, points);
                RPDucScoreMgr::sInstance->AddHits(player, 1);
                RPDucScoreMgr::sInstance->AddStreak(player, 1);
                RPDucScoreMgr::sInstance->AddCircleHits(player, 1);
                ShowScorePopup(points, player, true);
                EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
                    0xFFFFFFFF, 8, 0x20);
                Remove();
            }
            break;
        }
        return true;
    }
    return false;
}

void RPDucCircleTarget::SpawnHitEffects(s32 player) {
    f32 unitScale = 1.0f;
    nw4r::math::VEC3 pos(mPos.x, mPos.y, mPos.z);
    nw4r::math::VEC3 scale(unitScale, unitScale, unitScale);
    if (mType == 1) {
        RPSysEffect effect("Sht_target_b_hit", 0, 0);
        effect.create();
        effect.setPos(pos);
        effect.setScale(scale);
        effect.update();
    } else if (mType == 0) {
        RPSysEffect effect("Sht_target_a_hit", 0, 0);
        effect.create();
        effect.setPos(pos);
        effect.setScale(scale);
        effect.update();
    } else if (!IsOwnFace(mType, player)) {
        RPSysEffect smoke("Sht_target_a_smoke", 0, 0);
        smoke.create();
        smoke.setPos(pos);
        smoke.setScale(scale);
        smoke.update();
        RPSysEffect piece1("Sht_target_a_piece01", 0, 0);
        piece1.create();
        piece1.setPos(pos);
        piece1.setScale(scale);
        piece1.setColor(220, 117, 255, 255);
        piece1.update();
        RPSysEffect piece2("Sht_target_a_piece02", 0, 0);
        piece2.create();
        piece2.setPos(pos);
        piece2.setScale(scale);
        piece2.setColor(220, 117, 255, 255);
        piece2.update();
    }
}

void RPDucCircleTarget::PlaySpawnSound() {
    mpSndActor->startSound(28U);
}

void RPDucCircleTarget::PlayHitSound(s32 player) {
    bool versus = false;
    if (RPDucGame::sInstance->GetMode() == 1) {
        versus = true;
    }
    u32 flag = 0;
    if (player == 0) {
        flag = 1;
    } else if (player == 1) {
        flag = 2;
    }
    switch (mType) {
    case 0:
        mpSndActor->startSoundWithRemotePlayer(12U, flag, 0);
        break;
    case 1:
        mpSndActor->startSoundWithRemotePlayer(23U, flag, 0);
        break;
    case 2:
        if (player == 0) {
            mpSndActor->startSoundWithRemotePlayer(14U, flag, 0);
        } else if (!versus && player == 1) {
            mpSndActor->startSoundWithRemotePlayer(14U, flag, 0);
        } else {
            mpSndActor->startSoundWithRemotePlayer(13U, flag, 0);
        }
        break;
    case 3:
        if (player == 1) {
            mpSndActor->startSoundWithRemotePlayer(14U, flag, 0);
        } else {
            mpSndActor->startSoundWithRemotePlayer(13U, flag, 0);
        }
        break;
    }
}

void RPDucCircleTarget::StateCheckHit() {
    CheckShotHit();
}

void RPDucCircleTarget::StateGrowIn() {
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    f32 t = (f32)(mTimer + 1) / (f32)(mGrowFrames + 1);
    mHitRadius = mRadius * t;
    mHitScale = mScaleBase * t;
    f32 scale = mHitRadius / mParam.mBaseRadius;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    f32 angle = 540.0f - 450.0f * ((f32)mTimer / (f32)mGrowFrames);
    mRot.z.x = nw4r::math::CosDeg(angle);
    mRot.z.y = 0.0f;
    mRot.z.z = nw4r::math::SinDeg(angle);
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    if (!CheckShotHit()) {
        if (++mTimer >= mGrowFrames) {
            if (mType == 1) {
                mpModels[1]->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexSrt, 0, 0.0f);
            }
            mTimer = mStandFrames;
            mState = 2;
        }
    }
}

void RPDucCircleTarget::StateStand() {
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    mHitRadius = mRadius;
    mHitScale = mScaleBase;
    f32 scale = mHitRadius / mParam.mBaseRadius;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    f32 angle = 90.0f;
    mRot.z.x = nw4r::math::CosDeg(angle);
    mRot.z.y = 0.0f;
    mRot.z.z = nw4r::math::SinDeg(angle);
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    if (!CheckShotHit()) {
        if (--mTimer <= 0) {
            mTimer = mGrowFrames;
            mState = 3;
        }
    }
}

void RPDucCircleTarget::StateShrinkOut() {
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    f32 t = (f32)(mTimer + 1) / (f32)(mGrowFrames + 1);
    mHitRadius = mRadius * t;
    mHitScale = mScaleBase * t;
    f32 scale = mHitRadius / mParam.mBaseRadius;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    f32 angle = 450.0f * ((f32)mTimer / (f32)mGrowFrames) + -360.0f;
    mRot.z.x = nw4r::math::CosDeg(angle);
    mRot.z.y = 0.0f;
    mRot.z.z = nw4r::math::SinDeg(angle);
    PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    if (!CheckShotHit()) {
        if (--mTimer == 0) {
            Remove();
        }
    }
}

void RPDucCircleTarget::StateTopple() {
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    mHitRadius = mRadius;
    mHitScale = mScaleBase;
    f32 scale = mHitRadius / mParam.mBaseRadius;
    mScale.x = scale;
    mScale.y = scale;
    mScale.z = scale;
    mTopple += mParam.mToppleStep;
    if (mTopple > mParam.mToppleMax) {
        mTopple = mParam.mToppleMax;
    }
    nw4r::math::VEC3 prev = mPos;
    mPos.x = mBase.x;
    mPos.y = mHitRadius * nw4r::math::CosDeg(mTopple) + mBase.y;
    mPos.z = mBase.z - mHitRadius * nw4r::math::SinDeg(mTopple);
    mRot.z.x = 0.0f;
    mRot.z.y = nw4r::math::SinDeg(mTopple);
    mRot.z.z = nw4r::math::CosDeg(mTopple);
    mRot.x = RP_vecEx;
    PSVECCrossProduct(mRot.z, mRot.x, mRot.y);
    if (mTopple == mParam.mToppleMax) {
        mState = 5;
        mVelocity.x = 0.0f;
        mVelocity.y = mPos.y - prev.y;
        mVelocity.z = mPos.z - prev.z;
    }
}

void RPDucCircleTarget::StateFall() {
    mVelocity.y += mParam.mFallGravity;
    mVelocity.y *= mParam.mFallDrag;
    if (mPos.y < -20.0f) {
        Remove();
    }
}

void RPDucCircleTarget::Create() {
    RPDucTarget::Create();
}

void RPDucCircleTarget::Load() {
    LoadParams();
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_target_a.brres"));
    mpModels[0] = RPGrpModel::Construct(
        RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResMdl, file, "sht_target_a"), 1, 0, 0);
    mpModels[0]->SetDrawGroup(0);
    mpModels[0]->SetDrawScene(0);
    mpModels[0]->SetUnkCFlags(0x10);

    file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_target_b.brres"));
    mpModels[1] = RPGrpModel::Construct(
        RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResMdl, file, "sht_target_b"), 1, 0,
        0x204);
    mpModels[1]->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_TexSrt, 1);
    mpModels[1]->GetModelAnm()->Regist(
        RPGrpModelAnm::Anm_TexSrt, 0,
        RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmTexSrt, file, "sht_target_b"));
    mpModels[1]->SetDrawGroup(0);
    mpModels[1]->SetDrawScene(0);
    mpModels[1]->SetUnkCFlags(0x10);

    file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_target_c.brres"));
    mpModels[2] = RPGrpModel::Construct(
        RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResMdl, file, "sht_target_c"), 1, 0, 0);
    mpModels[2]->SetDrawGroup(0);
    mpModels[2]->SetDrawScene(0);
    mpModels[2]->SetUnkCFlags(0x10);

    file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_target_d.brres"));
    mpModels[3] = RPGrpModel::Construct(
        RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResMdl, file, "sht_target_d"), 1, 0, 0);
    mpModels[3]->SetDrawGroup(0);
    mpModels[3]->SetDrawScene(0);
    mpModels[3]->SetUnkCFlags(0x10);

    RPGrpTexture face1(RPMiiIconSet::sInstance->GetTexture(0));
    mpModels[2]->ReplaceTexture("face_dummy", face1, true);
    RPGrpTexture face2(RPMiiIconSet::sInstance->GetTexture(1));
    mpModels[3]->ReplaceTexture("face_dummy", face2, true);
}

void RPDucCircleTarget::ApplyVelocity() {
    mPos.x += mVelocity.x;
    mPos.y += mVelocity.y;
    mPos.z += mVelocity.z;
}

void RPDucCircleTarget::Calculate() {
    (this->*mStateFuncs[mState])();
    mpSndActor->set3DPosition(mPos);
}

void RPDucCircleTarget::CalcModelMtx() {
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, mRot, mScale, mPos);
    RPGrpModel* pModel = mpModels[mType];
    nw4r::g3d::ScnObj* pScnObj = pModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    pModel->Calc();
    pModel->Entry();
    pModel->UpdateFrame();
}

void RPDucCircleTarget::Remove() {
    RPDucCircleTargetMgr::sInstance->SetGridSlotUsed(mGridSlot, false);
    RPDucTarget::Remove();
}

#pragma defer_codegen off
const s32 RPDucCircleTarget::sPatternNum = 8;
