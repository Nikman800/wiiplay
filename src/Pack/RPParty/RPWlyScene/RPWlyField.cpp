// Access to the projection objects' camera/screen and the models' members
#include <Pack/RPParty/RPWlyScene/RPWlyField.h>
#include <Pack/RPParty/RPWlyScene/RPWlyCursor.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <egg/gfxe/eggScreen.h>
#include <egg/math.h>

#include <Pack/RPParty/RPWlyScene/RPWlyAvatar.h>
#include <Pack/RPParty/RPWlyScene/RPWlyAvatarMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyField_2.h>
#include <Pack/RPParty/RPWlyScene/RPWlyField_3.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLevelTable.h>

#undef protected
#undef private

class RPWlyCursor;


enum WlyGameState {
    STATE_CLEAR = 3,
    STATE_RESULT = 4,
};

static inline s32 GetTargetNum() {
    return RPWlyAvatarMgr::sInstance->mpStage->GetTargetNum();
}

static inline WlyGameState GetState() {
    return static_cast<WlyGameState>(RPWlyGameMgr::sInstance->mState);
}

static inline nw4r::math::VEC3 GetModelScale(RPSimpleModel* pModel) {
    return pModel->GetScale();
}

static inline void SetModelScale(RPSimpleModel* pModel, nw4r::math::VEC3 scale) {
    pModel->SetScale(scale);
}

static inline void SetSpotScale(RPGrpProjectionObject* pSpot, const nw4r::math::VEC3& rScale) {
    pSpot->GetScreen().SetScale(rScale);
}

RPWlyField::~RPWlyField() {}

void RPWlyField::Init() {
    mpGround = NULL;
    mpEarth = NULL;
    mpShootingStar = NULL;
    mpSpotlights[0] = NULL;
    mpSpotlights[1] = NULL;
    mSpotPos.x = sVecOne.x;
    mSpotPos.y = sVecOne.y;
    mSpotPos.z = sVecOne.z;

    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType) {
    case 1:
        mpGround = new RPWlyGround();
        break;
    case 2:
        mpEarth = new RPWlyEarth();
        mpShootingStar = new RPSysEffect("Wtd_shootingstar01", 0, 0);
        break;
    }

    if (RPWlyLevelTable::sInstance->IsSpotlightStage()) {
        mpSpotlights[0] = new RPGrpProjectionObject(1);
        mpSpotlights[1] = new RPGrpProjectionObject(2);
    }
}

