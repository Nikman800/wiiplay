#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPBomScene/RPBomGameManager.h>
#include <Pack/RPParty/RPBomScene/RPBomPlayer.h>
#include <Pack/RPParty/RPBomScene/RPBomQuestionParam.h>
#include <Pack/RPParty/RPBomScene/RPBomScore.h>
#include <Pack/RPParty/RPBomScene/RPBomSound.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <revolution/MTX.h>

// The original file instantiates TRPSndObject<0> too: its weak vtables are zero-filled
// duplicates in this file's .data, between RPBomPanel's vtable and the kept strings
template <int N> void BomPanelSndObjectInstantiate() {
    new TRPSndObject<N>(RPSndAudioMgr::instance()->getPlayer(),
                        *static_cast<nw4r::snd::Sound3DManager*>(NULL));
}
template void BomPanelSndObjectInstantiate<0>();

// After the sound classes: RPBomPanel's vtable comes before their weak ones
#include <Pack/RPParty/RPBomScene/RPBomPanel_2.h>

extern "C" f32 Bom_getScreenHalfWidth();
extern "C" f32 Bom_getScreenHalfHeight();

extern RPAxes Bom_rotXNeg90;

// RPBomCamera_2.cpp
extern const f32 Bom_panelMatchScale;

static inline nw4r::math::VEC3 GetPlayerPos(const RPBomPlayer* pPlayer) {
    return pPlayer->mPos;
}

static inline s32 GetActiveNum() {
    return RPBomPlayerManager::sInstance->mActiveNum;
}

static inline RPBomPlayer* GetPlayer(s32 i) {
    return RPBomPlayerManager::sInstance->mpPlayers[i];
}

static inline bool IsPosing(const RPBomPlayer* pPlayer) {
    return pPlayer->mState == 1 && !pPlayer->mInBubble;
}

static inline u32 RandU32() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    return RP_randSeedX ^ RP_randSeedY;
}

static inline f32 RandRange(f32 lo, f32 hi) {
    if (lo == hi) {
        return lo;
    }
    f32 r = 1.192093e-7f * (f32)(RandU32() & 0x7FFFFF);
    return __fmadds(hi - lo, r, lo);
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

static RPBomPanel::StateFunc sNoneFunc = &RPBomPanel::StateNone;
static RPBomPanel::StateFunc sFallFunc = &RPBomPanel::StateFall;
static RPBomPanel::StateFunc sHitFunc = &RPBomPanel::StateHit;

inline void RPBomPanel::GetEffectMtx(nw4r::math::MTX34& rMtx) const {
    MakeMtx(rMtx, mScale, Bom_rotXNeg90, mPos);
}

inline void RPBomPanel::CancelColorAnms() {
    for (s32 i = 0; i < 3; i++) {
        mpModels[i]->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexSrt, 0);
        mpModels[i]->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexSrt, 1);
        mpModels[i]->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexSrt, 2);
    }
}

RPBomPanel::RPBomPanel() {
    mVel = RP_vecZero;
    mState = 0;
    mStateFuncs[0] = sNoneFunc;
    mStateFuncs[1] = sFallFunc;
    mStateFuncs[2] = sHitFunc;
}

RPBomPanel::~RPBomPanel() {}

