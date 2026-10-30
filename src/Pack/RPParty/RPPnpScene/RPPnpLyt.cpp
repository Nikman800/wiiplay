// Table Tennis score board (pingpongScore_00.brlyt), two players.
// Unit 0x8022FA20-0x80230878 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPnpScene/RPPnpAudience.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPParty/RPPnpScene/RPPnpPhase.h>

#include <cwchar>

// Not inlined into their callers in the original
#pragma push
#pragma auto_inline off
void RPPnpScoreLyt::CalcSwitch() {
    s32 anm = ANM_SWITCH_2P;
    if (mSide == 0) {
        anm = ANM_SWITCH_1P;
    }

    if (mpAnmObjs[anm]->isFinished()) {
        mState = 1;
        mTimer = 0;

        if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_RESULT) {
            if (mIsOut) {
                mState = 4;
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
                mpAnmObjs[ANM_OUT]->restart();
            } else {
                mState = 3;
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObj(mpAnmObjs[anm]);
                mpAnmObjs[anm]->restart();
            }
        }
    }
}

void RPPnpScoreLyt::SetScore(s32 side, s32 score) {
    std::swprintf(mScoreText[side], 3, L"%02d", score);
    for (int i = 0; i < 3; i++) {
        if (mScoreText[side][i] == 0) {
            break;
        }

        mScoreText[side][i] += 0xFEE0;
    }

    if (side == 0) {
        mpLayout->findTextBox("T_scoreL_00")->setText(mScoreText[side], 0);
        mpLayout->findTextBox("T_scoreL_01")->setText(mScoreText[side], 0);
    } else {
        mpLayout->findTextBox("T_scoreR_00")->setText(mScoreText[side], 0);
        mpLayout->findTextBox("T_scoreR_01")->setText(mScoreText[side], 0);
    }
}
#pragma pop

void RPPnpScoreLyt::Create() {
    mState = 0;
    mTimer = 0;
    mSide = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_SWITCH_1P] = NULL;
    mpAnmObjs[ANM_SWITCH_2P] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
}

void RPPnpScoreLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("pingpong.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "pingpongScore_00.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongScore_00_inAll_00.brlan");
    mpAnmObjs[ANM_SWITCH_1P] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongScore_00_switch1P_00.brlan");
    mpAnmObjs[ANM_SWITCH_2P] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongScore_00_switch2P_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongScore_00_outAll_00.brlan");

    RPSysMessage* pMessage = RPPnpGame::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_playerLA_00")->setMessage(pMessage, 0x89E, 0, 0);
    mpLayout->findTextBox("T_playerLA_01")->setMessage(pMessage, 0x89E, 0, 0);
    mpLayout->findTextBox("T_playerLU_00")->setMessage(pMessage, 0x89E, 0, 0);
    mpLayout->findTextBox("T_playerLU_01")->setMessage(pMessage, 0x89E, 0, 0);
    mpLayout->findTextBox("T_playerRA_00")->setMessage(pMessage, 0x89E, 1, 0);
    mpLayout->findTextBox("T_playerRA_01")->setMessage(pMessage, 0x89E, 1, 0);
    mpLayout->findTextBox("T_playerRU_00")->setMessage(pMessage, 0x89E, 1, 0);
    mpLayout->findTextBox("T_playerRU_01")->setMessage(pMessage, 0x89E, 1, 0);
    mpLayout->findTextBox("T_scoreL_00")->setMessage(pMessage, 0x89F, 0, 0);
    mpLayout->findTextBox("T_scoreL_01")->setMessage(pMessage, 0x89F, 0, 0);
    mpLayout->findTextBox("T_scoreR_00")->setMessage(pMessage, 0x89F, 1, 0);
    mpLayout->findTextBox("T_scoreR_01")->setMessage(pMessage, 0x89F, 1, 0);

    mpServerPanes[0] = mpLayout->findPane("N_L_00");
    mpServerPanes[1] = mpLayout->findPane("N_R_00");
}

void RPPnpScoreLyt::Reset() {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpLayout->findPane("N_playerLA_00")->SetVisible(false);
    mpLayout->findPane("N_playerRU_00")->SetVisible(false);
    mpLayout->findPane("N_playerLU_00")->SetVisible(false);
    mpLayout->findPane("N_playerRA_00")->SetVisible(false);
    mState = 0;
    mTimer = 0;
    mIsOut = true;

    if (mpLayout != NULL) {
        for (int i = 0; i < 2; i++) {
            SetScore(i, 0);
        }
    }
}

