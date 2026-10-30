// Pose Mii title and theme banner (RPBomLytStart, poseManStart_00).
#include <Pack/RPParty/RPBomScene/RPBomLyt_2.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>

RPBomLytStart::RPBomLytStart() {}

RPBomLytStart::~RPBomLytStart() {}

void RPBomLytStart::UpdateStates() {
    if (mState == 1) {
        if (mpAnmObjs[0]->isFinished()) {
            mpLayout->unbindAnmObj(mpAnmObjs[0]);
            mpLayout->unbindAnmObj(mpAnmObjs[1]);
            mpLayout->bindAnmObj(mpAnmObjs[1]);
            mpAnmObjs[1]->restart();
            mState = 2;
        }
    } else if (mState == 3) {
        if (mpAnmObjs[2]->isFinished()) {
            RPBomLayoutMgr::sInstance->mpLevel->Show();
            mThemeShown = true;
            mState = 4;
        }
    } else if (mState == 5) {
        if (mpAnmObjs[2]->isFinished()) {
            mState = 0;
        }
    }
}

void RPBomLytStart::Load(EGG::Archive* pArchive) {
    const char* anmNames[4] = {"poseManStart_00_inAll_00.brlan",
                               "poseManStart_00_upDownMsg_00.brlan",
                               "poseManStart_00_outAll_00.brlan",
                               "poseManStart_00_signal_00.brlan"};

    AttachArcFile(pArchive, "poseMan.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "poseManStart_00.brlyt");

    int i = 0;
    do {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        if (i == 1 || i == 3) {
            mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_Loop);
        } else {
            mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
        }
    } while (++i < 4);
}

void RPBomLytStart::Reset() {
    mState = 0;
    mSignalOn = false;
    mThemeShown = false;

    mpLayout->reset();

    int i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 4);

    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_theme_00")->setMessage(pMessage, 0xAF0, 0, 0);
    mpLayout->findTextBox("T_theme_01")->setMessage(pMessage, 0xAF0, 0, 0);
    mpLayout->findTextBox("T_msg_00")->setMessage(pMessage, 0xAF1, 0, 0);
    mpLayout->findTextBox("T_msg_01")->setMessage(pMessage, 0xAF1, 0, 0);

    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
}

void RPBomLytStart::Calculate() {
    if (mState == 0) {
        return;
    }

    UpdateStates();

    if (mState == 4 && RPBomGameManager::sInstance->mState == 3) {
        f32 z = RPBomPanelPool::sInstance->GetMaxFallingZ();
        if (mSignalOn && z < 0.0f) {
            mpAnmObjs[3]->resetFrame();
            mpAnmObjs[3]->restart();
            mpAnmObjs[3]->stop();
            mpAnmObjs[3]->setFrame(0.0f);
            mSignalOn = false;
        } else if (!mSignalOn && z > 0.0f) {
            mpAnmObjs[3]->restart();
            mSignalOn = true;
        }
    }

    mpLayout->calc();
}

void RPBomLytStart::Draw() {
    if (mState != 0) {
        mpLayout->draw();
    }
}

void RPBomLytStart::VF_0x18() {}

void RPBomLytStart::ShowTitle() {
    if (mState == 2) {
        return;
    }

    mThemeShown = false;
    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_theme_00")->setMessage(pMessage, 0xAF0, 0, 0);
    mpLayout->findTextBox("T_theme_01")->setMessage(pMessage, 0xAF0, 0, 0);
    mpLayout->findTextBox("T_msg_00")->setMessage(pMessage, 0xAF1, 0, 0);
    mpLayout->findTextBox("T_msg_01")->setMessage(pMessage, 0xAF1, 0, 0);

    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mState = 1;
}

void RPBomLytStart::ShowTheme() {
    if (mState == 4) {
        return;
    }

    mSignalOn = false;
    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_msg_00")->setMessage(pMessage, 0xAF1, 3, 0);
    mpLayout->findTextBox("T_msg_01")->setMessage(pMessage, 0xAF1, 3, 0);

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObjToPane(mpAnmObjs[2], "N_theme_00", true);
    mpAnmObjs[2]->restart();
    mpLayout->bindAnmObjToPane(mpAnmObjs[3], "N_msg_00", true);
    mState = 3;
}

void RPBomLytStart::PlayThemeOut() {
    if (mState == 4) {
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObjToPane(mpAnmObjs[2], "N_msg_00", true);
        mpAnmObjs[2]->restart();
        mState = 5;
    }
}
