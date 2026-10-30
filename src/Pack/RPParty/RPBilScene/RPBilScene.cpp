// Billiards scene (RPBilScene): model scenes, screens, capture textures, views and lights, the
// resources and the scene sound; the hooks forward to the RPBilGameMgr singleton.
// Unit 0x802BA1A4-0x802BAB30 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPBilScene/RPBilScene.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpCaptureTexture.h>
#include <Pack/RPKernel/RPSysEffectCreator.h>
#include <Pack/RPParty/RPBilScene/RPBilCamera.h>
#include <Pack/RPParty/RPBilScene/RPBilGameMgr.h>
#include <Pack/RPParty/RPBilScene/RPBilPocketCamWindow.h>
#include <Pack/RPParty/RPBilScene/RPBilSound.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <egg/gfxe.h>

#include <cstdio>

// Wii Play: these shared entry points retain their original C symbol names.
extern "C" void RP_Cls803572E0_createInstance();

namespace {

inline void SetCaptureClearColor(RPGrpCaptureTexture* pCapture) {
    GXColor clear = {0, 0, 0, 0};
    pCapture->GetCapTexture()->setClearColor(clear);
}

inline void SetAmbientColor(EGG::LightManager* pLightMgr) {
    GXColor ambient = {0x91, 0xA0, 0x91, 0xFF};
    pLightMgr->GetAmbientColor(0) = ambient;
}

} // namespace

RPBilSceneSound::~RPBilSceneSound() {}

RPBilScene::~RPBilScene() {}

void RPBilSceneSound::initSound3D(s32 arg) {
    switch (arg) {
    case 0:
        // Wii Play: a never-read local, as in RPFshSceneSound::initSound3D
        f32 param[18];
        *(s32*)&param[12] = 0x20;
        param[1] = 1.0f;
        param[6] = 1.0f;
        param[11] = 1.0f;
        break;
    }
}