void RPBomPanel::LoadModels() {
    const char* fileNames[3] = {"G3D/bom_panelA_model.brres", "G3D/bom_panelB_model.brres",
                                "G3D/bom_panelC_model.brres"};
    const char* mdlNames[3] = {"panelA_model", "panelB_model", "panelC_model"};
    const char* chrNames[3] = {"panelA_model", "panelB_model", "panelC_model"};
    const char* srt0Names[3] = {"panel_color0", "panel_color0", "panel_color0"};
    const char* srt1Names[3] = {"panel_color1", "panel_color1", "panel_color1"};
    const char* srt2Names[3] = {"panel_color2", "panel_color2", "panel_color2"};

    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();

    for (s32 i = 0; i < 3; i++) {
        RPGrpHandle file = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResFile,
            RPSysResourceManager::GetFileFromArchive(pArchive, fileNames[i]));
        mpModels[i] = RPGrpModel::Construct(
            pResMgr->CreateData(RPGrpModelResManager::Type_ResMdl,
                                pResMgr->GetPtr(RPGrpModelResManager::Type_ResMdl, file,
                                                mdlNames[i])),
            1, 0, 0x204);
        mpModels[i]->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
        RPGrpHandle anm = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResAnmChr,
            pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmChr, file, chrNames[i]));
        mpModels[i]->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 0, anm);
        mpModels[i]->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_TexSrt, 3);
        anm = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResAnmTexSrt,
            pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmTexSrt, file, srt0Names[i]));
        mpModels[i]->GetModelAnm()->Regist(RPGrpModelAnm::Anm_TexSrt, 0, anm);
        anm = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResAnmTexSrt,
            pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmTexSrt, file, srt1Names[i]));
        mpModels[i]->GetModelAnm()->Regist(RPGrpModelAnm::Anm_TexSrt, 1, anm);
        anm = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResAnmTexSrt,
            pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmTexSrt, file, srt2Names[i]));
        mpModels[i]->GetModelAnm()->Regist(RPGrpModelAnm::Anm_TexSrt, 2, anm);
        mpModels[i]->SetDrawGroup(0);
        mpModels[i]->SetDrawScene(0);
        mpModels[i]->SetUnk4(0);
        mpModels[i]->SetCallback(this);
    }
}

void RPBomPanel::ResetState() {
    RPBomPoolObj::ResetState();
}

void RPBomPanel::InitFields() {
    mAngle = 0.0f;
    mRotSpeed = 0.0f;
    mBaseScale = 1.0f;
    mHitPlayer = -1;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    mOrbitRadius = 0.0f;
    mOrbitStep = 0.0f;
    mOrbitAngle = 0.0f;
    mIsStartBubble = false;
    mMatchPlayer = -1;
    mOverlapFrames = 0;
    mAlignedFrames = 0;
    mMatchDist = 0.0f;
    mMatched = 0;
    mPose = 0;
    mColorType = 0;
    mState = 1;
}

void RPBomPanel::Setup(u32 flags) {
    mPose = flags & 3;
    mIsStartBubble = flags >> 31;
    mColorType = (flags >> 4) & 3;

    CancelColorAnms();

    f32 rate = 1.0f;
    if (!mIsStartBubble) {
        rate = RPBomQuestionParam::sInstance->mSpeed;
    }
    mpModels[mPose]->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
    mpModels[mPose]->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, rate);

    switch (mColorType) {
    case 0:
        mpModels[mPose]->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexSrt, 0, 0.0f);
        break;
    case 1:
        mpModels[mPose]->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexSrt, 1, 0.0f);
        break;
    case 2:
        mpModels[mPose]->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexSrt, 2, 0.0f);
        break;
    }
}

void RPBomPanel::PlayOkEffects(s32 player, s32 combo) {
    const char* flyNames[2] = {"Bom_bubble_ok_fly_blu", "Bom_bubble_ok_fly_red"};
    u16 emitDivs[10] = {4, 6, 9, 12, 15, 18, 21, 24, 27, 30};
    f32 powers[10] = {1.7f, 1.8f, 1.9f, 2.1f, 2.2f, 2.3f, 2.4f, 2.6f, 2.7f, 2.8f};

    if (combo > 9) {
        combo = 9;
    }

    nw4r::math::MTX34 mtx;
    GetEffectMtx(mtx);

    RPSysEffect fly(flyNames[player], 0, 0);
    fly.create();
    fly.setMtx(mtx);
    fly.setEmitEmitDiv(emitDivs[combo]);
    fly.setPowerRadiationDir(powers[combo]);
    fly.update();

    RPSysEffect glow("Bom_bubble_ok_glow", 0, 0);
    glow.create();
    glow.setMtx(mtx);
    glow.update();

    RPSysEffect ptcl("Bom_bubble_ok_ptcl", 0, 0);
    ptcl.create();
    ptcl.setMtx(mtx);
    ptcl.setEmitEmitDiv(combo >= 5 ? 16 : 0);
    ptcl.update();

    RPSysEffect ptcl2("Bom_bubble_ok_ptcl2", 0, 0);
    ptcl2.create();
    ptcl2.setMtx(mtx);
    ptcl2.update();

    RPSysEffect ring("Bom_bubble_ok_ring", 0, 0);
    ring.create();
    ring.setMtx(mtx);
    ring.setEmitEmitDiv(combo == 9 ? 60 : 0);
    ring.update();

    if (mColorType == 2) {
        RPSysEffect spRing("Bom_bubble_sp_ring", 0, 0);
        spRing.create();
        spRing.setMtx(mtx);
        spRing.update();
    }
}

