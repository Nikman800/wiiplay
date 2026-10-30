// Pose Mii scene (RPBomScene): model scenes, screens and views, the Bom singletons, resources
// and the scene sound; one object pass per frame, then the model updates.
// Unit 0x802D3940-0x802D4914 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPBomScene/RPBomScene.h>

#include <Pack/RPParty/RPBomScene/RPBomBorder.h>
#include <Pack/RPParty/RPBomScene/RPBomCamera.h>
#include <Pack/RPParty/RPBomScene/RPBomCursor.h>
#include <Pack/RPParty/RPBomScene/RPBomGameManager.h>
#include <Pack/RPParty/RPBomScene/RPBomLyt.h>
#include <Pack/RPParty/RPBomScene/RPBomPanel.h>
#include <Pack/RPParty/RPBomScene/RPBomPlayer.h>
#include <Pack/RPParty/RPBomScene/RPBomScreenFx.h>
#include <Pack/RPParty/RPBomScene/RPBomStartBg.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPResSlots.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPScreenFade.h>
#include <Pack/RPGraphics/RPGrpDrawPathBloom.h>
#include <Pack/RPGraphics/RPGrpDrawPathLightMap.h>
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpLightManager.h>
#include <Pack/RPGraphics/RPGrpModelManager.h>
#include <Pack/RPGraphics/RPGrpModelResManager.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPKernel/RPSysKokeshiManager.h>
#include <Pack/RPKernel/RPSysTextWriter.h>

// These out-of-line entry points have no declarations in the shared headers.
extern "C" {
void Bom_allocShuffleBuffers();
void Bom_updateScreenSize();
void RP_ResSlots_createInstance();
void RP_ResSlots_init(RPResSlots* slots, u16 num);
extern RPResSlots* RP_resSlotsInstance;
void RP_ModelObj_setDrawGroup(u32 group);
void RP_seedRandomTicks();
}

static nw4r::math::VEC3 sSceneLightPos(RPBomScene::sZero, 10000.0f, RPBomScene::sZero);

RPBomSoundHolder::~RPBomSoundHolder() {}

RPBomScene::~RPBomScene() {}

void RPBomScene::CreateCameras() {
    RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();
    RPBomCamera::CreateInstance();
    mpCamera = RPBomCamera::sInstance;
    mpCamera->Reset();

    mpScreen3D = new RPGrpScreen(sZero, sZero, 640.0f, 528.0f);
    mpScreen3D->SetVpSizeAbsolute();
    mpScreen3D->SetCanvasMode(EGG::Screen::CANVAS_CC);
    pRenderer->CreateView3D(0, mpCamera, mpScreen3D);
    RPGrpView* pView = pRenderer->GetView(0);
    pView->SetProjType(RPGrpView::EProjType_Ortho);
    pView->SetDrawScene(1);

    mpScreen3D2 = new RPGrpScreen;
    mpScreen3D2->SetCanvasMode(EGG::Screen::CANVAS_CC);
    pRenderer->CreateView3D(1, mpCamera, mpScreen3D2);
    pView = pRenderer->GetView(1);
    pView->SetProjType(RPGrpView::EProjType_Ortho);
    pView->SetDrawScene(2);
    pRenderer->CreateView3D(2, mpCamera, mpScreen3D2);
    pView = pRenderer->GetView(2);
    pView->SetProjType(RPGrpView::EProjType_Ortho);
    pView->SetDrawScene(0);

    mpScreen2D = new RPGrpScreen;
    mpScreen2D->SetCanvasMode(EGG::Screen::CANVAS_CC);
    pRenderer->CreateView2D(3, mpScreen2D);
    pView = pRenderer->GetView(3);
    pView->SetDrawScene(3);
    pView->CreateScreenEffect();
    pRenderer->CreateView2D(4, mpScreen2D);
    pRenderer->GetView(4)->SetDrawScene(4);
    pRenderer->CreateView3D(5, mpCamera, mpScreen3D2);
    pView = pRenderer->GetView(5);
    pView->SetProjType(RPGrpView::EProjType_Ortho);
    pView->SetDrawScene(5);
    pView->SetFlag(1 << 3);
    pRenderer->LinkViews();
}

