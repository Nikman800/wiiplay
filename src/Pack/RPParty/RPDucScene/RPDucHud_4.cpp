// Shooting Range round start banner (RPDucStartLayout): round number, scores and the Mii faces.
// Unit 0x8024F2B4-0x8024FD3C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucHud.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPMiiIconSet.h>

RPDucStartLayout::RPDucStartLayout() {}

RPDucStartLayout::~RPDucStartLayout() {}

#pragma auto_inline off
void RPDucStartLayout::DrawMiiIcons() {
    if (mState == 1) {
        return;
    }
    if (RPDucGame::sInstance->GetCurrentRound() == 1) {
        return;
    }

    s32 best = RPDucScoreMgr::sInstance->GetScore(1);
    if (best <= RPDucScoreMgr::sInstance->GetScore(0)) {
        best = RPDucScoreMgr::sInstance->GetScore(0);
    }
    RPGrpScreen* const pScreen = RPGrpRenderer::GetCurrent()->GetView(1)->GetScreen();

    RPDrawKokeshiIcon(RPMiiIconSet::sInstance->GetIcon(0),
                      best >= 100 ? mpLayout->findPane("N_1pFaceA_01")
                                  : mpLayout->findPane("N_1pFaceB_01"),
                      TRUE, pScreen);
    RPDrawKokeshiIcon(RPMiiIconSet::sInstance->GetIcon(0),
                      best >= 100 ? mpLayout->findPane("N_1pFaceA_00")
                                  : mpLayout->findPane("N_1pFaceB_00"),
                      FALSE, pScreen);
    RPDrawKokeshiIcon(RPMiiIconSet::sInstance->GetIcon(1),
                      best >= 100 ? mpLayout->findPane("N_2pFaceA_01")
                                  : mpLayout->findPane("N_2pFaceB_01"),
                      TRUE, pScreen);
    RPDrawKokeshiIcon(RPMiiIconSet::sInstance->GetIcon(1),
                      best >= 100 ? mpLayout->findPane("N_2pFaceA_00")
                                  : mpLayout->findPane("N_2pFaceB_00"),
                      FALSE, pScreen);
}

#pragma auto_inline reset

void RPDucStartLayout::Load(EGG::Archive* pArchive) {
    const char* anmNames[2] = {"duckhuntStart_00_inAll_00.brlan",
                               "duckhuntStart_00_outAll_00.brlan"};
    AttachArcFile(pArchive, "duckhunt.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "duckhuntStart_00.brlyt");
    for (s32 i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    }
}

void RPDucStartLayout::Reset() {
    mState = 1;
    RPSysMessage* const pMsg = RPDucHud::sInstance->GetMessage();
    const s32& round = RPDucGame::sInstance->GetCurrentRound();
    s32 roundNum = RPDucGame::sInstance->GetRoundNum();
    RPSysStringUtility::GetStringHalfSizeNumber(round, mRoundText, 4, false);
    if (roundNum == round) {
        mpLayout->findTextBox("T_msgText_00")->setMessage(pMsg, 0x866, 1, 0);
        mpLayout->findTextBox("T_msgText_01")->setMessage(pMsg, 0x866, 1, 0);
    } else {
        mpLayout->findTextBox("T_msgText_00")->setMessage(pMsg, 0x866, 0, 1, mRoundText);
        mpLayout->findTextBox("T_msgText_01")->setMessage(pMsg, 0x866, 0, 1, mRoundText);
    }

    RPSysMessage* const pScoreMsg = RPDucHud::sInstance->GetMessage();
    RPSysStringUtility::GetStringFullSizeNumber(RPDucScoreMgr::sInstance->GetScore(0),
                                                mScoreText[0], 4, false);
    mpLayout->findTextBox("T_scoreNumL_00")->setMessage(pScoreMsg, 0x83C, 0, 1, mScoreText[0]);
    RPSysStringUtility::GetStringFullSizeNumber(RPDucScoreMgr::sInstance->GetScore(1),
                                                mScoreText[1], 4, false);
    mpLayout->findTextBox("T_scoreNumR_00")->setMessage(pScoreMsg, 0x83C, 1, 1, mScoreText[1]);

    mpLayout->reset();
    s32 i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 2);

    RPSysMessage* const pPlayerMsg = RPDucHud::sInstance->GetMessage();
    mpLayout->findTextBox("T_1pTextA_00")->setMessage(pPlayerMsg, 0x83D, 0, 0);
    mpLayout->findTextBox("T_1pTextB_00")->setMessage(pPlayerMsg, 0x83D, 0, 0);
    mpLayout->findTextBox("T_2pTextA_00")->setMessage(pPlayerMsg, 0x83D, 1, 0);
    mpLayout->findTextBox("T_2pTextB_00")->setMessage(pPlayerMsg, 0x83D, 1, 0);

    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
}

