#include <Pack/RPParty/RPPartyCommon/RPMenuBgModel.h>

#include <Pack/RPGraphics/RPGrpCamera.h>
#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpModelAnm.h>
#include <Pack/RPGraphics/RPGrpModelManager.h>
#include <Pack/RPGraphics/RPGrpModelResManager.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

namespace {
inline RPGrpHandle CreateResource(RPGrpModelResManager* pManager,
                                  RPGrpModelResManager::Type type, void* pData) {
    return pManager->CreateData(type, pData, 0);
}

inline void* GetResource(RPGrpModelResManager* pManager,
                         RPGrpModelResManager::Type type, RPGrpHandle file,
                         const char* pName) {
    return pManager->GetPtr(type, file, pName);
}
}

RPMenuBgModel::~RPMenuBgModel() {}

RPMenuBgModel::RPMenuBgModel(u8 viewNo, u8 sceneNo, u8 startFrame)
    : mViewNo(viewNo), mSceneNo(sceneNo), mStartFrame(startFrame), mpModel(NULL) {
    mpCamera = new RPGrpCamera;
    mpCamera->SetCameraType(RPGrpCamera::ECameraType_LookAt);

    RPGrpScreen* pScreen = new RPGrpScreen;
    pScreen->SetCanvasMode(EGG::Screen::CANVAS_CC);
    pScreen->SetFovy(30.0f);
    RPGrpRenderer::GetCurrent()->CreateView3D(viewNo, mpCamera, pScreen);
    RPGrpRenderer::GetCurrent()->GetView(viewNo)->SetDrawScene(sceneNo);

    GXColor ambient = {150, 150, 150, 255};
    RPGrpModelManager::GetCurrent()->GetLightManager(sceneNo)->GetAmbientColor(0) = ambient;
}

void RPMenuBgModel::Load(void* pArchive) {
    if (pArchive == NULL) {
        pArchive = RPSysResourceManager::LoadGameCommonArchive();
    }

    RPGrpModelResManager* pManager = RPGrpModelResManager::GetCurrent();
    void* pData = RPSysResourceManager::GetFileFromArchive(
        static_cast<EGG::Archive*>(pArchive), "G3D/menu_BG_model.brres");
    RPGrpHandle file = CreateResource(pManager, RPGrpModelResManager::Type_ResFile, pData);
    RPGrpHandle model = CreateResource(pManager, RPGrpModelResManager::Type_ResMdl,
        GetResource(pManager, RPGrpModelResManager::Type_ResMdl, file, "menu_BG_model"));
    RPGrpHandle chr = CreateResource(pManager, RPGrpModelResManager::Type_ResAnmChr,
        GetResource(pManager, RPGrpModelResManager::Type_ResAnmChr, file, "menu_BG_model"));
    RPGrpHandle texSrt = CreateResource(pManager, RPGrpModelResManager::Type_ResAnmTexSrt,
        GetResource(pManager, RPGrpModelResManager::Type_ResAnmTexSrt, file, "menu_BG_model"));
    RPGrpHandle clr = CreateResource(pManager, RPGrpModelResManager::Type_ResAnmClr,
        GetResource(pManager, RPGrpModelResManager::Type_ResAnmClr, file, "menu_BG_model"));

    mpModel = RPGrpModel::Construct(model, 1, 0, 0);
    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_TexSrt, 1);
    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);
    mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 0, chr, 0);
    mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_TexSrt, 0, texSrt, 0);
    mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Clr, 0, clr, 0);
    mpModel->SetDrawScene(mSceneNo);
}

void RPMenuBgModel::Reset() {
    mpCamera->SetPosition(nw4r::math::VEC3(0.0f, 0.0f, 65.0f));
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0);
    mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexSrt, 0);
    mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0);
    mpModel->GetModelAnm()->SetFrame(RPGrpModelAnm::Anm_Clr, 0, static_cast<u32>(mStartFrame));
    mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
}

void RPMenuBgModel::Calculate() {
    mpCamera->CalcMatrix();
    EGG::Matrix34f mtx;
    mtx.makeIdentity();
    mtx._03 = mPos.x;
    mtx._13 = mPos.y;
    mtx._23 = mPos.z;
    mpModel->GetModelEx()->setLocalMtx(&mtx);
    mpModel->SetBaseScale(mScale);
    mpModel->UpdateFrame();
    mpModel->Calc();
    mpModel->Entry();
}