#pragma auto_inline off
void RPBomScene::LoadLightMap() {
    EGG::Archive* archive = RPSysResourceManager::LoadGameCommonArchive(-1, NULL);
    RPGrpDrawPathManager* paths = RPGrpModelManager::GetCurrent()->GetDrawPathManager(0);
    RPSysKokeshiManager::CreateLightTexture(0);
    paths->GetDrawPathLightMap()->GetLightTextureManager()->onFlag2();

    void* binary = RPSysResourceManager::GetFileFromArchive(
        archive, "graphic/scene_Light.plight", NULL);
    RPGrpModelManager::GetCurrent()->GetLightManager(0)->SetBinary(binary);
    binary = RPSysResourceManager::GetFileFromArchive(
        archive, "graphic/scene_Lm.plmap", NULL);
    paths->GetDrawPathLightMap()->GetLightTextureManager()->SetBinary(binary);
}

void RPBomScene::CalcObjects() {
    mObjPassRun = 1;
    mObjPassNum++;
    mpCamera->Calculate();
    mpCursorMgr->Calculate();
    mpPlayerMgr->CalcState();
    mpPanelCtrl->Move();
    mpPanelCtrl->PreCalc();
    mpPanelCtrl->CalcHit();
    mpPanelCtrl->PostCalc();
    mpPlayerMgr->CalcMove();
    mpPanelCtrl->CalcItems();
    mpStartBg->Calculate();
    mpBorder->Calculate();
    mpScreenFx->Calculate();
    mpGameMgr->Calculate();
    mpPanelCtrl->IncrementAges();
}
#pragma auto_inline reset

void RPBomSoundHolder::initSound3D(s32 arg) {
    // Wii Play: a never-read local, as in RPPnpSceneSound::initSound3D
    struct {
        f32 value[28];
        s32 priority[4];
    } param;
    switch (arg) {
    case 0:
        param.priority[0] = 32;
        param.value[1] = RPBomScene::sOne;
        param.value[6] = RPBomScene::sOne;
        param.value[11] = RPBomScene::sOne;
        // NONMATCHING-DEBT: word 12 of the unknown param type holds an integer. A struct with an
        // s32 member there (f32[12], s32, f32[15]) changes the frame layout (0x410-byte frame).
        reinterpret_cast<s32&>(param.value[12]) = 32;
        param.value[17] = 5000.0f;
        param.value[22] = 2500.0f;
        param.value[27] = 2500.0f;
        break;
    }
}

void RPBomScene::Configure() {
    Bom_allocShuffleBuffers();
    RP_ModelObj_setDrawGroup(0);
    RP_ResSlots_createInstance();
    mpResSlots = RP_resSlotsInstance;
    RP_ResSlots_init(mpResSlots, 100);

    RPGrpModelManager* models = RPGrpModelManager::GetCurrent();
    models->CreateModelScene(0, 0x21, 1, NULL, NULL);
    models->CreateModelScene(1, 2, 1, NULL, NULL);
    models->CreateModelScene(2, 0x20, 1, NULL, NULL);
    models->CreateModelScene(3, 0, 1, NULL, NULL);
    models->CreateModelScene(4, 0, 1, NULL, NULL);
    models->CreateModelScene(5, 0, 1, NULL, NULL);

    RPCommonParam commonParam;
    commonParam.mGame = 3;
    commonParam.mPlayerNum = RPSceneInfo::spInstance->GetPlayerNum();
    RPCommon::CreateInstance(commonParam);
    mpCommon = RPCommon::GetInstance();
    CreateCameras();

    RPBomStartBg::CreateInstance();
    mpStartBg = RPBomStartBg::sInstance;
    mpStartBg->Create();
    RPBomBorder::CreateInstance();
    mpBorder = RPBomBorder::sInstance;
    mpBorder->Create();
    RPBomScreenFx::CreateInstance();
    mpScreenFx = RPBomScreenFx::sInstance;
    mpScreenFx->Create();
    RPBomPlayerManager::CreateInstance();
    mpPlayerMgr = RPBomPlayerManager::sInstance;
    mpPlayerMgr->CreatePlayers(2);
    RPBomPanelCtrl::CreateInstance();
    mpPanelCtrl = RPBomPanelCtrl::sInstance;
    mpPanelCtrl->Create();
    RPBomCursorManager::CreateInstance();
    mpCursorMgr = RPBomCursorManager::sInstance;
    mpCursorMgr->CreateCursors(2);
    RPBomLayoutMgr::CreateInstance();
    mpLayoutMgr = RPBomLayoutMgr::sInstance;
    mpLayoutMgr->Create();
    RPBomGameManager::CreateInstance();
    mpGameMgr = RPBomGameManager::sInstance;
    mpGameMgr->Create();

    RPSndAudioMgr::instance()->configureMiniGame(1);
    mpSound = new RPBomSoundHolder;
    mpSound->initSound3D(0);
    RPSndAudioMgr::instance()->setPlayerNum(RPSceneInfo::spInstance->GetPlayerNum());
}

