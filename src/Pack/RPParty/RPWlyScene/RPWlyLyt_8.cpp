#include <Pack/RPParty/RPWlyScene/RPWlyLyt_8.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>

#include <cwchar>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>

namespace {

// 0: player 1 leads, 1: player 2 leads, 2: draw
inline s32 GetWinner() {
    s32 s0 = RPWlyGameMgr::sInstance->mScores[0];
    if (s0 > RPWlyGameMgr::sInstance->mScores[1]) {
        return 0;
    }
    s32 winner = 2;
    if (s0 < RPWlyGameMgr::sInstance->mScores[1]) {
        winner = 1;
    }
    return winner;
}
}

void RPWlyDisplayLyt::UpdateScoreResult() {
    switch (GetWinner()) {
    case 0:
        if (mpSwitchAnmObjs[0]->isFinished()) {
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreL_00", false);
            mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreL_01", false);
            mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreR_00", false);
            mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreR_01", false);
            mpOutEndAnmObj->play(-1);
            mpLayout->bindAnmObjToPane(mpSwitchAnmObjs[0], "T_1pScore_00", true);
            mpSwitchAnmObjs[0]->play(-1);
        }
        break;
    case 1:
        if (mpSwitchAnmObjs[1]->isFinished()) {
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreL_00", false);
            mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreL_01", false);
            mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreR_00", false);
            mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreR_01", false);
            mpOutEndAnmObj->play(-1);
            mpLayout->bindAnmObjToPane(mpSwitchAnmObjs[1], "T_2pScore_00", true);
            mpSwitchAnmObjs[1]->play(-1);
        }
        break;
    }

    if (mpOutEndAnmObj->isFinished()) {
        mState = 1;
        mTimer = 0;
    }
}

void RPWlyDisplayLyt::SetPlayerScore(s32 player, s32 score) {
    if (score > 999) {
        score = 999;
    } else if (score < 0) {
        score = 0;
    }

    wchar_t* text = &mScoreText[player][0];
    std::swprintf(text, 4, L"%d", score);
    for (int i = 0; i < 4; i++) {
        if (mScoreText[player][i] == 0) {
            break;
        }
        mScoreText[player][i] += 0xFEE0;
    }

    if (player == 0) {
        mpLayout->findTextBox("T_1pScore_00")->setText(text, 0);
    } else {
        mpLayout->findTextBox("T_2pScore_00")->setText(text, 0);
    }
}

void RPWlyDisplayLyt::Init() {
    mState = 0;
    mTimer = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpInLevelAnmObj = NULL;
    mpOutLevelAnmObj = NULL;
    mpInLevelEndAnmObj = NULL;
    mpOutEndAnmObj = NULL;
    mpSwitchAnmObjs[0] = NULL;
    mpSwitchAnmObjs[1] = NULL;

    for (int i = 0; i < RPSceneInfo::spInstance->GetPlayerNum(); i++) {
        RPSysKokeshiGenInfo info;
        GXColor bgColor = {0, 0, 0, 0};

        info.SetGenType(RPSysKokeshiManager::GenType_Player);
        info.SetPlayerNo(i);
        info.SetIconWidth(0x60);
        info.SetIconHeight(0x60);
        info.SetIconBGType(RFLIconBG_Direct);
        info.SetIconBGColor(bgColor);
        info.SetIconMipMap(true);

        mIcons[i].mpIcon = new RPSysKokeshiIcon(&info);
    }
}

