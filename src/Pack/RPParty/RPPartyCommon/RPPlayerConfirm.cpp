// RPPlayerConfirm: second source file of the ledger unit RPPlayerConfirmLyt.cpp
// (.text 801E1B18-801E2C94). Needs the RPPlayerConfirm.h change (Params, Reset(RPMenuFrame*),
// Start(const Params&)) and the ledger renames listed in the report.
#include <Pack/RPParty/RPPartyCommon/RPPlayerConfirm.h>

#include <Pack/RPParty/RPPartyCommon/RPPlayerConfirmLyt.h>
#include <Pack/RPParty/RPPartyCommon/RPPlayAdviceLyt.h>
#include <Pack/RPParty/RPPartyCommon/RPABPress.h>
#include <Pack/RPParty/RPPartyCommon/RPCPadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPMenuFrame.h>
#include <Pack/RPKernel/RPSysController.h>
#include <Pack/RPKernel/RPSysCursorDrawMgr.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPSystem/RPSysPlayerMgr.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

RPPlayerConfirm::Params::Params() {
    playerNum = 0;
    for (s32 i = 0; i < 4; i++) {
        pHeads[i] = NULL;
        controllers[i] = -1;
    }
    adviceId = -1;
    checkABPress = false;
}

#pragma auto_inline off

void RPPlayerConfirm::CreateInstance(u32 noABPress) {
    sInstance = new RPPlayerConfirm(noABPress);
}

RPPlayerConfirm::~RPPlayerConfirm() {}

void RPPlayerConfirm::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPPlayerConfirm::RPPlayerConfirm(u32 noABPress) {
    typedef RPStateMachine<RPPlayerConfirm> StateMachine;
    typedef StateMachine::StateFunc StateFunc;
    mNoABPress = noABPress;
    mpStateMachine = new StateMachine(this, 9);
    StateFunc funcs[9][2] = {
        {NULL, NULL},
        {&RPPlayerConfirm::InEnter, &RPPlayerConfirm::InCalc},
        {&RPPlayerConfirm::SelectEnter, &RPPlayerConfirm::SelectCalc},
        {&RPPlayerConfirm::DecideEnter, &RPPlayerConfirm::DecideCalc},
        {&RPPlayerConfirm::BackEnter, &RPPlayerConfirm::BackCalc},
        {&RPPlayerConfirm::AdviceEnter, &RPPlayerConfirm::AdviceCalc},
        {&RPPlayerConfirm::MessageEnter, &RPPlayerConfirm::MessageCalc},
        {&RPPlayerConfirm::HideEnter, &RPPlayerConfirm::HideCalc},
        {&RPPlayerConfirm::DoneEnter, NULL},
    };
    s32 i = 0;
    do {
        mpStateMachine->SetStateFuncs(funcs[i][0], funcs[i][1], i);
    } while (++i < 9);
    mpLayout = new RPPlayerConfirmLyt;
    mpAdviceLyt = new RPPlayAdviceLyt;
    mpMenuFrame = NULL;
    mpCursor = new RPCPadCursor;
    if (mNoABPress == 0) {
        mpABPress = new RPABPress;
    } else {
        mpABPress = NULL;
    }
}

void RPPlayerConfirm::Load() {
    EGG::Archive* pStaticArchive = RPSysResourceManager::instance()->GetStaticLocalArchive();
    EGG::Archive* pGameArchive = RPSysResourceManager::LoadGameLocalArchive(-1, NULL);
    void* pMessageRes = RPSysResourceManager::GetMessageResource("common_message.bmg");
    mpMessage = new RPSysMessage(pMessageRes, NULL);
    mpLayout->Load(pGameArchive);
    mpAdviceLyt->Load(pGameArchive);
    mpCursor->Load(pStaticArchive);
    if (mpABPress != NULL) {
        mpABPress->Load(pStaticArchive, mpMessage, false);
    }
}

void RPPlayerConfirm::Reset(RPMenuFrame* pMenuFrame) {
    mpStateMachine->Reset(0);
    mResult = -1;
    mSelect = 1;
    mLastSelect = 1;
    mPointerPos = EGG::Vector2f::zero;
    mHitButton = -1;
    mPrevHitButton = -1;
    mOnBackButton = false;
    mpMenuFrame = pMenuFrame;
    mpLayout->Reset();
    mpAdviceLyt->Reset();
    mpCursor->Reset();
    mpCursor->SelectStyle(0);
    mpCursor->SetActive(0);
    if (mpABPress != NULL) {
        mpABPress->Reset();
    }
}

