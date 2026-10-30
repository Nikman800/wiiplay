// Tanks "Mission cleared" message with the destroyed-tank totals (tnkMsg_02.brlyt).
// Unit 0x80271D2C-0x80272624 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkLyt.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPTnkScene/RPTnkGameManager.h>

RPTnkClearMsg::RPTnkClearMsg() {}

// Wii Play: the destructor writes the one-player totals again
RPTnkClearMsg::~RPTnkClearMsg() {
    RPSysStringUtility::GetStringFullSizeNumber(RPTnkGameStats::sInstance->GetKills(0),
                                                mTotalText[0], 4, false);
    mpLayout->findTextBox("T_No00_00")->setText(mTotalText[0], 0);
    mpLayout->findTextBox("T_No01_00")->setText(mTotalText[0], 0);
}

void RPTnkClearMsg::Load(EGG::Archive* pArchive) {
    const char* anmNames[2] = {"tnkMsg_02_inCleare_00.brlan", "tnkMsg_02_inTotal_00.brlan"};
    AttachArcFile(pArchive, "tank.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "tnkMsg_02.brlyt");
    s32 i = 0;
    do {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    } while (++i < 2);
}

void RPTnkClearMsg::Reset() {
    mState = 0;
    mTimer = 120;
    mActive = false;
    s32 j = 0;
    do {
        mTotalText[0][j] = 0;
        mTotalText[1][j] = 0;
    } while (++j < 4);

    mpLayout->reset();
    s32 i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 2);

    RPSysMessage* pMsg = RPTnkLayoutManager::sInstance->GetMessage();
    mpLayout->findTextBox("T_clear_00")->setMessage(pMsg, 0xB55, 0, 0);
    mpLayout->findTextBox("T_clear_01")->setMessage(pMsg, 0xB55, 0, 0);
    mpLayout->findTextBox("T_total00_00")->setMessage(pMsg, 0xB56, 0, 0);
    mpLayout->findTextBox("T_total00_01")->setMessage(pMsg, 0xB56, 0, 0);
    mpLayout->findTextBox("T_player00_00")->setMessage(pMsg, 0xB63, 0, 0);
    mpLayout->findTextBox("T_player00_01")->setMessage(pMsg, 0xB62, 0, 0);
    mpLayout->findTextBox("T_player00_02")->setMessage(pMsg, 0xB62, 1, 0);

    mpLayout->findPane("N_clear_00")->SetVisible(false);
    mpLayout->findPane("N_total_00")->SetVisible(false);
    mpLayout->findPane("N_total_01")->SetVisible(false);
    mpLayout->unbindAllAnmObj();
}

void RPTnkClearMsg::Calculate() {
    if (mState == 1) {
        if (mpAnmObjs[0]->isFinished()) {
            ShowTotal();
        }
    } else if (mState == 2) {
        if (mpAnmObjs[1]->isFinished()) {
            mState = 3;
        }
    }

    if (mState == 3) {
        if (--mTimer <= 0) {
            mActive = true;
        }
    }

    mpLayout->calc();
}

void RPTnkClearMsg::Draw() {
    mpLayout->draw();
}

void RPTnkClearMsg::VF_0x18() {}

void RPTnkClearMsg::Show() {
    if (mState == 0) {
        mTimer = 120;
        mActive = false;
        if (RPTnkGameManager::sInstance->GetModeIdx() == 0) {
            RPSysStringUtility::GetStringFullSizeNumber(RPTnkGameStats::sInstance->GetKills(0),
                                                        mTotalText[0], 4, false);
            mpLayout->findTextBox("T_No00_00")->setText(mTotalText[0], 0);
            mpLayout->findTextBox("T_No01_00")->setText(mTotalText[0], 0);
        } else {
            RPSysStringUtility::GetStringFullSizeNumber(RPTnkGameStats::sInstance->GetKills(0),
                                                        mTotalText[0], 4, false);
            mpLayout->findTextBox("T_No00_01")->setText(mTotalText[0], 0);
            mpLayout->findTextBox("T_No01_01")->setText(mTotalText[0], 0);
            RPSysStringUtility::GetStringFullSizeNumber(RPTnkGameStats::sInstance->GetKills(1),
                                                        mTotalText[1], 4, false);
            mpLayout->findTextBox("T_No00_02")->setText(mTotalText[1], 0);
            mpLayout->findTextBox("T_No01_02")->setText(mTotalText[1], 0);
        }

        mpLayout->findPane("N_clear_00")->SetVisible(true);
        mpLayout->findPane("N_total_00")->SetVisible(false);
        mpLayout->findPane("N_total_01")->SetVisible(false);
        mpLayout->unbindAnmObj(mpAnmObjs[0]);
        mpLayout->unbindAnmObj(mpAnmObjs[1]);
        mpLayout->bindAnmObjToPane(mpAnmObjs[0], "N_clear_00", true);
        mpAnmObjs[0]->restart();
        mState = 1;
    }
}

void RPTnkClearMsg::ShowTotal() {
    if (mState == 1) {
        mpLayout->unbindAnmObj(mpAnmObjs[0]);
        mpLayout->unbindAnmObj(mpAnmObjs[1]);
        if (RPTnkGameManager::sInstance->GetModeIdx() == 0) {
            mpLayout->findPane("N_clear_00")->SetVisible(true);
            mpLayout->findPane("N_total_00")->SetVisible(true);
            mpLayout->findPane("N_total_01")->SetVisible(false);
            mpLayout->bindAnmObjToPane(mpAnmObjs[1], "N_total_00", true);
        } else {
            mpLayout->findPane("N_clear_00")->SetVisible(true);
            mpLayout->findPane("N_total_00")->SetVisible(false);
            mpLayout->findPane("N_total_01")->SetVisible(true);
            mpLayout->bindAnmObjToPane(mpAnmObjs[1], "N_total_01", true);
        }
        mpAnmObjs[1]->restart();
        RPSndAudioMgr::instance()->startSystemSe(0x24);
        mState = 2;
    }
}

void RPTnkClearMsg::Hide() {
    if (mState == 3) {
        mpLayout->findPane("N_clear_00")->SetVisible(false);
        mpLayout->findPane("N_total_00")->SetVisible(false);
        mpLayout->findPane("N_total_01")->SetVisible(false);
        mState = 0;
    }
}