void RPBomPanel::StateNone() {}

void RPBomPanel::StateFall() {
    mPos.y = 0.5f * mPos.z + 500.0f;
    Bom_getScreenHalfWidth();
    Bom_getScreenHalfHeight();

    if (mIsStartBubble) {
        f32 angle = mOrbitAngle + mOrbitStep;
        if (angle > 360.0f) {
            angle -= 360.0f;
        }
        f32 prev = nw4r::math::CosDeg(mOrbitAngle);
        mVel.z = mOrbitRadius * (nw4r::math::CosDeg(angle) - prev);
    } else {
        mVel.z = RPBomQuestionParam::sInstance->mSpeed;
    }

    if (RPBomGameManager::sInstance->mWaitTimer <= 0) {
        f32 angle = mOrbitAngle + mOrbitStep;
        if (angle > 360.0f) {
            angle -= 360.0f;
        }
        f32 prev = nw4r::math::SinDeg(mOrbitAngle);
        mVel.x = mOrbitRadius * (nw4r::math::SinDeg(angle) - prev);
        mOrbitAngle = angle;
    }

    f32 missZ = RPBomQuestionParam::sInstance->mpFile->mMissZ;
    if (mPos.z > missZ) {
        RPBomScore::sInstance->ResetCombos();

        u32 mask = 0;
        if (mColorType == 0) {
            mask |= 1;
        }
        if (mColorType == 1) {
            mask |= 2;
        }
        if (mColorType == 2) {
            mask |= 3;
        }
        RPBomPlayerManager::sInstance->SetStateIraByMask(mask);

        if (RPBomGameManager::sInstance->mMode == 1) {
            nw4r::math::VEC2 screenPos(mPos.x, -mPos.z);
            if (mColorType == 0) {
                RPBomGameManager::sInstance->SetPlayerMiss(0);
            } else if (mColorType == 1) {
                RPBomGameManager::sInstance->SetPlayerMiss(1);
            } else {
                RPBomGameManager::sInstance->SetPlayerMiss(-1);
            }
        } else {
            RPBomGameManager::sInstance->SetPlayerMiss(0);
        }
        Miss();
    }
}

void RPBomPanel::StateHit() {
    mVel = RP_vecZero;
    mRotSpeed = 0.0f;

    s32 state = RPBomPlayerManager::sInstance->mpPlayers[mHitPlayer]->mState;
    bool waiting = true;
    if (state != 2 && state != 3) {
        waiting = false;
    }
    if (waiting) {
        return;
    }
    Deactivate();
}

void RPBomPanel::Create() {
    mpPanelSound = new RPBomPlayerSound(RPSndAudioMgr::instance()->getPlayer());
}

void RPBomPanel::Load() {
    LoadModels();
}

void RPBomPanel::Reset() {
    ResetState();
    CancelColorAnms();
    mPose = 0;
}

void RPBomPanel::Move() {
    mPos.x += mVel.x;
    mPos.y += mVel.y;
    mPos.z += mVel.z;
    mAngle += mRotSpeed;

    if (mAngle >= 360.0f) {
        mAngle -= 360.0f;
    } else if (mAngle < 0.0f) {
        mAngle = 360.0f + mAngle;
    }

    mRot.z = RP_vecEy;
    mRot.x.x = nw4r::math::CosDeg(mAngle);
    mRot.x.y = 0.0f;
    mRot.x.z = nw4r::math::SinDeg(mAngle);
    PSVECCrossProduct(mRot.z, mRot.x, mRot.y);
}

