// Pose Mii level clear, bonus and final score (RPBomLytClear, poseManClear_00).
#include <Pack/RPParty/RPBomScene/RPBomLyt_9.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysStringUtility.h>

#include <nw4r/lyt.h>

#include <cwchar>

// 4 wide characters: this unit ends .sdata, and the section size counts the 2 pad bytes
wchar_t RPBomLytClear::sBonusFormat[4] = L"+0";

RPBomLytClear::RPBomLytClear() {}

RPBomLytClear::~RPBomLytClear() {}

// Adds the level clear bonus to the players without a miss
static inline void AddClearBonus(RPBomLytClear* pLyt) {
    if (RPBomGameManager::sInstance->mMode == 0) {
        s32 bonus = RPBomQuestionParam::sInstance->mpFile
                        ->mLevels[RPBomGameManager::sInstance->mLevel]
                        .unk0;
        if (RPBomGameManager::sInstance->mNoMiss[0]) {
            RPBomScore::sInstance->AddScore(0, bonus);
            pLyt->PlayPlusEffect(0);
            RPSndAudioMgr::instance()->startSystemSe(0x1AU);
        }
    } else {
        s32 bonus = RPBomQuestionParam::sInstance->mpFile
                        ->mLevels[RPBomGameManager::sInstance->mLevel]
                        .unk0;
        BOOL added = FALSE;
        if (RPBomGameManager::sInstance->mNoMiss[0]) {
            RPBomScore::sInstance->AddScore(0, bonus);
            pLyt->PlayPlusEffect(0);
            added = TRUE;
        }
        if (RPBomGameManager::sInstance->mNoMiss[1]) {
            RPBomScore::sInstance->AddScore(1, bonus);
            pLyt->PlayPlusEffect(1);
            added = TRUE;
        }
        if (added) {
            RPSndAudioMgr::instance()->startSystemSe(0x1AU);
        }
    }
}

#pragma push
#pragma auto_inline off
void RPBomLytClear::ShowBonusText2P() {
    RPSysStringUtility::GetStringFullSizeNumber(
        RPBomQuestionParam::sInstance->mpFile->mLevels[RPBomGameManager::sInstance->mLevel].unk0,
        mBonusText, 8, true);

    if (RPBomGameManager::sInstance->mNoMiss[0]) {
        mpLayout->findTextBox("T_bonusPT_02")->setText(mBonusText, 0);
        mpLayout->findTextBox("T_bonusPT_03")->setText(mBonusText, 0);
    } else {
        mpLayout->findTextBox("T_bonusPT_02")->setText(mBonusText2, 0);
        mpLayout->findTextBox("T_bonusPT_03")->setText(mBonusText2, 0);
    }

    if (RPBomGameManager::sInstance->mNoMiss[1]) {
        mpLayout->findTextBox("T_bonusPT_04")->setText(mBonusText, 0);
        mpLayout->findTextBox("T_bonusPT_05")->setText(mBonusText, 0);
    } else {
        mpLayout->findTextBox("T_bonusPT_04")->setText(mBonusText2, 0);
        mpLayout->findTextBox("T_bonusPT_05")->setText(mBonusText2, 0);
    }
}

void RPBomLytClear::ShowClearText1P() {
    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    mpLayout->findPane("N_1play_00")->SetVisible(true);
    mpLayout->findPane("N_p1Score_00")->SetVisible(true);
    mpLayout->findPane("N_2play_00")->SetVisible(false);
    mpLayout->findPane("N_p2Score_00")->SetVisible(false);

    bool allClear = true;
    if (!RPBomGameManager::sInstance->mNoMiss[0]) {
        allClear = false;
    }

    if (allClear) {
        mpLayout->findTextBox("T_Clear_00")->setMessage(pMessage, 0xB05, 0, 0);
        mpLayout->findTextBox("T_Clear_01")->setMessage(pMessage, 0xB05, 0, 0);
    } else {
        mpLayout->findTextBox("T_Clear_00")->setMessage(pMessage, 0xB04, 0, 0);
        mpLayout->findTextBox("T_Clear_01")->setMessage(pMessage, 0xB04, 0, 0);
    }
}

