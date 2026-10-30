// Pose Mii player Mii (RPBomPlayer) and its sound actors (RPBomPlayerSound, RPBomSoundActorBase).
#include <Pack/RPParty/RPBomScene/RPBomPlayer.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPKernel/RPSysKokeshi.h>
#include <Pack/RPKernel/RPSysKokeshiGenInfo.h>
#include <Pack/RPKernel/RPSysKokeshiManager.h>
#include <Pack/RPParty/RPBomScene/RPBomCursor.h>
#include <Pack/RPParty/RPBomScene/RPBomGameManager.h>
#include <Pack/RPParty/RPBomScene/RPBomLyt.h>
#include <Pack/RPParty/RPBomScene/RPBomPanel_3.h>
#include <Pack/RPParty/RPBomScene/RPBomSound.h>
#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <egg/core.h>
#include <revolution/MTX.h>

extern RPAxes Bom_rotXNeg90;

// Bom_playerStateFuncs: one PTMF object each, copied one by one by the constructor
static RPBomPlayer::StateFunc sMoveNop0Func = &RPBomPlayer::MoveNop0;
static RPBomPlayer::StateFunc sMoveFollowCursor1Func = &RPBomPlayer::MoveFollowCursor1;
static RPBomPlayer::StateFunc sMoveAlongBubble2Func = &RPBomPlayer::MoveAlongBubble2;
static RPBomPlayer::StateFunc sMoveAlongBubble3Func = &RPBomPlayer::MoveAlongBubble3;
static RPBomPlayer::StateFunc sMoveNop4Func = &RPBomPlayer::MoveNop4;
static RPBomPlayer::StateFunc sMoveFollowCursor5Func = &RPBomPlayer::MoveFollowCursor5;
static RPBomPlayer::StateFunc sMoveNop6Func = &RPBomPlayer::MoveNop6;
static RPBomPlayer::StateFunc sStateNop0Func = &RPBomPlayer::StateNop0;
static RPBomPlayer::StateFunc sStatePosingFunc = &RPBomPlayer::StatePosing;
static RPBomPlayer::StateFunc sStateHitWaitFunc = &RPBomPlayer::StateHitWait;
static RPBomPlayer::StateFunc sStateNop3Func = &RPBomPlayer::StateNop3;
static RPBomPlayer::StateFunc sStateNop4Func = &RPBomPlayer::StateNop4;
static RPBomPlayer::StateFunc sStateNop5Func = &RPBomPlayer::StateNop5;
static RPBomPlayer::StateFunc sStateNop6Func = &RPBomPlayer::StateNop6;

nw4r::math::VEC3 Bom_vecDown(0.0f, -1.0f, 0.0f);

// FAKEMATCH (see RPSndGameObjectHelper in RPSndObject.h): the original's zero-filled weak sound
// data (0x21C bytes after RPBomSoundActorBase's vtable) includes the helper's 0x90-byte vtable.
// Unreferenced, so the linker strips the function.
extern "C" void Bom_Player_instantiateSndHelper() {
    RPSndGameObjectHelper helper;
}

static inline RPBomCursor* GetCursor(s32 player) {
    return RPBomCursorManager::sInstance->mpCursors[player];
}

static inline f32 Max(f32 a, f32 b) {
    if (a > b) {
        return a;
    }
    return b;
}

static inline f32 Min(f32 a, f32 b) {
    if (a < b) {
        return a;
    }
    return b;
}

static inline f32 Clamp(f32 v, f32 lo, f32 hi) {
    return Min(hi, Max(lo, v));
}

static inline void MakeMtx(nw4r::math::MTX34& rMtx, const nw4r::math::VEC3& rScale,
                           const RPAxes& rRot, const nw4r::math::VEC3& rTrans) {
    rMtx._00 = rScale.x * rRot.x.x;
    rMtx._01 = rScale.y * rRot.y.x;
    rMtx._02 = rScale.z * rRot.z.x;
    rMtx._03 = rTrans.x;
    rMtx._10 = rScale.x * rRot.x.y;
    rMtx._11 = rScale.y * rRot.y.y;
    rMtx._12 = rScale.z * rRot.z.y;
    rMtx._13 = rTrans.y;
    rMtx._20 = rScale.x * rRot.x.z;
    rMtx._21 = rScale.y * rRot.y.z;
    rMtx._22 = rScale.z * rRot.z.z;
    rMtx._23 = rTrans.z;
}

