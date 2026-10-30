#include <Pack/RPParty/RPDucScene/RPDucHud.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPMiiIconSet.h>
class RPGrpCamera;
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysStringUtility.h>

RPDucScoreLayout::RPDucScoreLayout() {}
RPDucScoreLayout::~RPDucScoreLayout() {}

void RPDucScoreLayout::UpdateHitCounterP1() {
    RPSysMessage* message = RPDucHud::sInstance->GetMessage();
    s32 hits = RPDucScoreMgr::sInstance->GetStreak(0);
    RPSysStringUtility::GetStringFullSizeNumber(hits, mHitText[0], 4, false);
    if (RPDucGame::sInstance->GetState() != 3 && RPDucGame::sInstance->GetState() != 4) {
        mpLayout->findPane("N_hitL_00")->SetVisible(false);
        mpLayout->findPane("N_hitNumL_00")->SetVisible(false);
        mpLayout->findPane("N_bonusL_00")->SetVisible(false);
    } else if (mBonusState[0] != 0) {
        mpLayout->findPane("N_hitL_00")->SetVisible(false);
        mpLayout->findPane("N_hitNumL_00")->SetVisible(false);
        mpLayout->findPane("N_bonusL_00")->SetVisible(true);
        if (mBonusState[0] == 1 && --mBonusTimer[0] <= 0) {
            mpLayout->unbindAnmObj(mpAnmObjs[8]);
            mpLayout->unbindAnmObj(mpAnmObjs[10]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[10], "N_bonusL_00", true);
            mpAnmObjs[10]->restart();
            mBonusState[0] = 3;
        }
    } else {
            if (hits == 0) {
                mpLayout->findPane("N_hitL_00")->SetVisible(false);
                mpLayout->findPane("N_hitNumL_00")->SetVisible(false);
                mpLayout->findPane("N_bonusL_00")->SetVisible(false);
            } else if (hits == 1) {
                mpLayout->findPane("N_hitL_00")->SetVisible(true);
                mpLayout->findPane("N_hitNumL_00")->SetVisible(false);
                mpLayout->findPane("N_bonusL_00")->SetVisible(false);
                if (RPSysStringUtility::IsSingular(hits))
                    mpLayout->findTextBox("T_hitUnitL_00")->setMessage(message, 0x838, 0, 0);
                else
                    mpLayout->findTextBox("T_hitUnitL_00")->setMessage(message, 0x838, 1, 0);
            } else {
                mpLayout->findPane("N_hitL_00")->SetVisible(true);
                mpLayout->findPane("N_hitNumL_00")->SetVisible(true);
                mpLayout->findPane("N_bonusL_00")->SetVisible(false);
                mpLayout->findTextBox("T_hitNumL_00")->setMessage(message, 0x83a, 0, 1, mHitText[0]);
                if (RPSysStringUtility::IsSingular(hits))
                    mpLayout->findTextBox("T_hitUnitL_00")->setMessage(message, 0x838, 0, 0);
                else
                    mpLayout->findTextBox("T_hitUnitL_00")->setMessage(message, 0x838, 1, 0);
            }
    }
}

void RPDucScoreLayout::UpdateHitCounterP2() {
    s32 hits = RPDucScoreMgr::sInstance->GetStreak(1);
    RPSysMessage* message = RPDucHud::sInstance->GetMessage();
    RPSysStringUtility::GetStringFullSizeNumber(hits, mHitText[1], 4, false);
    if (RPDucGame::sInstance->GetState() != 3 && RPDucGame::sInstance->GetState() != 4) {
        mpLayout->findPane("N_hitR_00")->SetVisible(false);
        mpLayout->findPane("N_hitNumR_00")->SetVisible(false);
        mpLayout->findPane("N_bonusR_00")->SetVisible(false);
    } else if (mBonusState[1] != 0) {
        mpLayout->findPane("N_hitR_00")->SetVisible(false);
        mpLayout->findPane("N_hitNumR_00")->SetVisible(false);
        mpLayout->findPane("N_bonusR_00")->SetVisible(true);
        if (mBonusState[1] == 1 && --mBonusTimer[1] <= 0) {
            mpLayout->unbindAnmObj(mpAnmObjs[9]);
            mpLayout->unbindAnmObj(mpAnmObjs[11]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[11], "N_bonusR_00", true);
            mpAnmObjs[11]->restart();
            mBonusState[1] = 3;
        }
    } else {
            if (hits == 0) {
                mpLayout->findPane("N_hitR_00")->SetVisible(false);
                mpLayout->findPane("N_hitNumR_00")->SetVisible(false);
                mpLayout->findPane("N_bonusR_00")->SetVisible(false);
            } else if (hits == 1) {
                mpLayout->findPane("N_hitR_00")->SetVisible(true);
                mpLayout->findPane("N_hitNumR_00")->SetVisible(false);
                mpLayout->findPane("N_bonusR_00")->SetVisible(false);
                mpLayout->findTextBox("T_hitUnitR_00")->setMessage(message, 0x838, 0, 0);
            } else {
                mpLayout->findPane("N_hitR_00")->SetVisible(true);
                mpLayout->findPane("N_hitNumR_00")->SetVisible(true);
                mpLayout->findPane("N_bonusR_00")->SetVisible(false);
                mpLayout->findTextBox("T_hitNumR_00")->setMessage(message, 0x83a, 1, 1, mHitText[1]);
                mpLayout->findTextBox("T_hitUnitR_00")->setMessage(message, 0x838, 1, 0);
            }
    }
}

