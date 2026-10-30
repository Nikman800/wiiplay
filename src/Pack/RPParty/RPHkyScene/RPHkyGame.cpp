// Laser Hockey game manager: owns the world objects, phases and views; runs the game states.
// Unit 0x8021A4B4-0x8021B844 (dtk/game_units.tsv). With -inline deferred CodeWarrior emits a
// file's functions in reverse order, so the source runs from the last address to the first.
#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpDrawPathBloom.h>
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpDrawPathSV.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPKernel/RPSysTextWriter.h>
#include <Pack/RPParty/RPHkyScene/RPHkyBgmCtrl.h>
#include <Pack/RPParty/RPHkyScene/RPHkyField.h>
#include <Pack/RPParty/RPHkyScene/RPHkyHud.h>
#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>
#include <Pack/RPParty/RPHkyScene/RPHkyPaddle.h>
#include <Pack/RPParty/RPHkyScene/RPHkyPhase.h>
#include <Pack/RPParty/RPHkyScene/RPHkyPuck.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPLeadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPPauseText.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPScreenFade.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPSystem/RPSysSceneCreator.h>

#include <egg/core.h>
#include <egg/gfxe.h>

// Rink size, set by Create from the TV mode (RPHkyField's; this unit's .sdata)
f32 RPHkyField::sLength = 880.0f;
f32 RPHkyField::sWidth = 440.0f;

// Rink dimensions (RPHkyField, also read by RPHkyPaddle and RPHkyPuck). They sit at the start of
// this unit's .sdata2, before its literals, so they are defined here.
const f32 RPHkyField::cRinkDepth = 600.0f;
const f32 RPHkyField::cHalfDepth = 300.0f;
const f32 RPHkyField::cCorner = 80.0f;
const f32 RPHkyField::cGoalWidth = 200.0f;
const f32 RPHkyField::cGoalInset = 8.0f;

RPHkyGame* RPHkyGame::sInstance = NULL;

// Light position, set in Create (Hky_lightPos)
static nw4r::math::VEC3 sLightPos(0.0f, 745.0f, 0.0f);

void RPHkyGame::CreateInstance() {
    sInstance = new RPHkyGame();
}

void RPHkyGame::StepPhysics(BOOL playing) {
    mpPaddles[0]->UpdateControl();
    mpPaddles[1]->UpdateControl();
    if (playing) {
        mpPuck->SetInPlay();
    }

    s32 i = mSubSteps - 1;
    do {
        mpPaddles[0]->ApplyVelocity();
        mpPaddles[1]->ApplyVelocity();
        if (playing) {
            mpPuck->ApplyVelocity();
        }

        mpPaddles[0]->CollideField();
        mpPaddles[1]->CollideField();
        if (playing) {
            mpPuck->CollideField();
        }

        mpPaddles[0]->CollidePaddle(mpPaddles[1]);
        mpPaddles[1]->CollidePaddle(mpPaddles[0]);
        if (playing) {
            mpPuck->CollidePaddle(mpPaddles[0]);
        }
        if (playing) {
            mpPuck->CollidePaddle(mpPaddles[1]);
        }

        mpPaddles[0]->UpdateMovement();
        mpPaddles[1]->UpdateMovement();
        if (playing) {
            mpPuck->UpdatePhysics();
        }
    } while (--i >= 0);

    mpPaddles[0]->UpdateRumble();
    mpPaddles[1]->UpdateRumble();
    if (playing) {
        mpPuck->CheckGoal(&mpPaddles[0]->mScore, &mpPaddles[1]->mScore);
    }
    if (playing) {
        mpPuck->Calculate();
    }

    mpField->Calculate();
    mpPaddles[0]->Calculate();
    mpPaddles[1]->Calculate();
    mpPaddleRounds[0]->Calculate();
    mpPaddleRounds[1]->Calculate();
}

RPHkyGame::RPHkyGame() {
    RPGrpModelResManager::GetCurrent()->CreateResourceList(0x100);

    RPGrpModelManager* pModelMgr = RPGrpModelManager::GetCurrent();
    pModelMgr->CreateModelScene(0, 0x28, 1, NULL, NULL);
    pModelMgr->CreateModelScene(1, 0, 1, NULL, NULL);
    pModelMgr->CreateModelScene(2, 0, 1, NULL, NULL);

    GXColor shadowColor = {0, 0, 0, 0xC0};
    pModelMgr->GetDrawPathManager(0)->GetDrawPathSV()->GetShadowVolume()->SetShadowColor(
        shadowColor);

    RPCommonParam param;
    param.mGame = 4;
    param.mPlayerNum = RPSceneInfo::spInstance->GetPlayerNum();
    RPCommon::CreateInstance(param);

    mpPuck = new RPHkyPuck();
    mpField = new RPHkyField();
    mpPaddles[0] = new RPHkyPaddle(0);
    mpPaddles[1] = new RPHkyPaddle(1);
    mpPaddleRounds[0] = new RPHkyPaddleRound(0, mpPaddles[0]);
    mpPaddleRounds[1] = new RPHkyPaddleRound(1, mpPaddles[1]);

    RPHkyHud::CreateInstance();

    mpIntroPhase = new RPHkyIntroPhase();
    mpGoalPhase = new RPHkyGoalPhase();
    mpTimeUpPhase = new RPHkyTimeUpPhase();
    mpWaitPhase = new RPHkyWaitPhase();
    mpResultPhase = new RPHkyResultPhase();

    RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();
    mpScreen = new RPGrpScreen(0.0f, 0.0f, EGG::Screen::GetSizeXMax(),
                               EGG::Screen::GetSizeYMax(), NULL, EGG::Frustum::CANVAS_LU);
    mpScreen->SetFovy(40.0f);
    mpScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    mpCamera = new RPGrpCamera();

    pRenderer->CreateView3D(0, mpCamera, mpScreen);
    pRenderer->CreateView2D(1, mpScreen);
    pRenderer->CreateView2D(2, mpScreen);
    pRenderer->GetView(1)->SetDrawScene(1);
    pRenderer->GetView(2)->SetDrawScene(2);
    pRenderer->GetView(2)->CreateScreenEffect();
    pRenderer->LinkViews();
}

