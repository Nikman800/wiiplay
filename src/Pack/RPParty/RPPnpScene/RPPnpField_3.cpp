// Table Tennis net shadow: animated shadow model of the net (pin_net_shadow).
// Unit 0x802350F4-0x802352C8 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpField.h>

#include <Pack/RPGraphics.h>

RPPnpNetShadow::~RPPnpNetShadow() {}

void RPPnpNetShadow::Create() {
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mScale.x = 10.0f;
    mScale.y = 10.0f;
    mScale.z = 10.0f;
}

void RPPnpNetShadow::Load() {
    RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/pin_table.brres");
    RPSimpleModel::LoadModel("pin_net_shadow", file, 1, 0, 0);
    mpModel->SetDrawGroup(2);

    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle anm = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResAnmChr,
        pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmChr, file, "pin_net_shadow"));
    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
    mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 0, anm);
}

void RPPnpNetShadow::Reset() {}

void RPPnpNetShadow::Calculate() {
    CalcModelMtx();
}

void RPPnpNetShadow::Exit() {}

void RPPnpNetShadow::RestartAnim(f32 frame) {
    mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, frame);
    mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
}
