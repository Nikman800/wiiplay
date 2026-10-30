// Tanks field floor model (RPTnkFieldModel).
// Unit 0x802583DC-0x80258868: its functions, then the IRPGrpModelCallback thunks (end of file).
#include <Pack/RPParty/RPTnkScene/RPTnkField.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPSystem.h>
#include <egg/gfxe/eggScreen.h>
#include <egg/gfxe/eggDrawGX.h>
#include <Pack/RPParty/RPTnkScene/RPTnkBlock.h>
#include <Pack/RPParty/RPTnkScene/RPTnkStageMap.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTask.h>

RPTnkFieldModel::RPTnkFieldModel() {}

RPTnkFieldModel::~RPTnkFieldModel() {}

void RPTnkFieldModel::Create() {}

void RPTnkFieldModel::Load() {
    u16 std = EGG::Screen::GetTVMode() != EGG::Screen::TV_MODE_WIDE;
    const char* fileNames[2] = {"G3D/tnk_field_16_9.brres", "G3D/tnk_field_4_3.brres"};
    const char* modelNames[2] = {"tnk_field_16_9", "tnk_field_4_3"};
    const char* outNames[2] = {"tnk_field_16_9_out", "tnk_field_4_3_out"};

    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file =
        pResMgr->CreateData(RPGrpModelResManager::Type_ResFile,
                            RPSysResourceManager::GetFileFromArchive(pArchive, fileNames[std]));

    mpFloorModel = RPGrpModel::Construct(
        pResMgr->CreateData(RPGrpModelResManager::Type_ResMdl,
                            pResMgr->GetPtr(RPGrpModelResManager::Type_ResMdl, file,
                                            modelNames[std])),
        1, 0, 0);
    mpFloorModel->SetDrawGroup(0);
    mpFloorModel->SetDrawScene(0);
    mpFloorModel->SetUnkCFlags(0x12);
    RPGrpTexture tex(RPTnkField::sInstance->GetCaptureTexture());
    mpFloorModel->ReplaceTexture("caterpillar", tex, true);

    mpOutModel = RPGrpModel::Construct(
        pResMgr->CreateData(RPGrpModelResManager::Type_ResMdl,
                            pResMgr->GetPtr(RPGrpModelResManager::Type_ResMdl, file,
                                            outNames[std])),
        1, 0, 0);
    mpOutModel->SetDrawGroup(0);
    mpOutModel->SetDrawScene(0);
    mpOutModel->SetUnkCFlags(0x12);
}

void RPTnkFieldModel::Reset() {
    mPos = RP_vecZero;
    mScale = RP_vecOne;
    mRot = RP_mtx33Identity;
}

void RPTnkFieldModel::CalcModelMtx() {
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, mRot, mScale, mPos);
    nw4r::g3d::ScnObj* pScnObj = mpFloorModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    pScnObj = mpOutModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    mpFloorModel->UpdateFrame();
    mpOutModel->UpdateFrame();
    mpFloorModel->Calc();
    mpOutModel->Calc();
    mpFloorModel->Entry();
    mpOutModel->Entry();
}

void RPTnkFieldModel::SetGridSize(f32 width, f32 depth, s32 cols, s32 rows) {
    mWidth = width;
    mDepth = depth;
    mCols = cols;
    mRows = rows;
    mCellWidth = mWidth / mCols;
    mCellDepth = mDepth / mRows;
}

void RPTnkFieldModel::EditWorldMtx(RPGrpModel* pModel) {}