void RPWlyDisplayLyt::Create() {
    mpResAccessor = RPCreateLocalLytResAccessor("wally.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "wallyDisplay_00.brlyt");
    mpInLevelAnmObj =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyDisplay_00_inLevel1_00.brlan");
    mpOutLevelAnmObj =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyDisplay_00_outLevel_00.brlan");
    mpInLevelEndAnmObj =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyDisplay_00_inLevel2End_00.brlan");
    mpOutEndAnmObj =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyDisplay_00_outEnd_00.brlan");
    mpSwitchAnmObjs[0] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyDisplay_00_switch1p_00.brlan");
    mpSwitchAnmObjs[1] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyDisplay_00_switch2p_00.brlan");

    if (RPWlyGameMgr::sInstance->mMode == 0) {
        mpLayout->findPane("N_L_00")->SetVisible(false);
        mpLayout->findPane("N_R_00")->SetVisible(false);
    }

    RPSysMessage* message = RPWlyGameMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_1pText_00")->setMessage(message, 0x965, 0, 0);
    mpLayout->findTextBox("T_2pText_00")->setMessage(message, 0x965, 1, 0);
    mpLayout->findTextBox("T_1pScore_00")->setMessage(message, 0x964, 0, 0);
    mpLayout->findTextBox("T_2pScore_00")->setMessage(message, 0x964, 1, 0);

    if (RPWlyGameMgr::sInstance->mStageNo == 99) {
        mpLayout->findTextBox("T_msgStart_00")->setMessage(message, 0x966, 1, 0);
        mpLayout->findTextBox("T_msgStart_01")->setMessage(message, 0x966, 1, 0);
    } else {
        RPSysStringUtility::GetStringHalfSizeNumber(RPWlyGameMgr::sInstance->mStageNo, mStageText, 3, false);
        mpLayout->findTextBox("T_msgStart_00")->setMessage(message, 0x966, 0, 1, mStageText);
        mpLayout->findTextBox("T_msgStart_01")->setMessage(message, 0x966, 0, 1, mStageText);
    }

    mpLayout->findPane("N_msgEnd_00")->SetVisible(false);
    mIcons[0].mpPane = mpLayout->findPane("N_1pFace_00");
    mIcons[1].mpPane = mpLayout->findPane("N_2pFace_00");
    mIcons[0].mpPane2 = mpLayout->findPane("N_1pFace_01");
    mIcons[1].mpPane2 = mpLayout->findPane("N_2pFace_01");
}

void RPWlyDisplayLyt::Reset() {
    for (int i = 0; i < 6; i++) {
        (&mpInLevelAnmObj)[i]->resetFrame();
        (&mpInLevelAnmObj)[i]->getFrameCtrl()->reinit();
    }
    mpLayout->unbindAllAnmObj();
    if (RPWlyGameMgr::sInstance->mStageNo == 1) {
        mpLayout->bindAnmObj(mpInLevelAnmObj);
    } else {
        mpLayout->bindAnmObj(mpInLevelEndAnmObj);
    }
    mState = 0;
    mTimer = 0;

    for (int i = 0; i < 2; i++) {
        SetPlayerScore(i, RPWlyGameMgr::sInstance->mScores[i]);
    }

    mpLayout->findPane("N_msgStart_00")->SetVisible(false);
    mpLayout->findPane("N_msgEnd_00")->SetVisible(false);
}

void RPWlyDisplayLyt::Calculate() {
    switch (mState) {
    case 0:
        mTimer = 0;
        mState = 1;
        break;
    case 1:
        break;
    case 2:
        if (mpInLevelAnmObj->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    case 3:
        if (mpOutLevelAnmObj->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    case 4:
        if (mpInLevelEndAnmObj->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    case 5:
        UpdateScoreResult();
        break;
    case 6:
        switch (GetWinner()) {
        case 0:
            if (mpSwitchAnmObjs[0]->isFinished()) {
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObjToPane(mpSwitchAnmObjs[0], "T_1pScore_00", true);
                mpSwitchAnmObjs[0]->restart();
            }
            break;
        case 1:
            if (mpSwitchAnmObjs[1]->isFinished()) {
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObjToPane(mpSwitchAnmObjs[1], "T_2pScore_00", true);
                mpSwitchAnmObjs[1]->restart();
            }
            break;
        }
        break;
    }
    mpLayout->calc();
}

void RPWlyDisplayLyt::Destroy() {}

void RPWlyDisplayLyt::Draw() {
    mpLayout->draw();
    for (int i = 0; i < RPSceneInfo::spInstance->GetPlayerNum(); i++) {
        RPDrawKokeshiIcon(mIcons[i].mpIcon, mIcons[i].mpPane2, TRUE, NULL);
        RPDrawKokeshiIcon(mIcons[i].mpIcon, mIcons[i].mpPane, FALSE, NULL);
    }
}

void RPWlyDisplayLyt::PlayLevelIn() {
    mpLayout->findPane("N_msgStart_00")->SetVisible(true);
    mpLayout->unbindAllAnmObj();
    if (RPWlyGameMgr::sInstance->mStageNo == 1) {
        mState = 2;
        mpLayout->bindAnmObj(mpInLevelAnmObj);
        mpInLevelAnmObj->restart();
    } else {
        mState = 4;
        mpLayout->bindAnmObj(mpInLevelEndAnmObj);
        mpInLevelEndAnmObj->restart();
    }
}

void RPWlyDisplayLyt::PlayLevelOut() {
    mState = 3;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpOutLevelAnmObj);
    mpOutLevelAnmObj->restart();
}

void RPWlyDisplayLyt::PlayEndIn() {
    mpLayout->findPane("N_msgStart_00")->SetVisible(false);
    mState = 4;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpInLevelEndAnmObj);
    mpInLevelEndAnmObj->restart();
}

void RPWlyDisplayLyt::StartScoreResult() {
    mState = 5;
    mpLayout->unbindAllAnmObj();
    switch (GetWinner()) {
    case 0:
        mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreL_00", false);
        mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreL_01", false);
        mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreR_00", false);
        mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreR_01", false);
        mpOutEndAnmObj->restart();
        mpLayout->bindAnmObjToPane(mpSwitchAnmObjs[0], "T_1pScore_00", true);
        mpSwitchAnmObjs[0]->play(-1);
        break;
    case 1:
        mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreL_00", false);
        mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreL_01", false);
        mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreR_00", false);
        mpLayout->bindAnmObjToPane(mpOutEndAnmObj, "N_scoreR_01", false);
        mpOutEndAnmObj->play(-1);
        mpLayout->bindAnmObjToPane(mpSwitchAnmObjs[1], "T_2pScore_00", true);
        mpSwitchAnmObjs[1]->restart();
        break;
    case 2:
        mpLayout->bindAnmObj(mpOutEndAnmObj);
        mpOutEndAnmObj->restart();
        break;
    }
}

void RPWlyDisplayLyt::PlayWinnerScore() {
    switch (GetWinner()) {
    case 0:
        mState = 6;
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObjToPane(mpSwitchAnmObjs[0], "T_1pScore_00", true);
        mpSwitchAnmObjs[0]->restart();
        break;
    case 1:
        mState = 6;
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObjToPane(mpSwitchAnmObjs[1], "T_2pScore_00", true);
        mpSwitchAnmObjs[1]->restart();
        break;
    }
}