RPHkyGame::~RPHkyGame() {}

void RPHkyGame::Create() {
    mSubSteps = 16;
    mIsMultiPlayer = RPSceneInfo::spInstance->GetPlayerNum() != 1;

    RPSimpleModel::SetRandomSeed(OSGetTick());

    if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
        RPHkyField::sLength = 1000.0f;
        RPHkyField::sWidth = RPHkyField::sLength / 2;
    } else {
        RPHkyField::sLength = 800.0f;
        RPHkyField::sWidth = RPHkyField::sLength / 2;
    }

    EGG::LightManager* pLightMgr = RPGrpModelManager::GetCurrent()->GetLightManager(0);
    EGG::LightObject* pLight = pLightMgr->GetLightObj(0);
    pLight->SetPos(sLightPos);
    pLight->CalcDirDist();

    GXColor ambient = {100, 100, 100, 0};
    pLightMgr->GetAmbientColor(0) = ambient;

    mState = 0;
    mpPuck->Create();
    mpField->Create();
    mpPaddles[0]->Create();
    mpPaddles[1]->Create();

    EGG::CoreControllerMgr::instance()->getNthController(0)->setDistParam(0.0f, 1.0f);
    EGG::CoreControllerMgr::instance()->getNthController(1)->setDistParam(0.0f, 1.0f);

    mpPaddleRounds[0]->Create();
    mpPaddleRounds[1]->Create();

    RPHkyHud::GetInstance()->Create();
    mpIntroPhase->Create();
    mpGoalPhase->Create();
    mpTimeUpPhase->Create();
    mpWaitPhase->Create();
    mpResultPhase->Create();

    RPSysTextWriter::instance()->SetDrawFlag(0);
    RPSysTextWriter::instance()->SetFontSize(24.0f);
}

void RPHkyGame::Load() {
    void* pMsgFile = RPSysResourceManager::GetMessageResource("party_message.bmg");
    mpMessage = new RPSysMessage(pMsgFile, NULL);
    mpBgmCtrl = new RPHkyBgmCtrl();

    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpDrawPathBloom* pBloom =
        RPGrpModelManager::GetCurrent()->GetDrawPathManager(0)->GetDrawPathBloom();
    void* pBloomFile = RPSysResourceManager::GetFileFromArchive(pArchive, "/Environment/neon.pblm");
    pBloom->GetBloom()->SetBinary(pBloomFile);

    mpPuck->Load();
    mpField->Load();
    mpPaddles[0]->Load();
    mpPaddles[1]->Load();
    mpPaddleRounds[0]->Load();
    mpPaddleRounds[1]->Load();

    RPHkyHud::GetInstance()->Load();
    mpIntroPhase->Load();
    mpGoalPhase->Load();
    mpTimeUpPhase->Load();
    mpWaitPhase->Load();
    mpResultPhase->Load();
    RPCommon::GetInstance()->LoadResources();
}

void RPHkyGame::Reset() {
    mTimer = 7200;
    mState = 0;

    mpPuck->Reset();
    mpField->Reset();
    mpPaddles[0]->Reset();
    mpPaddles[1]->Reset();
    mpPaddleRounds[0]->Reset();
    mpPaddleRounds[1]->Reset();

    RPHkyHud::GetInstance()->Reset();
    mpIntroPhase->Reset();
    mpGoalPhase->Reset();
    mpTimeUpPhase->Reset();
    mpWaitPhase->Reset();
    mpResultPhase->Reset();
    RPCommon::GetInstance()->Reset();

    mpCamera->SetTarget(0.0f, 0.0f, 0.0f);
    mpCamera->SetDistance(-745.0f);
    mpCamera->SetRotation(-90.0f, 0.0f, 0.0f);
    mpCamera->SetCameraType(RPGrpCamera::ECameraType_Orbit);

    RPCommon::GetInstance()->GetLeadCursor()->SetTimers(30);

    RPSndAudioMgr::instance()->startBgm(0x43U);

    RPGrpView::ScreenEffect* pEffect = RPGrpRenderer::GetCurrent()->GetView(2)->GetScreenEffect();
    GXColor white = {255, 255, 255, 0};
    pEffect->SetColorMultiply(white);
    pEffect->SetBlurAlpha(0);
    pEffect->SetBlurOffset(1.0f);
}