void RPBomPanel::PreCalc() {
    RPBomPoolObj::PreCalc();
}

void RPBomPanel::CalcHit() {
    if (mState == 1 && RPBomGameManager::sInstance->mState != 4) {
        CheckPlayerMatch();
        return;
    }
    mMatchPlayer = -1;
    mOverlapFrames = 0;
    mAlignedFrames = 0;
    mMatchDist = 0.0f;
    mMatched = 0;
}

void RPBomPanel::PostCalc() {
    RPBomPoolObj::PostCalc();

    if (mState == 1 && mMatched == 1) {
        RPBomPlayer* player = RPBomPlayerManager::sInstance->mpPlayers[mMatchPlayer];
        if (player->mHitResult == 0) {
            if (!mIsStartBubble) {
                player->mHitResult = 1;
                player->mPanelIdx = mIndex;
                mHitPlayer = mMatchPlayer;
                nw4r::math::VEC2 screenPos(mPos.x, -mPos.z);
                RPBomScore* pScore = RPBomScore::sInstance;
                BOOL doubled = FALSE;
                if (RPBomGameManager::sInstance->mMode == 1) {
                    if ((mHitPlayer == 0 && mColorType == 1) ||
                        (mHitPlayer == 1 && mColorType == 0)) {
                        doubled = TRUE;
                    }
                }
                nw4r::math::VEC2 pos = screenPos;
                pScore->AddBubblePoints(mMatchPlayer, &pos, doubled);
                if (mColorType == 2) {
                    RPBomGameManager::sInstance->SetBubbleResult(mMatchPlayer);
                }
            } else {
                player->mHitResult = 2;
                player->mPanelIdx = mIndex;
                mHitPlayer = mMatchPlayer;
            }
            PlayOkEffects(mHitPlayer, 9);
            mState = 2;
        }
    }
}

void RPBomPanel::Calculate() {
    (this->*mStateFuncs[mState])();
    mpPanelSound->set3DPosition(mPos);
}

void RPBomPanel::CalcModelMtx() {
    nw4r::math::MTX34 mtx;
    GetEffectMtx(mtx);
    RPGrpModel* pModel = mpModels[mPose];
    nw4r::g3d::ScnObj* pScnObj = pModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    pModel->Calc();
    pModel->Entry();
    pModel->UpdateFrame();
}

void RPBomPanel::EditWorldMtx(RPGrpModel* pModel) {
    nw4r::math::MTX34* pNodeMtx = pModel->ReferWorldMtx(pModel->GetJointIndex("panel_node"));
    nw4r::math::MTX34 nodeMtx;
    PSMTXCopy(*pNodeMtx, nodeMtx);
    nodeMtx._03 = 0.0f;
    nodeMtx._13 = 0.0f;
    nodeMtx._23 = 0.0f;

    nw4r::math::MTX34 baseMtx;
    nw4r::math::VEC3 zero(0.0f, 0.0f, 0.0f);
    MakeMtx(baseMtx, RP_vecOne, Bom_rotXNeg90, zero);
    PSMTXInverse(baseMtx, baseMtx);
    PSMTXConcat(baseMtx, nodeMtx, nodeMtx);

    nw4r::math::VEC3 scale(nodeMtx._00, nodeMtx._11, nodeMtx._22);
    MakeMtx(*pNodeMtx, scale, mRot, mPos);
}

