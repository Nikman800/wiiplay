// Pose Mii score display (RPBomLytScore, poseManScore_00).
#include <Pack/RPParty/RPBomScene/RPBomLyt_3.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysStringUtility.h>

#include <egg/gfxe/eggScreen.h>
#include <nw4r/lyt.h>

RPBomLytScore::RPBomLytScore() {}

RPBomLytScore::~RPBomLytScore() {}

void RPBomLytScore::UpdateAnims() {
    if (mState == 1) {
        if (mpAnmObjs[0]->isFinished()) {
            mState = 2;
        }
    } else if (mState == 3) {
        if (mpAnmObjs[7]->isFinished()) {
            mState = 0;
        }
    }

    if (mMoveLeft) {
        if (mpAnmObjs[1]->isFinished()) {
            RPBomScore::sInstance->AddScore(1, mMovePoints);
            RPSndAudioMgr::instance()->startSystemSeWithRemote(0x15U, 2);
            PlayScorePlusEffect(1);
            mpLayout->unbindAnmObj(mpAnmObjs[1]);
            mpLayout->unbindAnmObj(mpAnmObjs[2]);
            mpLayout->bindAnmObj(mpAnmObjs[2]);
            mpAnmObjs[2]->restart();
            mMoveLeft = false;
        }
    } else if (mMoveRight) {
        if (mpAnmObjs[3]->isFinished()) {
            RPBomScore::sInstance->AddScore(0, mMovePoints);
            RPSndAudioMgr::instance()->startSystemSeWithRemote(0x14U, 1);
            PlayScorePlusEffect(0);
            mpLayout->unbindAnmObj(mpAnmObjs[3]);
            mpLayout->unbindAnmObj(mpAnmObjs[4]);
            mpLayout->bindAnmObj(mpAnmObjs[4]);
            mpAnmObjs[4]->restart();
            mMoveRight = false;
        }
    }
}

void RPBomLytScore::PlayScorePlusEffect(s32 player) {
    const char* scorePanes[2] = {"T_1pScore_00", "T_2pScore_00"};
    nw4r::math::MTX34 mtx = mpLayout->findPane(scorePanes[player])->GetGlobalMtx();
    s32 score = RPBomScore::sInstance->mScores[player];

    f32 digits = 1.0f;
    if (score >= 10000) {
        digits = 5.0f;
    } else if (score >= 1000) {
        digits = 4.0f;
    } else if (score >= 100) {
        digits = 3.0f;
    } else if (score >= 10) {
        digits = 2.0f;
    }

    const char* effectNames[2] = {"Bom_score_plus_blue", "Bom_score_plus_red"};
    nw4r::math::VEC3 pos;
    f32 width = digits * 21.0f + (digits - 1.0f) * 3.0f;
    pos.x = mtx._03 - width * 0.5f;
    pos.y = mtx._13;
    pos.z = mtx._23;

    RPSysEffect effect(effectNames[player], 4, 0);
    effect.create();
    effect.setPos(pos);
    nw4r::math::VEC3 scale(1.0f, 1.0f, 1.0f);
    effect.setScale(scale);
    effect.update();
}

void RPBomLytScore::Load(EGG::Archive* pArchive) {
    const char* anmNames[8] = {"poseManScore_00_inAll_00.brlan",
                               "poseManScore_00_moveScoreL_00.brlan",
                               "poseManScore_00_switchScoreR_00.brlan",
                               "poseManScore_00_moveScoreR_00.brlan",
                               "poseManScore_00_switchScoreL_00.brlan",
                               "poseManScore_00_switch1pSco_00.brlan",
                               "poseManScore_00_switch2pSco_00.brlan",
                               "poseManScore_00_outAll_00.brlan"};

    AttachArcFile(pArchive, "poseMan.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "poseManScore_00.brlyt");

    int i = 0;
    do {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        if (i == 5 || i == 6) {
            mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_Loop);
        } else {
            mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
        }
    } while (++i < 8);
}

void RPBomLytScore::Reset() {
    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    RPSysStringUtility::GetStringFullSizeNumber(RPBomScore::sInstance->mScores[0], mScoreText[0], 6,
                                                false);
    mpLayout->findTextBox("T_1pScore_00")->setMessage(pMessage, 0xAF2, 0, 1, mScoreText[0]);
    if (RPBomGameManager::sInstance->mMode == 1) {
        RPSysStringUtility::GetStringFullSizeNumber(RPBomScore::sInstance->mScores[1], mScoreText[1],
                                                    6, false);
        mpLayout->findTextBox("T_2pScore_00")->setMessage(pMessage, 0xAF2, 1, 1, mScoreText[1]);
    }

    mpLayout->reset();

    int i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 8);

    RPSysMessage* pMessage2 = RPBomLayoutMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_1pText_00")->setMessage(pMessage2, 0xAF3, 0, 0);
    mpLayout->findTextBox("T_2pText_00")->setMessage(pMessage2, 0xAF3, 1, 0);

    mpLayout->findPane("N_L_00")->SetVisible(false);
    mpLayout->findPane("N_R_00")->SetVisible(false);
    mpLayout->findPane("N_moveLA_00")->SetVisible(false);
    mpLayout->findPane("N_moveLB_00")->SetVisible(false);
    mpLayout->findPane("N_moveRA_00")->SetVisible(false);
    mpLayout->findPane("N_moveRB_00")->SetVisible(false);

    mpLayout->unbindAllAnmObj();
    mMoveLeft = false;
    mMoveRight = false;
    mMovePoints = 0;
    mState = 0;
}