void RPBilScene::Configure() {
    RPSimpleModel::SetRandomSeed(OSGetTick());
    RPGrpModelResManager::GetCurrent()->CreateResourceList(RPCommon::GetResourceNum() + 0x18);

    RPBilGameMgr::CreateInstance();
    RP_Cls803572E0_createInstance();

    RPGrpModelManager* pModelMgr = RPGrpModelManager::GetCurrent();
    pModelMgr->CreateModelScene(0, 1, 1, NULL, NULL);
    pModelMgr->CreateModelScene(1, 2, 4, NULL, pModelMgr->GetModelScene(0)->GetFogManager());
    pModelMgr->CreateModelScene(2, 0, 1, pModelMgr->GetModelScene(0)->GetLightManager(),
                                pModelMgr->GetModelScene(0)->GetFogManager());
    pModelMgr->CreateModelScene(4, 0, 1, pModelMgr->GetModelScene(0)->GetLightManager(),
                                pModelMgr->GetModelScene(0)->GetFogManager());

    RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();
    RPGrpScreen* pScreen = new RPGrpScreen();
    pScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    RPGrpScreen* pScreen2D = new RPGrpScreen();
    pScreen2D->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    RPGrpScreen* pSubScreen =
        new RPGrpScreen(0.0f, 0.0f, RPBilPocketCamWindow::GetSubWindowWidth(),
                        RPBilPocketCamWindow::GetSubWindowHeight());
    pSubScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);

    mpMainCapture = new RPGrpCaptureTexture(pScreen2D, GX_TF_RGBA8);
    mpCapture48 = new RPGrpCaptureTexture(pScreen, GX_TF_RGBA8);
    mpMainCapture->SetEfbFlags(7);
    mpCapture48->SetEfbFlags(7);
    SetCaptureClearColor(mpMainCapture);
    SetCaptureClearColor(mpCapture48);
    for (int i = 0; i < 2; i++) {
        mpSubWindowCaptures[i] = new RPGrpCaptureTexture(pSubScreen, GX_TF_RGB565);
        mpSubWindowCaptures[i]->SetEfbFlags(7);
        SetCaptureClearColor(mpSubWindowCaptures[i]);
    }

    pRenderer->CreateView3D(0, RPBilCameraMgr::sInstance->GetCamera(), pScreen);
    pRenderer->CreateView3D(1, RPBilCameraMgr::sInstance->GetAuxCamera(), pScreen);
    for (int i = 0; i < RPBilCameraMgr::POCKET_CAM_NUM; i++) {
        pRenderer->CreateView3D(i + 2, RPBilCameraMgr::sInstance->GetPocketCam(i), pSubScreen);
    }
    pRenderer->CreateView3D(4, RPBilCameraMgr::sInstance->GetCamera(), pScreen);
    pRenderer->CreateView2D(5, pScreen);
    pRenderer->CreateView3D(6, RPBilCameraMgr::sInstance->GetCamera(), pScreen);
    pRenderer->CreateView2D(7, pScreen2D);
    pRenderer->CreateView2D(8, pScreen);
    pRenderer->CreateView2D(9, pScreen);
    pRenderer->CreateView3D(10, RPBilCameraMgr::sInstance->GetCamera(), pScreen);
    pRenderer->LinkViews();

    pRenderer->GetView(0)->SetDrawScene(0);
    pRenderer->GetView(1)->SetDrawScene(1);
    pRenderer->GetView(2)->SetDrawScene(1);
    pRenderer->GetView(3)->SetDrawScene(1);
    pRenderer->GetView(4)->SetDrawScene(1);
    pRenderer->GetView(6)->SetDrawScene(2);
    pRenderer->GetView(9)->SetDrawScene(3);
    pRenderer->GetView(10)->SetDrawScene(4);
    pRenderer->GetView(6)->SetFlag(1 << 3);
    pRenderer->GetView(10)->SetFlag(1 << 3);
    pRenderer->GetView(2)->SetFlag(1 << 5);
    pRenderer->GetView(3)->SetFlag(1 << 5);
    pRenderer->GetView(4)->SetFlag(1 << 5);
    pRenderer->GetView(9)->CreateScreenEffect();

    for (int i = 0; i < 5; i++) {
        if (i == 0 || i == 1) {
            EGG::LightManager* pLightMgr = pModelMgr->GetLightManager(i);
            SetAmbientColor(pLightMgr);

            EGG::LightObject* pLight = pLightMgr->GetLightObj(0);
            pLight->SetAt(EGG::Vector3f::zero);
            pLight->CalcDirDist();

            if (i == 1) {
                pLight = pLightMgr->GetLightObj(1);
                pLight->Enable();
                pLight->SetAt(EGG::Vector3f::zero);
                pLight->CalcDirDist();
                pLight->SetPos(nw4r::math::VEC3(-5000.0f, 10000.0f, 5000.0f));
                pLight->CalcDirDist();
            }
        }
    }

    RPSndAudioMgr::instance()->configureMiniGame(1);
    RPSndAudioMgr::instance()->setPlayerNum(RPSceneInfo::spInstance->GetPlayerNum());

    mpSound = new RPBilSceneSound();
    mpSound->initSound3D(0);
}

void RPBilScene::LoadResource() {
    RPCls803572E0::GetInstance()->LoadMessage("party_message.bmg");

    const char* brresNames[8] = {"bad_table",  "bad_cue",           "bad_ball",
                                 "bad_ball_2", "bad_ball_0_face_2", "bad_next_ball",
                                 "bad_cursor", "bad_arrow"};

    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    for (int i = 0; i < 8; i++) {
        char path[64];
        snprintf(path, sizeof(path), "G3D/%s.brres", brresNames[i]);
        pResMgr->CreateDataAt(i, RPGrpModelResManager::Type_ResFile,
                              RPSysResourceManager::GetFileFromArchive(pArchive, path));
    }

    RPSndAudioMgr::instance()->attachArchive("rp_Bil_sound.brsar", false, NULL);
    RPSndAudioMgr::instance()->loadGroup(0U);

    void* pEffect = RPSysResourceManager::GetFileFromArchive(pArchive, "basic_billiards.breff");
    void* pTexture = RPSysResourceManager::GetFileFromArchive(pArchive, "basic_billiards.breft");
    getEffectCreator()->setEffectResource(pEffect, pTexture);

    RPBilGameMgr::sInstance->Load();
}

void RPBilScene::Reset() {
    RPBilGameMgr::sInstance->Reset();
}

void RPBilScene::Calculate() {
    RPBilGameMgr::sInstance->Calculate();
}

void RPBilScene::Exit() {
    RPBilGameMgr::sInstance->Exit();
    RPBilGameMgr::DestroyInstance();
    RPCls803572E0::DestroyInstance();
    RPSndAudioMgr::instance()->changeScene();
}

void RPBilScene::pauseCallBack(bool enter) {
    RPBilGameMgr::sInstance->SetCursorsVisible(enter);
}