static inline void SetRotFromAngle(RPBomPlayer* pPlayer, f32 angle) {
    pPlayer->mRot.x.x = nw4r::math::CosDeg(angle);
    pPlayer->mRot.x.y = 0.0f;
    pPlayer->mRot.x.z = nw4r::math::SinDeg(angle);
    pPlayer->mRot.z.x = RP_vecEy.x;
    pPlayer->mRot.z.y = RP_vecEy.y;
    pPlayer->mRot.z.z = RP_vecEy.z;
    nw4r::math::VEC3Cross(&pPlayer->mRot.y, &pPlayer->mRot.z, &pPlayer->mRot.x);
}

static inline void FollowCursor(RPBomPlayer* pPlayer) {
    nw4r::math::VEC3 pos = GetCursor(pPlayer->mPlayer)->mPos;
    pPlayer->mPos = pos;
    pPlayer->mScale.x = pPlayer->mBaseScale;
    pPlayer->mScale.y = pPlayer->mBaseScale;
    pPlayer->mScale.z = pPlayer->mBaseScale;
    f32 angle = GetCursor(pPlayer->mPlayer)->mAngle;
    f32 deg = angle;
    deg *= 4.0f;
    SetRotFromAngle(pPlayer, deg);
}

static inline void FollowCursorClamped(RPBomPlayer* pPlayer) {
    nw4r::math::VEC3 pos = GetCursor(pPlayer->mPlayer)->mPos;
    pPlayer->mPos = pos;
    pPlayer->mScale.x = pPlayer->mBaseScale;
    pPlayer->mScale.y = pPlayer->mBaseScale;
    pPlayer->mScale.z = pPlayer->mBaseScale;
    f32 angle = GetCursor(pPlayer->mPlayer)->mAngle;
    f32 deg = angle;
    deg *= 4.0f;
    SetRotFromAngle(pPlayer, Clamp(deg, -180.0f, 180.0f));
}

static inline void SetBubbleTransform(RPBomPlayer* pPlayer, const nw4r::math::VEC3& rPos,
                                      const RPAxes& rRot) {
    nw4r::math::VEC3 cursorPos = GetCursor(pPlayer->mPlayer)->mPos;
    pPlayer->mPos.x = rPos.x;
    pPlayer->mPos.y = cursorPos.y;
    pPlayer->mPos.z = rPos.z;
    pPlayer->mScale.x = pPlayer->mBaseScale;
    pPlayer->mScale.y = pPlayer->mBaseScale;
    pPlayer->mScale.z = pPlayer->mBaseScale;
    pPlayer->mRot = rRot;
}

static inline void UpdateEffectMtx(RPBomPlayer* pPlayer) {
    MakeMtx(pPlayer->mEffectMtx, pPlayer->mScale, pPlayer->mRot, pPlayer->mPos);
    pPlayer->mPrevAnmFrame = 0.0f;
}

RPBomPlayer::RPBomPlayer() {
    mPos = RP_vecZero;
    mScale = RP_vecOne;
    mRot = RP_mtx33Identity;
    mPlayer = 0;
    mState = 0;
    mpKokeshi = NULL;
    mMoveFuncs[0] = sMoveNop0Func;
    mMoveFuncs[1] = sMoveFollowCursor1Func;
    mMoveFuncs[2] = sMoveAlongBubble2Func;
    mMoveFuncs[3] = sMoveAlongBubble3Func;
    mMoveFuncs[4] = sMoveNop4Func;
    mMoveFuncs[5] = sMoveFollowCursor5Func;
    mMoveFuncs[6] = sMoveNop6Func;
    mStateFuncs[0] = sStateNop0Func;
    mStateFuncs[1] = sStatePosingFunc;
    mStateFuncs[2] = sStateHitWaitFunc;
    mStateFuncs[3] = sStateNop3Func;
    mStateFuncs[4] = sStateNop4Func;
    mStateFuncs[5] = sStateNop5Func;
    mStateFuncs[6] = sStateNop6Func;
}

RPBomPlayer::~RPBomPlayer() {}

