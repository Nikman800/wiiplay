// Pose Mii background (bg_model, bg_acc_model) and start wipe effect (RPBomStartBg).
#include <Pack/RPParty/RPBomScene/RPBomStartBg.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpModelMaterial.h>
#include <Pack/RPGraphics/RPGrpModelResManager.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPBomScene/RPBomQuestionParam.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <egg/gfxe/eggScreen.h>
#include <revolution/MTX.h>

RPBomStartBg* RPBomStartBg::sInstance;

extern RPAxes Bom_rotXNeg90;

namespace {
inline void* GetResData(RPGrpModelResManager* pResMgr, RPGrpModelResManager::Type type,
                        RPGrpHandle file, const char* pName) {
    return pResMgr->GetPtr(type, file, pName);
}

inline f32 FadeMax(f32 a, f32 b) {
    if (a > b) {
        return a;
    }
    return b;
}
inline f32 FadeMin(f32 a, f32 b) {
    if (a < b) {
        return a;
    }
    return b;
}
inline f32 FadeClamp(f32 v, f32 lo, f32 hi) {
    return FadeMin(hi, FadeMax(lo, v));
}

inline void SetMaterialColor(RPGrpModel* pModel, const char* pName, s32 value) {
    RPGrpModelMaterial* pMaterial = pModel->GetMaterial(pName);
    pMaterial->SetTevColor(RPGrpModelMaterial::ETevColor_Reg0, value, value, value);
    pMaterial->SetSyncFlags(RPGrpModelMaterial::ESyncFlag_ResMatTev);
}
} // namespace

void RPBomStartBg::CreateInstance() {
    sInstance = new RPBomStartBg;
}

void RPBomStartBg::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBomStartBg::~RPBomStartBg() {}

#pragma auto_inline off
void RPBomStartBg::LoadBgModel() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file =
        pResMgr->CreateData(RPGrpModelResManager::Type_ResFile,
                            RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/bg_model.brres"));

    mpBgModel = RPGrpModel::Construct(
        pResMgr->CreateData(RPGrpModelResManager::Type_ResMdl,
                            GetResData(pResMgr, RPGrpModelResManager::Type_ResMdl, file, "bg_model")),
        1, 0, 0);

    RPGrpHandle clr = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResAnmChr,
        GetResData(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "bg_model"));
    mpBgModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
    mpBgModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 0, clr);
    mpBgModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, 1.0f);

    RPGrpHandle pat = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResAnmTexPat,
        GetResData(pResMgr, RPGrpModelResManager::Type_ResAnmTexPat, file, "bg_model"));
    mpBgModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_TexPat, 1);
    mpBgModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_TexPat, 0, pat);

    mpBgModel->SetDrawGroup(0);
    mpBgModel->SetDrawScene(2);
    mpBgModel->SetUnk4(0);
    mpBgModel->SetCallback(this);
}

void RPBomStartBg::LoadBgAccModel() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/bom_bg_acc_model.brres"));

    mpBgAccModel = RPGrpModel::Construct(
        pResMgr->CreateData(RPGrpModelResManager::Type_ResMdl,
                            GetResData(pResMgr, RPGrpModelResManager::Type_ResMdl, file, "bg_acc_model")),
        1, 0, 0);

    RPGrpHandle clr;
    if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
        clr = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResAnmChr,
            GetResData(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "bg_acc_16_9"));
    } else {
        clr = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResAnmChr,
            GetResData(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "bg_acc_4_3"));
    }
    mpBgAccModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
    mpBgAccModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 0, clr);
    mpBgAccModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, 1.0f);

    mpBgAccModel->SetDrawGroup(0);
    mpBgAccModel->SetDrawScene(2);
    mpBgAccModel->SetUnk4(0);
}

void RPBomStartBg::UpdateFade() {
    if (mFade > mFadeTarget) {
        mFade -= 1.5f;
        mFade = mFadeTarget <= mFade ? mFade : mFadeTarget;
    } else if (mFade < mFadeTarget) {
        mFade += 3.0f;
        mFade = mFadeTarget <= mFade ? mFadeTarget : mFade;
    }

    mFade = FadeClamp(mFade, 0.0f, 255.0f);
    s32 value = mFade;
    SetMaterialColor(mpBgModel, "mt_bg_a", value);
    SetMaterialColor(mpBgModel, "mt_bg_b", value);
}

