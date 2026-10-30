#include <Pack/RPParty/RPBilScene/RPBilGameMgr.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPParty/RPBilScene/RPBilBreakLottery.h>
#include <Pack/RPParty/RPBilScene/RPBilCamera.h>
#include <Pack/RPParty/RPBilScene/RPBilHud.h>
#include <Pack/RPParty/RPBilScene/RPBilLyt.h>
#include <Pack/RPParty/RPBilScene/RPBilMarker.h>
#include <Pack/RPParty/RPBilScene/RPBilPlayer.h>
#include <Pack/RPParty/RPBilScene/RPBilPocketCamWindow.h>
#include <Pack/RPParty/RPBilScene/RPBilResult.h>
#include <Pack/RPParty/RPBilScene/RPBilTable.h>
#include <Pack/RPParty/RPBilScene/RPBilBallMgr.h>
#include <Pack/RPParty/RPBilScene/RPBilBall.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPLeadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPPauseText.h>
#include <Pack/RPParty/RPPartyCommon/RPScreenFade.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPSystem.h>

RPBilGameMgr* RPBilGameMgr::sInstance;
s32 RPBilGameMgr::sPlayerNum;
s32 RPBilGameMgr::sPlayTimeout[2] = {1200, 0};

void RPBilGameMgr::CreateInstance() {
    sInstance = new RPBilGameMgr();
}

void RPBilGameMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilGameMgr::~RPBilGameMgr() {}