void RPBomLytClear::ShowClearText2P() {
    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    mpLayout->findPane("N_1play_00")->SetVisible(false);
    mpLayout->findPane("N_p1Score_00")->SetVisible(false);
    mpLayout->findPane("N_2play_00")->SetVisible(true);
    mpLayout->findPane("N_p2Score_00")->SetVisible(true);

    bool allClear = true;
    for (int i = 0; i < 2; i++) {
        if (!RPBomGameManager::sInstance->mNoMiss[i]) {
            allClear = false;
        }
    }

    if (allClear) {
        mpLayout->findTextBox("T_Clear_00")->setMessage(pMessage, 0xB05, 0, 0);
        mpLayout->findTextBox("T_Clear_01")->setMessage(pMessage, 0xB05, 0, 0);
    } else {
        mpLayout->findTextBox("T_Clear_00")->setMessage(pMessage, 0xB04, 0, 0);
        mpLayout->findTextBox("T_Clear_01")->setMessage(pMessage, 0xB04, 0, 0);
    }
}

void RPBomLytClear::PlayPlusEffect(s32 player) {
    const char* scorePanes[3] = {"T_p1Score_00", "T_p2Score_00", "T_p2Score_02"};
    nw4r::lyt::Pane* pPane;
    if (RPBomGameManager::sInstance->mMode == 0) {
        pPane = mpLayout->findPane(scorePanes[0]);
    } else {
        pPane = mpLayout->findPane((scorePanes + 1)[player]);
    }

    nw4r::math::MTX34 mtx = pPane->GetGlobalMtx();
    nw4r::math::VEC3 pos;
    pos.x = mtx._03;
    pos.y = mtx._13;
    pos.z = mtx._23;

    const char* effectNames[2] = {"Bom_score_plus_blue", "Bom_score_plus_red"};
    RPSysEffect effect(effectNames[player], 4, 0);
    effect.create();
    effect.setPos(pos);
    nw4r::math::VEC3 scale(1.4142135f, 1.4142135f, 1.4142135f);
    effect.setScale(scale);
    effect.update();
}

void RPBomLytClear::UpdateAnims() {
    switch (mState) {
    case 0:
        break;
    case 1:
        if (mpAnmObjs[0]->getFrameCtrl()->getFrame() == 90.0f) {
            RPSndAudioMgr::instance()->startSystemSe(0x19U);
        }

        if (mpAnmObjs[0]->isFinished()) {
            if (RPBomGameManager::sInstance->mMode == 0) {
                mpLayout->unbindAnmObj(mpAnmObjs[1]);
                mpLayout->bindAnmObj(mpAnmObjs[1]);
                if (!RPBomGameManager::sInstance->mNoMiss[0]) {
                    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[1],
                                                    mpLayout->findPane("T_p1Score_01"), true);
                }
                mpAnmObjs[1]->restart();
            } else {
                mpLayout->unbindAnmObj(mpAnmObjs[1]);
                mpLayout->bindAnmObj(mpAnmObjs[1]);
                if (!RPBomGameManager::sInstance->mNoMiss[0]) {
                    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[1],
                                                    mpLayout->findPane("T_p2Score_01"), true);
                }
                if (!RPBomGameManager::sInstance->mNoMiss[1]) {
                    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[1],
                                                    mpLayout->findPane("T_p2Score_02"), true);
                }
                mpAnmObjs[1]->restart();
            }
            mState = 2;
        }
        break;
    case 2:
        if (mpAnmObjs[1]->getFrameCtrl()->getFrame() == 90.0f) {
            AddClearBonus(this);
        }

        if (mpAnmObjs[1]->isFinished()) {
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[2]);
            mpAnmObjs[2]->restart();
            mState = 3;
        }
        break;
    case 3:
        if (mpAnmObjs[2]->isFinished()) {
            mState = 4;
            mDone = true;
        }
        break;
    case 4:
        break;
    case 5:
        if (mpAnmObjs[3]->isFinished()) {
            mState = 0;
        }
        break;
    case 6:
        if (mpAnmObjs[4]->isFinished()) {
            if (RPBomGameManager::sInstance->mMode == 1) {
                s32 winner = RPBomScore::sInstance->GetWinner();
                if (winner != -1) {
                    mpLayout->unbindAnmObj(mpAnmObjs[6]);
                    if (winner == 0) {
                        mpLayout->bindAnmObjToPane(mpAnmObjs[6], "T_p2Score_01", true);
                    } else if (winner == 1) {
                        mpLayout->bindAnmObjToPane(mpAnmObjs[6], "T_p2Score_02", true);
                    }
                    mpAnmObjs[6]->restart();
                }
            }
            mState = 7;
        }
        break;
    case 7:
        break;
    case 8:
        if (mpAnmObjs[5]->isFinished()) {
            mState = 0;
            mScoreDone = true;
        }
        break;
    }
}

