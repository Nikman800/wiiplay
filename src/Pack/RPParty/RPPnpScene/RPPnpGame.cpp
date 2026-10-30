// Table Tennis game manager: owns the court objects, players, phases and views; runs the states.
// Unit 0x8022C8D0-0x8022DE10 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpDrawPathBloom.h>
#include <Pack/RPGraphics/RPGrpDrawPathLightMap.h>
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpDrawPathShadowTexture.h>
#include <Pack/RPGraphics/RPGrpDrawPathSV.h>
#include <Pack/RPGraphics/RPGrpLightManager.h>
#include <Pack/RPGraphics/RPGrpModelMisc.h>
#include <Pack/RPGraphics/RPGrpShadowTexture.h>
#include <Pack/RPKernel/RPSysKokeshiManager.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPKernel/RPSysTextWriter.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPLeadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPScreenFade.h>
#include <Pack/RPParty/RPPnpScene/RPPnpAudience.h>
#include <Pack/RPParty/RPPnpScene/RPPnpBall.h>
#include <Pack/RPParty/RPPnpScene/RPPnpBgmCtrl.h>
#include <Pack/RPParty/RPPnpScene/RPPnpField.h>
#include <Pack/RPParty/RPPnpScene/RPPnpHud.h>
#include <Pack/RPParty/RPPnpScene/RPPnpLevelTable.h>
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>
#include <Pack/RPParty/RPPnpScene/RPPnpPhase.h>
#include <Pack/RPParty/RPPnpScene/RPPnpPlayer.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPSystem/RPSysSceneCreator.h>
#include <Pack/RPUtility/RPUtlRandom.h>

#include <algorithm>

#include <egg/core.h>
#include <egg/gfxe.h>

s32 RPPnpGame::sRallyLimit = 100;
f32 RPPnpGame::sCameraOffsetNear = -15.0f;
f32 RPPnpGame::sCameraOffsetFar = 15.0f;

s32 RPPnpGame::sRallyLimitMode;
RPPnpGame* RPPnpGame::sInstance;

// Wii Play: never read (Pnp_cameraPos)
static nw4r::math::VEC3 sCameraPos(-275.0f, 800.0f, 190.0f);

void RPPnpGame::CreateInstance() {
    sInstance = new RPPnpGame();
}

void RPPnpGame::CheckPointEnd() {
    if (mpPointPhase->GetStep() == 3) {
        if (mpPointPhase->IsGameOver()) {
            mState = STATE_RESULT;
            mpResultPhase->Start();
        } else {
            mRallyCount = mpBall->GetHitCount() / 2;
            if (!mIsMultiPlayer && mRallyCount != 0) {
                RPPnpHud::GetInstance()->GetCountLyt()->PlayOut();
                mState = STATE_RESULT;
                mpResultPhase->Start();
            } else {
                mState = STATE_SERVE;
                mpServePhase->Start();
                mpBall->Reset();
            }
        }
    }
}

void RPPnpGame::StepPhysics(BOOL ballInPlay) {
    if (ballInPlay) {
        mpBall->Integrate();
    }

    for (int i = 0; i < 2; i++) {
        mpPlayers[i]->UpdateInput();
    }

    if (ballInPlay) {
        mpField->CheckBallFloorHit();
        mpTable->HandleBallCollision();
        for (int i = 0; i < 2; i++) {
            mpPlayers[i]->CheckBallHit();
        }
    }

    mpField->Calculate();
    mpTable->Calculate();
    mpNetShadow->Calculate();
    mpShadowTexture->ResetList();

    if (ballInPlay) {
        mpBall->Calculate();
    }

    for (int i = 0; i < 2; i++) {
        mpPlayers[i]->Calculate();
    }

    RPPnpAudience::GetInstance()->Calculate();
    RPPnpHud::GetInstance()->Calculate();
}