void RPPlayerConfirm::Calculate() {
    mPointerPos = RPSysCursorDrawMgr::instance()->getActiveCursorPos();
    mPrevHitButton = mHitButton;
    mHitButton = mpLayout->HitTestButtons(mPointerPos);
    mpStateMachine->Calculate();
    s32 state = mpStateMachine->GetState();
    if (state == 0 || (state == 8 && mResult != 0)) {
        return;
    }
    if (mSelect != -1) {
        mLastSelect = mSelect;
    }
    mpLayout->Calculate();
    mpAdviceLyt->Calculate();
    mpCursor->Calculate();
    if (mpABPress != NULL) {
        mpABPress->Calculate();
    }
}

void RPPlayerConfirm::InEnter() {
    mpLayout->Show();
    const s32 select = mSelect;
    mPadMode = true;
    mHitButton = select;
}

void RPPlayerConfirm::InCalc() {
    if (mpLayout->mpStateMachine->IsStateSettled(2)) {
        mpStateMachine->ChangeState(2);
    }
}

void RPPlayerConfirm::SelectEnter() {
    mPrevHitButton = -1;
}

void RPPlayerConfirm::SelectCalc() {
    s32 prevSelect = mSelect;
    if (mHitButton != -1) {
        mSelect = mHitButton;
        mPadMode = false;
        if (!mpMenuFrame->IsPointOnBackButton(mPointerPos)) {
            if (mHitButton != mPrevHitButton || mOnBackButton) {
                RPSysCoreController::getActiveController()->rumble(
                    RPSysCoreController::ERumblePattern_Cursor);
                mpLayout->PlayCursorSe(mSelect);
            }
        }
    } else if (!mPadMode) {
        if (RPSysCoreController::getActiveController()->downTrigger(0xF000F)) {
            mPadMode = true;
            mSelect = mLastSelect;
            mpLayout->PlayCursorSe(mSelect);
        } else {
            mSelect = -1;
        }
    } else {
        if (RPSysCoreController::getActiveController()->getCursorHold() & 0x40001) {
            mSelect = 0;
        } else if (RPSysCoreController::getActiveController()->getCursorHold() & 0x80002) {
            mSelect = 1;
        }
        if (mSelect != prevSelect) {
            mpLayout->PlayCursorSe(mSelect);
        }
    }
    if (mpMenuFrame->IsPointOnBackButton(mPointerPos)) {
        mpMenuFrame->SetBackButtonActive(true,
                                         RPSysCoreController::getActiveController()->getPlayerNo());
        mSelect = -1;
        mPadMode = false;
        mOnBackButton = true;
    } else {
        mpMenuFrame->SetBackButtonActive(false, -1);
        mOnBackButton = false;
    }
    if (mSelect != -1) {
        mpLayout->ActivateButton(mSelect);
    } else {
        mpLayout->DeactivateButtons();
    }
    if (mSelect != -1 && RPSysCoreController::getActiveController()->downTrigger(0x800)) {
        mpStateMachine->ChangeState(3);
    } else if (mpMenuFrame->IsBackButtonIdle()) {
        if (RPSysCoreController::getActiveController()->downTrigger(0x1400) ||
            (mpMenuFrame->IsPointOnBackButton(mPointerPos) &&
             RPSysCoreController::getActiveController()->downTrigger(0x800))) {
            mpStateMachine->ChangeState(4);
            mSelect = -1;
            mPadMode = false;
        }
    }
    if (mPadMode) {
        mpCursor->SetActive(1);
        mpCursor->SetTarget(mpLayout->mpButtonBoundings[mSelect]);
    } else {
        mpCursor->SetActive(0);
    }
}

void RPPlayerConfirm::DecideEnter() {
    mpLayout->PushButton(mSelect, RPSysPlayerMgr::instance()->getSpeakerFlag(-1));
    mPadMode = false;
    mpCursor->SetActive(0);
    switch (mSelect) {
    case -1:
        break;
    case 1:
        mResult = 0;
        break;
    case 0:
        mResult = 1;
        break;
    }
}

