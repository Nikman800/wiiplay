// "Paused" text shown per player (RPPauseTextLyt, ptPauseText_00.brlyt).
// Unit 0x801FAC1C-0x801FB680. Functions in reverse address order; the state machine
// instantiation follows at the end of the file.
#include <Pack/RPParty/RPPartyCommon/RPPauseText.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

namespace {
struct RPPauseTextLytStateFuncs {
    RPPauseTextSeq::StateFunc mEnter; // at 0x0
    RPPauseTextSeq::StateFunc mCalc;  // at 0xC
};

// RPSceneInfo::unk2C: pause text allowed
bool IsPauseTextAllowed() {
    return RPSceneInfo::spInstance->unk2C;
}
} // namespace

RPPauseTextLyt::RPPauseTextLyt() {
    for (s32 i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
        mpStateMachines[i] = new RPPauseTextSeq(this, 4);

        RPPauseTextLytStateFuncs funcs[4] = {
            {&RPPauseTextLyt::EnterIdle, &RPPauseTextLyt::CalcIdle},
            {&RPPauseTextLyt::EnterIn, &RPPauseTextLyt::CalcIn},
            {&RPPauseTextLyt::EnterShown, &RPPauseTextLyt::CalcShown},
            {&RPPauseTextLyt::EnterOut, &RPPauseTextLyt::CalcOut},
        };
        for (s32 j = 0; j < 4; j++) {
            mpStateMachines[i]->SetStateFuncs(funcs[j].mEnter, funcs[j].mCalc, j);
        }
    }

    mpResAccessor = RPSysLytResAccessor::create(NULL);
}

void RPPauseTextLyt::Load() {
    RPLytObjBase::AttachArcFile(RPSysResourceManager::instance()->GetStaticLocalArchive(),
                                "ptPauseText.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "ptPauseText_00.brlyt");

    const char* anmNames[2] = {"ptPauseText_00_in_00.brlan", "ptPauseText_00_out_00.brlan"};
    for (s32 i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
        for (s32 j = 0; j < 2; j++) {
            mpAnmObjs[i][j] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[j]);
        }
    }

    mpPausePanes[0] = mpLayout->findPane("N_pause_00");
    mpPausePanes[1] = mpLayout->findPane("N_pause_01");

    void* pMsgRes = RPSysResourceManager::GetMessageResource("party_message.bmg");
    mpMessage = new RPSysMessage(pMsgRes, NULL);

    mpLayout->findTextBox("T_pause_00")->setMessage(mpMessage, 0x2383, 0, 0);
    mpLayout->findTextBox("T_pause_01")->setMessage(mpMessage, 0x2383, 0, 0);
}

void RPPauseTextLyt::Reset() {
    for (s32 i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
        mpStateMachines[i]->Reset(0);
        mShowRequest[i] = false;
        mHideRequest[i] = false;
    }

    for (s32 i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
        for (s32 j = 0; j < 2; j++) {
            mpAnmObjs[i][j]->resetFrame();
            mpAnmObjs[i][j]->stop();
            mpAnmObjs[i][j]->setFrame(0.0f);
        }
    }

    mpLayout->unbindAllAnmObj();

    for (s32 i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
        mpPausePanes[i]->UnbindAllAnimation(true);
    }
}

void RPPauseTextLyt::Calculate() {
    for (s32 i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
        mCurPlayer = i;
        mpStateMachines[i]->Calculate();
    }

    bool visible = false;
    for (s32 i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
        if (mpStateMachines[i]->GetState() != 0) {
            visible = true;
        }
    }

    if (visible) {
        mpLayout->calc();
    }
}

void RPPauseTextLyt::EnterIdle() {
    mpPausePanes[mCurPlayer]->SetVisible(false);
}

void RPPauseTextLyt::CalcIdle() {
    if (mShowRequest[mCurPlayer]) {
        mpStateMachines[mCurPlayer]->ChangeState(1);
    }
}

void RPPauseTextLyt::EnterIn() {
    mpPausePanes[mCurPlayer]->SetVisible(true);
    mShowRequest[mCurPlayer] = false;
    mpPausePanes[mCurPlayer]->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[mCurPlayer][0], mpPausePanes[mCurPlayer], true);
    mpAnmObjs[mCurPlayer][0]->restart();
}

void RPPauseTextLyt::CalcIn() {
    if (mpAnmObjs[mCurPlayer][0]->isFinished()) {
        mpStateMachines[mCurPlayer]->ChangeState(2);
    }
}

void RPPauseTextLyt::EnterShown() {}

void RPPauseTextLyt::CalcShown() {
    if (mHideRequest[mCurPlayer]) {
        mpStateMachines[mCurPlayer]->ChangeState(3);
    }
}

void RPPauseTextLyt::EnterOut() {
    mHideRequest[mCurPlayer] = false;
    mpPausePanes[mCurPlayer]->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[mCurPlayer][1], mpPausePanes[mCurPlayer], true);
    mpAnmObjs[mCurPlayer][1]->restart();
}

void RPPauseTextLyt::CalcOut() {
    if (mpAnmObjs[mCurPlayer][1]->isFinished()) {
        mpStateMachines[mCurPlayer]->ChangeState(0);
    }
}

void RPPauseTextLyt::Draw() {
    bool visible = false;
    for (s32 i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
        if (mpStateMachines[i]->GetState() != 0) {
            visible = true;
        }
    }

    if (visible) {
        mpLayout->draw();
    }
}

void RPPauseTextLyt::Show(s32 player) {
    if (IsPauseTextAllowed() && mpStateMachines[player]->GetState() == 0) {
        mShowRequest[player] = true;
        mHideRequest[player] = false;
    }
}

void RPPauseTextLyt::Hide(s32 player) {
    mHideRequest[player] = true;
    mShowRequest[player] = false;
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPPauseTextLyt>::RPStateMachine(RPPauseTextLyt*, s32);
template void RPStateMachine<RPPauseTextLyt>::SetStateFuncs(const StateFunc&, const StateFunc&,
                                                            s32);
template void RPStateMachine<RPPauseTextLyt>::Reset(s32);
template void RPStateMachine<RPPauseTextLyt>::ChangeState(s32);
template void RPStateMachine<RPPauseTextLyt>::Calculate();
template void RPStateMachine<RPPauseTextLyt>::CallEnter();
template void RPStateMachine<RPPauseTextLyt>::CallCalc();
template void RPStateMachine<RPPauseTextLyt>::CallExit();