RPPnpGame::RPPnpGame() {
    RPGrpModelResManager::GetCurrent()->CreateResourceList(0x100);

    RPGrpModelManager* pModelMgr = RPGrpModelManager::GetCurrent();
    pModelMgr->CreateModelScene(0, 0x2B, 1, NULL, NULL);
    pModelMgr->CreateModelScene(1, 0, 1, NULL, NULL);
    pModelMgr->CreateModelScene(2, 0, 1, NULL, NULL);
    pModelMgr->CreateModelScene(3, 0, 1, NULL, NULL);

    GXColor shadowColor = {0, 0, 0, 0x60};
    pModelMgr->GetDrawPathManager(0)->GetDrawPathSV()->GetShadowVolume()->SetShadowColor(
        shadowColor);

    RPCommonParam param;
    param.mGame = 2;
    param.mPlayerNum = RPSceneInfo::spInstance->GetPlayerNum();
    RPCommon::CreateInstance(param);

    RPPnpLevelTable::CreateInstance();

    mIsMultiPlayer = RPSceneInfo::spInstance->GetPlayerNum() != 1;

    mpField = new RPPnpField();
    mpTable = new RPPnpTable();
    mpNetShadow = new RPPnpNetShadow();
    mpBall = new RPPnpBall();
    mpPlayers[0] = new RPPnpPlayer(0, FALSE);
    mpPlayers[1] = new RPPnpPlayer(1, mIsMultiPlayer == FALSE);

    RPPnpAudience::CreateInstance();
    RPPnpHud::CreateInstance();

    mpIntroPhase = new RPPnpIntroPhase();
    mpReadyPhase = new RPPnpReadyPhase();
    mpServePhase = new RPPnpServePhase();
    mpPointPhase = new RPPnpPointPhase();
    mpResultPhase = new RPPnpResultPhase();

    RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();
    mpScreen = new RPGrpScreen(0.0f, 0.0f, EGG::Screen::GetSizeXMax(),
                               EGG::Screen::GetSizeYMax(), NULL, EGG::Frustum::CANVAS_LU);
    mpScreen->SetFovy(15.8f);
    mpScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    mpCamera = new RPGrpCamera();

    pRenderer->CreateView3D(0, mpCamera, mpScreen);
    pRenderer->CreateView2D(1, mpScreen);
    pRenderer->CreateView2D(2, mpScreen);
    pRenderer->CreateView2D(3, mpScreen);
    pRenderer->GetView(1)->SetDrawScene(1);
    pRenderer->GetView(2)->SetDrawScene(2);
    pRenderer->GetView(3)->SetDrawScene(3);
    pRenderer->GetView(3)->CreateScreenEffect();
    pRenderer->LinkViews();
}

RPPnpGame::~RPPnpGame() {}

void RPPnpGame::Create() {
    RPSimpleModel::SetRandomSeed(OSGetTick());

    mRallyCount = 0;
    mState = STATE_RALLY;

    mpField->Create();
    mpTable->Create();
    mpNetShadow->Create();
    mpBall->Create();

    for (int i = 0; i < 2; i++) {
        mpPlayers[i]->Create();
        EGG::CoreControllerMgr::instance()->getNthController(i)->setDistParam(0.0f, 1.0f);
    }

    RPPnpAudience::GetInstance()->Create();
    RPPnpHud::GetInstance()->Create();
    mpIntroPhase->Create();
    mpReadyPhase->Create();
    mpServePhase->Create();
    mpPointPhase->Create();
    mpResultPhase->Create();

    RPSysTextWriter::instance()->SetDrawFlag(0);
    RPSysTextWriter::instance()->SetFontSize(24.0f);
}

void RPPnpGame::Load() {
    // declared first: gives pMsgFile r27 (as the original) instead of r28
    RPGrpLightManager* pLightMgr;
    void* pMsgFile = RPSysResourceManager::GetMessageResource("party_message.bmg");
    mpMessage = new RPSysMessage(pMsgFile, NULL);
    mpBgmCtrl = new RPPnpBgmCtrl();

    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPSysKokeshiManager::CreateLightTexture(0);

    pLightMgr =
        static_cast<RPGrpLightManager*>(RPGrpModelManager::GetCurrent()->GetLightManager(0));
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle sceneFile = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sceneFile.brres"));
    pLightMgr->LoadScnLight(sceneFile, "RPScene");
    pLightMgr->SetBinary(
        RPSysResourceManager::GetFileFromArchive(pArchive, "/Environment/scene_Light.plight"));

    void* pLightMap =
        RPSysResourceManager::GetFileFromArchive(pArchive, "/Environment/scene_Lm.plmap");
    RPGrpDrawPathManager* pDrawPathMgr = RPGrpModelManager::GetCurrent()->GetDrawPathManager(0);
    pDrawPathMgr->GetDrawPathLightMap()->GetLightTextureManager()->SetBinary(pLightMap);

    RPGrpDrawPathBloom* pBloom = pDrawPathMgr->GetDrawPathBloom();
    void* pBloomFile =
        RPSysResourceManager::GetFileFromArchive(pArchive, "/Environment/scene_Bloom.pblm");
    pBloom->GetBloom()->SetBinary(pBloomFile);

    mpField->Load();
    mpTable->Load();
    mpNetShadow->Load();
    mpBall->Load();

    for (int i = 0; i < 2; i++) {
        mpPlayers[i]->Load();
    }

    RPPnpAudience::GetInstance()->Load();
    RPPnpHud::GetInstance()->Load();
    mpIntroPhase->Load();
    mpReadyPhase->Load();
    mpServePhase->Load();
    mpPointPhase->Load();
    mpResultPhase->Load();

    mpShadowTexture = new RPGrpShadowTexture(0x1C0, 1, 0x80);
    mpShadowTexture->InitList();
    mpShadowTexture->Replace(mpTable->GetModel(), "dummy_shadow");
    mpShadowTexture->BYTE_0x72 = 0x50;
    mpShadowTexture->FLOAT_0x74 = 1.15f;
    mpShadowTexture->VEC_0x78 = nw4r::math::VEC3(0.0f, -1.0f, 0.0f);
    RPGrpModelManager::GetCurrent()
        ->GetModelScene(0)
        ->GetDrawPathManager()
        ->GetDrawPathShadowTexture()
        ->SetTexture(0, mpShadowTexture);
    mpShadowTexture->ResetList();

    void* pMiscFile =
        RPSysResourceManager::GetFileFromArchive(pArchive, "/Environment/scene_ModelMisc.pmmisc");
    RPGrpModelManager::GetCurrent()->GetModelMisc()->SetBinary(pMiscFile);

    RPCommon::GetInstance()->LoadResources();
}

