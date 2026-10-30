// Table Tennis fault message layout (pingpongMsg_02.brlyt, message 0x8CD).
// Unit 0x80233B70-0x80234098 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPnpScene/RPPnpBall.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>

void RPPnpFaultMsgLyt::Create() {
    mState = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
}

void RPPnpFaultMsgLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("pingpong.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "pingpongMsg_02.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongMsg_02_inAll_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongMsg_02_outAll_00.brlan");
}

void RPPnpFaultMsgLyt::Reset() {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->reset();
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mTimer = 0;
    mState = 0;
}

void RPPnpFaultMsgLyt::Calculate() {
    switch (mState) {
    case 0:
        mState = 1;
        break;
    case 1:
        break;
    case 2:
        if (mpAnmObjs[ANM_IN]->isFinished()) {
            if (mTimer++ > 90) {
                mState = 3;
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
                mpAnmObjs[ANM_OUT]->restart();
            }
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

void RPPnpFaultMsgLyt::Exit() {}

void RPPnpFaultMsgLyt::Draw() {
    mpLayout->draw();
}

void RPPnpFaultMsgLyt::Show() {
    RPSysMessage* pMessage = RPPnpGame::GetInstance()->GetMessage();

    switch (RPPnpGame::GetInstance()->GetBall()->GetState()) {
    case 10:
        mpLayout->findTextBox("T_msgTextL_00")->setMessage(pMessage, 0x8CD, 0, 0);
        mpLayout->findTextBox("T_msgTextR_00")->setMessage(pMessage, 0x8CD, 0, 0);
        mpLayout->findTextBox("T_msgTextC_00")->setMessage(pMessage, 0x8CD, 0, 0);
        mpLayout->findTextBox("T_msgTextC_01")->setMessage(pMessage, 0x8CD, 0, 0);
        break;
    case 9:
        mpLayout->findTextBox("T_msgTextL_00")->setMessage(pMessage, 0x8CD, 1, 0);
        mpLayout->findTextBox("T_msgTextR_00")->setMessage(pMessage, 0x8CD, 1, 0);
        mpLayout->findTextBox("T_msgTextC_00")->setMessage(pMessage, 0x8CD, 1, 0);
        mpLayout->findTextBox("T_msgTextC_01")->setMessage(pMessage, 0x8CD, 1, 0);
        break;
    case 12:
        mpLayout->findTextBox("T_msgTextL_00")->setMessage(pMessage, 0x8CD, 2, 0);
        mpLayout->findTextBox("T_msgTextR_00")->setMessage(pMessage, 0x8CD, 2, 0);
        mpLayout->findTextBox("T_msgTextC_00")->setMessage(pMessage, 0x8CD, 2, 0);
        mpLayout->findTextBox("T_msgTextC_01")->setMessage(pMessage, 0x8CD, 2, 0);
        break;
    case 11:
        break;
    }

    mState = 2;
    mTimer = 0;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpAnmObjs[ANM_IN]->restart();
    mpAnmObjs[ANM_OUT]->resetFrame();
}

bool RPPnpFaultMsgLyt::IsDone() const {
    return mpAnmObjs[ANM_OUT]->isFinished();
}
