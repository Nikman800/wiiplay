// Billiards foul message window (RPBilMsgWindow, billiard_01.brlyt, singleton RPBilMsgWindow::sInstance).
// Unit 0x802CE3B8-0x802CED28. Functions in reverse address order; the state machine
// instantiation follows at the end of the file.
#include <Pack/RPParty/RPBilScene/RPBilLyt.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPSystem.h>

RPBilMsgWindow* RPBilMsgWindow::sInstance;

inline RPBilMsgWindow::RPBilMsgWindow() {
    mpStateMachine = new RPStateMachine<RPBilMsgWindow>(this, STATE_NUM);

    RPBilStateFuncs<RPBilMsgWindow> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPBilMsgWindow::OpeningEnter, &RPBilMsgWindow::OpeningCalc},
        {&RPBilMsgWindow::OpenEnter, &RPBilMsgWindow::OpenCalc},
        {&RPBilMsgWindow::CloseEnter, &RPBilMsgWindow::CloseCalc},
    };
    for (int i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));
}

void RPBilMsgWindow::CreateInstance() {
    sInstance = new RPBilMsgWindow();
}

void RPBilMsgWindow::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilMsgWindow::~RPBilMsgWindow() {}

void RPBilMsgWindow::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "billiard.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "billiard_01.brlyt");

    const char* anmNames[2] = {"billiard_01_inAll_00.brlan", "billiard_01_outAll_00.brlan"};
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x8FD, 0, 0);
}

void RPBilMsgWindow::Reset() {
    mpStateMachine->Reset(STATE_CLOSED);

    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }

    mpLayout->unbindAllAnmObj();
}

void RPBilMsgWindow::Calculate() {
    mpStateMachine->Calculate();
    mpLayout->calc();
}

void RPBilMsgWindow::OpeningEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
}

void RPBilMsgWindow::OpeningCalc() {
    if (mpAnmObjs[0]->isFinished()) {
        mpStateMachine->ChangeState(STATE_OPEN);
    }
}

void RPBilMsgWindow::OpenEnter() {}

void RPBilMsgWindow::OpenCalc() {}

void RPBilMsgWindow::CloseEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
}

void RPBilMsgWindow::CloseCalc() {
    if (mpAnmObjs[1]->isFinished()) {
        mpStateMachine->ChangeState(STATE_CLOSED);
    }
}

void RPBilMsgWindow::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() != 9) {
        return;
    }
    if (RPGrpRenderer::GetDrawStep() != 5) {
        return;
    }
    if (mpStateMachine->GetState() == STATE_CLOSED) {
        return;
    }
    mpLayout->draw();
}

void RPBilMsgWindow::ShowFoul(s32 foul) {
    if (mpStateMachine->IsStateSettled(STATE_CLOSED)) {
        mFoul = foul;
        mpStateMachine->ChangeState(STATE_OPENING);

        const s32 msgIndices[4] = {4, 2, 3, 1};
        mpLayout->findTextBox("T_msgText_01")
            ->setMessage(RPCls803572E0::GetInstance()->GetMessage(), 0x8FD, msgIndices[foul], 0);
    }
}

void RPBilMsgWindow::Close() {
    if (!mpStateMachine->IsStateOrNext(STATE_CLOSE)) {
        switch (mpStateMachine->IsStateOrNext(STATE_CLOSED)) {
        case false:
            mpStateMachine->ChangeState(STATE_CLOSE);
            break;
        }
    }
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPBilMsgWindow>::RPStateMachine(RPBilMsgWindow*, s32);
template void RPStateMachine<RPBilMsgWindow>::SetStateFuncs(const StateFunc&, const StateFunc&,
                                                            s32);
template void RPStateMachine<RPBilMsgWindow>::Reset(s32);
template void RPStateMachine<RPBilMsgWindow>::ChangeState(s32);
template void RPStateMachine<RPBilMsgWindow>::Calculate();
template void RPStateMachine<RPBilMsgWindow>::CallExit();
template void RPStateMachine<RPBilMsgWindow>::CallCalc();
template void RPStateMachine<RPBilMsgWindow>::CallEnter();
template bool RPStateMachine<RPBilMsgWindow>::IsStateSettled(s32) const;
template bool RPStateMachine<RPBilMsgWindow>::IsStateOrNext(s32) const;