void RPBomPanel::Spawn(const nw4r::math::VEC3& rPos, u32 flags, RPBomSpawnInfo info) {
    ResetState();
    InitFields();
    Setup(flags);

    mPos = rPos;
    mVel = info.mVel;
    mAngle = info.mAngle;
    mRotSpeed = info.mRotSpeed;
    mBaseScale = info.mScale;
    mOrbitRadius = info.mOrbitRadius;
    mOrbitStep = info.mOrbitStep;
    mOrbitAngle = (RandU32() & 0x10) ? 0.0f : 180.0f;

    mScale.x = mBaseScale;
    mScale.y = mBaseScale;
    mScale.z = mBaseScale;
    mRot.z = RP_vecEy;
    mRot.x.x = nw4r::math::CosDeg(mAngle);
    mRot.x.y = 0.0f;
    mRot.x.z = nw4r::math::SinDeg(mAngle);
    PSVECCrossProduct(mRot.z, mRot.x, mRot.y);

    if (mIsStartBubble) {
        mOrbitRadius = 7.0f;
        mOrbitStep = 1.5f;
        mOrbitAngle = RandRange(0.0f, 360.0f);
    }

    mAge = 0;
    SetUnk40(1);

    {
        RPSysEffect effect("Bom_keyhole_appear", 0, 0);
        effect.create();
        nw4r::math::MTX34 mtx;
        GetEffectMtx(mtx);
        effect.setMtx(mtx);
        effect.update();
    }

    mpPanelSound->startSound(15U);
}

// STAND-IN: dead-stripped; puts the u32->f32 magic double (lbl_804417B0) after 14.2222f
// (804417AC) and before Spawn's 180.0f (804417B8), as in the original .sdata2. Without it the
// double lands after 360.0f, before StateFall's 500.0f (804417D0).
extern "C" f32 Bom_panelPoolOrderU32(u32 x) {
    return (f32)x;
}

void RPBomPanel::Deactivate() {
    RPBomPoolObj::Deactivate();
    switch (mColorType) {
    case 0:
        mpModels[mPose]->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexSrt, 0);
        break;
    case 1:
        mpModels[mPose]->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexSrt, 1);
        break;
    case 2:
        mpModels[mPose]->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_TexSrt, 2);
        break;
    }
}

void RPBomPanel::Miss() {
    {
        RPSysEffect effect("Bom_bubble_miss", 0, 0);
        effect.create();
        nw4r::math::MTX34 mtx;
        GetEffectMtx(mtx);
        effect.setMtx(mtx);
        effect.update();
    }

    mpPanelSound->startSound(27U);
    Deactivate();
}

void RPBomPanel::CheckPlayerMatch() {
    f32 zero = 0.0f; // unused: puts 0.0 first in the .sdata2 pool
    RPBomPlayer* player;
    f32 range = mBaseScale * (0.7f * Bom_panelMatchScale);
    s32 num = GetActiveNum();
    if (num == 0) {
        return;
    }

    f32 minDist = 10000.0f;
    s32 nearest = -1;
    s32 i = 0;
    do {
        if (IsPosing(GetPlayer(i))) {
            player = GetPlayer(i);
            nw4r::math::VEC3 playerPos;
            playerPos = GetPlayerPos(player);
            nw4r::math::VEC3 diff;
            diff.x = playerPos.x - mPos.x;
            diff.y = 0.0f;
            diff.z = playerPos.z - mPos.z;
            f32 dist = PSVECMag(diff);
            if (dist < range && dist < minDist) {
                minDist = dist;
                nearest = i;
            }
        }
    } while (++i < num);

    if (nearest == -1) {
        mMatchPlayer = -1;
        mOverlapFrames = 0;
        mAlignedFrames = 0;
        mMatched = 0;
        return;
    }

    if (mMatchPlayer != nearest) {
        mOverlapFrames = 0;
        mAlignedFrames = 0;
    }
    mMatchDist = minDist;
    mMatchPlayer = nearest;
    mOverlapFrames++;

    player = GetPlayer(nearest);
    RPAxes rot = player->mRot;
    if (nw4r::math::VEC3Dot(&rot.x, &mRot.x) > nw4r::math::CosDeg(20.0f)) {
        mAlignedFrames++;
    }

    if (mOverlapFrames >= 4 && mAlignedFrames >= 1 && mPose == player->mPose) {
        mMatched = 1;
    } else {
        mMatched = 0;
    }
}
