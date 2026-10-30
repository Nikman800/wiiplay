// "New rank" banner on the single-player result screen (RPRankGetLyt, ptRankGet_00.brlyt).
// Unit 0x801E4BD8-0x801E556C. Functions in reverse address order; the state machine
// instantiation follows at the end of the file.
#include <Pack/RPParty/RPPartyCommon/RPMedalResult.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>

namespace {
struct RPRankGetStepMsgs {
    s32 mIdx[9];
};

const RPRankGetStepMsgs sStepMsgs = {{0, 1, 2, 3, 4, 5, 7, 8, 9}};

struct RPRankGetLytStateFuncs {
    RPRankGetSeq::StateFunc mEnter; // at 0x0
    RPRankGetSeq::StateFunc mCalc;  // at 0xC
};
} // namespace

RPRankGetLyt::RPRankGetLyt() {
    mpStateMachine = new RPRankGetSeq(this, 5);

    RPRankGetLytStateFuncs funcs[5] = {
        {NULL, NULL},
        {&RPRankGetLyt::EnterIn, &RPRankGetLyt::CalcIn},
        {&RPRankGetLyt::EnterShow, &RPRankGetLyt::CalcShow},
        {&RPRankGetLyt::EnterOut, &RPRankGetLyt::CalcOut},
        {NULL, NULL},
    };
    for (int i = 0; i < 5; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }
}

void RPRankGetLyt::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "myRecord.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "ptRankGet_00.brlyt");

    const char* anmNames[2] = {"ptRankGet_00_in_00.brlan", "ptRankGet_00_out_00.brlan"};
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    RPCommon* pCommon = RPCommon::GetInstance();
    mpLayout->findTextBox("T_rankGet_00")->setMessage(pCommon->GetMessage(), 0x1F4A, 0, 0);
    mpLayout->findTextBox("T_rankGet_01")->setMessage(pCommon->GetMessage(), 0x1F4A, 0, 0);
    mpLayout->findTextBox("T_congrats_00")->setMessage(pCommon->GetMessage(), 0x1F4D, 0, 0);
    mpLayout->findTextBox("T_congrats_01")->setMessage(pCommon->GetMessage(), 0x1F4D, 0, 0);

    mpPlayerPosPane = mpLayout->findPane("N_pPosition_00");
    mpRankPosPane = mpLayout->findPane("N_rPosition_00");
}

void RPRankGetLyt::Reset() {
    mpStateMachine->Reset(0);

    mpAnmObjs[0]->resetFrame();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
    mpAnmObjs[1]->resetFrame();
    mpAnmObjs[1]->stop();
    mpAnmObjs[1]->setFrame(0.0f);

    mpLayout->unbindAllAnmObj();
}

void RPRankGetLyt::Calculate() {
    mpStateMachine->Calculate();

    if (mpStateMachine->GetState() != 0 && mpStateMachine->GetState() != 4) {
        mpLayout->calc();
    }
}

void RPRankGetLyt::EnterIn() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
}

void RPRankGetLyt::CalcIn() {
    if (mpAnmObjs[0]->isFinished()) {
        mpStateMachine->ChangeState(2);
    }
}

void RPRankGetLyt::EnterShow() {}

void RPRankGetLyt::CalcShow() {}

void RPRankGetLyt::EnterOut() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
}

void RPRankGetLyt::CalcOut() {
    if (mpAnmObjs[1]->isFinished()) {
        mpStateMachine->ChangeState(4);
    }
}

void RPRankGetLyt::DrawLayer1A() {}

void RPRankGetLyt::DrawLayer1C() {
    if (mpStateMachine->GetState() != 0 && mpStateMachine->GetState() != 4) {
        mpLayout->draw();
    }
}

// Wii Play: compiled at a lower optimization level (the game index and rank - 1 stay in
// registers across the calls)
#pragma push
#pragma optimization_level 3
void RPRankGetLyt::Open(s32 rank) {
    mRank = rank;

    if (mpStateMachine->GetState() == 0) {
        mpStateMachine->ChangeState(1);

        RPRankGetStepMsgs stepMsgs = sStepMsgs;
        RPCommon* pCommon = RPCommon::GetInstance();
        s32 game = pCommon->GetGame();
        mpLayout->findTextBox("T_step_00")
            ->setMessage(pCommon->GetMessage(), 0x1F4B, stepMsgs.mIdx[game], 0);
        s32 rankIdx = rank - 1;
        mpLayout->findTextBox("T_rankGet_00")
            ->setMessage(pCommon->GetMessage(), 0x1F4C, rankIdx, 0);
        mpLayout->findTextBox("T_rankGet_01")
            ->setMessage(pCommon->GetMessage(), 0x1F4C, rankIdx, 0);
    }
}
#pragma pop

void RPRankGetLyt::Close() {
    if (!mpStateMachine->IsStateOrNext(3)) {
        switch (mpStateMachine->IsStateOrNext(4)) {
        case false:
            mpStateMachine->ChangeState(3);
            break;
        }
    }
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPRankGetLyt>::RPStateMachine(RPRankGetLyt*, s32);
template void RPStateMachine<RPRankGetLyt>::SetStateFuncs(const StateFunc&, const StateFunc&,
                                                          s32);
template void RPStateMachine<RPRankGetLyt>::Reset(s32);
template void RPStateMachine<RPRankGetLyt>::ChangeState(s32);
template void RPStateMachine<RPRankGetLyt>::Calculate();
template void RPStateMachine<RPRankGetLyt>::CallExit();
template void RPStateMachine<RPRankGetLyt>::CallCalc();
template void RPStateMachine<RPRankGetLyt>::CallEnter();
template bool RPStateMachine<RPRankGetLyt>::IsStateOrNext(s32) const;