RPBilGameMgr::RPBilGameMgr() {
    sPlayerNum = RPSceneInfo::spInstance->GetPlayerNum();

    mpStateMachine = new RPBilGameSeq(this, STATE_NUM);

    RPBilStateFuncs<RPBilGameMgr> funcs[STATE_NUM] = {
        {&RPBilGameMgr::IntroEnter, &RPBilGameMgr::IntroCalc},
        {&RPBilGameMgr::StartEnter, &RPBilGameMgr::StartCalc},
        {&RPBilGameMgr::PlayEnter, &RPBilGameMgr::PlayCalc},
        {&RPBilGameMgr::ScoreEnter, &RPBilGameMgr::ScoreCalc},
        {&RPBilGameMgr::FoulEnter, &RPBilGameMgr::FoulCalc},
        {&RPBilGameMgr::GameOverEnter, &RPBilGameMgr::GameOverCalc},
    };
    for (s32 i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    RPBilCameraMgr::CreateInstance();
    RPBilTable::CreateInstance();
    RPBilPlayerMgr::CreateInstance();
    RPBilBallMgr::CreateInstance();
    RPBilBallNumberTag::CreateInstance();
    RPBilBreakLottery::CreateInstance();
    RPBilPocketCamWindow::CreateInstance();
    RPBilNextBallMarker::CreateInstance();
    RPBilHud::CreateInstance();
    RPBilMsgWindow::CreateInstance();
    RPBilResult::CreateInstance();
    RPBilScorePopup::CreateInstance();
    RPBilHint::CreateInstance();
    RPBilStrokeGuide::CreateInstance();
    RPBilMusic::CreateInstance();

    RPCommonParam param;
    param.mGame = 5;
    param.mPlayerNum = sPlayerNum;
    RPCommon::CreateInstance(param);
}

void RPBilGameMgr::Load() {
    EGG::Archive* archive = RPSysResourceManager::LoadGameLocalArchive();

    RPBilTable::sInstance->Load();
    RPBilPlayerMgr::sInstance->Load();
    RPBilBallMgr::sInstance->Load();
    RPBilBallNumberTag::sInstance->Load(archive);
    RPBilBreakLottery::sInstance->Load();
    RPBilResult::sInstance->Load();
    RPBilPocketCamWindow::sInstance->Load(archive);
    RPBilNextBallMarker::sInstance->Load();
    RPBilHud::sInstance->Load(archive);
    RPBilMsgWindow::sInstance->Load(archive);
    RPBilScorePopup::sInstance->Load();
    RPBilHint::sInstance->Load();
    RPBilStrokeGuide::sInstance->Load(archive);
    RPCommon::GetInstance()->LoadResources();
}

void RPBilGameMgr::Reset() {
    mpStateMachine->Reset(STATE_INTRO);
    RPBilCameraMgr::sInstance->Reset();
    RPBilTable::sInstance->Reset();
    RPBilPlayerMgr::sInstance->Reset();
    RPBilBallMgr::sInstance->Reset();
    RPBilBallNumberTag::sInstance->Reset();
    RPBilBreakLottery::sInstance->Reset();
    RPBilResult::sInstance->Reset();
    RPBilPocketCamWindow::sInstance->Reset();
    RPBilNextBallMarker::sInstance->Reset();
    RPBilHud::sInstance->Reset();
    RPBilMsgWindow::sInstance->Reset();
    RPBilScorePopup::sInstance->Reset();
    RPBilHint::sInstance->Reset();
    RPBilStrokeGuide::sInstance->Reset();
    RPCommon::GetInstance()->Reset();
    RPCommon::GetInstance()->GetLeadCursor()->SetTimers(15);

    if (sPlayerNum == 1) {
        RPCommon::GetInstance()->GetPauseText()->SetEnabled(0, true);
    }

    RPGrpView::ScreenEffect* pEffect =
        RPGrpRenderer::GetCurrent()->GetView(9)->GetScreenEffect();
    GXColor color = {255, 255, 255, 0};
    pEffect->SetColorMultiply(color);
    pEffect->SetBlurAlpha(0);
    pEffect->SetBlurOffset(1.0f);
}

// Wii Play: defined in this file (RPBilScorePopup's vtable slot at 0x20)
void RPBilScorePopup::Exit() {}

void RPBilGameMgr::Exit() {
    RPBilTable::sInstance->Exit();
    RPBilPlayerMgr::sInstance->Exit();
    RPBilBallMgr::sInstance->Exit();
    RPBilBallNumberTag::sInstance->VF_0x18();
    RPBilBreakLottery::sInstance->Exit();
    RPBilResult::sInstance->Exit();
    RPBilPocketCamWindow::sInstance->Exit();
    RPBilNextBallMarker::sInstance->Exit();
    RPBilHud::sInstance->VF_0x18();
    RPBilMsgWindow::sInstance->VF_0x18();
    RPBilScorePopup::sInstance->Exit();

    RPBilCameraMgr::DestroyInstance();
    RPBilTable::DestroyInstance();
    RPBilPlayerMgr::DestroyInstance();
    RPBilBallMgr::DestroyInstance();
    RPBilBallNumberTag::DestroyInstance();
    RPBilBreakLottery::DestroyInstance();
    RPBilResult::DestroyInstance();
    RPBilPocketCamWindow::DestroyInstance();
    RPBilNextBallMarker::DestroyInstance();
    RPBilHud::DestroyInstance();
    RPBilMsgWindow::DestroyInstance();
    RPCommon::DestroyInstance();
    RPBilScorePopup::DestroyInstance();
    RPBilHint::DestroyInstance();
    RPBilStrokeGuide::DestroyInstance();
    RPBilMusic::DestroyInstance();
}

void RPBilGameMgr::Calculate() {
    mpStateMachine->Calculate();
    RPBilMusic::sInstance->GetState()->Calculate();

    RPGrpView::ScreenEffect* pEffect =
        RPGrpRenderer::GetCurrent()->GetView(9)->GetScreenEffect();
    GXColor color = RPCommon::GetInstance()->GetScreenFade()->GetColor();
    pEffect->SetColorMultiply(color);
    pEffect->SetBlurAlpha(color.a);
}

void RPBilGameMgr::IntroEnter() {
    RPBilBall* pBall = RPBilBallMgr::sInstance->GetBall(0);
    RPBilCamera* pCamera = RPBilCameraMgr::sInstance->GetCamera();
    pCamera->MoveToLookAt(static_cast<const EGG::Vector3f&>(pBall->GetPos()),
                          static_cast<const EGG::Vector3f&>(pBall->GetAxisY()));
    RPBilBreakLottery::sInstance->Start(sPlayerNum);
    RPBilMusic::sInstance->GetState()->Start();
}

void RPBilGameMgr::IntroCalc() {
    RPBilCameraMgr::sInstance->Calculate();
    RPBilTable::sInstance->Calculate();
    RPBilBallMgr::sInstance->Calculate();
    RPBilBreakLottery::sInstance->Calculate();
    RPCommon::GetInstance()->Calculate();

    if (!RPBilCameraMgr::sInstance->GetCamera()->IsActive()) {
        mpStateMachine->ChangeState(STATE_START);
    }
}

void RPBilGameMgr::StartEnter() {}

static inline void StartFirstTurn(RPBilBreakLottery* pLottery) {
    RPBilPlayerMgr* pPlayerMgr = RPBilPlayerMgr::sInstance;
    RPBilBallMgr* pBallMgr = RPBilBallMgr::sInstance;
    pPlayerMgr->AssignChannels();
    pPlayerMgr->StartTurn(pLottery->GetWinner(), false);
    pBallMgr->SetCueBallFace(pLottery->GetWinner(), -1);
}

void RPBilGameMgr::StartCalc() {
    RPBilCameraMgr::sInstance->Calculate();
    RPBilTable::sInstance->Calculate();
    RPBilBallMgr::sInstance->Calculate();
    RPBilBreakLottery::sInstance->Calculate();
    RPCommon::GetInstance()->Calculate();

    RPBilBreakLottery* pLottery = RPBilBreakLottery::sInstance;
    if (pLottery->GetState() == RPBilBreakLottery::STATE_END) {
        StartFirstTurn(pLottery);
        mpStateMachine->ChangeState(STATE_PLAY);
    }
}

void RPBilGameMgr::PlayEnter() {}

static inline void Bil_playCalc(RPBilGameSeq*& rSM) {
    RPBilPlayerMgr* pPlayerMgr = RPBilPlayerMgr::sInstance;
    RPBilBallMgr* pBallMgr = RPBilBallMgr::sInstance;

    if (pPlayerMgr->GetUnkF()) {
        pBallMgr->NextRackMode();
        pPlayerMgr->SetUnkF(false);
    }

    RPBilCameraMgr::sInstance->Calculate();
    RPBilTable::sInstance->Calculate();
    RPBilPlayerMgr::sInstance->Calculate();
    RPBilBallMgr::sInstance->Calculate();
    RPBilBallNumberTag::sInstance->Calculate();
    RPBilPocketCamWindow::sInstance->Calculate();
    RPBilNextBallMarker::sInstance->Calculate();
    RPBilHud::sInstance->Calculate();
    RPBilHint::sInstance->Calculate();
    RPBilStrokeGuide::sInstance->Calculate();
    RPCommon::GetInstance()->Calculate();

    if (pPlayerMgr->GetUnkD()) {
        pBallMgr->SetUnk78(RPBilGameMgr::sPlayTimeout[0]);
    }

    if (pPlayerMgr->GetUnkC() && pBallMgr->AreAllBallsStopped()) {
        pBallMgr->SetUnk78(-1);
        s32 foul = pBallMgr->GetUnk70();
        if (foul == -1 && pBallMgr->GetFirstHitBall() == -1) {
            foul = 2;
        }

        if (foul != -1) {
            RPBilMsgWindow::sInstance->ShowFoul(foul);
            rSM->ChangeState(RPBilGameMgr::STATE_FOUL);
            pPlayerMgr->IncPlayerFoulCount(pPlayerMgr->GetCurrentPlayer());
        } else {
            pPlayerMgr->AddPlayerScore(pPlayerMgr->GetCurrentPlayer(),
                                       pBallMgr->SumPocketedNumbers());
            pPlayerMgr->UpdatePlayerMaxStat(pPlayerMgr->GetCurrentPlayer(),
                                            pBallMgr->CountPocketed());
            rSM->ChangeState(RPBilGameMgr::STATE_SCORE);
        }
    }
}

void RPBilGameMgr::PlayCalc() {
    Bil_playCalc(mpStateMachine);
}

void RPBilGameMgr::ScoreEnter() {
    RPBilScorePopup::sInstance->Start(sPlayerNum, RPBilPlayerMgr::sInstance->GetCurrentPlayer());
}

void RPBilGameMgr::ScoreCalc() {
    RPBilPlayerMgr* pPlayerMgr;
    RPBilBallMgr* pBallMgr;
    pBallMgr = RPBilBallMgr::sInstance;
    pPlayerMgr = RPBilPlayerMgr::sInstance;

    RPBilCameraMgr::sInstance->Calculate();
    RPBilTable::sInstance->Calculate();
    RPBilPlayerMgr::sInstance->Calculate();
    RPBilBallMgr::sInstance->Calculate();
    RPBilBallNumberTag::sInstance->Calculate();
    RPBilPocketCamWindow::sInstance->Calculate();
    RPBilHud::sInstance->Calculate();
    RPBilHint::sInstance->Calculate();
    RPCommon::GetInstance()->Calculate();
    RPBilScorePopup::sInstance->Calculate();

    if (RPBilScorePopup::sInstance->GetStateMachine()->IsStateSettled(RPBilScorePopup::STATE_CLOSED)) {
        if (!pBallMgr->HasObjectBallsOnTable()) {
            mpStateMachine->ChangeState(STATE_GAME_OVER);
            return;
        }

        s32 pocketed = 0;
        for (s32 i = 0; i < pBallMgr->GetBallNum(); i++) {
            if (i != 0 && pBallMgr->IsBallPocketed(i)) {
                pocketed++;
            }
        }

        pBallMgr->EndShot();
        if (pocketed > 0) {
            pPlayerMgr->StartTurn(pPlayerMgr->GetCurrentPlayer(), false);
        } else {
            pPlayerMgr->StartTurn((pPlayerMgr->GetCurrentPlayer() + 1) % pPlayerMgr->GetPlayerNum(),
                                  false);
        }
        pBallMgr->SetCueBallFace(pPlayerMgr->GetCurrentPlayer(), -1);
        mpStateMachine->ChangeState(STATE_PLAY);
    }
}

void RPBilGameMgr::FoulEnter() {}

static inline void Bil_foulCalc(RPBilGameSeq*& rSM) {
    RPBilCameraMgr::sInstance->Calculate();
    RPBilTable::sInstance->Calculate();
    RPBilPlayerMgr::sInstance->Calculate();
    RPBilBallMgr::sInstance->Calculate();
    RPBilBallNumberTag::sInstance->Calculate();
    RPBilPocketCamWindow::sInstance->Calculate();
    RPBilHud::sInstance->Calculate();
    RPBilHint::sInstance->Calculate();
    RPBilMsgWindow::sInstance->Calculate();
    RPCommon::GetInstance()->Calculate();

    RPBilMsgWindow* pMsgWindow = RPBilMsgWindow::sInstance;
    RPBilPlayerMgr* pPlayerMgr = RPBilPlayerMgr::sInstance;
    RPBilBallMgr* pBallMgr = RPBilBallMgr::sInstance;
    bool close = false;
    if (pMsgWindow->GetStateMachine()->GetState() == RPBilMsgWindow::STATE_OPEN &&
        pMsgWindow->GetStateMachine()->GetTimer() >= 300) {
        close = true;
    }
    if (close || (pMsgWindow->GetStateMachine()->GetState() == RPBilMsgWindow::STATE_OPEN &&
                  pPlayerMgr->IsAorBTriggered())) {
        pMsgWindow->Close();
    }

    if (pMsgWindow->GetStateMachine()->GetState() == RPBilMsgWindow::STATE_CLOSED) {
        if (!pBallMgr->HasObjectBallsOnTable()) {
            rSM->ChangeState(RPBilGameMgr::STATE_GAME_OVER);
        } else {
            pBallMgr->EndShot();
            pPlayerMgr->StartTurn((pPlayerMgr->GetCurrentPlayer() + 1) % pPlayerMgr->GetPlayerNum(),
                                  true);
            pBallMgr->SetCueBallFace(pPlayerMgr->GetCurrentPlayer(), -1);
            rSM->ChangeState(RPBilGameMgr::STATE_PLAY);
        }
    }
}

void RPBilGameMgr::FoulCalc() {
    Bil_foulCalc(mpStateMachine);
}

void RPBilGameMgr::GameOverEnter() {
    RPBilResult::sInstance->ComputeScores();
    RPBilHud::sInstance->HidePrompt();
    RPBilHud::sInstance->HideTurnPanel();
    RPBilMusic::sInstance->GetState()->SetState(5);
}

void RPBilGameMgr::GameOverCalc() {
    RPBilCameraMgr::sInstance->Calculate();
    RPBilTable::sInstance->Calculate();
    RPBilPlayerMgr::sInstance->Calculate();
    RPBilBallMgr::sInstance->Calculate();
    RPBilBallNumberTag::sInstance->Calculate();
    RPBilResult::sInstance->Calculate();
    RPBilPocketCamWindow::sInstance->Calculate();
    RPBilHud::sInstance->Calculate();
    RPBilHint::sInstance->Calculate();
    RPCommon::GetInstance()->Calculate();

    if (RPBilResult::sInstance->GetStateMachine()->GetState() == RPBilResult::STATE_DONE &&
        RPSysSceneMgr::instance()->getCurrentSceneRP()->isSceneChangeEnable()) {
        if (RPBilResult::sInstance->IsRetry()) {
            RPSysSceneCreator::instance()->changeSceneAfterFade(0x14);
        } else {
            RPSysSceneCreator::instance()->changeSceneAfterFade(0x1B);
        }

        RPSndAudioMgr::instance()->stopBgm(10);
    }
}

void RPBilGameMgr::SetCursorsVisible(bool visible) {
    RPBilPlayerMgr::sInstance->SetCursorsVisible(visible);
}

#pragma defer_codegen off
#pragma auto_inline off
template void RPStateMachine<RPBilGameMgr>::Calculate();
template void RPStateMachine<RPBilGameMgr>::ChangeState(s32);
template void RPStateMachine<RPBilGameMgr>::Reset(s32);
template void RPStateMachine<RPBilGameMgr>::SetStateFuncs(const StateFunc&,
                                                          const StateFunc&, s32);
template RPStateMachine<RPBilGameMgr>::RPStateMachine(RPBilGameMgr*, s32);
template bool RPStateMachine<RPBilScorePopup>::IsStateSettled(s32) const;
template void RPStateMachine<RPBilGameMgr>::CallEnter();
template void RPStateMachine<RPBilGameMgr>::CallCalc();
template void RPStateMachine<RPBilGameMgr>::CallExit();