#pragma auto_inline reset

void RPBomStartBg::Create() {
    if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
        mpEffect = new RPSysEffect("Bom_startbg", 2, 0);
    } else {
        mpEffect = new RPSysEffect("Bom_startbg_st", 2, 0);
    }
}

void RPBomStartBg::Load() {
    LoadBgModel();
    LoadBgAccModel();
}

void RPBomStartBg::Reset() {
    mPos = RP_vecZero;
    mPos.y = -1000.0f;
    mScale = RP_vecOne;
    mScale.x = 27.5f;
    mScale.y = 27.5f;
    mScale.z = 27.5f;
    mRot = Bom_rotXNeg90;
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, mRot, mScale, mPos);

    mLevel = 0;
    mLevelUpTimer = 0;
    mLevelUpPending = false;
    mFade = 255.0f;
    mFadeTarget = 255.0f;

    mpBgModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0);
    mpBgModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexPat, 0);
    mpBgModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_TexPat, 0, 0.0f);
    mpBgAccModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0);

    mpEffect->create();
    nw4r::math::VEC3 pos = mPos;
    pos.y += 0.2f;
    nw4r::math::MTX34 effectMtx;
    RPDucMakeMtx(&effectMtx, Bom_rotXNeg90, mScale, pos);
    mpEffect->setMtx(effectMtx);
    mpEffect->update();
}

inline void RPBomStartBg::StartLevelFrame() {
    s32 frame = RPBomQuestionParam::sInstance->mpFile->mLevels[mLevel].mBgFrame;
    mpBgModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0);
    mpBgModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexPat, 0, frame);
    mpBgModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_TexPat, 0, 0.0f);
    mLevelUpTimer = 0;
    mLevelUpPending = false;
}

void RPBomStartBg::Calculate() {
    if (mLevelUpPending && --mLevelUpTimer <= 0) {
        StartLevelFrame();
    }
    UpdateFade();
}

void RPBomStartBg::CalcModelMtx() {
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, mRot, RP_vecOne, mPos);
    nw4r::g3d::ScnObj* pScnObj = mpBgModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    pScnObj = mpBgAccModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    mpBgModel->SetBaseScale(mScale);
    mpBgAccModel->SetBaseScale(mScale);
    mpBgModel->UpdateFrame();
    mpBgAccModel->UpdateFrame();
    mpBgModel->Calc();
    mpBgAccModel->Calc();
    mpBgModel->Entry();
    mpBgAccModel->Entry();
}

void RPBomStartBg::EditWorldMtx(RPGrpModel* pModel) {
    PSMTXCopy(*pModel->ReferWorldMtx(pModel->GetJointIndex("bg_model")), mNodeMtx);
}

void RPBomStartBg::PlayWipe() {
    const char* names[] = {"Bom_startbg_wipe_st", "Bom_startbg_wipe"};
    RPSysEffect effect(names[EGG::Screen::GetTVMode()], 2, 0);
    effect.create();
    nw4r::math::VEC3 pos = mPos;
    pos.y += 0.2f;
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, Bom_rotXNeg90, mScale, pos);
    effect.setMtx(mtx);
    effect.update();
    mpEffect->forceKill();
}

void RPBomStartBg::SetLevel(s32 level, BOOL levelUp) {
    mLevel = level;
    if (levelUp) {
        {
            RPSysEffect effect("Bom_levelup01", 2, 0);
            effect.create();
            effect.setMtx(mNodeMtx);
            effect.update();
        }
        RPSndAudioMgr::instance()->startSystemSe(3U);
        mLevelUpTimer = 17;
        mLevelUpPending = true;
    } else {
        StartLevelFrame();
    }
}

void RPBomStartBg::SetFadeTargetDim() {
    mFadeTarget = 120.0f;
}

void RPBomStartBg::SetFadeTargetFull() {
    mFadeTarget = 255.0f;
}

// Stand-in (unreferenced, dead-stripped at link): compiled first, it puts 0.0f and 1.0f ahead
// of the setFadeTarget constants in .sdata2, as the original (whose first-compiled function
// using them is not in the binary).
extern "C" void Bom_StartBg_poolOrder(RPBomStartBg* pBg) {
    pBg->mFade = 0.0f;
    pBg->mFadeTarget = 1.0f;
}
