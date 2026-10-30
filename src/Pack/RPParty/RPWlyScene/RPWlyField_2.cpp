#include <Pack/RPParty/RPWlyScene/RPWlyField_2.h>

#include <Pack/RPGraphics.h>

RPWlyEarth::~RPWlyEarth() {}

RPWlyEarth::RPWlyEarth() {
    mPos.x = 70.0f;
    mPos.y = -170.0f;
    mPos.z = -500.0f;
    mScale.x = 0.4f;
    mScale.y = 0.4f;
    mScale.z = 0.4f;
}

void RPWlyEarth::Load() {
    RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/wtd_field03.brres");
    RPSimpleModel::LoadModel("wtd_field03_earth", file, 1, 0, 0);
    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    const char* name = "wtd_field03_earth";
    void* pData = pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmChr, file, name);
    RPGrpHandle anm = pResMgr->CreateData(RPGrpModelResManager::Type_ResAnmChr, pData);
    mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 0, anm);
    mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
    mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, 1.0f);
}

void RPWlyEarth::Reset() {}

void RPWlyEarth::Calculate() {
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
