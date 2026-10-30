// Laser Hockey "Goal!" message with the new score (airHockeyMsg_01.brlyt).
// Unit 0x80223670-0x802240E4 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPHkyScene/RPHkyPaddle.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>

#include <cwchar>

void RPHkyGoalMsgLyt::SetScoreText(s32 side, s32 score) {
    wchar_t* pText = mScoreText[side];
    std::swprintf(pText, 3, L"%02d", score);
    for (int i = 0; i < 3; i++) {
        if (mScoreText[side][i] == 0) {
            break;
        }

        mScoreText[side][i] += 0xFEE0;
    }

    switch (side) {
    case SIDE_LEFT:
        mpLayout->findTextBox("T_scoreTextL_00")->setText(pText, 0);
        break;
    case SIDE_RIGHT:
        mpLayout->findTextBox("T_scoreTextR_00")->setText(pText, 0);
        break;
    }
}

void RPHkyGoalMsgLyt::CalcResultSwitch() {
    if (!mpAnmObjs[ANM_IN]->isFinished()) {
        return;
    }

    mState = 6;

    s32 score0 = RPHkyGame::GetInstance()->GetPaddle(0)->GetScore();
    s32 score1 = RPHkyGame::GetInstance()->GetPaddle(1)->GetScore();
    if (score0 > score1) {
        mSide = SIDE_LEFT;
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpAnmObjs[ANM_SWITCH_L]);
        mpAnmObjs[ANM_SWITCH_L]->restart();
    } else if (score0 < score1) {
        mSide = SIDE_RIGHT;
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpAnmObjs[ANM_SWITCH_R]);
        mpAnmObjs[ANM_SWITCH_R]->restart();
    } else {
        mState = 1;
    }
}

void RPHkyGoalMsgLyt::Create() {
    mState = 0;
    mSide = SIDE_LEFT;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_SWITCH_L] = NULL;
    mpAnmObjs[ANM_SWITCH_R] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
}

void RPHkyGoalMsgLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("airHockey.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "airHockeyMsg_01.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyMsg_01_inAll_00.brlan");
    mpAnmObjs[ANM_SWITCH_L] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyMsg_01_switchScoreL_00.brlan");
    mpAnmObjs[ANM_SWITCH_R] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyMsg_01_switchScoreR_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyMsg_01_outAll_00.brlan");

    RPSysMessage* pMessage = RPHkyGame::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_1pGoalText_00")->setMessage(pMessage, 0xA28, 0, 0);
    mpLayout->findTextBox("T_1pGoalText_01")->setMessage(pMessage, 0xA28, 0, 0);
    mpLayout->findTextBox("T_1pGoalText_02")->setMessage(pMessage, 0xA28, 0, 0);
    mpLayout->findTextBox("T_2pGoalText_00")->setMessage(pMessage, 0xA28, 0, 0);
    mpLayout->findTextBox("T_2pGoalText_01")->setMessage(pMessage, 0xA28, 0, 0);
    mpLayout->findTextBox("T_2pGoalText_02")->setMessage(pMessage, 0xA28, 0, 0);
}

void RPHkyGoalMsgLyt::Reset() {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpLayout->findPane("N_1pGoalMsg_00")->SetVisible(false);
    mpLayout->findPane("N_2pGoalMsg_00")->SetVisible(false);
    mState = 0;

    for (int i = 0; i < 2; i++) {
        SetScoreText(i, 0);
    }
}

void RPHkyGoalMsgLyt::Calculate() {
    switch (mState) {
    case 0:
        mState = 1;
        break;
    case 1:
        break;
    case 2:
        if (mpAnmObjs[ANM_IN]->isFinished()) {
            mState = 3;
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(GetSwitchAnmObj());
            GetSwitchAnmObj()->restart();
        }
        break;
    case 3:
        if (GetSwitchAnmObj()->isFinished()) {
            mState = 4;
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
            mpAnmObjs[ANM_OUT]->restart();
        }
        break;
    case 4:
        if (mpAnmObjs[ANM_OUT]->isFinished()) {
            mState = 1;
        }
        break;
    case 5:
        CalcResultSwitch();
        break;
    case 6:
        if (GetSwitchAnmObj()->isFinished()) {
            mState = 6;
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(GetSwitchAnmObj());
            GetSwitchAnmObj()->restart();
        }
        break;
    case 7:
        if (mpAnmObjs[ANM_OUT]->isFinished()) {
            mState = 1;
        }
        break;
    }

    mpLayout->calc();
}

void RPHkyGoalMsgLyt::Exit() {}

void RPHkyGoalMsgLyt::Draw() {
    mpLayout->draw();
}

void RPHkyGoalMsgLyt::ShowGoal(s32 side) {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
    }

    mState = 2;
    mSide = side;

    switch (side) {
    case SIDE_LEFT:
        mpLayout->findPane("N_1pGoalMsg_00")->SetVisible(true);
        mpLayout->findPane("N_2pGoalMsg_00")->SetVisible(false);
        break;
    case SIDE_RIGHT:
        mpLayout->findPane("N_1pGoalMsg_00")->SetVisible(false);
        mpLayout->findPane("N_2pGoalMsg_00")->SetVisible(true);
        break;
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpAnmObjs[ANM_IN]->restart();
}

bool RPHkyGoalMsgLyt::IsOutDone() const {
    return mpAnmObjs[ANM_OUT]->isFinished();
}

void RPHkyGoalMsgLyt::StartResult() {
    mpLayout->findPane("N_1pGoalMsg_00")->SetVisible(false);
    mpLayout->findPane("N_2pGoalMsg_00")->SetVisible(false);
    mState = 5;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpAnmObjs[ANM_IN]->restart();
}

void RPHkyGoalMsgLyt::PlayOut() {
    mState = 7;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
    mpAnmObjs[ANM_OUT]->restart();
}
