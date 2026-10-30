// Shooting Range cloud model (RPDucShtCloud).
// Unit 0x8024D30C-0x8024D58C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucField.h>

#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPSystem.h>

RPDucShtCloud::RPDucShtCloud() {}

RPDucShtCloud::~RPDucShtCloud() {}

void RPDucShtCloud::Create() {}

void RPDucShtCloud::Load() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_cloud.brres"));
    mpModel = RPGrpModel::Construct(
        pResMgr->CreateData(RPGrpModelResManager::Type_ResMdl,
                            pResMgr->GetPtr(RPGrpModelResManager::Type_ResMdl, file, 0)),
        1, 0, 0x204);

    RPGrpHandle srt = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmTexSrt, file, "sht_cloud");
    CreateAnmBuffer(RPGrpModelAnm::Anm_TexSrt, 1);
    RegistAnm(RPGrpModelAnm::Anm_TexSrt, 0, srt);
    mpModel->SetDrawGroup(0);
    mpModel->SetDrawScene(0);
    mpModel->SetUnk4(0);
}

void RPDucShtCloud::Reset() {
    mPos = RP_vecZero;
    mScale = RP_vecOne;
    mRot = RP_mtx33Identity;
    StartAnm(RPGrpModelAnm::Anm_TexSrt, 0, 0.0f);
}
