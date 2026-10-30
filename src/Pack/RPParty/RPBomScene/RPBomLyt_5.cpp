// Pose Mii game end message (RPBomLytEnd, poseManEnd_00).
#include <Pack/RPParty/RPBomScene/RPBomLyt_5.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>

#pragma opt_unroll_loops off

RPBomLytEnd::RPBomLytEnd() {}

RPBomLytEnd::~RPBomLytEnd() {}

void RPBomLytEnd::Load(EGG::Archive* pArchive) {
    const char* anmNames[2] = {"poseManEnd_00_inEnd_00.brlan", "poseManEnd_00_outEnd_00.brlan"};

    AttachArcFile(pArchive, "poseMan.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "poseManEnd_00.brlyt");

    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    }
}

void RPBomLytEnd::Reset() {
    mState = 1;
    mTimer = 180;
    mDone = false;

    mpLayout->reset();

    int i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 2);

    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_endMsg_00")->setMessage(pMessage, 0xAF6, 0, 0);
    mpLayout->findTextBox("T_endMsg_01")->setMessage(pMessage, 0xAF6, 0, 0);

    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
}

void RPBomLytEnd::Calculate() {
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

void RPBomLytEnd::Draw() {
    mpLayout->draw();
}

void RPBomLytEnd::VF_0x18() {}

void RPBomLytEnd::Show(BOOL allClear) {
    if (mState == 0) {
        return;
    }

    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;
    if (allClear) {
        mpLayout->findTextBox("T_endMsg_00")->setMessage(pMessage, 0xB08, 0, 0);
        mpLayout->findTextBox("T_endMsg_01")->setMessage(pMessage, 0xB08, 0, 0);
    } else {
        mpLayout->findTextBox("T_endMsg_00")->setMessage(pMessage, 0xAF6, 0, 0);
        mpLayout->findTextBox("T_endMsg_01")->setMessage(pMessage, 0xAF6, 0, 0);
    }

    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mState = 2;
}