void RPBomScene::LoadResource() {
    EGG::Archive* archive = RPSysResourceManager::LoadGameCommonArchive(-1, NULL);
    RPGrpModelResManager* resources = RPGrpModelResManager::GetCurrent();
    void* binary = RPSysResourceManager::GetFileFromArchive(
        archive, "G3D/sceneFile.brres", NULL);
    RPGrpHandle handle = resources->CreateData(RPGrpModelResManager::Type_ResFile, binary, 0);
    EGG::LightManager* light = RPGrpModelManager::GetCurrent()->GetLightManager(0);
    static_cast<RPGrpLightManager*>(light)->LoadScnLight(handle, "RPScene", sZero, -1);
    EGG::LightObject* lightObj = light->GetLightObj(0);
    lightObj->SetPos(sSceneLightPos);
    lightObj->CalcDirDist();

    archive = RPSysResourceManager::LoadGameCommonArchive(-1, NULL);
    binary = RPSysResourceManager::GetFileFromArchive(
        archive, "graphic/scene_Bloom.pblm", NULL);
    RPGrpModelManager::GetCurrent()->GetDrawPathManager(0)->GetDrawPathBloom()->GetBloom()->SetBinary(binary);
    LoadLightMap();
    archive = RPSysResourceManager::LoadGameCommonArchive(-1, NULL);
    mpStartBg->Load();
    mpBorder->Load();
    mpScreenFx->Load();
    mpPlayerMgr->Load();
    mpPanelCtrl->Load();
    mpCursorMgr->Load();
    mpLayoutMgr->Load();
    mpGameMgr->Load();
    mpCommon->LoadResources();

    void* effect = RPSysResourceManager::GetFileFromArchive(archive, "basic_bomb.breff", NULL);
    void* texture = RPSysResourceManager::GetFileFromArchive(archive, "basic_bomb.breft", NULL);
    getEffectCreator()->setEffectResource(effect, texture);
    RPSndAudioMgr::instance()->attachArchive("rp_Bom_sound.brsar", false, NULL);
    RPSndAudioMgr::instance()->loadGroup(0U, NULL, 0);

    mPlayerNum = 1;
    if (RPSceneInfo::spInstance->GetPlayerNum() == 1) {
        mPlayerNum = 1;
    } else if (RPSceneInfo::spInstance->GetPlayerNum() == 2) {
        mPlayerNum = 2;
    }
}