void RPDucStartLayout::Calculate() {
    if (mState == 2) {
        if (mpAnmObjs[0]->isFinished()) {
            mState = 0;
        }
    } else if (mState == 3) {
        if (mpAnmObjs[1]->isFinished()) {
            mState = 1;
        }
    }
    mpLayout->calc();
}

void RPDucStartLayout::Draw() {
    mpLayout->draw();
    if (RPDucGame::sInstance->GetMode() == 1) {
        DrawMiiIcons();
    }
}

void RPDucStartLayout::VF_0x18() {}

void RPDucStartLayout::PlayIn() {
    if (mState == 0) {
        return;
    }

    s32 currentRound = RPDucGame::sInstance->GetCurrentRound();
    s32 mode = RPDucGame::sInstance->GetMode();
    if (currentRound == 1 || mode != 1) {
        mpLayout->findPane("N_msg_00")->SetVisible(true);
        mpLayout->findPane("N_score_00")->SetVisible(false);
    } else {
        mpLayout->findPane("N_msg_00")->SetVisible(true);
        mpLayout->findPane("N_score_00")->SetVisible(true);
        s32 best =
            RPDucScoreMgr::sInstance->GetScore(1) <= RPDucScoreMgr::sInstance->GetScore(0)
                ? RPDucScoreMgr::sInstance->GetScore(0)
                : RPDucScoreMgr::sInstance->GetScore(1);
        if (best >= 100) {
            mpLayout->findPane("N_figureA_00")->SetVisible(true);
            mpLayout->findPane("N_figureB_00")->SetVisible(false);
        } else {
            mpLayout->findPane("N_figureA_00")->SetVisible(false);
            mpLayout->findPane("N_figureB_00")->SetVisible(true);
        }
    }

    RPSysMessage* const pMsg = RPDucHud::sInstance->GetMessage();
    const s32& round = RPDucGame::sInstance->GetCurrentRound();
    s32 roundNum = RPDucGame::sInstance->GetRoundNum();
    RPSysStringUtility::GetStringHalfSizeNumber(round, mRoundText, 4, false);
    if (roundNum == round) {
        mpLayout->findTextBox("T_msgText_00")->setMessage(pMsg, 0x866, 1, 0);
        mpLayout->findTextBox("T_msgText_01")->setMessage(pMsg, 0x866, 1, 0);
    } else {
        mpLayout->findTextBox("T_msgText_00")->setMessage(pMsg, 0x866, 0, 1, mRoundText);
        mpLayout->findTextBox("T_msgText_01")->setMessage(pMsg, 0x866, 0, 1, mRoundText);
    }

    RPSysMessage* const pScoreMsg = RPDucHud::sInstance->GetMessage();
    RPSysStringUtility::GetStringFullSizeNumber(RPDucScoreMgr::sInstance->GetScore(0),
                                                mScoreText[0], 4, false);
    mpLayout->findTextBox("T_scoreNumL_00")->setMessage(pScoreMsg, 0x83C, 0, 1, mScoreText[0]);
    RPSysStringUtility::GetStringFullSizeNumber(RPDucScoreMgr::sInstance->GetScore(1),
                                                mScoreText[1], 4, false);
    mpLayout->findTextBox("T_scoreNumR_00")->setMessage(pScoreMsg, 0x83C, 1, 1, mScoreText[1]);

    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mState = 2;
}

void RPDucStartLayout::PlayOut() {
    if (mState == 1) {
        return;
    }
    mpLayout->unbindAnmObj(mpAnmObjs[1]);
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
    mState = 3;
}
