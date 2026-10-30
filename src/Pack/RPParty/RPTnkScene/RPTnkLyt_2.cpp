// Tanks stage fades with the mission, enemy count, lives and bonus texts (tnkfade.brlyt).
// Unit 0x8026E350-0x8026F418 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkLyt.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPMiiIconSet.h>
#include <Pack/RPParty/RPTnkScene/RPTnkGameManager.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTankManager.h>
#include <nw4r/lyt.h>

RPTnkFade::RPTnkFade() {}

RPTnkFade::~RPTnkFade() {}

void RPTnkFade::UpdateStates() {
    s32 state = mState;
    if (state == 1) {
        if (mpAnmObjs[0]->isFinished()) {
            mpAnmObjs[0]->setRate(1.0f);
            if (!mFast) {
                RPTnkGameManager::sInstance->SetStageReady(true);
            }
            mpLayout->unbindAnmObj(mpAnmObjs[2]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[2], "N_life_00", true);
            mpAnmObjs[2]->restart();
            if (mBonus) {
                mpLayout->unbindAnmObj(mpAnmObjs[4]);
                mpLayout->bindAnmObjToPane(mpAnmObjs[4], "N_msg_01", true);
                mpAnmObjs[4]->restart();
                RPSndAudioMgr::instance()->startSystemSe(0x21);
            }
            mState = 2;
        }
    } else if (state == 2) {
        if (mBonus) {
            if (mpAnmObjs[2]->isFinished() && mpAnmObjs[4]->isFinished()) {
                mState = 3;
            }
        } else if (mpAnmObjs[2]->isFinished()) {
            mState = 3;
        }
    } else if (state == 4) {
        if (mpAnmObjs[5]->isFinished()) {
            // Same mission display as in Calculate (an inline member computes the names up front)
            mTimer = 140;
            RPSysMessage* pMsg = RPTnkLayoutManager::sInstance->GetMessage();
            mMission = RPTnkGameManager::sInstance->GetStage() + 1;
            RPSysStringUtility::GetStringFullSizeNumber(mMission, mMissionText, 4, false);
            if (RPTnkGameManager::sInstance->GetStage() == RPTnkGameManager::sInstance->GetLastStage()) {
                mpLayout->findTextBox("T_mission_00")->setMessage(pMsg, 0xB5D, 1, 0);
                mpLayout->findTextBox("T_mission_01")->setMessage(pMsg, 0xB5D, 1, 0);
            } else {
                mpLayout->findTextBox("T_mission_00")->setMessage(pMsg, 0xB5D, 0, 1, mMissionText);
                mpLayout->findTextBox("T_mission_01")->setMessage(pMsg, 0xB5D, 0, 1, mMissionText);
            }

            pMsg = RPTnkLayoutManager::sInstance->GetMessage();
            mEnemyNum = RPTnkTankManager::sInstance->GetEnemyAliveNum();
            RPSysStringUtility::GetStringFullSizeNumber(mEnemyNum, mEnemyText, 4, false);
            mpLayout->findTextBox("T_enemy_00")->setMessage(pMsg, 0xB5E, 0, 1, mEnemyText);
            mpLayout->findTextBox("T_enemy_01")->setMessage(pMsg, 0xB5E, 0, 1, mEnemyText);

            mpLayout->unbindAnmObj(mpAnmObjs[8]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[8], "N_msg_00", true);
            mpAnmObjs[8]->restart();
            mpLayout->unbindAnmObj(mpAnmObjs[9]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[9], "N_enemy_00", true);
            mpAnmObjs[9]->restart();
            RPTnkGameManager::sInstance->StartBgm();
            mState = 5;
        }
    } else if (state == 5) {
        if (mpAnmObjs[8]->isFinished() && mpAnmObjs[9]->isFinished()) {
            mState = 6;
        }
    } else if (state == 7) {
        if (mpAnmObjs[3]->isFinished()) {
            mpLayout->unbindAnmObj(mpAnmObjs[1]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[1], "N_loop_00", true);
            mpAnmObjs[1]->restart();
            mpLayout->unbindAnmObj(mpAnmObjs[10]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[10], "P_loop_00", true);
            RPTnkGameManager::sInstance->StartStageBgm();
            mState = 8;
        }
    } else if (state == 8) {
        if (mpAnmObjs[1]->isFinished()) {
            mState = 0;
            mDone = true;
        }
    }
}

