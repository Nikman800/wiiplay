// Result menu and its five-state sequence, 0x801E9204-0x801E9E54.
#include <Pack/RPParty/RPPartyCommon/RPMedalResult_5.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysController.h>
#include <Pack/RPKernel/RPSysCursorDrawMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPCPadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#pragma auto_inline off

RPResultMenu::~RPResultMenu() {}

RPResultMenu::RPResultMenu() {
    mpStateMachine = new RPResultMenuSeq(this, 5);

    typedef RPResultMenuSeq::StateFunc StateFunc;
    struct StateFuncs {
        StateFunc mEnter;
        StateFunc mCalc;
    };
    StateFuncs funcs[5] = {
        {NULL, NULL},
        {&RPResultMenu::EnterShow, &RPResultMenu::CalcShow},
        {&RPResultMenu::EnterSelect, &RPResultMenu::CalcSelect},
        {&RPResultMenu::EnterDecide, &RPResultMenu::CalcDecide},
        {NULL, NULL},
    };
    for (s32 i = 0; i < 5; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    mpChoiceLyt = new RPChoiceLyt;
    mpCursor = new RPCPadCursor;
    mpVsResultLyt = new RPVsResultLyt;
}

void RPResultMenu::Load() {
    EGG::Archive* pArchive = RPSysResourceManager::instance()->GetStaticLocalArchive();
    mpChoiceLyt->Load(pArchive);
    mpCursor->Load(pArchive);
    mpVsResultLyt->Load(pArchive);
}

void RPResultMenu::Reset() {
    mpStateMachine->Reset(0);
    mChoice = 0;
    mLastChoice = 0;
    mHitButton = -1;
    mPrevHitButton = -1;
    mUsePad = true;
    mpChoiceLyt->Reset();
    mpCursor->Reset();
    mpCursor->SelectStyle(0);
    mpCursor->SetActive(0);
    mpVsResultLyt->Reset();
}

void RPResultMenu::Calculate() {
    const EGG::Vector2f& rPos = RPSysCursorDrawMgr::instance()->getActiveCursorPos();
    nw4r::math::VEC2 pos = rPos;
    mPrevHitButton = mHitButton;
    mHitButton = mpChoiceLyt->HitTest(pos);
    if (mChoice != -1) {
        mLastChoice = mChoice;
    }
    mpStateMachine->Calculate();
}

void RPResultMenu::EnterShow() {
    if (mPlayerNum >= 2) {
        RPSceneInfo* pInfo = RPSceneInfo::spInstance;
        ResultMenuSceneWinner(pInfo) = mWinner;
        if (mWinner == 0) {
            s32 streak = ResultMenuSceneStreak(pInfo) + 1;
            if (streak < -3) {
                streak = -3;
            } else if (streak > 3) {
                streak = 3;
            }
            ResultMenuSceneStreak(pInfo) = streak;
        }
        if (mWinner == 1) {
            s32 streak = ResultMenuSceneStreak(pInfo) - 1;
            if (streak < -3) {
                streak = -3;
            } else if (streak > 3) {
                streak = 3;
            }
            ResultMenuSceneStreak(pInfo) = streak;
        }

        s32 wins[RP_PARTY_MAX_PLAYERS];
        for (s32 i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
            wins[i] = ResultMenuSceneWins(pInfo, i);
            if (i == mWinner && wins[i] < 99) {
                wins[i]++;
                u32 count = wins[i];
                if (count > 99) {
                    count = 99;
                }
                ResultMenuSceneWins(pInfo, i) = count;
            }
        }
        mpVsResultLyt->Open(wins);
    }
    mpChoiceLyt->Open();
}

void RPResultMenu::CalcShow() {
    if (mpChoiceLyt->GetState() == 2 &&
        (mPlayerNum == 1 || mpVsResultLyt->GetState() == 2)) {
        mpStateMachine->ChangeState(2);
    }
    mpChoiceLyt->Calculate();
    mpCursor->Calculate();
    mpVsResultLyt->Calculate();
}

void RPResultMenu::EnterSelect() {}

void RPResultMenu::CalcSelect() {
    s32 prevChoice = mChoice;
    bool prevUsePad = mUsePad;
    if (mHitButton != -1) {
        mChoice = mHitButton;
        mUsePad = false;
    } else if (!mUsePad) {
        if (RPSysCoreController::getActiveController()->downTrigger(0xF000F)) {
            mUsePad = true;
            mChoice = mLastChoice;
        } else {
            mChoice = -1;
        }
    } else {
        s32 game = RPCommon::GetInstance()->GetGame();
        u32 buttons = RPSysCoreController::getActiveController()->getCursorHold();
        if ((buttons & 0x10008) || ((buttons & 0x80002) && game == 7)) {
            mChoice = 0;
        }
        buttons = RPSysCoreController::getActiveController()->getCursorHold();
        if ((buttons & 0x20004) || ((buttons & 0x40001) && game == 7)) {
            mChoice = 1;
        }
    }

    mpChoiceLyt->SetCursor(mChoice);
    if (mChoice != -1 && RPCommon::IsAPressed(mPlayerNum)) {
        mpStateMachine->ChangeState(3);
    }
    if (mUsePad) {
        mpCursor->SetActive(1);
        mpCursor->SetTarget(mpChoiceLyt->GetButtonPane(mChoice));
    } else {
        mpCursor->SetActive(0);
    }
    if (mChoice != -1 &&
        ((ResultMenuPreviousState(mpStateMachine) == 1 && mpStateMachine->GetTimer() == 0) ||
         mChoice != prevChoice || (prevUsePad && !mUsePad))) {
        mpChoiceLyt->PlayCursorSe(!mUsePad);
    }
    mpChoiceLyt->Calculate();
    mpCursor->Calculate();
    mpVsResultLyt->Calculate();
}

void RPResultMenu::EnterDecide() {
    mpChoiceLyt->Decide();
    if (mPlayerNum >= 2) {
        mpVsResultLyt->Close();
    }
    mpCursor->SetActive(0);
    if (mChoice == 0) {
        RPSndAudioMgr::instance()->setAutoFadeOnRetry();
        ResultMenuClearRetryFlag(RPSceneInfo::spInstance);
    }
}

void RPResultMenu::CalcDecide() {
    if (mpChoiceLyt->GetState() == 5 &&
        (mPlayerNum == 1 || mpVsResultLyt->GetState() == 4)) {
        mpStateMachine->ChangeState(4);
    }
    mpChoiceLyt->Calculate();
    mpCursor->Calculate();
    mpVsResultLyt->Calculate();
}

void RPResultMenu::DrawLayer1A() {
    if (mpStateMachine->GetState() != 0) {
        mpChoiceLyt->DrawLayer1A();
        mpVsResultLyt->DrawLayer1A();
    }
}

void RPResultMenu::DrawLayer1C() {
    if (mpStateMachine->GetState() != 0) {
        mpChoiceLyt->DrawLayer1C();
        mpCursor->Draw();
        mpVsResultLyt->DrawLayer1C();
    }
}

void RPResultMenu::Start(s32 playerNum, s32 winner) {
    if (mpStateMachine->GetState() == 0) {
        mPlayerNum = playerNum;
        mWinner = winner;
        RPSysCursorDrawMgr::instance()->startDpdCheck();
        RPSysCursorDrawMgr::instance()->createActiveCursor();
        mpStateMachine->ChangeState(1);
    }
}

s32 RPResultMenu::GetChoice() const {
    if (mpStateMachine->GetState() == 4) {
        return mChoice;
    }
    return -1;
}

#pragma defer_codegen off
template void RPStateMachine<RPResultMenu>::Calculate();
template void RPStateMachine<RPResultMenu>::ChangeState(s32);
template void RPStateMachine<RPResultMenu>::Reset(s32);
template void RPStateMachine<RPResultMenu>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template RPStateMachine<RPResultMenu>::RPStateMachine(RPResultMenu*, s32);
template void RPStateMachine<RPResultMenu>::CallEnter();
template void RPStateMachine<RPResultMenu>::CallCalc();
template void RPStateMachine<RPResultMenu>::CallExit();