void RPDucScoreLayout::PlayScoreAnimP1(s32 type) {
    mpLayout->unbindAnmObj(mpAnmObjs[2]);
    mpLayout->bindAnmObjToPane(mpAnmObjs[2], "T_1pScore_00", true);
    mpAnmObjs[2]->restart();
    if (type == 0 || type == 3) return;
    if (mBonusState[0] == 0) {
        mpLayout->unbindAnmObj(mpAnmObjs[4]);
        mpLayout->unbindAnmObj(mpAnmObjs[5]);
        switch (type) {
        case 1:
            mpLayout->bindAnmObjToPane(mpAnmObjs[4], "N_hitL_00", true);
            mpAnmObjs[4]->restart();
            break;
        case 2:
            mpLayout->bindAnmObjToPane(mpAnmObjs[5], "N_hitL_00", true);
            mpAnmObjs[5]->restart();
            break;
        }
    }
}

void RPDucScoreLayout::PlayScoreAnimP2(s32 type) {
    mpLayout->unbindAnmObj(mpAnmObjs[3]);
    mpLayout->bindAnmObjToPane(mpAnmObjs[3], "T_2pScore_00", true);
    mpAnmObjs[3]->restart();
    if (type == 0 || type == 3) return;
    if (mBonusState[1] == 0) {
        mpLayout->unbindAnmObj(mpAnmObjs[6]);
        mpLayout->unbindAnmObj(mpAnmObjs[7]);
        switch (type) {
        case 1:
            mpLayout->bindAnmObjToPane(mpAnmObjs[6], "N_hitR_00", true);
            mpAnmObjs[6]->restart();
            break;
        case 2:
            mpLayout->bindAnmObjToPane(mpAnmObjs[7], "N_hitR_00", true);
            mpAnmObjs[7]->restart();
            break;
        }
    }
}

void RPDucScoreLayout::UpdateAnimStates() {
    if (static_cast<u32>(mState) == 1) {
        if (mpAnmObjs[0]->isFinished()) mState = 2;
    } else if (static_cast<u32>(mState) == 3 && mpAnmObjs[1]->isFinished()) {
        mState = 0;
    }
    if (mBonusState[0] == 2) {
        if (mpAnmObjs[8]->isFinished()) mBonusState[0] = 1;
    } else if (mBonusState[0] == 3 && mpAnmObjs[10]->isFinished()) {
        if (RPDucScoreMgr::sInstance->GetStreak(0) > 0) {
            mpLayout->unbindAnmObj(mpAnmObjs[4]);
            mpLayout->unbindAnmObj(mpAnmObjs[5]);
            if (RPDucScoreMgr::sInstance->HasStreakBonus(0)) {
                mpLayout->bindAnmObjToPane(mpAnmObjs[5], "N_hitL_00", true);
                mpAnmObjs[5]->restart();
            } else {
                mpLayout->bindAnmObjToPane(mpAnmObjs[4], "N_hitL_00", true);
                mpAnmObjs[4]->restart();
            }
        }
        mBonusState[0] = 0;
    }
    if (mBonusState[1] == 2) {
        if (mpAnmObjs[9]->isFinished()) mBonusState[1] = 1;
    } else if (mBonusState[1] == 3 && mpAnmObjs[11]->isFinished()) {
        if (RPDucScoreMgr::sInstance->GetStreak(1) > 0) {
            mpLayout->unbindAnmObj(mpAnmObjs[6]);
            mpLayout->unbindAnmObj(mpAnmObjs[7]);
            if (RPDucScoreMgr::sInstance->HasStreakBonus(1)) {
                mpLayout->bindAnmObjToPane(mpAnmObjs[7], "N_hitR_00", true);
                mpAnmObjs[7]->restart();
            } else {
                mpLayout->bindAnmObjToPane(mpAnmObjs[6], "N_hitR_00", true);
                mpAnmObjs[6]->restart();
            }
        }
        mBonusState[1] = 0;
    }
}