void RPBomPlayer::TriggerPoseEffects() {
    RPGrpModelAnm* pBodyAnm = mpKokeshi->GetBodyModel()->GetModelAnm();
    if (pBodyAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr) == 0) {
        f32 frame = pBodyAnm->GetFrame(RPGrpModelAnm::Anm_Chr, 0);
        if (!mPoseReverse) {
            if (mPrevAnmFrame < 13.0f && frame >= 13.0f) {
                mpPoseEffects[0]->create();
            } else if (mPrevAnmFrame < 32.0f && frame >= 32.0f) {
                mpPoseEffects[1]->create();
            } else if (mPrevAnmFrame < 51.0f && frame >= 51.0f) {
                mpPoseEffects[2]->create();
            }
        } else {
            if (frame < 13.0f && mPrevAnmFrame >= 13.0f) {
                mpPoseEffects[2]->create();
            } else if (frame < 32.0f && mPrevAnmFrame >= 32.0f) {
                mpPoseEffects[0]->create();
            } else if (frame < 51.0f && mPrevAnmFrame >= 51.0f) {
                mpPoseEffects[1]->create();
            }
        }
        mPrevAnmFrame = frame;
    }
}

#pragma auto_inline off
void RPBomPlayer::PlayHitSound(s32 colorType) {
    if ((u32)colorType == 2) {
        mpSound->startSound(9U);
    } else if ((u32)colorType == (u32)mPlayer) {
        mpSound->startSound(7U);
    } else {
        mpSound->startSound(8U);
    }
}
#pragma auto_inline reset

void RPBomPlayer::MoveNop0() {}

void RPBomPlayer::MoveFollowCursor1() {
    FollowCursor(this);
}

void RPBomPlayer::MoveAlongBubble2() {
    nw4r::math::VEC3 pos;
    RPAxes rot;
    s32 colorType;
    RPBomPanelPool::sInstance->GetPanelTransform(pos, rot, colorType, mPanelIdx);
    SetBubbleTransform(this, pos, rot);
}

void RPBomPlayer::MoveAlongBubble3() {
    nw4r::math::VEC3 pos;
    RPAxes rot;
    s32 colorType;
    RPBomPanelPool::sInstance->GetPanelTransform(pos, rot, colorType, mPanelIdx);
    SetBubbleTransform(this, pos, rot);
}

void RPBomPlayer::MoveNop4() {}

void RPBomPlayer::MoveFollowCursor5() {
    FollowCursor(this);
}

void RPBomPlayer::MoveNop6() {}

void RPBomPlayer::StateNop0() {}

static inline void ChangePose(RPBomPlayer* pPlayer, s32 pose, BOOL reverse) {
    pPlayer->mPose = pose;
    pPlayer->mPoseFrame = pose * 20.0f;
    RPBomLayoutMgr::sInstance->mpPoseSel[pPlayer->mPlayer]->ChangePose(pPlayer->mPlayer, pose, reverse);
    pPlayer->mpSound->startSound(2U);
}

