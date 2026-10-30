// Pose Mii "Ready" message (RPBomLytReady, poseManReady_00).
#include <Pack/RPParty/RPBomScene/RPBomLyt_7.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>

RPBomLytReady::RPBomLytReady() {}

RPBomLytReady::~RPBomLytReady() {}

void RPBomLytReady::Load(EGG::Archive* pArchive) {
    const char* anmNames[3] = {"poseManReady_00_in_00.brlan", "poseManReady_00_wait_00.brlan",
                               "poseManReady_00_out_00.brlan"};

    AttachArcFile(pArchive, "poseMan.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "poseManReady_00.brlyt");

    int i = 0;
    do {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        if (i == 1) {
            mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_Loop);
        } else {
            mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
        }
    } while (++i < 3);
}

void RPBomLytReady::Reset() {
    mState = 1;
    mDone = false;

    mpLayout->reset();

    int i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 3);

    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_Ready_00")->setMessage(pMessage, 0xAF8, 0, 0);
    mpLayout->findTextBox("T_Ready_01")->setMessage(pMessage, 0xAF8, 0, 0);

    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
}

void RPBomLytReady::Calculate() {
    if (mState == 1) {
        return;
    }

    if (mState == 2) {
        if (mpAnmObjs[0]->isFinished()) {
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[1]);
            mpAnmObjs[1]->restart();
            mState = 0;
        }
    } else if (mState == 3) {
        if (mpAnmObjs[2]->isFinished()) {
            RPBomLayoutMgr::sInstance->mpLevel->Show();
            mState = 1;
            mDone = true;
        }
    }

    mpLayout->calc();
}

void RPBomLytReady::Draw() {
    if (mState != 1) {
        mpLayout->draw();
    }
}

void RPBomLytReady::VF_0x18() {}

void RPBomLytReady::Show() {
    if (mState == 1) {
        mDone = false;
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpAnmObjs[0]);
        mpAnmObjs[0]->restart();
        mState = 2;
    }
}

void RPBomLytReady::PlayOut() {
    if (mState != 1) {
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpAnmObjs[2]);
        mpAnmObjs[2]->restart();
        mState = 3;
    }
}
