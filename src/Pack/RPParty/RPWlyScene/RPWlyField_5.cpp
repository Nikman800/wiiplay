#include <Pack/RPParty/RPWlyScene/RPWlyField_5.h>

#include <Pack/RPGraphics.h>
#include <cmath>

#include <Pack/RPParty/RPWlyScene/RPWlyAvatarMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLyt.h>

RPWlyWipe::~RPWlyWipe() {}

RPWlyWipe::RPWlyWipe() {
    mCloseRequested = false;
    mDone = false;
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mScale.x = 0.01f;
    mScale.y = 0.01f;
    mScale.z = 0.01f;
}

void RPWlyWipe::Init() {}

void RPWlyWipe::Create() {
    RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/wtd_wipe_a.brres");
    RPSimpleModel::LoadModel("wtd_wipe_a", file, 1, 0, 0x30C);
    mpModel->SetDrawScene(2);
    const char* anmNames[2] = {"wtd_wipe_a2", "wtd_wipe_a1"};
    RPGrpModelAnm* pAnm = mpModel->GetModelAnm();
    RPGrpHandle anms[2];

    pAnm->CreateBuffer(RPGrpModelAnm::Anm_Clr, 2);
    for (u32 i = 0; i < 2; i++) {
        RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
        RPGrpHandle anm = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResAnmClr,
            pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmClr, file, anmNames[(u16)i]));
        anms[(u16)i] = anm;
        pAnm->Regist(RPGrpModelAnm::Anm_Clr, i, anms[(u16)i]);
    }
    pAnm->Start(RPGrpModelAnm::Anm_Clr, 0, 3.0f);
    pAnm->SetUpdateRate(RPGrpModelAnm::Anm_Clr, 0, 1.0f);
    pAnm->SetFrame(RPGrpModelAnm::Anm_Clr, 1, 0.0f);

    pAnm->CreateBuffer(RPGrpModelAnm::Anm_TexSrt, 2);
    for (u32 i = 0; i < 2; i++) {
        RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
        RPGrpHandle anm = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResAnmTexSrt,
            pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmTexSrt, file, anmNames[(u16)i]));
        anms[(u16)i] = anm;
        pAnm->Regist(RPGrpModelAnm::Anm_TexSrt, i, anms[(u16)i]);
    }
    pAnm->Start(RPGrpModelAnm::Anm_TexSrt, 0, 3.0f);
    pAnm->SetUpdateRate(RPGrpModelAnm::Anm_TexSrt, 0, 1.0f);
    pAnm->SetFrame(RPGrpModelAnm::Anm_TexSrt, 1, 0.0f);
}

void RPWlyWipe::Reset() {}

void RPWlyWipe::Calculate() {
    if (mDone) {
        return;
    }

    RPGrpModelAnm* pAnm = mpModel->GetModelAnm();
    if (mCloseRequested) {
        if ((f32)std::fmod(pAnm->GetFrame(RPGrpModelAnm::Anm_TexSrt, 0), 50.0) < 1.0f) {
            pAnm->Start(RPGrpModelAnm::Anm_Clr, 1, 0.0f);
            pAnm->Start(RPGrpModelAnm::Anm_TexSrt, 1, 0.0f);
            mCloseRequested = false;
        }
    } else {
        f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Clr, 1);
        if (frame > pAnm->GetNumFrame(RPGrpModelAnm::Anm_Clr, 1)) {
            RPWlyLayoutSet::sInstance->mpMessage->Show();
            RPWlyAvatarMgr::sInstance->StartSwimEffects();
            RPWlyAvatarMgr::sInstance->mMotionFlags[0] = TRUE;
            RPWlyAvatarMgr::sInstance->mMotionFlags[1] = TRUE;
            RPWlyAvatarMgr::sInstance->mMotionFlags[2] = TRUE;
            if (RPWlyGameMgr::sInstance->mState == 2) {
                RPWlyGameMgr::sInstance->mpTimerSe->StartRoundSound();
            }
            mDone = true;
        }
    }

    nw4r::math::VEC3 pos = mPos;
    nw4r::math::VEC3 scale = mScale;
    // NONMATCHING-DEBT: RPSimpleModel::sMtxIdentity is declared VEC3[3] (its static initializer
    // copy-constructs the rows, RPKcl_2.cpp); this copy is one 9-word MTX33 block copy, which only
    // an MTX33 source gives. Fix with the RPSimpleModel declaration (PartyCommon), not here.
    nw4r::math::MTX33 rotation = *reinterpret_cast<const nw4r::math::MTX33*>(sMtxIdentity);
    mRot = rotation;
    mPos.x = pos.x;
    mPos.y = pos.y;
    mPos.z = pos.z;
    mScale.x = scale.x;
    mScale.y = scale.y;
    mScale.z = scale.z;
    CalcModelMtx();
}

void RPWlyWipe::Destroy() {}

void RPWlyWipe::StartClose() {
    mCloseRequested = true;
}