void RPWlyField::Create() {
    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType) {
    case 0:
        if (RPWlyLevelTable::sInstance->IsSpotlightStage()) {
            RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/wtd_field07.brres");
            RPSimpleModel::LoadModel("wtd_field07", file, 1, 0, 0x108);
            mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);
            RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
            const char* name = "wtd_field07";
            void* pData = pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmClr, file, name);
            RPGrpHandle anm = pResMgr->CreateData(RPGrpModelResManager::Type_ResAnmClr, pData);
            mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Clr, 0, anm);
            mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
            mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
            if (RPWlyGameMgr::sInstance->mMode == 0) {
                mpModel->GetModelAnm()->SetFrame(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
            } else {
                mpModel->GetModelAnm()->SetFrame(RPGrpModelAnm::Anm_Clr, 0, 1.0f);
            }
            RPWlySoundActor::sFieldIndex = 5;
        } else {
            RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/wtd_field01.brres");
            RPSimpleModel::LoadModel("wtd_field01", file, 1, 0, 0);
            RPWlySoundActor::sFieldIndex = 0;
        }
        break;
    case 1: {
        RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/wtd_field02.brres");
        RPSimpleModel::LoadModel("wtd_field02", file, 1, 0, 0x204);
        mpModel->SetDrawScene(1);
        mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_TexSrt, 1);
        RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
        const char* name = "wtd_field02";
        void* pData = pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmTexSrt, file, name);
        RPGrpHandle anm = pResMgr->CreateData(RPGrpModelResManager::Type_ResAnmTexSrt, pData);
        mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_TexSrt, 0, anm);
        mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexSrt, 0, 0.0f);
        mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_TexSrt, 0, 1.0f);
        mpGround->Load();
        if (RPWlyLevelTable::sInstance->IsSpotlightStage()) {
            RPWlySoundActor::sFieldIndex = 6;
        } else {
            RPWlySoundActor::sFieldIndex = 1;
        }
        break;
    }
    case 2: {
        RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/wtd_field03.brres");
        RPSimpleModel::LoadModel("wtd_field03", file, 1, 0, 0x108);
        mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);
        RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
        const char* name = "wtd_field03";
        void* pData = pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmClr, file, name);
        RPGrpHandle anm = pResMgr->CreateData(RPGrpModelResManager::Type_ResAnmClr, pData);
        mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Clr, 0, anm);
        mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
        mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, 0, 1.0f);
        mpEarth->Load();
        if (RPWlyLevelTable::sInstance->IsSpotlightStage()) {
            RPWlySoundActor::sFieldIndex = 7;
        } else {
            RPWlySoundActor::sFieldIndex = 2;
        }
        break;
    }
    case 3:
        if (RPWlyLevelTable::sInstance->IsSpotlightStage()) {
            RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/wtd_field08.brres");
            RPSimpleModel::LoadModel("wtd_field08", file, 1, 0, 0x108);
            mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);
            RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
            const char* name = "wtd_field08";
            void* pData = pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmClr, file, name);
            RPGrpHandle anm = pResMgr->CreateData(RPGrpModelResManager::Type_ResAnmClr, pData);
            mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Clr, 0, anm);
            mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
            mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
            if (RPWlyGameMgr::sInstance->mMode == 0) {
                mpModel->GetModelAnm()->SetFrame(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
            } else {
                mpModel->GetModelAnm()->SetFrame(RPGrpModelAnm::Anm_Clr, 0, 1.0f);
            }
            RPWlySoundActor::sFieldIndex = 8;
        } else {
            RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/wtd_field04.brres");
            RPSimpleModel::LoadModel("wtd_field04", file, 1, 0, 0);
            RPWlySoundActor::sFieldIndex = 3;
        }
        break;
    case 4:
        if (RPWlyLevelTable::sInstance->IsSpotlightStage()) {
            RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/wtd_field09.brres");
            RPSimpleModel::LoadModel("wtd_field09", file, 1, 0, 0x108);
            mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);
            RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
            const char* name = "wtd_field09";
            void* pData = pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmClr, file, name);
            RPGrpHandle anm = pResMgr->CreateData(RPGrpModelResManager::Type_ResAnmClr, pData);
            mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Clr, 0, anm);
            mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
            mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
            if (RPWlyGameMgr::sInstance->mMode == 0) {
                mpModel->GetModelAnm()->SetFrame(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
            } else {
                mpModel->GetModelAnm()->SetFrame(RPGrpModelAnm::Anm_Clr, 0, 1.0f);
            }
            const char* escNames[3] = {"wtd_field09_esc1", "wtd_field09_esc2", "wtd_field09_esc3"};
            f32 escRates[3];
            escRates[0] = 0.9f;
            escRates[1] = 1.1f;
            escRates[2] = 0.9f;
            RPGrpHandle escAnms[3];
            mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 3);
            for (u8 i = 0; i < 3; i++) {
                RPGrpModelResManager* pEscMgr = RPGrpModelResManager::GetCurrent();
                RPGrpHandle anm = pEscMgr->CreateData(
                    RPGrpModelResManager::Type_ResAnmChr,
                    pEscMgr->GetPtr(RPGrpModelResManager::Type_ResAnmChr, file, escNames[i]));
                escAnms[i] = anm;
                mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, i, escAnms[i]);
                mpModel->GetModelAnm()->SetBlendAnm(RPGrpModelAnm::Anm_Chr, i, i);
                mpModel->GetModelAnm()->SetBlendWeight(RPGrpModelAnm::Anm_Chr, i, 1.0f, 0);
                mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, i, escRates[i]);
            }
            RPWlySoundActor::sFieldIndex = 9;
        } else {
            RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/wtd_field05.brres");
            RPSimpleModel::LoadModel("wtd_field05", file, 1, 0, 0);
            const char* escNames[3] = {"wtd_field05_esc1", "wtd_field05_esc2", "wtd_field05_esc3"};
            f32 escRates[3];
            escRates[0] = 0.9f;
            escRates[1] = 1.1f;
            escRates[2] = 0.9f;
            RPGrpHandle escAnms[3];
            mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 3);
            for (u8 i = 0; i < 3; i++) {
                RPGrpModelResManager* pEscMgr = RPGrpModelResManager::GetCurrent();
                RPGrpHandle anm = pEscMgr->CreateData(
                    RPGrpModelResManager::Type_ResAnmChr,
                    pEscMgr->GetPtr(RPGrpModelResManager::Type_ResAnmChr, file, escNames[i]));
                escAnms[i] = anm;
                mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, i, escAnms[i]);
                mpModel->GetModelAnm()->SetBlendAnm(RPGrpModelAnm::Anm_Chr, i, i);
                mpModel->GetModelAnm()->SetBlendWeight(RPGrpModelAnm::Anm_Chr, i, 1.0f, 0);
                mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, i, escRates[i]);
            }
            RPWlySoundActor::sFieldIndex = 4;
        }
        break;
    }

    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType == 1) {
        RPGrpTexture texture(RPWlyGameMgr::sInstance->mpCapTexture);
        mpModel->ReplaceTexture("check01", texture, true);
    }

    if (RPWlyLevelTable::sInstance->IsSpotlightStage()) {
        EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
        RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
        RPGrpHandle scene = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResFile,
            RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sceneFile.brres"));
        for (int i = 0; i < 2; i++) {
            mpSpotlights[i]->LoadScnCamera(scene, "WNTScene", 0.0f);
            RPGrpModelManager::GetCurrent()->GetModelScene(0)->SetProjection(mpSpotlights[i]);
            mpSpotlights[i]->GetScreen().SetProjectionType(EGG::Frustum::PROJ_ORTHO);
            mpSpotlights[i]->GetScreen().SetCanvasMode(EGG::Frustum::CANVAS_CC);
        }

        mSpotPos.x = RPWlyLevelTable::sInstance->GetCurrentLevel()->mSpotPos.x;
        mSpotPos.y = RPWlyLevelTable::sInstance->GetCurrentLevel()->mSpotPos.y;
        mSpotPos.z = RPWlyLevelTable::sInstance->GetCurrentLevel()->mSpotPos.z;

        switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType) {
        case 0:
            mSpotScale.x = 1.5f;
            mSpotScale.y = 3.0f;
            mSpotScale.z = 1.0f;
            break;
        case 3:
            mSpotScale.x = 2.0f;
            mSpotScale.y = 2.0f;
            mSpotScale.z = 1.0f;
            break;
        case 4:
            mSpotScale.x = 2.5f;
            mSpotScale.y = 2.0f;
            mSpotScale.z = 1.0f;
            break;
        default:
            mSpotScale.x = 1.0f;
            mSpotScale.y = 1.0f;
            mSpotScale.z = 1.0f;
            break;
        }

        mSpotScaleRate = 1.0f;
        mPlaneY = RPWlyLevelTable::sInstance->GetCurrentLevel()->mPlaneY;
        mPlaneZ = RPWlyLevelTable::sInstance->GetCurrentLevel()->mPlaneZ;
    }
}

