// Tanks check mark ("X" left where a tank was destroyed, RPTnkCheckMark).
// Unit 0x80274FE0-0x802753A4 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkCheckMark.h>

#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPSystem.h>

static RPTnkCheckMark::StateFunc sCheckMarkStateFuncs[] = {
    &RPTnkCheckMark::StateStop,
};

RPTnkCheckMark::RPTnkCheckMark() {
    mStateFuncs[0] = sCheckMarkStateFuncs[0];
}

RPTnkCheckMark::~RPTnkCheckMark() {}

void RPTnkCheckMark::OnSpawn() {
    // Locals in this order put 0.2f before 1.0f in .sdata2.
    f32 h = 0.2f;
    f32 one = 1.0f;
    mScale.x = one;
    mPos.y = h;
    mScale.y = one;
    mScale.z = one;
    mRot = RP_mtx33Identity;
    mState = 0;
}

void RPTnkCheckMark::ApplySpawnParam(u32 param) {
    mColor = param & 0xF;
}

void RPTnkCheckMark::StateStop() {
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
}

void RPTnkCheckMark::Create() {
    RPTnkActor::Create();
}

void RPTnkCheckMark::Load() {
    s32 i;
    const char* modelNames[3] = {"tnk_check_blue", "tnk_check_red", "tnk_check_white"};
    const char** pName; // explicit name pointer: register order i, pName, pResMgr
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/tnk_check.brres"));
    for (i = 0, pName = modelNames; i < 3; i++, pName++) {
        mpModels[i] = RPGrpModel::Construct(pResMgr->CreateData(RPGrpModelResManager::Type_ResMdl,
            pResMgr->GetPtr(RPGrpModelResManager::Type_ResMdl, file, *pName)), 1, 0, 0);
        mpModels[i]->SetDrawGroup(0);
        mpModels[i]->SetDrawScene(0);
        mpModels[i]->SetUnk4(0);
    }
}

void RPTnkCheckMark::ApplyVelocity() {
    mPos.x += mVelocity.x;
    mPos.y += mVelocity.y;
    mPos.z += mVelocity.z;
}

void RPTnkCheckMark::Calculate() {
    (this->*mStateFuncs[mState])();
}

void RPTnkCheckMark::CalcModelMtx() {
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, mRot, RP_vecOne, mPos);
    RPGrpModel* pModel = mpModels[mColor];
    nw4r::g3d::ScnObj* pScnObj = pModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    pModel->SetBaseScale(mScale);
    pModel->UpdateFrame();
    pModel->Calc();
    pModel->Entry();
}
