// Two-player winner banner layout (RPVsWinnerLyt, ptVsWinner_00.brlyt).
// Unit 0x801ECA34-0x801ED2D0. Functions in reverse address order; the state machine
// instantiation follows at the end of the file.
#include <Pack/RPParty/RPPartyCommon/RPVsWinner.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel.h>

namespace {
struct RPVsWinnerLytStateFuncs {
    RPVsWinnerLytSeq::StateFunc mEnter; // at 0x0
    RPVsWinnerLytSeq::StateFunc mCalc;  // at 0xC
};
} // namespace

RPVsWinnerLyt::RPVsWinnerLyt() {
    mpStateMachine = new RPVsWinnerLytSeq(this, 5);

    RPVsWinnerLytStateFuncs funcs[5] = {
        {NULL, NULL},
        {&RPVsWinnerLyt::EnterIn, &RPVsWinnerLyt::CalcIn},
        {&RPVsWinnerLyt::EnterShow, &RPVsWinnerLyt::CalcShow},
        {&RPVsWinnerLyt::EnterOut, &RPVsWinnerLyt::CalcOut},
        {NULL, NULL},
    };
    for (int i = 0; i < 5; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }
}

void RPVsWinnerLyt::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "ptVsResult.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "ptVsWinner_00.brlyt");

    const char* anmNames[2] = {"ptVsWinner_00_in_00.brlan", "ptVsWinner_00_out_00.brlan"};
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    const char* paneNames[RP_PARTY_MAX_PLAYERS] = {"N_pLPosition_00", "N_pRPosition_00"};
    for (int i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
        mpPlayerPosPanes[i] = mpLayout->findPane(paneNames[i]);
    }

    mpTrophyPosPane = mpLayout->findPane("N_tPosition_00");
}

void RPVsWinnerLyt::Reset() {
    mpStateMachine->Reset(0);

    mpAnmObjs[0]->resetFrame();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
    mpAnmObjs[1]->resetFrame();
    mpAnmObjs[1]->stop();
    mpAnmObjs[1]->setFrame(0.0f);

    mpLayout->unbindAllAnmObj();
}

void RPVsWinnerLyt::Calculate() {
    mpStateMachine->Calculate();

    if (mpStateMachine->GetState() != 0 && mpStateMachine->GetState() != 4) {
        mpLayout->calc();
    }
}

void RPVsWinnerLyt::EnterIn() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();

    RPSndAudioMgr::instance()->startSystemSe("RP_COM_GAME_SE_2P_WIN_LOSE_01");
}

void RPVsWinnerLyt::CalcIn() {
    if (mpAnmObjs[0]->isFinished()) {
        mpStateMachine->ChangeState(2);
    }
}

void RPVsWinnerLyt::EnterShow() {}

void RPVsWinnerLyt::CalcShow() {}

void RPVsWinnerLyt::EnterOut() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
}

void RPVsWinnerLyt::CalcOut() {
    if (mpAnmObjs[1]->isFinished()) {
        mpStateMachine->ChangeState(4);
    }
}

void RPVsWinnerLyt::DrawLayer1A() {}

void RPVsWinnerLyt::DrawLayer1C() {}

void RPVsWinnerLyt::Open() {
    if (mpStateMachine->GetState() == 0) {
        mpStateMachine->ChangeState(1);
    }
}

void RPVsWinnerLyt::Close() {
    if (!mpStateMachine->IsStateOrNext(3)) {
        switch (mpStateMachine->IsStateOrNext(4)) {
        case false:
            mpStateMachine->ChangeState(3);
            break;
        }
    }
}

EGG::Vector2f RPVsWinnerLyt::GetPlayerPanePos(s32 player) const {
    const nw4r::math::MTX34& rMtx = mpPlayerPosPanes[player]->GetGlobalMtx();
    return EGG::Vector2f(rMtx._03, rMtx._13);
}

EGG::Vector2f RPVsWinnerLyt::GetTrophyPanePos() const {
    const nw4r::math::MTX34& rMtx = mpTrophyPosPane->GetGlobalMtx();
    return EGG::Vector2f(rMtx._03, rMtx._13);
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPVsWinnerLyt>::RPStateMachine(RPVsWinnerLyt*, s32);
template void RPStateMachine<RPVsWinnerLyt>::SetStateFuncs(const StateFunc&, const StateFunc&,
                                                           s32);
template void RPStateMachine<RPVsWinnerLyt>::Reset(s32);
template void RPStateMachine<RPVsWinnerLyt>::ChangeState(s32);
template void RPStateMachine<RPVsWinnerLyt>::Calculate();
template void RPStateMachine<RPVsWinnerLyt>::CallExit();
template void RPStateMachine<RPVsWinnerLyt>::CallCalc();
template void RPStateMachine<RPVsWinnerLyt>::CallEnter();
template bool RPStateMachine<RPVsWinnerLyt>::IsStateOrNext(s32) const;