void RPBomPlayer::StatePosing() {
    RPGrpModel* pBodyModel = mpKokeshi->GetBodyModel();

    if (mHitResult != 0) {
        nw4r::math::VEC3 pos;
        RPAxes rot;
        s32 colorType;
        RPBomPanelPool::sInstance->GetPanelTransform(pos, rot, colorType, mPanelIdx);
        SetBubbleTransform(this, pos, rot);

        if ((u32)colorType == 2) {
            EGG::CoreControllerMgr::instance()->getNthController((u16)mPlayer)->startSimpleRumble(
                0x0FFFFFFF, 20, 32);
        } else {
            EGG::CoreControllerMgr::instance()->getNthController((u16)mPlayer)->startSimpleRumble(
                0x0FFFFFFF, 10, 32);
        }
        PlayHitSound(colorType);

        if (mHitResult == 1) {
            mTimer = 10;
            mState = 2;
            return;
        }
        if (mHitResult == 2) {
            mState = 3;
            return;
        }
    }

    TriggerPoseEffects();

    bool missed = false;
    if (RPBomGameManager::sInstance->mState == 5 && mPlayer != RPBomGameManager::sInstance->mBubbleResult) {
        missed = true;
    }

    if (!mInBubble && missed) {
        mpBubbleModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
        pBodyModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 1, 0.0f);
        mInBubble = true;
    } else if (mInBubble && !missed) {
        {
            RPSysEffect effect("Bom_bubble_miss", 0, 0);
            effect.create();
            nw4r::math::MTX34 mtx;
            MakeMtx(mtx, mScale, Bom_rotXNeg90, mPos);
            effect.setMtx(mtx);
            effect.update();
        }
        StartPosing(mPose, mBaseScale);
        mpSound->startSound(28U);
        mInBubble = false;
    }

    BOOL selShown = RPBomLayoutMgr::sInstance->mpPoseSel[mPlayer]->GetState() == 0;
    if (--mPoseCooldown < 0) {
        mPoseCooldown = 0;
    }
    if (mPoseCooldown == 0 && !mInBubble && selShown) {
        EGG::CoreController* pController =
            EGG::CoreControllerMgr::instance()->getNthController(mPlayer);
        if (pController->downTrigger(0x800)) {
            if (mPose == 0) {
                mpSound->startSound(1U);
                RPBomLayoutMgr::sInstance->mpPoseSel[mPlayer]->PlayBlocked(mPlayer, FALSE);
            } else {
                ChangePose(this, (mPose + 1) % 3, FALSE);
                mPoseReverse = false;
                mPoseCooldown = 10;
            }
        } else if (pController->downTrigger(0x400)) {
            if (mPose == 1) {
                mpSound->startSound(1U);
                RPBomLayoutMgr::sInstance->mpPoseSel[mPlayer]->PlayBlocked(mPlayer, TRUE);
            } else {
                if (--mPose == -1) {
                    mPose = 2;
                }
                ChangePose(this, mPose, TRUE);
                // The first (dead) store gives the original's li 10 / li 1 order.
                mPoseCooldown = 10;
                mPoseReverse = true;
                mPoseCooldown = 10;
            }
        }
    }
}

void RPBomPlayer::StateHitWait() {
    if (--mTimer <= 0) {
        mPanelIdx = -1;
        mTimer = 0;
        FollowCursorClamped(this);
        mPanelIdx = -1;
        mTimer = 0;
        mState = 1;
    }
}

void RPBomPlayer::StateNop3() {}

void RPBomPlayer::StateNop4() {}

void RPBomPlayer::StateNop5() {}

void RPBomPlayer::StateNop6() {}

RPBomSoundActorBase::~RPBomSoundActorBase() {}

void RPBomPlayer::Create() {
    UpdateEffectMtx(this);

    mpPoseEffects[0] = new RPSysEffect("Bom_change_pose01", 0, 0);
    mpPoseEffects[1] = new RPSysEffect("Bom_change_pose02", 0, 0);
    mpPoseEffects[2] = new RPSysEffect("Bom_change_pose03", 0, 0);

    RPSysKokeshiGenInfo info;
    info.SetGenType(RPSysKokeshiManager::GenType_Player);
    info.SetPlayerNo(mPlayer);
    if (RPSceneInfo::spInstance->GetPlayerNum() == 1) {
        info.SetPlayerNo(0);
    }
    info.SetExpressionFlag(0xB);
    info.SetBodyType(5);
    mpKokeshi = new RPSysKokeshi(&info);

    mpSound = new RPBomPlayerSound(RPSndAudioMgr::instance()->getPlayer());
    u32 remoteFlag = 0;
    if (mPlayer == 0) {
        remoteFlag = 1;
    } else if (mPlayer == 1) {
        remoteFlag = 2;
    }
    mpSound->SetRemoteFlag(remoteFlag);
}

static inline void* GetResPtr(RPGrpModelResManager* pResMgr, RPGrpModelResManager::Type type,
                              RPGrpHandle file, const char* pName) {
    return pResMgr->GetPtr(type, file, pName);
}

