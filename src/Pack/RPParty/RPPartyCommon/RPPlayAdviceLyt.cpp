#include <Pack/RPParty/RPPartyCommon/RPPlayAdviceLyt.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytPicture.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPPartyCommon/RPPlayerConfirm.h>
#include <Pack/RPSystem/RPSysSaveDataMgr.h>

extern const f32 sAdviceStartFrame;

RPPlayAdviceLyt::RPPlayAdviceLyt() {
    mpStateMachine = new RPStateMachine<RPPlayAdviceLyt>(this, 7);
    typedef RPStateMachine<RPPlayAdviceLyt>::StateFunc StateFunc;
    StateFunc funcs[7][2] = {
        {NULL, NULL},
        {&RPPlayAdviceLyt::InEnter, &RPPlayAdviceLyt::InCalc},
        {&RPPlayAdviceLyt::AButtonInEnter, &RPPlayAdviceLyt::AButtonInCalc},
        {&RPPlayAdviceLyt::BlinkEnter, &RPPlayAdviceLyt::BlinkCalc},
        {&RPPlayAdviceLyt::DecideEnter, &RPPlayAdviceLyt::DecideCalc},
        {&RPPlayAdviceLyt::OutEnter, &RPPlayAdviceLyt::OutCalc},
        {NULL, NULL},
    };
    for (s32 i = 0; i < 7; i++) {
        mpStateMachine->SetStateFuncs(funcs[i][0], funcs[i][1], i);
    }
}

void RPPlayAdviceLyt::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "playAdvice.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "playAdvice_00.brlyt");
    const char* names[] = {
        "playAdvice_00_in_00.brlan",
        "playAdvice_00_inAButton_00.brlan",
        "playAdvice_00_blinkAButton_00.brlan",
        "playAdvice_00_out_00.brlan",
    };
    for (s32 i = 0; i < 4; i++) {
        (&mpInAnmObj)[i] = mpLayout->createAnmObj(NULL, mpResAccessor, names[i]);
    }
}

void RPPlayAdviceLyt::Reset() {
    mpStateMachine->Reset(0);
    mpInAnmObj->resetFrame();
    mpInAnmObj->stop();
    mpInAnmObj->setFrame(sAdviceStartFrame);
    mpAButtonInAnmObj->resetFrame();
    mpAButtonInAnmObj->stop();
    mpAButtonInAnmObj->setFrame(sAdviceStartFrame);
    mpBlinkAnmObj->resetFrame();
    mpBlinkAnmObj->stop();
    mpBlinkAnmObj->setFrame(sAdviceStartFrame);
    mpOutAnmObj->resetFrame();
    mpOutAnmObj->stop();
    mpOutAnmObj->setFrame(sAdviceStartFrame);
    mpLayout->unbindAllAnmObj();
    mpBlinkAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_Loop);
}

void RPPlayAdviceLyt::Calculate() {
    mpStateMachine->Calculate();
    if (mpStateMachine->GetState() != 0) {
        mpLayout->calc();
    }
}

void RPPlayAdviceLyt::InEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpInAnmObj);
    mpInAnmObj->restart();
}

void RPPlayAdviceLyt::InCalc() {
    if (mpInAnmObj->isFinished() && RPSysSaveDataMgr::instance()->isAsyncFinish()) {
        mpStateMachine->ChangeState(2);
    }
}

void RPPlayAdviceLyt::AButtonInEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAButtonInAnmObj);
    mpAButtonInAnmObj->restart();
}

void RPPlayAdviceLyt::AButtonInCalc() {
    if (mpAButtonInAnmObj->isFinished()) {
        mpStateMachine->ChangeState(3);
    }
}

void RPPlayAdviceLyt::BlinkEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpBlinkAnmObj);
    mpBlinkAnmObj->restart();
}

void RPPlayAdviceLyt::BlinkCalc() {}

void RPPlayAdviceLyt::DecideEnter() {
    RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_A_04_CTSP", mDecideTimer);
    RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_A_04_TVSP", mDecideTimer);
    mpBlinkAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
}

void RPPlayAdviceLyt::DecideCalc() {
    if (mpBlinkAnmObj->isFinished()) {
        if (mVisible) {
            mpStateMachine->ChangeState(5);
        } else {
            mpStateMachine->ChangeState(6);
        }
    }
}

void RPPlayAdviceLyt::OutEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpOutAnmObj);
    mpOutAnmObj->restart();
}

void RPPlayAdviceLyt::OutCalc() {
    if (mpOutAnmObj->isFinished()) {
        mpStateMachine->ChangeState(6);
    }
}

void RPPlayAdviceLyt::Draw() {
    if (mpStateMachine->GetState() != 0) {
        mpLayout->draw();
    }
}

void RPPlayAdviceLyt::Show(s32 advice, bool visible) {
    if (mpStateMachine->IsStateSettled(0)) {
        RPSysMessage* pMessage = RPPlayerConfirm::sInstance->mpMessage;
        RPSysLytTextBox* pText = mpLayout->findTextBox("T_advice_00");
        RPSysLytPicture* pBreak = mpLayout->findPicture("P_break_00");
        RPSysLytPicture* pRange = mpLayout->findPicture("P_range_00");
        if (advice == 0) {
            pText->setMessage(pMessage, 901, 0, 0);
            pBreak->SetVisible(true);
            pRange->SetVisible(false);
        } else {
            pText->setMessage(pMessage, 900, 0, 0);
            pBreak->SetVisible(false);
            pRange->SetVisible(true);
        }
        mVisible = visible;
        mpStateMachine->ChangeState(1);
    }
}

bool RPPlayAdviceLyt::PressA(s32 playerFlag) {
    if (!mpStateMachine->IsStateSettled(3)) {
        return false;
    }
    mDecideTimer = playerFlag;
    mpStateMachine->ChangeState(4);
    return true;
}

#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPPlayAdviceLyt>::RPStateMachine(RPPlayAdviceLyt*, s32);
template void RPStateMachine<RPPlayAdviceLyt>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template void RPStateMachine<RPPlayAdviceLyt>::Reset(s32);
template void RPStateMachine<RPPlayAdviceLyt>::ChangeState(s32);
template void RPStateMachine<RPPlayAdviceLyt>::Calculate();
template void RPStateMachine<RPPlayAdviceLyt>::CallExit();
template void RPStateMachine<RPPlayAdviceLyt>::CallCalc();
template void RPStateMachine<RPPlayAdviceLyt>::CallEnter();

const f32 sAdviceStartFrame = 0.0f;