void RPTnkFade::Load(EGG::Archive* pArchive) {
    const char* anmNames[11] = {
        "tnkfade_in_00.brlan",      "tnkfade_out_00.brlan",      "tnkfade_inLife_00.brlan",
        "tnkfade_outLife_00.brlan", "tnkfade_inBonus_00.brlan",  "tnkfade_outBonus_00.brlan",
        "tnkfade_addLife_00.brlan", "tnkfade_addLife_01.brlan",  "tnkfade_inMission_00.brlan",
        "tnkfade_inEnemy_00.brlan", "tnkfade_loop_00.brlan",
    };
    AttachArcFile(pArchive, "tank.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "tnkfade.brlyt");
    s32 i = 0;
    do {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        if (i == 10) {
            mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_Loop);
        } else {
            mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
        }
    } while (++i < 11);
}

void RPTnkFade::Reset() {
    mState = 0;
    mTimer = 0;
    mLives = 0;
    RPSysStringUtility::GetStringFullSizeNumber(mLives, mLivesText, 4, false);
    mMission = 0;
    RPSysStringUtility::GetStringFullSizeNumber(mMission, mMissionText, 4, false);
    mEnemyNum = 0;
    RPSysStringUtility::GetStringFullSizeNumber(mEnemyNum, mEnemyText, 4, false);
    mFast = false;
    mBonus = false;
    mDone = false;

    mpLayout->reset();
    s32 i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 11);

    RPSysMessage* pMsg = RPTnkLayoutManager::sInstance->GetMessage();
    mpLayout->findTextBox("T_bonus_00")->setMessage(pMsg, 0xB57, 0, 0);
    mpLayout->findTextBox("T_bonus_01")->setMessage(pMsg, 0xB57, 0, 0);

    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
    mpLayout->calc();
    mFrame = 0;
}

void RPTnkFade::Calculate() {
    mLives = RPTnkGameStats::sInstance->GetLives(0);
    RPSysStringUtility::GetStringFullSizeNumber(mLives, mLivesText, 4, false);
    mpLayout->findTextBox("T_lifeNo_00")->setText(mLivesText, 0);
    mpLayout->findTextBox("T_lifeNo_01")->setText(mLivesText, 0);

    if (mState == 3) {
        if (--mTimer == 0) {
            if (mBonus) {
                mpLayout->unbindAnmObj(mpAnmObjs[5]);
                mpLayout->bindAnmObjToPane(mpAnmObjs[5], "N_msg_01", true);
                mpAnmObjs[5]->restart();
                mState = 4;
            } else {
                mTimer = 140;
                RPSysMessage* pMsg = RPTnkLayoutManager::sInstance->GetMessage();
                mMission = RPTnkGameManager::sInstance->GetStage() + 1;
                RPSysStringUtility::GetStringFullSizeNumber(mMission, mMissionText, 4, false);
                if (RPTnkGameManager::sInstance->GetStage() == RPTnkGameManager::sInstance->GetLastStage()) {
                    mpLayout->findTextBox("T_mission_00")->setMessage(pMsg, 0xB5D, 1, 0);
                    mpLayout->findTextBox("T_mission_01")->setMessage(pMsg, 0xB5D, 1, 0);
                } else {
                    mpLayout->findTextBox("T_mission_00")->setMessage(pMsg, 0xB5D, 0, 1, mMissionText);
                    mpLayout->findTextBox("T_mission_01")->setMessage(pMsg, 0xB5D, 0, 1, mMissionText);
                }

                pMsg = RPTnkLayoutManager::sInstance->GetMessage();
                mEnemyNum = RPTnkTankManager::sInstance->GetEnemyAliveNum();
                RPSysStringUtility::GetStringFullSizeNumber(mEnemyNum, mEnemyText, 4, false);
                mpLayout->findTextBox("T_enemy_00")->setMessage(pMsg, 0xB5E, 0, 1, mEnemyText);
                mpLayout->findTextBox("T_enemy_01")->setMessage(pMsg, 0xB5E, 0, 1, mEnemyText);

                mpLayout->unbindAnmObj(mpAnmObjs[8]);
                mpLayout->bindAnmObjToPane(mpAnmObjs[8], "N_msg_00", true);
                mpAnmObjs[8]->restart();
                mpLayout->unbindAnmObj(mpAnmObjs[9]);
                mpLayout->bindAnmObjToPane(mpAnmObjs[9], "N_enemy_00", true);
                mpAnmObjs[9]->restart();
                RPTnkGameManager::sInstance->StartBgm();
                mState = 5;
            }
        } else if (mBonus && mTimer == 120) {
            RPTnkGameStats::sInstance->AddLives(0, 1);
            if (RPTnkGameManager::sInstance->GetModeIdx() == 1) {
                RPTnkGameStats::sInstance->AddLives(1, 1);
            }
            mpLayout->unbindAnmObj(mpAnmObjs[6]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[6], "N_life_00", true);
            mpAnmObjs[6]->restart();
            mpLayout->unbindAnmObj(mpAnmObjs[7]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[7], "N_msg_01", true);
            mpAnmObjs[7]->restart();
            RPSndAudioMgr::instance()->startSystemSe(0x22);
        }
    } else if (mState == 6) {
        if (--mTimer == 0) {
            mpLayout->unbindAnmObj(mpAnmObjs[3]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[3], "N_tnk_00", true);
            mpAnmObjs[3]->restart();
            mpLayout->unbindAnmObj(mpAnmObjs[10]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[10], "P_loop_00", true);
            RPTnkLayoutManager::sInstance->GetHud()->SetShowBalloons(true);
            mState = 7;
        }
    }

    UpdateStates();
    mpLayout->calc();
    mFrame++;
}