void RPHkyGame::Calculate() {
    mpCamera->CalcMatrix();

    RPSndAudioMgr* pAudioMgr = RPSndAudioMgr::instance();
    const nw4r::math::MTX34& rMtx = mpCamera->GetViewMatrix();
    if (pAudioMgr->getCameraCount() > 0) {
        pAudioMgr->setListenerMatrix(0, rMtx);
    }

    if (mState == 1 && mTimer < 0) {
        mState = 3;
        mpTimeUpPhase->Start();
        mpPuck->Hide();
        mpBgmCtrl->Request(RPHkyBgmCtrl::REQUEST_TIME_UP);
    }

    if (mState == 1) {
        StepPhysics(TRUE);
        if (!mIsMultiPlayer) {
            mTimer--;
        }
    } else {
        StepPhysics(FALSE);
    }

    switch (mState) {
    case 0:
        if (mpIntroPhase->GetStep() == 2) {
            if (sInstance->IsMultiPlayer() == TRUE) {
                RPHkyHud::GetInstance()->GetMsg03Lyt()->PlayIn();
            }
            mState = 1;
            mpPuck->Serve();
        }
        break;
    case 2:
        if (mpGoalPhase->GetStep() == 2) {
            mState = 1;
            mpPuck->Serve();
            if (mIsMultiPlayer == TRUE &&
                (mpPaddles[0]->GetScore() >= 8 || mpPaddles[1]->GetScore() >= 8)) {
                mTimer = -1;
            }
        }
        break;
    case 3:
        if (mpTimeUpPhase->GetStep() == 2) {
            mState = 4;
            mpWaitPhase->Start();
            if (!sInstance->IsMultiPlayer()) {
                RPHkyHud::GetInstance()->GetTimeLyt()->PlayOut();
            }
            RPHkyHud::GetInstance()->GetScoreLyt()->PlayResultSwitch();
        }
        break;
    case 4:
        if (mpWaitPhase->GetStep() == 2) {
            mState = 5;
            mpResultPhase->Start();
        }
        break;
    case 5:
        if (mpResultPhase->GetStep() == 4) {
            if (mpResultPhase->IsRetry()) {
                RPSysSceneCreator::instance()->changeSceneAfterFade(0x11);
            } else {
                RPSysSceneCreator::instance()->changeSceneAfterFade(0x1B);
            }
        }
        break;
    }

    RPCommon::GetInstance()->GetLeadCursor()->SetAllEnabled(mState == 1);

    RPPauseText* pPauseText = RPCommon::GetInstance()->GetPauseText();
    pPauseText->SetEnabled(1, mState == 0 && !mIsMultiPlayer);

    RPHkyHud::GetInstance()->Calculate();
    mpIntroPhase->Calculate();
    mpGoalPhase->Calculate();
    mpTimeUpPhase->Calculate();
    mpWaitPhase->Calculate();
    mpResultPhase->Calculate();
    RPCommon::GetInstance()->Calculate();

    mpBgmCtrl->Update(mTimer <= 0 ? 0 : (mTimer - 1) / 60 + 1);

    RPGrpView::ScreenEffect* pEffect = RPGrpRenderer::GetCurrent()->GetView(2)->GetScreenEffect();
    GXColor color = RPCommon::GetInstance()->GetScreenFade()->GetColor();
    pEffect->SetColorMultiply(color);
    pEffect->SetBlurAlpha(color.a);
}

void RPHkyGame::Exit() {
    mpPuck->Exit();
    mpField->Exit();
    mpPaddles[0]->Exit();
    mpPaddles[1]->Exit();
    mpPaddleRounds[0]->Exit();
    mpPaddleRounds[1]->Exit();

    RPHkyHud::GetInstance()->Exit();
    mpIntroPhase->Exit();
    mpGoalPhase->Exit();
    mpTimeUpPhase->Exit();
    mpWaitPhase->Exit();
    mpResultPhase->Exit();

    delete sInstance;
    sInstance = NULL;
    RPHkyHud::DestroyInstance();
    RPCommon::DestroyInstance();
}

void RPHkyGame::Draw() {
    RPHkyHud::GetInstance()->Draw();

    if (RPGrpRenderer::GetCurrentViewNo() == 1 && RPGrpRenderer::GetDrawStep() == 5) {
        if (mState == 0) {
            mpIntroPhase->Draw();
        }
        if (mState == 2) {
            mpGoalPhase->Draw();
        }
        if (mState == 3) {
            mpTimeUpPhase->Draw();
        }
        if (mState == 4) {
            mpWaitPhase->Draw();
        }
        if (mState == 5) {
            mpResultPhase->Draw();
        }
    }
}
