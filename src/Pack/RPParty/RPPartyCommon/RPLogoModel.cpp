#include <Pack/RPParty/RPPartyCommon/RPMenuBgModel.h>

#include <Pack/RPGraphics/RPGrpCamera.h>
#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpModelAnm.h>
#include <Pack/RPGraphics/RPGrpModelResManager.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <egg/math/eggMatrix.h>

static const char* const sLogoResourceFile = "G3D/logo_model.brres";
static const char* const sLogoResourceModel = "logo_model";

static inline RPGrpHandle CreateLogoPart(RPGrpModelResManager* pManager, RPGrpHandle file,
                                         RPGrpModelResManager::Type type) {
    const char* pName = "logo_model";
    void* pData = pManager->GetPtr(type, file, pName);
    return pManager->CreateData(type, pData, 0);
}

RPLogoModel::~RPLogoModel() {}

RPLogoModel::RPLogoModel(u8 viewNo, u8 sceneNo) {
    mViewNo = viewNo;
    mSceneNo = sceneNo;
    mpModel = NULL;
    mpCamera = new RPGrpCamera;
    mpCamera->SetCameraType(RPGrpCamera::ECameraType_LookAt);
    RPGrpScreen* pScreen = new RPGrpScreen;
    pScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    pScreen->SetFovy(10.0f);
    RPGrpRenderer::GetCurrent()->CreateView3D(viewNo, mpCamera, pScreen);
    RPGrpRenderer::GetCurrent()->GetView(mViewNo)->SetFlag(8);
    RPGrpRenderer::GetCurrent()->GetView(viewNo)->SetDrawScene(sceneNo);
}

void RPLogoModel::Load(void* pArchive) {
    if (pArchive == NULL) {
        pArchive = RPSysResourceManager::LoadGameCommonArchive(-1, NULL);
    }
    RPGrpModelResManager* pManager = RPGrpModelResManager::GetCurrent();
    void* pData = RPSysResourceManager::GetFileFromArchive(
        static_cast<EGG::Archive*>(pArchive), "G3D/logo_model.brres", NULL);
    RPGrpHandle file = pManager->CreateData(RPGrpModelResManager::Type_ResFile, pData, 0);
    RPGrpHandle model = CreateLogoPart(pManager, file, RPGrpModelResManager::Type_ResMdl);
    RPGrpHandle anm = CreateLogoPart(pManager, file, RPGrpModelResManager::Type_ResAnmChr);
    mpModel = RPGrpModel::Construct(model, 1, 0, 0);
    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
    mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 0, anm, 0);
    mpModel->SetDrawScene(mSceneNo);
}

void RPLogoModel::Reset() {
    nw4r::math::VEC3 position(0.0f, 0.0f, 65.0f);
    mpCamera->SetPosition(position);
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mScale.x = 13.0f;
    mScale.y = 13.0f;
    mScale.z = 13.0f;
    mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0);
}

void RPLogoModel::Calculate() {
    mpCamera->CalcMatrix();
    EGG::Matrix34f mtx;
    mtx.makeIdentity();
    mtx._03 = mPos.x;
    mtx._13 = mPos.y;
    mtx._23 = mPos.z;
    nw4r::g3d::ScnObj* pScnObj = mpModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    mpModel->SetBaseScale(mScale);
    mpModel->UpdateFrame();
    mpModel->Calc();
    mpModel->Entry();
}

void RPLogoModel::SetPosFromScreen(const nw4r::math::VEC2& rScreenPos) {
    RPGrpRenderer::GetCurrent()->GetView(mViewNo)->GetScreenToWorld(&mPos, rScreenPos);
}