void RPPnpGame::Reset() {
    s32 medal = RPCommon::GetInstance()->GetSavedMedal();

    unk18 = 0;
    mCourtSwapped = false;

    if (mIsMultiPlayer == TRUE) {
        if (RPSceneInfo::spInstance->GetUnk1C() == 0) {
            mCourtSwapped = static_cast<u32>(2.0f * RPUtlRandom::getF32()) == 0;
            if (mCourtSwapped) {
                RPSceneInfo::spInstance->SetUnk1C(1);
            } else {
                RPSceneInfo::spInstance->SetUnk1C(2);
            }
        } else {
            switch (RPSceneInfo::spInstance->GetUnk1C()) {
            case 1:
                mCourtSwapped = false;
                RPSceneInfo::spInstance->SetUnk1C(2);
                break;
            case 2:
                mCourtSwapped = true;
                RPSceneInfo::spInstance->SetUnk1C(1);
                break;
            }
        }
    }

    switch (sRallyLimitMode) {
    case 0:
        if (medal >= 3) {
            sRallyLimit = 999;
        } else {
            sRallyLimit = 100;
        }
        break;
    case 1:
        sRallyLimit = 100;
        break;
    case 2:
        sRallyLimit = 999;
        break;
    }

    mpField->Reset();
    mpTable->Reset();
    mpNetShadow->Reset();
    mpBall->Reset();

    for (int i = 0; i < 2; i++) {
        mpPlayers[i]->Reset();
    }

    RPPnpAudience::GetInstance()->Reset();
    RPPnpHud::GetInstance()->Reset();
    mpIntroPhase->Reset();
    mpReadyPhase->Reset();
    mpServePhase->Reset();
    mpPointPhase->Reset();
    mpResultPhase->Reset();
    RPCommon::GetInstance()->Reset();

    if (mCourtSwapped) {
        mpCamera->SetTarget(0.0f, 80.0f, sCameraOffsetFar);
        mpCamera->SetRotation(-33.0f, 180.0f, 0.0f);
    } else {
        mpCamera->SetTarget(0.0f, 80.0f, sCameraOffsetNear);
        mpCamera->SetRotation(-33.0f, 0.0f, 0.0f);
    }
    mpCamera->SetDistance(-1150.0f);
    mpCamera->SetCameraType(RPGrpCamera::ECameraType_Orbit);

    mState = STATE_INTRO;

    RPSndAudioMgr::instance()->startSystemSe(0x6A);
    RPPnpReadyPhase::StartBgm(mIsMultiPlayer == TRUE);

    RPCommon::GetInstance()->GetLeadCursor()->SetTimers(30);

    RPGrpView::ScreenEffect* pEffect = RPGrpRenderer::GetCurrent()->GetView(3)->GetScreenEffect();
    GXColor white = {255, 255, 255, 0};
    pEffect->SetColorMultiply(white);
    pEffect->SetBlurAlpha(0);
    pEffect->SetBlurOffset(1.0f);
}