void RPDucScoreLayout::Load(EGG::Archive* pArchive) {
    const char* names[12] = {
        "duckhuntScore_00_inAll_00.brlan",
        "duckhuntScore_00_outAll_00.brlan",
        "duckhuntScore_00_switchScoreL_00.brlan",
        "duckhuntScore_00_switchScoreR_00.brlan",
        "duckhuntScore_00_switchHitL_00.brlan",
        "duckhuntScore_00_switchHitL_01.brlan",
        "duckhuntScore_00_switchHitR_00.brlan",
        "duckhuntScore_00_switchHitR_01.brlan",
        "duckhuntScore_00_inBonusL_00.brlan",
        "duckhuntScore_00_inBonusR_00.brlan",
        "duckhuntScore_00_outBonusL_00.brlan",
        "duckhuntScore_00_outBonusR_00.brlan"
    };
    AttachArcFile(pArchive, "duckhunt.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "duckhuntScore_00.brlyt");
    for (s32 i = 0; i < 12; ++i) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, names[i]);
        mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    }
}

void RPDucScoreLayout::Reset() {
    RPSysStringUtility::GetStringFullSizeNumber(RPDucScoreMgr::sInstance->GetScore(0), mScoreText[0], 4, false);
    mpLayout->findTextBox("T_1pScore_00")->setText(mScoreText[0], 0);
    if (RPDucGame::sInstance->GetMode() == 1) {
        RPSysStringUtility::GetStringFullSizeNumber(RPDucScoreMgr::sInstance->GetScore(1), mScoreText[1], 4, false);
        mpLayout->findTextBox("T_2pScore_00")->setText(mScoreText[1], 0);
    }
    mpLayout->reset();
    s32 i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 12);
    RPSysMessage* message = RPDucHud::sInstance->GetMessage();
    mpLayout->findTextBox("T_1pText_00")->setMessage(message, 0x837, 0, 0);
    mpLayout->findTextBox("T_1pText_01")->setMessage(message, 0x837, 0, 0);
    mpLayout->findTextBox("T_2pText_00")->setMessage(message, 0x837, 1, 0);
    mpLayout->findTextBox("T_2pText_01")->setMessage(message, 0x837, 1, 0);
    mpLayout->findPane("N_L_00")->SetVisible(false);
    mpLayout->findPane("N_hitL_00")->SetVisible(false);
    mpLayout->findPane("N_bonusL_00")->SetVisible(false);
    mpLayout->findPane("N_R_00")->SetVisible(false);
    mpLayout->findPane("N_hitR_00")->SetVisible(false);
    mpLayout->findPane("N_bonusR_00")->SetVisible(false);
    mpLayout->unbindAllAnmObj();
    mState = 0;
    RPSysStringUtility::GetStringFullSizeNumber(0, mBonusText[0], 8, false);
    RPSysStringUtility::GetStringFullSizeNumber(0, mBonusText[1], 8, false);
    mBonusState[0] = 0;
    mBonusState[1] = 0;
    mBonusTimer[0] = 0;
    mBonusTimer[1] = 0;
}

void RPDucScoreLayout::Calculate() {
    RPSysStringUtility::GetStringFullSizeNumber(RPDucScoreMgr::sInstance->GetScore(0), mScoreText[0], 4, false);
    mpLayout->findTextBox("T_1pScore_00")->setText(mScoreText[0], 0);
    if (RPDucGame::sInstance->GetMode() == 1) {
        RPSysStringUtility::GetStringFullSizeNumber(RPDucScoreMgr::sInstance->GetScore(1), mScoreText[1], 4, false);
        mpLayout->findTextBox("T_2pScore_00")->setText(mScoreText[1], 0);
    }
    UpdateHitCounterP1();
    if (RPDucGame::sInstance->GetMode() == 1) UpdateHitCounterP2();
    UpdateAnimStates();
    mpLayout->calc();
}