void RPBomPlayer::Load() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    mpKokeshi->LoadResource();
    mpKokeshi->ApplyLightTexture(0);

    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle avatarFile = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/avatarA2_anim.brres"));
    RPGrpHandle poseFile = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/bom_pose_anim.brres"));

    RPGrpModel* pBodyModel = mpKokeshi->GetBodyModel();
    pBodyModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 4);
    RPGrpHandle anm0 = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResAnmChr,
        GetResPtr(pResMgr, RPGrpModelResManager::Type_ResAnmChr, poseFile, "bom_pose_anim"));
    pBodyModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 0, anm0);
    RPGrpHandle anm1 = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResAnmChr,
        GetResPtr(pResMgr, RPGrpModelResManager::Type_ResAnmChr, avatarFile, "ira_pose_C"));
    pBodyModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 1, anm1);
    RPGrpHandle anm2 = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResAnmChr,
        GetResPtr(pResMgr, RPGrpModelResManager::Type_ResAnmChr, avatarFile, "ira_pose_win"));
    pBodyModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 2, anm2);
    RPGrpHandle anm3 = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResAnmChr,
        GetResPtr(pResMgr, RPGrpModelResManager::Type_ResAnmChr, avatarFile, "ira_pose_lose"));
    pBodyModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 3, anm3);
    pBodyModel->SetCallback(this);

    RPGrpHandle bubbleFile = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/bom_bubble_model.brres"));
    mpBubbleModel = RPGrpModel::Construct(
        pResMgr->CreateData(RPGrpModelResManager::Type_ResMdl,
                            GetResPtr(pResMgr, RPGrpModelResManager::Type_ResMdl, bubbleFile,
                                      "bom_bubble")),
        1, 0, 0);
    mpBubbleModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
    RPGrpHandle bubbleAnm = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResAnmChr,
        GetResPtr(pResMgr, RPGrpModelResManager::Type_ResAnmChr, bubbleFile, "bom_bubble"));
    mpBubbleModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 0, bubbleAnm);
    mpBubbleModel->SetDrawGroup(0);
    mpBubbleModel->SetDrawScene(0);
    mpBubbleModel->SetUnk4(0);
}

void RPBomPlayer::Reset() {
    mPos = RP_vecZero;
    mScale = RP_vecOne;
    mRot = RP_mtx33Identity;
    mBaseScale = 13.0f;

    RPGrpModel* pFaceModel = mpKokeshi->GetNigaoeModel();
    RPGrpModel* pBodyModel = mpKokeshi->GetBodyModel();
    mHitResult = 0;
    mPanelIdx = -1;
    mTimer = 0;
    unk5C = 0;
    mPoseCooldown = 0;
    mFrame = 0;
    mPoseReverse = false;
    mInBubble = false;
    mPose = 0;
    pBodyModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
    pBodyModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, 2.0f);
    RPSysKokeshiManager::ChangeExpression(pFaceModel, RFLExpression(0));

    UpdateEffectMtx(this);
    mState = 0;
}

void RPBomPlayer::CalcState() {
    (this->*mMoveFuncs[mState])();
    mHitResult = 0;
}

void RPBomPlayer::CalcMove() {
    (this->*mStateFuncs[mState])();
    if (mState != 0) {
        mFrame++;
    }
    mpSound->set3DPosition(mPos);
}

static inline bool InRange(f32 v, f32 lo, f32 hi) {
    bool in = false;
    if (lo <= v && hi >= v) {
        in = true;
    }
    return in;
}

