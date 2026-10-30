// Table Tennis point message layout (pingpongMsg_04.brlyt), two players.
// Unit 0x80234098-0x80234654 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPParty/RPPnpScene/RPPnpPhase.h>

void RPPnpPointMsgLyt::Create() {
    mState = 0;
    mSide = 2;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
}

void RPPnpPointMsgLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("pingpong.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "pingpongMsg_04.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongMsg_04_inAll_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongMsg_04_outAll_00.brlan");
}

void RPPnpPointMsgLyt::Reset() {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpLayout->findPane("N_1pGoalMsg_00")->SetVisible(false);
    mpLayout->findPane("N_2pGoalMsg_00")->SetVisible(false);
    mTimer = 0;
    mState = 0;
}

void RPPnpPointMsgLyt::Calculate() {
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

void RPPnpPointMsgLyt::Exit() {}

void RPPnpPointMsgLyt::Draw() {
    mpLayout->draw();
}

void RPPnpPointMsgLyt::Show(s32 side) {
    mSide = side;
    mpLayout->findPane("N_1pGoalMsg_00")->SetVisible(false);
    mpLayout->findPane("N_2pGoalMsg_00")->SetVisible(false);

    RPSysMessage* pMessage = RPPnpGame::GetInstance()->GetMessage();
    if (mSide == 0) {
        mpLayout->findTextBox("T_1pGoalText_00")->setMessage(pMessage, 0x8D4, 0, 0);
        mpLayout->findTextBox("T_1pGoalText_01")->setMessage(pMessage, 0x8D4, 0, 0);
        mpLayout->findTextBox("T_1pGoalText_02")->setMessage(pMessage, 0x8D4, 0, 0);
        mpLayout->findTextBox("T_1pGoalText_03")->setMessage(pMessage, 0x8D4, 0, 0);
        mpLayout->findPane("N_1pGoalMsg_00")->SetVisible(true);
    } else {
        mpLayout->findTextBox("T_2pGoalText_00")->setMessage(pMessage, 0x8D4, 1, 0);
        mpLayout->findTextBox("T_2pGoalText_01")->setMessage(pMessage, 0x8D4, 1, 0);
        mpLayout->findTextBox("T_2pGoalText_02")->setMessage(pMessage, 0x8D4, 1, 0);
        mpLayout->findTextBox("T_2pGoalText_03")->setMessage(pMessage, 0x8D4, 1, 0);
        mpLayout->findPane("N_2pGoalMsg_00")->SetVisible(true);
    }

    nw4r::lyt::Pane* pPane = mpLayout->findPane("N_center_00");
    nw4r::math::VEC3 pos = pPane->GetTranslate();

    bool even = !RPPnpGame::GetInstance()->GetPointPhase()->IsServeTurnOdd();
    if (RPPnpGame::GetInstance()->GetPointPhase()->IsServeChangePoint()) {
        even = !even;
    }

    bool first = mSide == 0;
    if ((even && first) || (!even && !first)) {
        pos.y = -30.0f;
    } else {
        pos.y = 80.0f;
    }
    pPane->SetTranslate(pos);

    mState = 2;
    mTimer = 0;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpAnmObjs[ANM_IN]->restart();
    mpAnmObjs[ANM_OUT]->resetFrame();
}

bool RPPnpPointMsgLyt::IsDone() const {
    return mpAnmObjs[ANM_OUT]->isFinished();
}
