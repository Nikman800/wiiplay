#include <Pack/RPParty/RPDucScene/RPDucScene.h>

#include <Pack/RPParty/RPDucScene/RPDucCamera.h>
#include <Pack/RPParty/RPDucScene/RPDucField.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPParty/RPDucScene/RPDucHud.h>
#include <Pack/RPParty/RPDucScene/RPDucShot.h>
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPMiiIconSet.h>
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
void RP_ResSlots_createInstance();
void RP_ResSlots_init(RPResSlots* slots, u16 num);
extern RPResSlots* RP_resSlotsInstance;
void RP_ModelObj_setDrawGroup(u32 group);
void RP_seedRandomTicks();
}

static nw4r::math::VEC3 sSceneLightPos(19000.0f, 10000.0f, 10000.0f);

RPDucSceneSound::~RPDucSceneSound() {}

RPDucScene::~RPDucScene() {}

void RPDucScene::CreateCameras() {
    RPGrpRenderer* renderer = RPGrpRenderer::GetCurrent();
    RPDucCamera::CreateInstance();
    mpCamera = RPDucCamera::sInstance;
    mpCamera->Reset();

    mpScreen3D = new RPGrpScreen;
    mpScreen3D->SetFovy(30.0f);
    mpScreen3D->SetCanvasMode(EGG::Screen::CANVAS_CC);
    renderer->CreateView3D(0, mpCamera, mpScreen3D);

    mpScreen2D = new RPGrpScreen;
    mpScreen2D->SetCanvasMode(EGG::Screen::CANVAS_CC);
    renderer->CreateView2D(1, mpScreen2D);
    RPGrpView* view = renderer->GetView(1);
    view->SetDrawScene(1);
    view->CreateScreenEffect();
    renderer->CreateView2D(2, mpScreen2D);
    renderer->GetView(2)->SetDrawScene(2);
    renderer->CreateView3D(3, mpCamera, mpScreen3D);
    renderer->GetView(3)->SetDrawScene(3);
    renderer->LinkViews();
}

