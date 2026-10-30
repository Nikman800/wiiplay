// Two-player win counts after a minigame round (RPVsResultLyt, ptVsResult_00.brlyt).
// Unit 0x801E9E54-0x801EA4A8. Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPResultMenu.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>

RPVsResultLyt::RPVsResultLyt() {}

void RPVsResultLyt::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "ptVsResult.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "ptVsResult_00.brlyt");

    const char* anmNames[2] = {"ptVsResult_00_in_00.brlan", "ptVsResult_00_out_00.brlan"};
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    RPCommon* pCommon = RPCommon::GetInstance();
    mpLayout->findTextBox("T_P1_00")->setMessage(pCommon->GetMessage(), 0x1F56, 0, 0);
    mpLayout->findTextBox("T_P1_01")->setMessage(pCommon->GetMessage(), 0x1F56, 0, 0);
    mpLayout->findTextBox("T_P2_00")->setMessage(pCommon->GetMessage(), 0x1F56, 1, 0);
    mpLayout->findTextBox("T_P2_01")->setMessage(pCommon->GetMessage(), 0x1F56, 1, 0);
}

void RPVsResultLyt::Reset() {
    mState = 0;
    mNextState = 0;
    mIsChanging = false;
    mStateTimer = 0;

    mpAnmObjs[0]->resetFrame();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
    mpAnmObjs[1]->resetFrame();
    mpAnmObjs[1]->stop();
    mpAnmObjs[1]->setFrame(0.0f);

    mpLayout->unbindAllAnmObj();
}

void RPVsResultLyt::Calculate() {
    typedef void (RPVsResultLyt::*StateFunc)();
    struct StateFuncs {
        StateFunc mEnter;
        StateFunc mCalc;
    };
    StateFuncs funcs[5] = {
        {NULL, NULL},
        {&RPVsResultLyt::EnterIn, &RPVsResultLyt::CalcIn},
        {NULL, NULL},
        {&RPVsResultLyt::EnterOut, &RPVsResultLyt::CalcOut},
        {NULL, NULL},
    };

    if (mIsChanging) {
        mState = mNextState;
        mIsChanging = false;
        mStateTimer = 0;

        StateFunc enter = funcs[mState].mEnter;
        if (enter != NULL) {
            (this->*enter)();
        }
    }

    StateFunc calc = funcs[mState].mCalc;
    if (calc != NULL) {
        (this->*calc)();
    }

    if (mState != 0 && mState != 4) {
        mpLayout->calc();
    }
}

void RPVsResultLyt::EnterIn() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();

    const char* winNames[RP_PARTY_MAX_PLAYERS][2] = {
        {"T_P1wins_00", "T_P1wins_01"},
        {"T_P2wins_00", "T_P2wins_01"},
    };
    RPCommon* pCommon = RPCommon::GetInstance();
    for (int i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
        for (int j = 0; j < 2; j++) {
            RPSysLytTextBox* pTextBox = mpLayout->findTextBox(winNames[i][j]);
            wchar_t number[20];
            RPSysStringUtility::GetStringFullSizeNumber(mWins[i], number, 20, false);
            pTextBox->setMessage(pCommon->GetMessage(), 0x1F54, 0, 1, number);
            if (!RPSysStringUtility::IsSingular(mWins[i])) {
                pTextBox->setMessage(pCommon->GetMessage(), 0x1F54, 1, 1, number);
            }
        }
    }
}

void RPVsResultLyt::CalcIn() {
    if (mpAnmObjs[0]->isFinished()) {
        mNextState = 2;
        mIsChanging = true;
    }
}

void RPVsResultLyt::EnterOut() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
}

void RPVsResultLyt::CalcOut() {
    if (mpAnmObjs[1]->isFinished()) {
        mNextState = 4;
        mIsChanging = true;
    }
}

void RPVsResultLyt::DrawLayer1A() {}

void RPVsResultLyt::DrawLayer1C() {
    if (mState != 0 && mState != 4) {
        mpLayout->draw();
    }
}

void RPVsResultLyt::Open(const s32* pWins) {
    if (mState != 0 || mIsChanging) {
        return;
    }

    mWins[0] = pWins[0];
    mWins[1] = pWins[1];
    mNextState = 1;
    mIsChanging = true;
}

void RPVsResultLyt::Close() {
    if (mState != 2 || mIsChanging) {
        return;
    }

    mNextState = 3;
    mIsChanging = true;
}
