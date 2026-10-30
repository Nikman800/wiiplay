// The calls into the RPStateMachine<RPSelectScene> instances below are not inlined.
#pragma auto_inline off

#include <Pack/RPParty/RPPartyCommon/RPAvatarScene.h>
#include <Pack/RPParty/RPPartyCommon/RPCrowdMii.h>
#include <Pack/RPParty/RPPartyCommon/RPDoll.h>
#include <Pack/RPParty/RPPartyCommon/RPPtMenuBg.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPSelectScene.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPSystem/RPSysSceneCreator.h>

#include <cstdio>


namespace {
inline void LoadSceneLightMap(void* pData) {
    RPGrpDrawPathLightMap* pMap = RPGrpModelManager::GetCurrent()->GetDrawPathManager(1)
                                    ->GetDrawPathLightMap();
    pMap->GetLightTextureManager()->SetBinary(pData);
    pMap->GetLightTextureManager()->onFlag2();
}

inline RPPtMenuBg* GetMenuBackground() {
    return RPPtMenuBg::sInstance;
}

}


// RPStateMachine<RPSelectScene>::ChangeState and IsStateSettled are instantiated here in the
// original; RPSelectScene's own copies are dropped by the linker.
template void RPStateMachine<RPSelectScene>::ChangeState(s32);
template bool RPStateMachine<RPSelectScene>::IsStateSettled(s32) const;

RPAvatarScene::~RPAvatarScene() {}

void RPAvatarScene::Configure() {
    RPGrpModelManager* pModels = RPGrpModelManager::GetCurrent();
    RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();
    RPGrpModelResManager* pResources = RPGrpModelResManager::GetCurrent();
    pModels->CreateModelScene(0, 0, 1, NULL, NULL);
    pModels->CreateModelScene(1, 5, 1, NULL, NULL);

    s32 count = RPSelectScene::GetResourceNum();
    count += RPDollMgr::GetResourceNum();
    s32 total = count + RPCrowdMgr::GetResourceNum();
    pResources->CreateResourceList(total + 4);
    RPPtMenuBg::CreateInstance(0, 0, false);

    RPGrpCamera* pCamera = new RPGrpCamera;
    pCamera->SetPosition(EGG::Vector3f::zero);
    pCamera->SetTarget(nw4r::math::VEC3(0.0f, 0.0f, -1.0f));
    pCamera->SetUp(nw4r::math::VEC3(0.0f, 1.0f, 0.0f));
    pCamera->SetCameraType(RPGrpCamera::ECameraType_LookAt);
    pCamera->CalcMatrix();

    RPGrpScreen* pScreen = new RPGrpScreen;
    pScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    pRenderer->CreateView2D(1, pScreen);
    pRenderer->CreateView3D(2, pCamera, pScreen);
    pRenderer->CreateView2D(3, pScreen);
    pRenderer->LinkViews();
    pRenderer->GetView(2)->SetDrawScene(1);
    pRenderer->GetView(2)->SetFlag(8);
    pRenderer->GetView(3)->SetDrawScene(2);
    RPSelectScene::CreateInstance();
}

void RPAvatarScene::LoadResource() {
    void* pLightMap;
    const char* names[] = {"sceneFile", "avatar_anim"};
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResources = RPGrpModelResManager::GetCurrent();
    for (int i = 0; i < 2; i++) {
        char path[64];
        std::snprintf(path, sizeof(path), "G3D/%s.brres", names[i]);
        void* pData = RPSysResourceManager::GetFileFromArchive(pArchive, path, NULL);
        pResources->CreateDataAt(i, RPGrpModelResManager::Type_ResFile, pData, 0);
    }

    static_cast<RPGrpLightManager*>(RPGrpModelManager::GetCurrent()->GetModelScene(1)
                                       ->GetLightManager())->LoadScnLight(0, "RPScene", 0.0f, -1);
    RPGrpDrawPathXluSnap* pXlu = RPGrpModelManager::GetCurrent()->GetDrawPathManager(1)
                                   ->GetDrawPathXluSnap();
    pXlu->GetXluSnap()->offFlag(4);
    pXlu->GetXluSnap()->onFlag(2);
    pXlu->GetXluSnap()->onFlag(1);
    RPSysKokeshiManager::CreateLightTexture(1);

    void* pLight = RPSysResourceManager::GetFileFromArchive(
        pArchive, "Environment/scene_Light.plight", NULL);
    RPGrpModelManager::GetCurrent()->GetLightManager(1)->SetBinary(pLight);
    pLightMap = RPSysResourceManager::GetFileFromArchive(
        pArchive, "Environment/scene_Lm.plmap", NULL);
    LoadSceneLightMap(pLightMap);
    void* pEffect = RPSysResourceManager::GetFileFromArchive(
        pArchive, "basic_selectmenu.breff", NULL);
    void* pTexture = RPSysResourceManager::GetFileFromArchive(
        pArchive, "basic_selectmenu.breft", NULL);
    getEffectCreator()->setEffectResource(pEffect, pTexture);

    RPPtMenuBg* pBackground = GetMenuBackground();
    pBackground->Load(RPSysResourceManager::LoadGameCommonArchive());
    RPSelectScene::GetInstance()->Load();
}

void RPAvatarScene::Reset() {
    RPSceneInfo* pInfo = RPSceneInfo::spInstance;
    pInfo->unk1C = 0;
    pInfo->unk20 = 0;
    pInfo->unk24 = 0;
    pInfo->unk2C = true;
    RPSysCursorDrawMgr::instance()->startDpdCheck();
    RPSysCursorDrawMgr::instance()->createActiveCursor();
    RPPtMenuBg::sInstance->Reset(pInfo->unk28);
    RPSelectScene::GetInstance()->Reset();
    RPSelectScene::GetInstance()->mpStateMachine->ChangeState(1);
}

void RPAvatarScene::Calculate() {
    RPPtMenuBg::sInstance->Calculate();
    RPSelectScene* pSelect = RPSelectScene::GetInstance();
    pSelect->Calculate();
    if (pSelect->mpStateMachine->IsStateSettled(15) && isSceneChangeEnable()) {
        if (pSelect->unk68 == 0) {
            s32 scenes[] = {18, 22, 19, 24, 17, 20, 16, 21, 23};
            RPSysSceneCreator::instance()->changeSceneAfterFade(
                scenes[pSelect->mCursor], false);
        } else {
            RPSysSceneCreator::instance()->changeSceneAfterFade(1, false);
        }
    }
}

void RPAvatarScene::UserDraw() {}

void RPAvatarScene::Exit() {
    RPPtMenuBg::DestroyInstance();
    RPSelectScene::DestroyInstance();
}