void RPBomLytClear::UpdateSlide() {
    if (RPBomGameManager::sInstance->mMode == 0) {
        return;
    }

    switch (mSlideState) {
    case 0:
        mSlideY = 0.0f;
        break;
    case 1:
        mSlideY = -69.0f;
        break;
    case 2:
        mSlideY -= 4.0f;
        if (mSlideY < -69.0f) {
            mSlideY = -69.0f;
        }
        break;
    }

    nw4r::lyt::Pane* pRoot = mpLayout->findPane("RootPane");
    const nw4r::math::VEC3& rTrans = pRoot->GetTranslate();
    nw4r::math::VEC3 trans(rTrans.x, rTrans.y, rTrans.z);
    trans.y = mSlideY;
    pRoot->SetTranslate(trans);
}

#pragma pop

void RPBomLytClear::Load(EGG::Archive* pArchive) {
    const char* anmNames[7] = {"poseManClear_00_in_00.brlan",       "poseManClear_00_bonus_00.brlan",
                               "poseManClear_00_outBg_00.brlan",    "poseManClear_00_out_00.brlan",
                               "poseManClear_00_inScore_00.brlan",  "poseManClear_00_outScore_00.brlan",
                               "poseManClear_00_blink_00.brlan"};

    AttachArcFile(pArchive, "poseMan.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "poseManClear_00.brlyt");

    int i = 0;
    do {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        if (i == 6) {
            mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_Loop);
        } else {
            mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
        }
    } while (++i < 7);
}

void RPBomLytClear::Reset() {
    mState = 0;
    mSlideState = 0;
    mTimer = 180;
    mDone = false;
    mSlideY = 0.0f;

    mpLayout->reset();

    int i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 7);

    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    RPSysStringUtility::GetStringFullSizeNumber(1, mBonusText, 8, true);

    int j = 0;
    do {
        mBonusText2[j] = 0;
    } while (++j < 8);
    std::swprintf(mBonusText2, 3, sBonusFormat);
    RPSysStringUtility::ChangeToFullSize(mBonusText2);

    if (RPBomGameManager::sInstance->mMode == 0) {
        RPSysStringUtility::GetStringFullSizeNumber(RPBomScore::sInstance->mScores[0], mScoreText[0], 6,
                                                    false);
        mpLayout->findTextBox("T_p1Score_00")->setText(mScoreText[0], 0);
        mpLayout->findTextBox("T_p1Score_01")->setText(mScoreText[0], 0);
    } else {
        RPSysStringUtility::GetStringFullSizeNumber(RPBomScore::sInstance->mScores[0], mScoreText[0], 6,
                                                    false);
        mpLayout->findTextBox("T_p2Score_00")->setText(mScoreText[0], 0);
        mpLayout->findTextBox("T_p2Score_01")->setText(mScoreText[0], 0);
        RPSysStringUtility::GetStringFullSizeNumber(RPBomScore::sInstance->mScores[1], mScoreText[1], 6,
                                                    false);
        mpLayout->findTextBox("T_p2Score_02")->setText(mScoreText[1], 0);
        mpLayout->findTextBox("T_p2Score_03")->setText(mScoreText[1], 0);
    }

    mpLayout->findTextBox("T_bonus_00")->setMessage(pMessage, 0xB06, 0, 0);
    mpLayout->findTextBox("T_bonus_01")->setMessage(pMessage, 0xB06, 0, 0);
    mpLayout->findTextBox("T_bonus_02")->setMessage(pMessage, 0xB06, 0, 0);
    mpLayout->findTextBox("T_bonus_03")->setMessage(pMessage, 0xB06, 0, 0);
    mpLayout->findTextBox("T_P1_00")->setMessage(pMessage, 0xB07, 0, 0);
    mpLayout->findTextBox("T_P1_01")->setMessage(pMessage, 0xB07, 0, 0);
    mpLayout->findTextBox("T_P2_00")->setMessage(pMessage, 0xB07, 1, 0);

    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
}