void RPDucScoreLayout::Draw() {
    mpLayout->draw();
    if (mState != 0) {
        RPGrpScreen* screen = RPGrpRenderer::GetCurrent()->GetView(1)->GetScreen();
        nw4r::lyt::Pane* pane = mpLayout->findPane("N_1pFace_00");
        RPDrawKokeshiIcon(RPMiiIconSet::sInstance->GetIcon(0), pane, false, screen);
        if (RPDucGame::sInstance->GetMode() == 1) {
            pane = mpLayout->findPane("N_2pFace_00");
            RPDrawKokeshiIcon(RPMiiIconSet::sInstance->GetIcon(1), pane, false, screen);
        }
    }
}

void RPDucScoreLayout::VF_0x18() {}

void RPDucScoreLayout::ShowHitCounter() {
    if (mState == 0) {
        if (RPDucGame::sInstance->GetMode() == 1) {
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
}

void RPDucScoreLayout::HideHitCounter() {
    if (static_cast<u32>(mState) == 2) {
        if (RPDucGame::sInstance->GetMode() == 1) {
            mpLayout->findPane("N_L_00")->SetVisible(true);
            mpLayout->findPane("N_R_00")->SetVisible(true);
        } else {
            mpLayout->findPane("N_L_00")->SetVisible(true);
            mpLayout->findPane("N_R_00")->SetVisible(false);
        }
        mpLayout->unbindAnmObj(mpAnmObjs[1]);
        mpLayout->bindAnmObjToPane(mpAnmObjs[1], "N_L_00", true);
        mpLayout->bindAnmObjToPane(mpAnmObjs[1], "N_R_00", true);
        mpAnmObjs[1]->restart();
        mState = 3;
    }
}

void RPDucScoreLayout::PlayScoreAnim(s32 player, s32 type) {
    if (static_cast<u32>(mState) != 2) return;
    if (player == 0) PlayScoreAnimP1(type);
    else PlayScoreAnimP2(type);
}

void RPDucScoreLayout::ShowBonus(s32 player, s32 bonus) {
    if (player == 0) {
        RPSysMessage* message = RPDucHud::sInstance->GetMessage();
        mpLayout->unbindAnmObj(mpAnmObjs[8]);
        mpLayout->unbindAnmObj(mpAnmObjs[10]);
        mpLayout->bindAnmObjToPane(mpAnmObjs[8], "N_bonusL_00", true);
        mpAnmObjs[8]->restart();
        RPSysStringUtility::GetStringFullSizeNumber(bonus, mBonusText[0], 8, true);
        mpLayout->findTextBox("T_bonusNumL_00")->setMessage(message, 0x83b, 0, 1, mBonusText[0]);
        mpLayout->findTextBox("T_bonusNumL_01")->setMessage(message, 0x83b, 0, 1, mBonusText[0]);
        mpLayout->findTextBox("T_bonusUnitL_00")->setMessage(message, 0x839, 0, 0);
        mpLayout->findTextBox("T_bonusUnitL_01")->setMessage(message, 0x839, 0, 0);
        mBonusState[0] = 2;
        mBonusTimer[0] = 120;
    } else {
        RPSysMessage* message = RPDucHud::sInstance->GetMessage();
        mpLayout->unbindAnmObj(mpAnmObjs[9]);
        mpLayout->unbindAnmObj(mpAnmObjs[11]);
        mpLayout->bindAnmObjToPane(mpAnmObjs[9], "N_bonusR_00", true);
        mpAnmObjs[9]->restart();
        RPSysStringUtility::GetStringFullSizeNumber(bonus, mBonusText[1], 8, true);
        mpLayout->findTextBox("T_bonusNumR_00")->setMessage(message, 0x83b, 1, 1, mBonusText[1]);
        mpLayout->findTextBox("T_bonusNumR_01")->setMessage(message, 0x83b, 1, 1, mBonusText[1]);
        mpLayout->findTextBox("T_bonusUnitR_00")->setMessage(message, 0x839, 0, 0);
        mpLayout->findTextBox("T_bonusUnitR_01")->setMessage(message, 0x839, 0, 0);
        mBonusState[1] = 2;
        mBonusTimer[1] = 120;
    }
}