void RPPnpScoreLyt::Calculate() {
    if (RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE) {
        switch (mState) {
        case 0:
            mTimer = 0;
            mState = 1;
            break;
        case 1:
            break;
        case 2:
            if (mpAnmObjs[ANM_IN]->isFinished()) {
                mState = 1;
                mTimer = 0;

                if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_RESULT) {
                    RPSndAudioMgr::instance()->startFanfare(0x6D);

                    RPPnpPointPhase* pPoint = RPPnpGame::GetInstance()->GetPointPhase();
                    if (pPoint->GetScore(0) > pPoint->GetScore(1)) {
                        mSide = 0;
                    } else {
                        mSide = 1;
                    }

                    s32 anm = mSide == 0 ? ANM_SWITCH_1P : ANM_SWITCH_2P;

                    mpLayout->unbindAllAnmObj();
                    mpLayout->bindAnmObj(mpAnmObjs[anm]);
                    mpAnmObjs[anm]->restart();
                    mState = 3;
                }
            }
            break;
        case 3:
            CalcSwitch();
            break;
        case 4:
            if (mpAnmObjs[ANM_OUT]->isFinished()) {
                mState = 1;
                mTimer = 0;
            }
            break;
        }

        mpLayout->calc();
    }
}

void RPPnpScoreLyt::Exit() {}

void RPPnpScoreLyt::Draw() {
    if (RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE) {
        mpLayout->draw();
    }
}

void RPPnpScoreLyt::PlayIn() {
    if (RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE && mIsOut) {
        mState = 2;
        mpLayout->findPane("N_playerLA_00")->SetVisible(false);
        mpLayout->findPane("N_playerRU_00")->SetVisible(false);
        mpLayout->findPane("N_playerLU_00")->SetVisible(false);
        mpLayout->findPane("N_playerRA_00")->SetVisible(false);

        nw4r::math::VEC3 pos;
        if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_RESULT) {
            mpLayout->findPane("N_playerRU_00")->SetVisible(true);
            mpLayout->findPane("N_playerLU_00")->SetVisible(true);

            pos = mpServerPanes[0]->GetTranslate();
            pos.y = 90.0f;
            mpServerPanes[0]->SetTranslate(pos);
            pos = mpServerPanes[1]->GetTranslate();
            pos.y = 90.0f;
            mpServerPanes[1]->SetTranslate(pos);
        } else if (RPPnpGame::GetInstance()->GetPointPhase()->GetServeTurn() % 2 == 0) {
            mpLayout->findPane("N_playerLA_00")->SetVisible(true);
            mpLayout->findPane("N_playerRU_00")->SetVisible(true);

            pos = mpServerPanes[0]->GetTranslate();
            pos.y = -172.0f;
            mpServerPanes[0]->SetTranslate(pos);
            pos = mpServerPanes[1]->GetTranslate();
            pos.y = 172.0f;
            mpServerPanes[1]->SetTranslate(pos);
        } else {
            mpLayout->findPane("N_playerLU_00")->SetVisible(true);
            mpLayout->findPane("N_playerRA_00")->SetVisible(true);

            pos = mpServerPanes[0]->GetTranslate();
            pos.y = 172.0f;
            mpServerPanes[0]->SetTranslate(pos);
            pos = mpServerPanes[1]->GetTranslate();
            pos.y = -172.0f;
            mpServerPanes[1]->SetTranslate(pos);
        }

        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
        mpAnmObjs[ANM_IN]->restart();
        mIsOut = false;
    }
}

void RPPnpScoreLyt::PlayPointSwitch(s32 side) {
    if (RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE) {
        mSide = side;

        s32 anm = ANM_SWITCH_2P;
        if (side == 0) {
            anm = ANM_SWITCH_1P;
        }

        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpAnmObjs[anm]);
        mpAnmObjs[anm]->restart();

        SetScore(mSide, RPPnpGame::GetInstance()->GetPointPhase()->GetScore(mSide));
        mState = 3;
        RPPnpAudience::GetInstance()->CheerTimed(-1);

        if (RPPnpGame::GetInstance()->GetPointPhase()->IsGameOver()) {
            RPSndAudioMgr::instance()->startSystemSe(0x3D);
        } else {
            RPSndAudioMgr::instance()->startSystemSe(0x36);
        }

        if (mSide == 0) {
            RPSndAudioMgr::instance()->startSystemSe(0x2D);
        } else {
            RPSndAudioMgr::instance()->startSystemSe(0x2E);
        }
    }
}

void RPPnpScoreLyt::PlayOut() {
    if (RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE && !mIsOut) {
        if (RPPnpGame::GetInstance()->GetState() != RPPnpGame::STATE_RESULT) {
            mState = 4;
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
            mpAnmObjs[ANM_OUT]->restart();
        }

        mIsOut = true;
    }
}