void RPPlayerConfirm::DecideCalc() {
    if (mSelect != -1) {
        mpLayout->ActivateButton(mSelect);
    } else {
        mpLayout->DeactivateButtons();
    }
    if (mpLayout->IsPushDone()) {
        if (mSelect == 1) {
            if (mParams.adviceId == -1) {
                mpStateMachine->ChangeState(8);
            } else {
                mpStateMachine->ChangeState(5);
            }
        } else {
            mpStateMachine->ChangeState(7);
        }
    }
}

void RPPlayerConfirm::BackEnter() {
    mpMenuFrame->PushBackButton(RPSysPlayerMgr::instance()->getSpeakerFlag(-1));
    mpCursor->SetActive(0);
    mResult = 2;
}

void RPPlayerConfirm::BackCalc() {
    mpLayout->DeactivateButtons();
    if (mpMenuFrame->IsBackPushDone()) {
        mpStateMachine->ChangeState(7);
    }
}

void RPPlayerConfirm::AdviceEnter() {
    bool checkABPress = false;
    if (mParams.checkABPress && mpABPress != NULL && mpABPress->IsNeeded(-1)) {
        checkABPress = true;
    }
    mpAdviceLyt->Show(mParams.adviceId, checkABPress);
    mpCursor->SetActive(0);
}

void RPPlayerConfirm::AdviceCalc() {
    if (mpAdviceLyt->mpStateMachine->IsStateSettled(3) &&
        RPSysCoreController::getActiveController()->downTrigger(0x800)) {
        mpAdviceLyt->PressA(RPSysPlayerMgr::instance()->getSpeakerFlag(-1));
    }
    if (mpAdviceLyt->mpStateMachine->IsStateSettled(6)) {
        if (mParams.checkABPress && mpABPress != NULL && mpABPress->IsNeeded(-1)) {
            mpStateMachine->ChangeState(6);
        } else {
            mpStateMachine->ChangeState(8);
        }
    }
}

void RPPlayerConfirm::MessageEnter() {
    for (s32 i = 0; i < 4; i++) {
        mpABPress->mPlayers[i].mpHead = mParams.pHeads[i];
    }
    mpABPress->Start(-1);
}

void RPPlayerConfirm::MessageCalc() {
    if (mpABPress->IsDone()) {
        mpStateMachine->ChangeState(8);
    }
}

void RPPlayerConfirm::HideEnter() {
    mpLayout->Hide();
}

void RPPlayerConfirm::HideCalc() {
    if (mpLayout->mpStateMachine->IsStateSettled(5)) {
        mpStateMachine->ChangeState(8);
    }
}

void RPPlayerConfirm::DoneEnter() {}

void RPPlayerConfirm::DrawLayout() {
    s32 state = mpStateMachine->GetState();
    if (state == 0 || (state == 8 && mResult != 0)) {
        return;
    }
    mpLayout->Draw();
}

void RPPlayerConfirm::DrawCursor() {
    s32 state = mpStateMachine->GetState();
    if (state == 0 || (state == 8 && mResult != 0)) {
        return;
    }
    mpCursor->Draw();
}

void RPPlayerConfirm::DrawOverlay() {
    s32 state = mpStateMachine->GetState();
    if (state == 0 || (state == 8 && mResult != 0)) {
        return;
    }
    mpAdviceLyt->Draw();
    if (mpABPress != NULL) {
        mpABPress->Draw();
    }
}

void RPPlayerConfirm::Start(const Params& rParams) {
    Reset(mpMenuFrame);
    mParams = rParams;
    mResult = -1;
    mpStateMachine->ChangeState(1);
}

nw4r::lyt::Pane* RPPlayerConfirm::GetPlayerModelPane(s32 player) const {
    return mpLayout->GetNameTagModelPane(player);
}

#pragma defer_codegen off

template void RPStateMachine<RPPlayerConfirm>::Calculate();
template void RPStateMachine<RPPlayerConfirm>::ChangeState(s32);
template void RPStateMachine<RPPlayerConfirm>::Reset(s32);
template void RPStateMachine<RPPlayerConfirm>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template RPStateMachine<RPPlayerConfirm>::RPStateMachine(RPPlayerConfirm*, s32);
template bool RPStateMachine<RPPlayAdviceLyt>::IsStateSettled(s32) const;
template void RPStateMachine<RPPlayerConfirm>::CallEnter();
template void RPStateMachine<RPPlayerConfirm>::CallCalc();
template void RPStateMachine<RPPlayerConfirm>::CallExit();
