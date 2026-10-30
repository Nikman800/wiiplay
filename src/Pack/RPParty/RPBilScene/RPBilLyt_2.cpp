// Billiards score popup layout (RPBilScorePopupLyt, billiard_06.brlyt): "+N" score or a ball
// background. Unit 0x802D0EC8-0x802D17A4. Functions in reverse address order; the state
// machine instantiation follows at the end of the file.
#include <Pack/RPParty/RPBilScene/RPBilLyt_2.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPSystem.h>

RPBilScorePopupLyt::RPBilScorePopupLyt() {
    mpStateMachine = new RPStateMachine<RPBilScorePopupLyt>(this, STATE_NUM);

    RPBilStateFuncs<RPBilScorePopupLyt> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPBilScorePopupLyt::OpeningEnter, &RPBilScorePopupLyt::OpeningCalc},
        {&RPBilScorePopupLyt::OpenEnter, &RPBilScorePopupLyt::OpenCalc},
        {&RPBilScorePopupLyt::CloseEnter, &RPBilScorePopupLyt::CloseCalc},
    };
    for (int i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    mpLayout = NULL;
}

void RPBilScorePopupLyt::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "billiard.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "billiard_06.brlyt");

    const char* anmNames[2] = {"billiard_06_inAll_00.brlan", "billiard_06_outAll_00.brlan"};
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    mpLayout->findTextBox("T_msgText_00")
        ->setMessage(RPCls803572E0::GetInstance()->GetMessage(), 0x911, 0, 0);
    mBallBgY = mpLayout->findPane("N_ballBg_00")->GetTranslate().y;
}

void RPBilScorePopupLyt::Reset() {
    mpStateMachine->Reset(STATE_CLOSED);

    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }

    mpLayout->unbindAllAnmObj();
}

void RPBilScorePopupLyt::Calculate() {
    mpStateMachine->Calculate();
    mpLayout->calc();
}

void RPBilScorePopupLyt::OpeningEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
}

void RPBilScorePopupLyt::OpeningCalc() {
    if (mpAnmObjs[0]->isFinished()) {
        mpStateMachine->ChangeState(STATE_OPEN);
    }
}

void RPBilScorePopupLyt::OpenEnter() {}

void RPBilScorePopupLyt::OpenCalc() {}

void RPBilScorePopupLyt::CloseEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
}

void RPBilScorePopupLyt::CloseCalc() {
    if (mpAnmObjs[1]->isFinished()) {
        mpStateMachine->ChangeState(STATE_CLOSED);
    }
}

void RPBilScorePopupLyt::Draw() {
    mpLayout->draw();
}

void RPBilScorePopupLyt::Show(s32 type, s32 player, s32 score) {
    if (mpStateMachine->IsStateSettled(STATE_CLOSED)) {
        mType = type;
        mPlayer = player;
        mpStateMachine->ChangeState(STATE_OPENING);

        nw4r::lyt::Pane* pScorePane = mpLayout->findPane("N_score_00");
        RPSysLytTextBox* pScoreText = mpLayout->findTextBox("T_scoreText_00");
        if (mType == 1) {
            pScorePane->SetVisible(false);
        } else {
            pScorePane->SetVisible(true);
            wchar_t text[10];
            RPSysStringUtility::GetStringFullSizeNumber(score, text, 10, true);
            pScoreText->setText(text, 0);
        }
    }
}

void RPBilScorePopupLyt::Hide() {
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
template RPStateMachine<RPBilScorePopupLyt>::RPStateMachine(RPBilScorePopupLyt*, s32);
template void RPStateMachine<RPBilScorePopupLyt>::SetStateFuncs(const StateFunc&,
                                                                const StateFunc&, s32);
template void RPStateMachine<RPBilScorePopupLyt>::Reset(s32);
template void RPStateMachine<RPBilScorePopupLyt>::ChangeState(s32);
template void RPStateMachine<RPBilScorePopupLyt>::Calculate();
template void RPStateMachine<RPBilScorePopupLyt>::CallExit();
template void RPStateMachine<RPBilScorePopupLyt>::CallCalc();
template void RPStateMachine<RPBilScorePopupLyt>::CallEnter();
template bool RPStateMachine<RPBilScorePopupLyt>::IsStateSettled(s32) const;
template bool RPStateMachine<RPBilScorePopupLyt>::IsStateOrNext(s32) const;