#pragma auto_inline off
void RPDucScene::LoadLight() {
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
#pragma auto_inline reset

void RPDucScene::CalcSubsteps() {
    mpTargetRegistry->ClearHitRecords();
    mpShotMgr->ProcessHitsAll();
    mSubstep = 0;
    do {
        mpTargetRegistry->ApplyVelocityAll();
        mpTargetRegistry->ClearForcesHookAll();
        mpTargetRegistry->VF_0x80All();
        mpTargetRegistry->ApplyForcesHookAll();
        mpTargetRegistry->CalculateAll();
        mpTargetRegistry->IncAgeAll();
        mpTargetRegistry->ClearHitRecords();
        ++mSubstepTotal;
    } while (++mSubstep < 3);
    mpCamera->Calculate();
    mpShotMgr->Calculate();
    mpField->Calculate();
    mpGame->Calculate();
}

void RPDucSceneSound::initSound3D(s32 arg) {
    switch (arg) {
    case 0: {
        EGG::TAudioSound3DMgr<4>::TAudioSound3DMgrArg param;
        param.maxPrioReduction = 32;
        param.interiorSize = 2400.0f;
        param.maxVolumeDistance = 80.0f;
        param.unitDistance = 1200.0f;
        mpAudioMgr->setParam3D(&param);
        break;
    }
    }
}

void RPDucScene::Configure() {
    mPlayMode = 1;
    if (RPSceneInfo::spInstance->GetPlayerNum() == 1) {
        mPlayMode = 3;
    } else if (RPSceneInfo::spInstance->GetPlayerNum() == 2) {
        mPlayMode = 2;
    } else {
        mPlayMode = 1;
    }

    RP_ModelObj_setDrawGroup(0);
    RP_ResSlots_createInstance();
    mpResSlots = RP_resSlotsInstance;
    RP_ResSlots_init(mpResSlots, 120);
    RPMiiIconSet::CreateInstance();
    mpMiiIconSet = RPMiiIconSet::sInstance;
    RPMiiIconSet::Param iconParam;
    iconParam.mNum = 2;
    iconParam.mWidth = 128;
    iconParam.mHeight = 128;
    mpMiiIconSet->CreateIcons(iconParam);

    RPGrpModelManager* models = RPGrpModelManager::GetCurrent();
    models->CreateModelScene(0, 0x21, 1, NULL, NULL);
    models->CreateModelScene(1, 0, 1, NULL, NULL);
    models->CreateModelScene(2, 0, 1, NULL, NULL);
    models->CreateModelScene(3, 0, 1, NULL, NULL);

    RPCommonParam commonParam;
    commonParam.mGame = 0;
    commonParam.mPlayerNum = RPSceneInfo::spInstance->GetPlayerNum();
    RPCommon::CreateInstance(commonParam);
    mpPartyCommon = RPCommon::GetInstance();
    CreateCameras();

    RPDucField::CreateInstance();
    mpField = RPDucField::sInstance;
    mpField->Create();
    RPDucTargetRegistry::CreateInstance();
    mpTargetRegistry = RPDucTargetRegistry::sInstance;
    mpTargetRegistry->AddManager(0, 16);
    mpTargetRegistry->AddManager(1, 8);
    mpTargetRegistry->AddManager(2, 24);
    mpTargetRegistry->AddManager(3, 18);
    mpTargetRegistry->AddManager(4, 10);
    mpTargetRegistry->AddManager(5, 8);
    mpTargetRegistry->AddManager(6, 8);
    RPDucShotMgr::CreateInstance();
    mpShotMgr = RPDucShotMgr::sInstance;
    mpShotMgr->CreateShots(2);
    RPDucHud::CreateInstance();
    mpHud = RPDucHud::sInstance;
    mpHud->Create();
    RPDucGame::CreateInstance();
    mpGame = RPDucGame::sInstance;
    mpGame->Create();
    mSubstep = 0;
    mSubstepTotal = 0;
    RPSndAudioMgr::instance()->configureMiniGame(1);
    mpSound = new RPDucSceneSound;
    mpSound->initSound3D(0);
    RPSndAudioMgr::instance()->setPlayerNum(2);
}

void RPDucScene::LoadResource() {
    EGG::Archive* archive = RPSysResourceManager::LoadGameCommonArchive(-1, NULL);
    RPGrpModelResManager* resources = RPGrpModelResManager::GetCurrent();
    void* binary = RPSysResourceManager::GetFileFromArchive(
        archive, "G3D/sceneFile.brres", NULL);
    RPGrpHandle handle = resources->CreateData(RPGrpModelResManager::Type_ResFile, binary, 0);
    EGG::LightManager* light = RPGrpModelManager::GetCurrent()->GetLightManager(0);
    static_cast<RPGrpLightManager*>(light)->LoadScnLight(handle, "RPScene", sLightFrame, -1);
    EGG::LightObject* lightObj = light->GetLightObj(0);
    lightObj->SetPos(sSceneLightPos);
    lightObj->CalcDirDist();

    archive = RPSysResourceManager::LoadGameCommonArchive(-1, NULL);
    binary = RPSysResourceManager::GetFileFromArchive(
        archive, "graphic/scene_Bloom.pblm", NULL);
    RPGrpModelManager::GetCurrent()->GetDrawPathManager(0)->GetDrawPathBloom()->GetBloom()->SetBinary(binary);
    LoadLight();
    mpMiiIconSet->Load();
    archive = RPSysResourceManager::LoadGameCommonArchive(-1, NULL);
    mpField->Load();
    mpTargetRegistry->Load();
    mpShotMgr->Load();
    mpHud->Load();
    mpGame->Load();
    mpPartyCommon->LoadResources();

    void* effect = RPSysResourceManager::GetFileFromArchive(
        archive, "basic_shooting.breff", NULL);
    void* texture = RPSysResourceManager::GetFileFromArchive(
        archive, "basic_shooting.breft", NULL);
    getEffectCreator()->setEffectResource(effect, texture);
    RPSndAudioMgr::instance()->attachArchive("rp_Duc_sound.brsar", false, NULL);
    RPSndAudioMgr::instance()->loadGroup(0U, NULL, 0);
}

void RPDucScene::Reset() {
    RP_seedRandomTicks();
    mpCamera->SetMode(1);
    RPSndAudioMgr* audio = RPSndAudioMgr::instance();
    nw4r::math::MTX34 view = mpCamera->GetViewMatrix();
    if (audio->getCameraCount() > 0) {
        audio->setListenerMatrix(0, view);
    }
    mpGame->Setup(mPlayMode);
    mpPartyCommon->Reset();
    mpField->Reset();
    mpTargetRegistry->Reset();
    mpShotMgr->Reset();
    mpHud->Reset();
    mSubstep = 0;
    mSubstepTotal = 0;

    RPGrpView::ScreenEffect* effect = RPGrpRenderer::GetCurrent()->GetView(1)->GetScreenEffect();
    GXColor color = {255, 255, 255, 0};
    effect->SetColorMultiply(color);
    effect->SetBlurAlpha(0);
    effect->SetBlurOffset(1.0f);
    for (int i = 0; i < 4; ++i) {
        EGG::CoreControllerMgr::instance()->getNthController(i)->setPosParam(0.06f, 1.0f);
    }
    RPSysTextWriter::instance()->SetDrawFlag(0);
    mpGame->ResetGame();
}

const f32 RPDucScene::sLightFrame = 0.0f;

void RPDucScene::Calculate() {
    EGG::CoreControllerMgr::instance()->getNthController(0);
    EGG::CoreControllerMgr::instance()->func_800A6F50(0);
    CalcSubsteps();
    mpCamera->CalcMatrix();
    mpField->CalcModelMtx();
    mpTargetRegistry->CalcModelMtxAll();
    mpHud->Calculate();
    mpPartyCommon->Calculate();

    RPGrpView::ScreenEffect* effect = RPGrpRenderer::GetCurrent()->GetView(1)->GetScreenEffect();
    GXColor color = RPCommon::GetInstance()->GetScreenFade()->GetColor();
    effect->SetColorMultiply(color);
    effect->SetBlurAlpha(color.a);

    if (mpGame->IsRetry()) {
        RPSysSceneCreator::instance()->changeSceneAfterFade(0x1B, false);
    } else if (mpGame->IsQuit()) {
        RPSysSceneCreator::instance()->changeSceneAfterFade(0x12, false);
    }
}

void RPDucScene::Exit() {
    RPSndAudioMgr::instance()->changeScene();
    RPResSlots::DestroyInstance();
    RPMiiIconSet::DestroyInstance();
    mpField->Exit();
    RPDucField::DestroyInstance();
    mpTargetRegistry->DestroyManagers();
    RPDucTargetRegistry::DestroyInstance();
    mpShotMgr->Exit();
    RPDucShotMgr::DestroyInstance();
    RPDucCamera::DestroyInstance();
    mpHud->Exit();
    RPDucHud::DestroyInstance();
    mpGame->Exit();
    RPDucGame::DestroyInstance();
    RPCommon::DestroyInstance();
}

void RPDucScene::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() == 0) {
        return;
    }
    if (RPGrpRenderer::GetCurrentViewNo() == 1) {
        if (RPGrpRenderer::GetDrawStep() == 5) {
            mpHud->DrawLayouts();
        }
    }
}

void RPDucScene::pauseCallBack(bool enter) {
    if (enter) {
        mpShotMgr->SetCursorModeAll(0);
    } else if (mpGame->GetMode() == 2 && !mpGame->IsP2Joined()) {
        if (mpGame->GetState() == 6 || mpGame->GetState() == 7) {
            mpShotMgr->SetCursorMode(0, 0);
        } else if (mpGame->GetState() == 1) {
            mpShotMgr->SetCursorMode(0, 1);
        } else {
            mpShotMgr->SetCursorMode(0, 2);
        }
    } else if (mpGame->GetState() == 6 || mpGame->GetState() == 7) {
        mpShotMgr->SetCursorModeAll(0);
    } else if (mpGame->GetState() == 1) {
        mpShotMgr->SetCursorModeAll(1);
    } else {
        mpShotMgr->SetCursorModeAll(2);
    }
}
