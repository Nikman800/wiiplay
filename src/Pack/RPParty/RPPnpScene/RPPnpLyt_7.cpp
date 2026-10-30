// Table Tennis start message layout (pingpongMsg_00.brlyt).
// Unit 0x80233788-0x80233B70 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>

void RPPnpStartMsgLyt::Create() {
    mState = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
}

void RPPnpStartMsgLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("pingpong.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "pingpongMsg_00.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongMsg_00_inAll_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongMsg_00_outAll_00.brlan");
}

void RPPnpStartMsgLyt::Reset() {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mState = 0;
}

void RPPnpStartMsgLyt::Calculate() {
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

void RPPnpStartMsgLyt::Exit() {}

void RPPnpStartMsgLyt::Draw() {
    mpLayout->draw();
}

void RPPnpStartMsgLyt::Show() {
    RPSysMessage* pMessage = RPPnpGame::GetInstance()->GetMessage();

    if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
        if (RPPnpGame::sRallyLimit == 999) {
            mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x89A, 1, 0);
            mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x89A, 1, 0);
        } else {
            mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x89A, 0, 0);
            mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x89A, 0, 0);
        }
    } else {
        mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x89D, 0, 0);
        mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x89D, 0, 0);
    }

    mState = 2;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpAnmObjs[ANM_IN]->restart();
}

void RPPnpStartMsgLyt::PlayOut() {
    mState = 3;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
    mpAnmObjs[ANM_OUT]->restart();
}