void RPTnkFade::Draw() {
    mpLayout->draw();
    if (RPTnkGameManager::sInstance->GetModeIdx() == 0) {
        if (mState != 0) {
            RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(2)->GetScreen();
            nw4r::lyt::Pane* pPane = mpLayout->findPane("N_player00_00");
            RPDrawKokeshiIcon(RPMiiIconSet::sInstance->GetIcon(0), pPane, FALSE, pScreen);
        }
    } else if (mState != 0) {
        RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(2)->GetScreen();
        nw4r::lyt::Pane* pPane = mpLayout->findPane("N_player00_01");
        RPDrawKokeshiIcon(RPMiiIconSet::sInstance->GetIcon(0), pPane, FALSE, pScreen);
        pPane = mpLayout->findPane("N_player01_01");
        RPDrawKokeshiIcon(RPMiiIconSet::sInstance->GetIcon(1), pPane, FALSE, pScreen);
    }
}

void RPTnkFade::VF_0x18() {}

void RPTnkFade::Start(bool bonus, bool fast) {
    if (mState == 0) {
        mFast = fast;
        mBonus = bonus;
        mTimer = mBonus ? 150 : 4;

        const char* paneNames[2] = {"N_lifetnk_00", "N_lifetnk_01"};
        if (RPTnkGameManager::sInstance->GetMode() != RPTnkGameManager::MODE_2P) {
            mpLayout->findPane(paneNames[0])->SetVisible(true);
            mpLayout->findPane(paneNames[1])->SetVisible(false);
        } else {
            mpLayout->findPane(paneNames[0])->SetVisible(false);
            mpLayout->findPane(paneNames[1])->SetVisible(true);
            mpLayout->findPane("N_lifeNo_00")->SetVisible(false);
            mpLayout->findPane("T_lifeX_00")->SetVisible(false);
            mpLayout->findPane("T_lifeX_01")->SetVisible(false);
        }

        mpLayout->unbindAllAnmObj();
        mpLayout->unbindAnmObj(mpAnmObjs[0]);
        mpLayout->bindAnmObj(mpAnmObjs[0]);
        mpAnmObjs[0]->restart();
        if (mFast) {
            mpAnmObjs[0]->setRate(15.0f);
        } else {
            mpAnmObjs[0]->setRate(1.0f);
        }

        mpLayout->bindAnmObjToPane(mpAnmObjs[10], "P_loop_00", true);
        mpAnmObjs[10]->restart();
        mDone = false;
        mState = 1;
    }
}

// Aggregate literals are collected when parsed, before deferred function emission.
static inline void RPTnkFadeStringOrder() {
    const char* names[] = {
        "T_mission_00", "T_mission_01", "T_enemy_00", "T_enemy_01", "N_msg_00",
        "N_enemy_00", "N_msg_01", "N_life_00", "N_tnk_00", "P_loop_00", "N_loop_00"
    };
}

// Stand-in for an unused function the original defined last (dead-stripped by the linker): it is
// compiled first, so its int->float constant comes first in .sdata2, before Start's floats.
void RPTnkFade_RestartStandIn(RPSysLytAnmObj* pAnmObj) { pAnmObj->restart(); }