void RPBomLytClear::Calculate() {
    if (mState == 0) {
        return;
    }

    if (RPBomGameManager::sInstance->mMode == 0) {
        RPSysStringUtility::GetStringFullSizeNumber(RPBomScore::sInstance->mScores[0], mScoreText[0], 6,
                                                    false);
        mpLayout->findTextBox("T_p1Score_00")->setText(mScoreText[0], 0);
        mpLayout->findTextBox("T_p1Score_01")->setText(mScoreText[0], 0);
    } else {
        RPSysStringUtility::GetStringFullSizeNumber(RPBomScore::sInstance->mScores[0], mScoreText[0], 6,
                                                    false);
        mpLayout->findTextBox("T_p2Score_00")->setText(mScoreText[0], 0);
        mpLayout->findTextBox("T_p2Score_01")->setText(mScoreText[0], 0);
        RPSysStringUtility::GetStringFullSizeNumber(RPBomScore::sInstance->mScores[1], mScoreText[1], 6,
                                                    false);
        mpLayout->findTextBox("T_p2Score_02")->setText(mScoreText[1], 0);
        mpLayout->findTextBox("T_p2Score_03")->setText(mScoreText[1], 0);
    }

    UpdateAnims();
    UpdateSlide();
    mpLayout->calc();
}

void RPBomLytClear::Draw() {
    if (mState != 0) {
        mpLayout->draw();
    }
}

void RPBomLytClear::VF_0x18() {}

// Shows the level clear bonus (ledger name; called by RPBomGameManager)
void RPBomLytClear::UpdateStates() {
    if (mState != 0) {
        return;
    }

    mTimer = 180;
    mDone = false;

    if (RPBomGameManager::sInstance->mMode == 0) {
        RPSysStringUtility::GetStringFullSizeNumber(
            RPBomQuestionParam::sInstance->mpFile->mLevels[RPBomGameManager::sInstance->mLevel].unk0,
            mBonusText, 8, true);
        if (RPBomGameManager::sInstance->mNoMiss[0]) {
            mpLayout->findTextBox("T_bonusPT_00")->setText(mBonusText, 0);
            mpLayout->findTextBox("T_bonusPT_01")->setText(mBonusText, 0);
        } else {
            mpLayout->findTextBox("T_bonusPT_00")->setText(mBonusText2, 0);
            mpLayout->findTextBox("T_bonusPT_01")->setText(mBonusText2, 0);
        }
        ShowClearText1P();
    } else if (RPBomGameManager::sInstance->mMode == 1) {
        ShowBonusText2P();
        ShowClearText2P();
    }

    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mState = 1;
}

// Plays the "out" animation (ledger name; called by RPBomGameManager)
void RPBomLytClear::PlayScorePlusEffect() {
    if (mState != 0) {
        mpLayout->unbindAnmObj(mpAnmObjs[3]);
        mpLayout->bindAnmObj(mpAnmObjs[3]);
        mpAnmObjs[3]->restart();
        mState = 5;
    }
}

// Shows the final score (ledger name; called by RPBomGameManager)
void RPBomLytClear::SetClearText2P() {
    if (mState != 0) {
        return;
    }

    mTimer = 180;
    mScoreDone = false;

    if (RPBomGameManager::sInstance->mMode == 0) {
        mpLayout->findPane("N_p1Score_00")->SetVisible(true);
        mpLayout->findPane("N_p2Score_00")->SetVisible(false);
    } else if (RPBomGameManager::sInstance->mMode == 1) {
        mpLayout->findPane("N_p1Score_00")->SetVisible(false);
        mpLayout->findPane("N_p2Score_00")->SetVisible(true);
    }

    mpLayout->unbindAnmObj(mpAnmObjs[4]);
    mpLayout->bindAnmObj(mpAnmObjs[4]);
    mpAnmObjs[4]->restart();
    mSlideState = 1;
    mState = 6;
}

// Plays the final score's "out" animation (ledger name; called by RPBomGameManager)
void RPBomLytClear::SetClearText1P() {
    if (mState != 0) {
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpAnmObjs[5]);
        mpAnmObjs[5]->restart();
        mState = 8;
    }
}

// Slides the layout up and blinks the winner's score (ledger name; called by RPBomGameManager)
void RPBomLytClear::SetBonusText2P() {
    mSlideState = 2;

    s32 winner = RPBomScore::sInstance->GetWinner();
    if (winner != -1) {
        mpLayout->unbindAnmObj(mpAnmObjs[6]);
        if (winner == 0) {
            mpLayout->bindAnmObjToPane(mpAnmObjs[6], "T_p2Score_01", true);
        } else if (winner == 1) {
            mpLayout->bindAnmObjToPane(mpAnmObjs[6], "T_p2Score_02", true);
        }
        mpAnmObjs[6]->restart();
    }
}
