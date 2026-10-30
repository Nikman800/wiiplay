// Laser Hockey score board layout (airHockeyScore_00.brlyt), drawn on view 2.
// Unit 0x80220D1C-0x8022150C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPHkyScene/RPHkyHud.h>
#include <Pack/RPParty/RPHkyScene/RPHkyPaddle.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>

#include <cwchar>

inline const char* HkyScoreTextBoxName(int side) {
    const char* names[] = {"T_scoreL_00", "T_scoreR_00"};
    return names[side];
}

void RPHkyScoreLyt::Create() {
    mState = 0;
    mTimer = 0;
    mGoalSide = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
    mpAnmObjs[ANM_SWITCH_GOAL] = NULL;
    mpAnmObjs[ANM_SWITCH_RESULT] = NULL;
}

void RPHkyScoreLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("airHockey.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "airHockeyScore_00.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyScore_00_inAll_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyScore_00_outAll_00.brlan");
    mpAnmObjs[ANM_SWITCH_GOAL] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyScore_00_switchGoal_00.brlan");
    mpAnmObjs[ANM_SWITCH_RESULT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyScore_00_switchResult_00.brlan");

    RPSysMessage* pMessage = RPHkyGame::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_playerL_00")->setMessage(pMessage, 0xA2B, 0, 0);
    mpLayout->findTextBox("T_playerL_01")->setMessage(pMessage, 0xA2B, 0, 0);
    mpLayout->findTextBox("T_playerR_00")
        ->setMessage(pMessage, 0xA2B, RPHkyGame::GetInstance()->IsMultiPlayer() == TRUE ? 1 : 2,
                     0);
    mpLayout->findTextBox("T_playerR_01")
        ->setMessage(pMessage, 0xA2B, RPHkyGame::GetInstance()->IsMultiPlayer() == TRUE ? 1 : 2,
                     0);
}

void RPHkyScoreLyt::Reset() {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mState = 0;

    for (int i = 0; i < 2; i++) {
        wchar_t* pText = mScoreText[i];
        std::swprintf(pText, 3, L"%02d", 0);
        for (int j = 0; j < 3; j++) {
            if (mScoreText[i][j] == 0) {
                break;
            }

            mScoreText[i][j] += 0xFEE0;
        }

        mpLayout->findTextBox(HkyScoreTextBoxName(i))->setText(pText, 0);
    }
}

void RPHkyScoreLyt::Calculate() {
    switch (mState) {
    case 0:
    case 1:
        break;
    case 2:
        if (mpAnmObjs[ANM_IN]->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    case 3:
        if (mpAnmObjs[ANM_SWITCH_GOAL]->isFinished()) {
            u32 side = mGoalSide;
            s32 score = RPHkyGame::GetInstance()->GetPaddle(mGoalSide)->GetScore();
            wchar_t* pText = mScoreText[side];
            std::swprintf(pText, 3, L"%02d", score);
            for (int j = 0; j < 3; j++) {
                if (mScoreText[side][j] == 0) {
                    break;
                }

                mScoreText[side][j] += 0xFEE0;
            }

            mpLayout->findTextBox(HkyScoreTextBoxName(side))->setText(pText, 0);

            RPHkyHud::GetInstance()->GetGoalMsgLyt()->SetScoreText(mGoalSide, score);
            // Keep the intermediate pointer type to preserve the original register allocation.
            void* pAudioMgr = RPSndAudioMgr::instance();
            static_cast<RPSndAudioMgr*>(pAudioMgr)->startSystemSe(mGoalSide == 0 ? 0xA : 0xB);
            mState = 1;
            mTimer = 0;
        }
        break;
    case 4:
        break;
    case 5:
        if (mpAnmObjs[ANM_OUT]->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    }

    mpLayout->calc();
}

void RPHkyScoreLyt::Exit() {}

void RPHkyScoreLyt::Draw() {
    mpLayout->draw();
}

void RPHkyScoreLyt::PlayIn() {
    mState = 2;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpAnmObjs[ANM_IN]->restart();
}

void RPHkyScoreLyt::PlayGoalSwitch(s32 side) {
    mGoalSide = side;
    mState = 3;
    mpLayout->unbindAllAnmObj();

    switch (mGoalSide) {
    case 0:
        mpLayout->bindAnmObjToPane(mpAnmObjs[ANM_SWITCH_GOAL], "T_scoreL_00", true);
        break;
    case 1:
        mpLayout->bindAnmObjToPane(mpAnmObjs[ANM_SWITCH_GOAL], "T_scoreR_00", true);
        break;
    }

    mpLayout->bindAnmObjToPane(mpAnmObjs[ANM_SWITCH_GOAL], "W_scoreBg_00", true);
    mpAnmObjs[ANM_SWITCH_GOAL]->restart();
}

void RPHkyScoreLyt::PlayResultSwitch() {
    mState = 4;
    mpLayout->unbindAllAnmObj();

    s32 score0 = RPHkyGame::GetInstance()->GetPaddle(0)->GetScore();
    s32 score1 = RPHkyGame::GetInstance()->GetPaddle(1)->GetScore();
    if (score0 > score1) {
        mpLayout->bindAnmObjToPane(mpAnmObjs[ANM_SWITCH_RESULT], "T_scoreL_00", true);
    } else if (score0 < score1) {
        mpLayout->bindAnmObjToPane(mpAnmObjs[ANM_SWITCH_RESULT], "T_scoreR_00", true);
    }

    mpLayout->bindAnmObjToPane(mpAnmObjs[ANM_SWITCH_RESULT], "W_scoreBg_00", true);
    mpAnmObjs[ANM_SWITCH_RESULT]->restart();
}

void RPHkyScoreLyt::PlayOut() {
    mState = 5;
    mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
    mpAnmObjs[ANM_OUT]->restart();
}
