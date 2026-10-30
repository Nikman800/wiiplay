// Shooting Range ground model (RPDucShtField).
// Unit 0x80237774-0x80237A8C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucField.h>

#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPSystem.h>


RPDucShtField::RPDucShtField() {}

RPDucShtField::~RPDucShtField() {}

void RPDucShtField::Create() {}

void RPDucShtField::Load() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_field.brres"));
    mpModel = RPGrpModel::Construct(
        pResMgr->CreateData(RPGrpModelResManager::Type_ResMdl,
                            pResMgr->GetPtr(RPGrpModelResManager::Type_ResMdl, file, 0)),
        1, 0, 0x204);

    RPGrpHandle chr = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "sht_field");
    RPGrpHandle srt = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmTexSrt, file, "sht_field");
    CreateAnmBuffer(RPGrpModelAnm::Anm_Chr, 1);
    RegistAnm(RPGrpModelAnm::Anm_Chr, 0, chr);
    CreateAnmBuffer(RPGrpModelAnm::Anm_TexSrt, 1);
    RegistAnm(RPGrpModelAnm::Anm_TexSrt, 0, srt);
    mpModel->SetDrawGroup(0);
    mpModel->SetDrawScene(0);
    mpModel->SetUnk4(0);
}

void RPDucShtField::Reset() {
    mPos = RP_vecZero;
    mScale = RP_vecOne;
    mRot = RP_mtx33Identity;
    StartAnm(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
    StartAnm(RPGrpModelAnm::Anm_TexSrt, 0, 0.0f);
}
