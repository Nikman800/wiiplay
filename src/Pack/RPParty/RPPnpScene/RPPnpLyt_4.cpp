// Table Tennis rally result layout (pingpongResult_00.brlyt).
// Unit 0x802317BC-0x80231F54 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

#include <Pack/RPKernel.h>
#include <Pack/RPKernel/RPSysStringUtility.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>

#include <cwchar>

// Not inlined into Show in the original
#pragma push
#pragma auto_inline off
void RPPnpResultLyt::SetRallyText() {
    s32 rally = RPPnpGame::GetInstance()->GetRallyCount();

    mpLayout->findPane("N_unitA_00")->SetVisible(false);
    mpLayout->findPane("N_unitB_00")->SetVisible(false);
    mpLayout->findPane("N_unitC_00")->SetVisible(false);

    if (rally < 10) {
        mpLayout->findPane("N_unitA_00")->SetVisible(true);
    } else if (rally < 100) {
        mpLayout->findPane("N_unitB_00")->SetVisible(true);
    } else {
        mpLayout->findPane("N_unitC_00")->SetVisible(true);
    }

    RPSysMessage* pMessage = RPPnpGame::GetInstance()->GetMessage();
    if (rally < 100) {
        mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x89B, 0, 0);
        mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x89B, 0, 0);
    } else if (rally < 999) {
        mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x89B, 1, 0);
        mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x89B, 1, 0);
    } else {
        mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x89B, 2, 0);
        mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x89B, 2, 0);
    }

    if (RPSysStringUtility::IsSingular(rally)) {
        mpLayout->findTextBox("T_unitA_00")->setMessage(pMessage, 0x89C, 0, 0);
        mpLayout->findTextBox("T_unitA_01")->setMessage(pMessage, 0x89C, 0, 0);
        mpLayout->findTextBox("T_unitB_00")->setMessage(pMessage, 0x89C, 0, 0);
        mpLayout->findTextBox("T_unitB_01")->setMessage(pMessage, 0x89C, 0, 0);
        mpLayout->findTextBox("T_unitC_00")->setMessage(pMessage, 0x89C, 0, 0);
        mpLayout->findTextBox("T_unitC_01")->setMessage(pMessage, 0x89C, 0, 0);
    } else {
        mpLayout->findTextBox("T_unitA_00")->setMessage(pMessage, 0x89C, 1, 0);
        mpLayout->findTextBox("T_unitA_01")->setMessage(pMessage, 0x89C, 1, 0);
        mpLayout->findTextBox("T_unitB_00")->setMessage(pMessage, 0x89C, 1, 0);
        mpLayout->findTextBox("T_unitB_01")->setMessage(pMessage, 0x89C, 1, 0);
        mpLayout->findTextBox("T_unitC_00")->setMessage(pMessage, 0x89C, 1, 0);
        mpLayout->findTextBox("T_unitC_01")->setMessage(pMessage, 0x89C, 1, 0);
    }

    std::swprintf(mRallyText, 5, L"%d", rally);
    for (int i = 0; i < 5; i++) {
        if (mRallyText[i] == 0) {
            break;
        }

        mRallyText[i] += 0xFEE0;
    }

    const char* names[] = {"T_number_00", "T_number_01"};
    for (int i = 0; i < 2; i++) {
        mpLayout->findTextBox(names[i])->setText(mRallyText, 0);
    }
}
#pragma pop

void RPPnpResultLyt::Create() {
    mState = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
}

void RPPnpResultLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("pingpong.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "pingpongResult_00.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongResult_00_inAll_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongResult_00_outAll_00.brlan");
}

void RPPnpResultLyt::Reset() {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mState = 0;
}

void RPPnpResultLyt::Calculate() {
    switch (mState) {
    case 0:
        mState = 1;
        break;
    case 1:
        break;
    case 2:
        if (mpAnmObjs[ANM_IN]->isFinished()) {
            mState = 1;
        }
        break;
    case 3:
        if (mpAnmObjs[ANM_OUT]->isFinished()) {
            mState = 1;
        }
        break;
    }

    mpLayout->calc();
}

void RPPnpResultLyt::Exit() {}

void RPPnpResultLyt::Draw() {
    if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_RESULT) {
        mpLayout->draw();
    }
}

void RPPnpResultLyt::Show() {
    SetRallyText();
    mState = 2;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpAnmObjs[ANM_IN]->restart();
}

void RPPnpResultLyt::PlayOut() {
    mState = 3;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
    mpAnmObjs[ANM_OUT]->restart();
}