void RPBomLytScore::Calculate() {
    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    RPSysStringUtility::GetStringFullSizeNumber(RPBomScore::sInstance->mScores[0], mScoreText[0], 6,
                                                false);
    mpLayout->findTextBox("T_1pScore_00")->setMessage(pMessage, 0xAF2, 0, 1, mScoreText[0]);
    if (RPBomGameManager::sInstance->mMode == 1) {
        RPSysStringUtility::GetStringFullSizeNumber(RPBomScore::sInstance->mScores[1], mScoreText[1],
                                                    6, false);
        mpLayout->findTextBox("T_2pScore_00")->setMessage(pMessage, 0xAF2, 1, 1, mScoreText[1]);
    }

    UpdateAnims();
    mpLayout->calc();
}

void RPBomLytScore::Draw() {
    mpLayout->draw();
}

void RPBomLytScore::VF_0x18() {}

void RPBomLytScore::PlayIn() {
    if (mState != 0) {
        return;
    }

    if (RPBomGameManager::sInstance->mMode == 1) {
        mpLayout->findPane("N_L_00")->SetVisible(true);
        mpLayout->findPane("N_R_00")->SetVisible(true);
    } else {
        mpLayout->findPane("N_L_00")->SetVisible(true);
        mpLayout->findPane("N_R_00")->SetVisible(false);
    }

    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObjToPane(mpAnmObjs[0], "N_L_00", true);
    mpLayout->bindAnmObjToPane(mpAnmObjs[0], "N_R_00", true);
    mpAnmObjs[0]->restart();
    mState = 1;
}

void RPBomLytScore::PlayOut() {
    if (mState != 2) {
        return;
    }

    if (RPBomGameManager::sInstance->mMode == 1) {
        mpLayout->findPane("N_L_00")->SetVisible(true);
        mpLayout->findPane("N_R_00")->SetVisible(true);
    } else {
        mpLayout->findPane("N_L_00")->SetVisible(true);
        mpLayout->findPane("N_R_00")->SetVisible(false);
    }

    mpLayout->unbindAnmObj(mpAnmObjs[7]);
    mpLayout->bindAnmObjToPane(mpAnmObjs[7], "N_L_00", true);
    mpLayout->bindAnmObjToPane(mpAnmObjs[7], "N_R_00", true);
    mpAnmObjs[7]->restart();
    mState = 3;
}

void RPBomLytScore::ShowMoveLeft(s32 points) {
    bool std = EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_STD;
    RPSysStringUtility::GetStringFullSizeNumber(points, mMoveText, 6, false);

    const char* movePanes[2] = {"N_moveLA_00", "N_moveLB_00"};
    if (std) {
        mpLayout->findPane(movePanes[0])->SetVisible(true);
        mpLayout->findPane(movePanes[1])->SetVisible(false);
    } else {
        mpLayout->findPane(movePanes[0])->SetVisible(false);
        mpLayout->findPane(movePanes[1])->SetVisible(true);
    }

    const char* textPanes[4] = {"T_moveLA_00", "T_moveLA_01", "T_moveLB_00", "T_moveLB_01"};
    int i = 0;
    do {
        mpLayout->findTextBox(textPanes[i])->setText(mMoveText, 0);
        mpLayout->findTextBox(textPanes[i + 2])->setText(mMoveText, 0);
    } while (++i < 2);

    mpLayout->unbindAnmObj(mpAnmObjs[1]);
    mpLayout->unbindAnmObj(mpAnmObjs[2]);
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
    RPSndAudioMgr::instance()->startSystemSe(4U);
    mMovePoints = points;
    mMoveLeft = true;
}

void RPBomLytScore::ShowMoveRight(s32 points) {
    bool std = EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_STD;
    RPSysStringUtility::GetStringFullSizeNumber(points, mMoveText, 6, false);

    const char* movePanes[2] = {"N_moveRA_00", "N_moveRB_00"};
    if (std) {
        mpLayout->findPane(movePanes[0])->SetVisible(true);
        mpLayout->findPane(movePanes[1])->SetVisible(false);
    } else {
        mpLayout->findPane(movePanes[0])->SetVisible(false);
        mpLayout->findPane(movePanes[1])->SetVisible(true);
    }

    const char* textPanes[4] = {"T_moveRA_00", "T_moveRA_01", "T_moveRB_00", "T_moveRB_01"};
    int i = 0;
    do {
        mpLayout->findTextBox(textPanes[i])->setText(mMoveText, 0);
        mpLayout->findTextBox(textPanes[i + 2])->setText(mMoveText, 0);
    } while (++i < 2);

    mpLayout->unbindAnmObj(mpAnmObjs[3]);
    mpLayout->unbindAnmObj(mpAnmObjs[4]);
    mpLayout->bindAnmObj(mpAnmObjs[3]);
    mpAnmObjs[3]->restart();
    RPSndAudioMgr::instance()->startSystemSe(5U);
    mMovePoints = points;
    mMoveRight = true;
}