void RPPnpGame::Calculate() {
    mpCamera->CalcMatrix();

    RPSndAudioMgr* pAudioMgr = RPSndAudioMgr::instance();
    const nw4r::math::MTX34& rMtx = mpCamera->GetViewMatrix();
    if (pAudioMgr->getCameraCount() > 0) {
        pAudioMgr->setListenerMatrix(0, rMtx);
    }

    switch (mState) {
    case STATE_INTRO:
        StepPhysics(FALSE);
        if (mpIntroPhase->GetStep() == 2) {
            mState = STATE_READY;
        }
        break;
    case STATE_READY:
        StepPhysics(FALSE);
        if (mpReadyPhase->GetStep() == 5) {
            mState = STATE_SERVE;
        }
        break;
    case STATE_SERVE:
        StepPhysics(TRUE);
        break;
    case STATE_RALLY:
        StepPhysics(TRUE);
        switch (mpBall->GetState()) {
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            mState = STATE_POINT;
            mpPointPhase->Start();
            break;
        }
        break;
    case STATE_POINT:
        StepPhysics(TRUE);
        CheckPointEnd();
        break;
    case STATE_RESULT:
        StepPhysics(FALSE);
        if (mpResultPhase->GetStep() == 6) {
            if (mpResultPhase->IsRetry()) {
                RPSysSceneCreator::instance()->changeSceneAfterFade(0x13);
            } else {
                RPSysSceneCreator::instance()->changeSceneAfterFade(0x1B);
            }
        }
        break;
    }

    RPCommon::GetInstance()->GetLeadCursor()->SetAllEnabled(mState != STATE_RESULT);

    mpIntroPhase->Calculate();
    mpReadyPhase->Calculate();
    mpServePhase->Calculate();
    mpPointPhase->Calculate();
    mpResultPhase->Calculate();
    RPCommon::GetInstance()->Calculate();

    if (!mIsMultiPlayer) {
        f32 ballSpeed;
        s32 cpuLevel;
        s32 rally = mpBall->GetHitCount() / 2;
        RPPnpLevelTable::GetInstance()->LookupRally(rally, &ballSpeed, &cpuLevel);
        mpBgmCtrl->UpdateRally(ballSpeed, rally);
    } else {
        s32 count = std::max(mpPointPhase->GetScore(0), mpPointPhase->GetScore(1));
        if (mState == STATE_POINT) {
            mpBgmCtrl->SetCount(1, count);
        } else {
            mpBgmCtrl->SetCount(0, count);
        }
    }

    RPGrpView::ScreenEffect* pEffect = RPGrpRenderer::GetCurrent()->GetView(3)->GetScreenEffect();
    GXColor color = RPCommon::GetInstance()->GetScreenFade()->GetColor();
    pEffect->SetColorMultiply(color);
    pEffect->SetBlurAlpha(color.a);
}

void RPPnpGame::Exit() {
    mpField->Exit();
    mpTable->Exit();
    mpNetShadow->Exit();
    mpBall->Exit();

    for (int i = 0; i < 2; i++) {
        mpPlayers[i]->Exit();
    }

    RPPnpAudience::GetInstance()->Exit();
    RPPnpHud::GetInstance()->Exit();
    mpIntroPhase->Exit();
    mpReadyPhase->Exit();
    mpServePhase->Exit();
    mpPointPhase->Exit();
    mpResultPhase->Exit();

    delete sInstance;
    sInstance = NULL;
    RPPnpLevelTable::DestroyInstance();
    RPPnpAudience::DestroyInstance();
    RPPnpHud::DestroyInstance();
    RPCommon::DestroyInstance();
}

void RPPnpGame::Draw() {
    if (mState != STATE_RESULT) {
        mpBall->DrawShadow();
    }

    for (int i = 0; i < 2; i++) {
        mpPlayers[i]->DrawDebug();
    }

    RPPnpAudience::GetInstance()->Draw();
    RPPnpHud::GetInstance()->Draw();

    if (RPGrpRenderer::GetCurrentViewNo() == 1 && RPGrpRenderer::GetDrawStep() == 5) {
        if (mState == STATE_INTRO) {
            mpIntroPhase->Draw();
        }
        if (mState == STATE_READY) {
            mpReadyPhase->Draw();
        }
        if (mState == STATE_SERVE) {
            mpServePhase->Draw();
        }
        if (mState == STATE_POINT) {
            mpPointPhase->Draw();
        }
        if (mState == STATE_RESULT) {
            mpResultPhase->Draw();
        }
    }
}

s32 RPPnpGame::GetOpponentSide(s32 side) const {
    s32 opponent = 2;
    switch (side) {
    case 0:
        opponent = 1;
        break;
    case 1:
        opponent = 0;
        break;
    }

    return opponent;
}