void RPWlyField::Reset() {
    if (mpGround != NULL) {
        mpGround->Reset();
    } else if (mpEarth != NULL) {
        mpEarth->Reset();
    }

    if (mpShootingStar != NULL) {
        mpShootingStar->create();
    }
}

// The cursor loop reloads RPWlyGameMgr::sInstance at its top instead of reusing the loop test's
// load; no source form tried reproduced that without disabling common subexpressions.
#pragma push
#pragma opt_common_subs off
void RPWlyField::Calculate() {
    CalcModelMtx();

    if (mpGround != NULL) {
        mpGround->Calculate();
        SetModelScale(this, GetModelScale(mpGround));
    } else if (mpEarth != NULL) {
        RPGrpModel* pModel = mpEarth->GetModel();
        nw4r::math::MTX34 mtx;
        pModel->GetWorldMtx(pModel->GetJointIndex("wtd_field03_earth"), &mtx);
        mpEarth->Calculate();
        nw4r::math::VEC3 pos(mtx._03, mtx._13, mtx._23);
        mpShootingStar->setPos(pos);
        mpShootingStar->setScale(GetModelScale(mpEarth));
        mpShootingStar->update();
    }

    if (RPWlyLevelTable::sInstance->IsSpotlightStage()) {
        if (GetState() == STATE_CLEAR || GetState() == STATE_RESULT) {
            s32 num = GetTargetNum();
            switch (num) {
            case 1:
                mpModel->GetModelAnm()->SetFrame(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
                break;
            case 2:
                mpModel->GetModelAnm()->SetFrame(RPGrpModelAnm::Anm_Clr, 0, 1.0f);
                break;
            }

            for (int i = 0; i < num; i++) {
                nw4r::math::VEC3 pos = RPWlyAvatarMgr::sInstance->mpAvatars[i]->mPos;
                RPGrpCamera& rCamera = mpSpotlights[i]->GetCamera();
                rCamera.SetTarget(pos.x, 100.0f, pos.z);
                rCamera.SetRotation(-90.0f, 0.0f, 0.0f);
                rCamera.SetCameraType(RPGrpCamera::ECameraType_Orbit);
                rCamera.CalcMatrix();
                RPGrpProjectionObject* pSpot = mpSpotlights[i];
                if (GetState() == STATE_CLEAR) {
                    SetSpotScale(pSpot, mSpotScale);
                } else {
                    mSpotScaleRate += 0.5f;
                    if (mSpotScaleRate >= 1000.0f) {
                        mSpotScaleRate = 1000.0f;
                    }
                    SetSpotScale(pSpot, mSpotScale * mSpotScaleRate);
                }
            }
        } else {
            for (int i = 0; i < RPWlyGameMgr::sInstance->mPlayerNum; i++) {
                EGG::Vector2f pos = RPWlyGameMgr::sInstance->mpCursors[i]->GetPos();
                nw4r::math::VEC3 hit;
                RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
                nw4r::math::VEC3 planePos(0.0f, mPlaneZ, 0.0f);
                nw4r::math::VEC3 normal(0.0f, 1.0f, mPlaneY);
                RPWlyGameMgr::sInstance->mpCamera->GetScreenToPlane(
                    &hit, planePos, normal, pScreen,
                    pos.x * (f32)(u16)(EGG::Screen::GetSizeXMax() / 2),
                    pos.y * (f32)(u16)(EGG::Screen::GetSizeYMax() / 2));
                RPGrpCamera& rCamera = mpSpotlights[i]->GetCamera();
                rCamera.SetTarget(hit.x, 100.0f, hit.z);
                rCamera.SetRotation(-90.0f, 0.0f, 0.0f);
                rCamera.SetCameraType(RPGrpCamera::ECameraType_Orbit);
                rCamera.CalcMatrix();
                SetSpotScale(mpSpotlights[i], mSpotPos);
            }
        }
    }
}

#pragma pop

void RPWlyField::Destroy() {}
