// Billiards score popup controller (RPBilScorePopup, singleton RPBilScorePopup::sInstance): opens the
// layout, shows the pocketed ball icons and closes after 90 frames or on A/B.
// Unit 0x802D17A4-0x802D1FE4. Functions in reverse address order; the state machine
// instantiation follows at the end of the file.
#include <Pack/RPParty/RPBilScene/RPBilScorePopup.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPBilScene/RPBilBallMgr.h>
#include <Pack/RPParty/RPBilScene/RPBilPlayer.h>
#include <Pack/RPSystem.h>

#include <egg/core.h>

RPBilScorePopup* RPBilScorePopup::sInstance;
s32 RPBilScorePopup::sShowFrames = 90;

inline RPBilScorePopup::RPBilScorePopup() {
    mpStateMachine = new RPStateMachine<RPBilScorePopup>(this, STATE_NUM);

    RPBilStateFuncs<RPBilScorePopup> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPBilScorePopup::OpenEnter, &RPBilScorePopup::OpenCalc},
        {&RPBilScorePopup::ShowEnter, &RPBilScorePopup::ShowCalc},
        {&RPBilScorePopup::CloseEnter, &RPBilScorePopup::CloseCalc},
    };
    for (int i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    mpLyt = new RPBilScorePopupLyt();
    RPGrpRenderer::GetCurrent()->AppendDrawObject(this);
}

void RPBilScorePopup::CreateInstance() {
    sInstance = new RPBilScorePopup();
}

void RPBilScorePopup::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilScorePopup::~RPBilScorePopup() {}

void RPBilScorePopup::Load() {
    mpLyt->Load(RPSysResourceManager::LoadGameLocalArchive(-1, NULL));
}

void RPBilScorePopup::Reset() {
    mpStateMachine->Reset(STATE_CLOSED);
    mpLyt->Reset();
}

void RPBilScorePopup::Calculate() {
    mpStateMachine->Calculate();
    if (mpStateMachine->GetState() != STATE_CLOSED) {
        mpLyt->Calculate();
    }
}

void RPBilScorePopup::OpenEnter() {
    RPBilBallMgr* pBallMgr = RPBilBallMgr::sInstance;
    mpLyt->Show(mType, mPlayer, pBallMgr->SumPocketedNumbers());
    pBallMgr->ShowPocketedIcons(mpLyt->GetBallBgY(), mPlayer);
}

void RPBilScorePopup::OpenCalc() {
    if (mpLyt->IsOpen() && RPBilBallMgr::sInstance->AreIconsDone()) {
        mpStateMachine->ChangeState(STATE_SHOW);
    }
}

void RPBilScorePopup::ShowEnter() {}

void RPBilScorePopup::ShowCalc() {
    s32 channel = RPBilPlayerMgr::sInstance->GetPlayer(mPlayer)->GetChannel();
    if (EGG::CoreControllerMgr::instance()->getNthController(channel)->downTrigger(
            EGG::cCORE_BUTTON_A) ||
        EGG::CoreControllerMgr::instance()->getNthController(channel)->downTrigger(
            EGG::cCORE_BUTTON_B) ||
        mpStateMachine->GetTimer() > sShowFrames) {
        mpStateMachine->ChangeState(STATE_CLOSE);
    }
}

void RPBilScorePopup::CloseEnter() {
    mpLyt->Hide();
    RPBilBallMgr::sInstance->ResetIcons();
}

void RPBilScorePopup::CloseCalc() {
    if (mpLyt->IsClosed() && RPBilBallMgr::sInstance->AreIconsIdle()) {
        mpStateMachine->ChangeState(STATE_CLOSED);
    }
}

void RPBilScorePopup::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() != 9) {
        return;
    }
    if (RPGrpRenderer::GetDrawStep() != 5) {
        return;
    }
    if (mpStateMachine->GetState() == STATE_CLOSED) {
        return;
    }
    mpLyt->Draw();
}

void RPBilScorePopup::Start(s32 type, s32 player) {
    if (mpStateMachine->IsStateSettled(STATE_CLOSED) && RPBilBallMgr::sInstance->SumPocketedNumbers() != 0) {
        mType = type;
        mPlayer = player;
        mpStateMachine->ChangeState(STATE_OPEN);
    }
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template void RPStateMachine<RPBilScorePopup>::Calculate();
template void RPStateMachine<RPBilScorePopup>::ChangeState(s32);
template void RPStateMachine<RPBilScorePopup>::Reset(s32);
template void RPStateMachine<RPBilScorePopup>::SetStateFuncs(const StateFunc&, const StateFunc&,
                                                             s32);
template RPStateMachine<RPBilScorePopup>::RPStateMachine(RPBilScorePopup*, s32);
template void RPStateMachine<RPBilScorePopup>::CallEnter();
template void RPStateMachine<RPBilScorePopup>::CallCalc();
template void RPStateMachine<RPBilScorePopup>::CallExit();