void RPBomPlayer::UpdateModel() {
    if (mState == 0) {
        return;
    }

    RPGrpModel* pFaceModel = mpKokeshi->GetNigaoeModel();
    RPGrpModel* pBodyModel = mpKokeshi->GetBodyModel();

    nw4r::math::MTX34 bodyMtx;
    MakeMtx(bodyMtx, RP_vecOne, mRot, mPos);
    if (pBodyModel->GetScnObj() != NULL) {
        pBodyModel->GetScnObj()->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &bodyMtx);
    }
    pBodyModel->SetBaseScale(mScale);
    pBodyModel->Calc();

    EGG::Matrix34f headMtx;
    mpKokeshi->GetHeadJointMtx(&headMtx);
    if (pFaceModel->GetScnObj() != NULL) {
        pFaceModel->GetScnObj()->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &headMtx);
    }
    pFaceModel->SetBaseScale(mScale);
    pFaceModel->Calc();
    pFaceModel->Entry();
    pBodyModel->Entry();
    pFaceModel->UpdateFrame();

    if (pBodyModel->GetModelAnm()->GetPlayIndex(RPGrpModelAnm::Anm_Chr) != 0) {
        pBodyModel->UpdateFrame();
    } else {
        f32 frame = pBodyModel->GetModelAnm()->GetFrame(RPGrpModelAnm::Anm_Chr, 0);
        switch (mPose) {
        case 0:
            if (InRange(frame, 2.0f, 60.0f)) {
                pBodyModel->UpdateFrame();
            } else {
                pBodyModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
            }
            break;
        case 1:
            if (InRange(frame, 0.0f, 18.0f) || InRange(frame, 22.0f, 60.0f)) {
                pBodyModel->UpdateFrame();
            } else {
                pBodyModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 20.0f);
            }
            break;
        case 2:
            if (InRange(frame, 0.0f, 38.0f) || InRange(frame, 42.0f, 60.0f)) {
                pBodyModel->UpdateFrame();
            } else {
                pBodyModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 40.0f);
            }
            break;
        }
        if (mPoseReverse) {
            pBodyModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, -2.0f);
        } else {
            pBodyModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, 2.0f);
        }
    }

    if (mInBubble) {
        nw4r::math::MTX34 bubbleMtx;
        MakeMtx(bubbleMtx, mScale, mRot, mPos);
        if (mpBubbleModel->GetScnObj() != NULL) {
            mpBubbleModel->GetScnObj()->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &bubbleMtx);
        }
        mpBubbleModel->UpdateFrame();
        mpBubbleModel->Calc();
        mpBubbleModel->Entry();
    }
}

void RPBomPlayer::EditWorldMtx(RPGrpModel* pModel) {
    nw4r::math::MTX34Copy(&mEffectMtx, pModel->ReferWorldMtx(pModel->GetJointIndex("hip")));
    int i = 0;
    do {
        mpPoseEffects[i]->setMtx(mEffectMtx);
        mpPoseEffects[i]->update();
    } while (++i < 3);
}

void RPBomPlayer::SetStateIdle() {
    mPanelIdx = -1;
    mTimer = 0;
    mState = 0;
}

#pragma auto_inline off
void RPBomPlayer::StartPosing(s32 pose, f32 scale) {
    mBaseScale = scale;
    FollowCursorClamped(this);
    mPose = pose;
    mPoseFrame = pose * 20.0f;
    mPrevAnmFrame = mPoseFrame;
    mpKokeshi->GetBodyModel()->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, pose * 20.0f);
    RPSysKokeshiManager::ChangeExpression(mpKokeshi->GetNigaoeModel(), RFLExpression(0));
    mPoseReverse = false;
    mPanelIdx = -1;
    mTimer = 0;
    mPoseCooldown = 0;
    mState = 1;
}
#pragma auto_inline reset

void RPBomPlayer::RestartPosing2() {
    StartPosing(mPose, mBaseScale);
}

void RPBomPlayer::SetStateIra(BOOL playAnim) {
    if (playAnim) {
        mpKokeshi->GetBodyModel()->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 1, 0.0f);
    }
    mState = 4;
}

void RPBomPlayer::RestartPosing() {
    StartPosing(mPose, mBaseScale);
}

void RPBomPlayer::SetStateWin(BOOL playAnim) {
    RPGrpModel* pFaceModel = mpKokeshi->GetNigaoeModel();
    RPGrpModel* pBodyModel = mpKokeshi->GetBodyModel();
    if (playAnim) {
        pBodyModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 2, 0.0f);
        RPSysKokeshiManager::ChangeExpression(pFaceModel, RFLExpression(1));
    }
    mPanelIdx = -1;
    mTimer = 0;
    mState = 5;
}

void RPBomPlayer::SetStateResult() {
    mPanelIdx = -1;
    mTimer = 0;
    mState = 6;
}

void RPBomPlayer::PlayResultAnim(BOOL win) {
    RPGrpModel* pFaceModel = mpKokeshi->GetNigaoeModel();
    RPGrpModel* pBodyModel = mpKokeshi->GetBodyModel();
    if (win) {
        pBodyModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 2, 0.0f);
        RPSysKokeshiManager::ChangeExpression(pFaceModel, RFLExpression(1));
    } else {
        pBodyModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 3, 0.0f);
        RPSysKokeshiManager::ChangeExpression(pFaceModel, RFLExpression(3));
    }
}
