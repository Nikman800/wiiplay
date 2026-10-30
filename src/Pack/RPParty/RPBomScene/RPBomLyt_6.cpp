// Pose Mii level number (RPBomLytLevel, poseManLevel_00).
#include <Pack/RPParty/RPBomScene/RPBomLyt_6.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysStringUtility.h>

#include <nw4r/lyt.h>

RPBomLytLevel::RPBomLytLevel() {}

RPBomLytLevel::~RPBomLytLevel() {}

void RPBomLytLevel::Load(EGG::Archive* pArchive) {
    const char* anmNames[2] = {"poseManLevel_00_in_00.brlan", "poseManLevel_00_out_00.brlan"};

    AttachArcFile(pArchive, "poseMan.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "poseManLevel_00.brlyt");

    int i = 0;
    do {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    } while (++i < 2);
}

void RPBomLytLevel::Reset() {
    mState = 1;
    mTimer = 110;
    mDone = false;

    mpLayout->reset();

    int i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 2);

    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    RPSysStringUtility::GetStringHalfSizeNumber(1, mLevelText, 4, false);
    mpLayout->findTextBox("T_levelMsg_00")->setMessage(pMessage, 0xAF7, 0, 1, mLevelText);
    mpLayout->findTextBox("T_levelMsg_01")->setMessage(pMessage, 0xAF7, 0, 1, mLevelText);
    mpLayout->findTextBox("T_levelMsg_02")->setMessage(pMessage, 0xAF7, 1, 0);
    mpLayout->findTextBox("T_levelMsg_03")->setMessage(pMessage, 0xAF7, 1, 0);

    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
}

void RPBomLytLevel::Calculate() {
    if (mState == 1) {
        return;
    }

    if (mState == 2) {
        if (mpAnmObjs[0]->isFinished()) {
            mState = 0;
        }
    } else if (mState == 3) {
        if (mpAnmObjs[1]->isFinished()) {
            mState = 1;
            mDone = true;
        }
    }

    if (mState == 2 || mState == 0) {
        if (--mTimer <= 0 && mState != 1) {
            mpLayout->unbindAnmObj(mpAnmObjs[1]);
            mpLayout->bindAnmObj(mpAnmObjs[1]);
            mpAnmObjs[1]->restart();
            mState = 3;
        }
    }

    mpLayout->calc();
}

void RPBomLytLevel::Draw() {
    if (mState != 1) {
        mpLayout->draw();
    }
}

void RPBomLytLevel::VF_0x18() {}

void RPBomLytLevel::Show() {
    if (mState != 1) {
        return;
    }

    mTimer = 110;
    mDone = false;

    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    if (RPBomGameManager::sInstance->mLevel == RPBomGameManager::sInstance->mLevelMax) {
        mpLayout->findPane("N_levelMsg_00")->SetVisible(false);
        mpLayout->findPane("N_levelMsg_01")->SetVisible(true);
    } else {
        RPSysStringUtility::GetStringHalfSizeNumber(RPBomGameManager::sInstance->mLevel + 1, mLevelText, 4, false);
        mpLayout->findTextBox("T_levelMsg_00")->setMessage(pMessage, 0xAF7, 0, 1, mLevelText);
        mpLayout->findTextBox("T_levelMsg_01")->setMessage(pMessage, 0xAF7, 0, 1, mLevelText);
        mpLayout->findPane("N_levelMsg_00")->SetVisible(true);
        mpLayout->findPane("N_levelMsg_01")->SetVisible(false);
    }

    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mState = 2;
}