void RPBomScene::Reset() {
    RP_seedRandomTicks();
    Bom_updateScreenSize();
    mObjPassRun = 0;
    mObjPassNum = 0;
    mpGameMgr->Reset(mPlayerNum);
    mpCommon->Reset();
    mpCamera->SetMode(1);
    RPSndAudioMgr* audio = RPSndAudioMgr::instance();
    nw4r::math::MTX34 view = mpCamera->GetViewMatrix();
    if (audio->getCameraCount() > 0) {
        audio->setListenerMatrix(0, view);
    }
    mpStartBg->Reset();
    mpBorder->Reset();
    mpScreenFx->Reset();
    mpScreenFx->SetAlpha(0.92f);
    mpPlayerMgr->Reset();
    mpPanelCtrl->Reset();
    mpCursorMgr->Reset();
    mpLayoutMgr->Reset();

    RPGrpView::ScreenEffect* effect = RPGrpRenderer::GetCurrent()->GetView(3)->GetScreenEffect();
    GXColor color = {255, 255, 255, 0};
    effect->SetColorMultiply(color);
    effect->SetBlurAlpha(0);
    effect->SetBlurOffset(sOne);
    for (int i = 0; i < 4; ++i) {
        EGG::CoreControllerMgr::instance()->getNthController(i)->setPosParam(0.06f, sOne);
        EGG::CoreControllerMgr::instance()->getNthController(i)->setHoriParam(0.01f, sOne);
    }
    RPSysTextWriter::instance()->SetDrawFlag(0);
    mpGameMgr->Start();
}

// Defined after the code that reads them: loaded from memory, ahead of the literal pool.
const f32 RPBomScene::sOne = 1.0f;
const f32 RPBomScene::sZero = 0.0f;

void RPBomScene::Calculate() {
    EGG::CoreControllerMgr::instance()->getNthController(0);
    EGG::CoreControllerMgr::instance()->func_800A6F50(0);
    CalcObjects();
    mpCamera->CalcMatrix();
    mpStartBg->CalcModelMtx();
    mpBorder->CalcModelMtx();
    mpPlayerMgr->UpdateModels();
    mpPanelCtrl->UpdateModels();
    mpLayoutMgr->Calculate();
    mpCommon->Calculate();

    RPGrpView::ScreenEffect* effect = RPGrpRenderer::GetCurrent()->GetView(3)->GetScreenEffect();
    GXColor color = RPCommon::GetInstance()->GetScreenFade()->GetColor();
    effect->SetColorMultiply(color);
    effect->SetBlurAlpha(color.a);

    if (mpGameMgr->mQuit) {
        RPSysSceneCreator::instance()->changeSceneAfterFade(0x1B, false);
    } else if (mpGameMgr->mRetry) {
        RPSysSceneCreator::instance()->changeSceneAfterFade(0x18, false);
    }
}

void RPBomScene::Exit() {
    RPSndAudioMgr::instance()->changeScene();
    RPResSlots::DestroyInstance();
    mpStartBg->Exit();
    RPBomStartBg::DestroyInstance();
    mpBorder->Exit();
    RPBomBorder::DestroyInstance();
    mpScreenFx->Exit();
    RPBomScreenFx::DestroyInstance();
    mpPlayerMgr->Exit();
    RPBomPlayerManager::DestroyInstance();
    mpPanelCtrl->Exit();
    RPBomPanelCtrl::DestroyInstance();
    mpCursorMgr->Exit();
    RPBomCursorManager::DestroyInstance();
    mpLayoutMgr->Exit();
    RPBomLayoutMgr::DestroyInstance();
    mpGameMgr->Exit();
    RPBomGameManager::DestroyInstance();
    RPCommon::DestroyInstance();
    RPBomCamera::DestroyInstance();
}

void RPBomScene::UserDraw() {
    u8 viewNo = RPGrpRenderer::GetCurrentViewNo();
    if (viewNo == 0) {
        if (RPGrpRenderer::GetDrawStep() == 3) {
            mpScreenFx->CaptureState();
        }
    } else if (viewNo == 2) {
        if (RPGrpRenderer::GetDrawStep() == 3) {
            mpScreenFx->DrawState();
            mpGameMgr->DrawAfterScreenFx();
        }
    } else if (viewNo == 3) {
        if (RPGrpRenderer::GetDrawStep() == 3) {
            mpLayoutMgr->DrawLayouts();
            mpGameMgr->DrawAfterLayouts();
        }
    }
}

void RPBomScene::pauseCallBack(bool enter) {}
