// Table Tennis serve message layout "here / over there" (pingpongMsg_03.brlyt, message 0x8CE).
// Unit 0x80234654-0x80234AC8 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>

void RPPnpServeMsgLyt::Create() {
    mState = 0;
    mSide = 2;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
}

void RPPnpServeMsgLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("pingpong.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "pingpongMsg_03.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongMsg_03_inAll_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongMsg_03_outAll_00.brlan");

    RPSysMessage* pMessage = RPPnpGame::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_msgTextHere_00")->setMessage(pMessage, 0x8CE, 0, 0);
    mpLayout->findTextBox("T_msgTextHere_01")->setMessage(pMessage, 0x8CE, 0, 0);
    mpLayout->findTextBox("T_msgTextOver_00")->setMessage(pMessage, 0x8CE, 0, 0);
    mpLayout->findTextBox("T_msgTextOver_01")->setMessage(pMessage, 0x8CE, 0, 0);
}

void RPPnpServeMsgLyt::Reset() {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpLayout->findPane("N_msgHere_00")->SetVisible(false);
    mpLayout->findPane("N_msgOver_00")->SetVisible(false);
    mTimer = 0;
    mState = 0;
}

void RPPnpServeMsgLyt::Calculate() {
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

void RPPnpServeMsgLyt::Exit() {}

void RPPnpServeMsgLyt::Draw() {
    mpLayout->draw();
}

void RPPnpServeMsgLyt::Show(s32 side) {
    mSide = side;
    mpLayout->findPane("N_msgHere_00")->SetVisible(false);
    mpLayout->findPane("N_msgOver_00")->SetVisible(false);

    if (mSide == 0) {
        mpLayout->findPane("N_msgHere_00")->SetVisible(true);
    } else {
        mpLayout->findPane("N_msgOver_00")->SetVisible(true);
    }

    mState = 2;
    mTimer = 0;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpAnmObjs[ANM_IN]->restart();
    mpAnmObjs[ANM_OUT]->resetFrame();
}

bool RPPnpServeMsgLyt::IsDone() const {
    return mpAnmObjs[ANM_OUT]->isFinished();
}
