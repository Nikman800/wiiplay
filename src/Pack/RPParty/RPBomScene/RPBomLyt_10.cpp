// Pose Mii miss message (RPBomLytMiss, poseManMiss_00).
#include <Pack/RPParty/RPBomScene/RPBomLyt_10.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>

#include <nw4r/lyt.h>

RPBomLytMiss::RPBomLytMiss() {}

RPBomLytMiss::~RPBomLytMiss() {}

void RPBomLytMiss::SetMissText(s32 player) {
    RPSysMessage* pMessage = RPBomLayoutMgr::sInstance->mpMessage;

    if (RPBomGameManager::sInstance->mMode == 0) {
        mpLayout->findTextBox("T_miss_00")->setMessage(pMessage, 0xB09, 0, 0);
        mpLayout->findTextBox("T_miss_01")->setMessage(pMessage, 0xB09, 0, 0);
        mpLayout->changeFontColor("T_miss_00", 0);
        mpLayout->findPane("N_1Pbg_00")->SetVisible(true);
        mpLayout->findPane("N_2Pbg_00")->SetVisible(false);
        mpLayout->findPane("N_changebg_00")->SetVisible(false);
    } else {
        switch (player) {
        case 0:
            mpLayout->findTextBox("T_miss_00")->setMessage(pMessage, 0xB09, 1, 0);
            mpLayout->findTextBox("T_miss_01")->setMessage(pMessage, 0xB09, 1, 0);
            mpLayout->changeFontColor("T_miss_00", 0);
            mpLayout->findPane("N_1Pbg_00")->SetVisible(true);
            mpLayout->findPane("N_2Pbg_00")->SetVisible(false);
            mpLayout->findPane("N_changebg_00")->SetVisible(false);
            break;
        case 1:
            mpLayout->findTextBox("T_miss_00")->setMessage(pMessage, 0xB09, 2, 0);
            mpLayout->findTextBox("T_miss_01")->setMessage(pMessage, 0xB09, 2, 0);
            mpLayout->changeFontColor("T_miss_00", 1);
            mpLayout->findPane("N_1Pbg_00")->SetVisible(false);
            mpLayout->findPane("N_2Pbg_00")->SetVisible(true);
            mpLayout->findPane("N_changebg_00")->SetVisible(false);
            break;
        case -1:
            mpLayout->findTextBox("T_miss_00")->setMessage(pMessage, 0xB09, 0, 0);
            mpLayout->findTextBox("T_miss_01")->setMessage(pMessage, 0xB09, 0, 0);
            mpLayout->changeFontColor("T_miss_00", 2);
            mpLayout->findPane("N_1Pbg_00")->SetVisible(true);
            mpLayout->findPane("N_2Pbg_00")->SetVisible(true);
            mpLayout->findPane("N_changebg_00")->SetVisible(true);
            break;
        }
    }
}

void RPBomLytMiss::Load(EGG::Archive* pArchive) {
    const char* anmNames[1] = {"poseManMiss_00_inOutAll_00.brlan"};

    AttachArcFile(pArchive, "poseMan.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "poseManMiss_00.brlyt");

    int i = 0;
    do {
        (&mpAnmObj)[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        (&mpAnmObj)[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    } while (++i < 1);
}

void RPBomLytMiss::Reset() {
    mShown = false;

    mpLayout->reset();

    int i = 0;
    do {
        (&mpAnmObj)[i]->resetFrame();
        (&mpAnmObj)[i]->stop();
        (&mpAnmObj)[i]->setFrame(0.0f);
    } while (++i < 1);

    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObj);
    mpLayout->bindAnmObj(mpAnmObj);
    mpAnmObj->restart();
    mpAnmObj->stop();
    mpAnmObj->setFrame(0.0f);
}

void RPBomLytMiss::Calculate() {
    if (!mShown) {
        return;
    }

    if (mShown && mpAnmObj->isFinished()) {
        mShown = false;
    }

    mpLayout->calc();
}

void RPBomLytMiss::Draw() {
    if (mShown) {
        mpLayout->draw();
    }
}

void RPBomLytMiss::VF_0x18() {}

void RPBomLytMiss::Show(s32 player) {
    if (!mShown) {
        mShown = true;
        SetMissText(player);
        mpLayout->unbindAnmObj(mpAnmObj);
        mpLayout->bindAnmObj(mpAnmObj);
        mpAnmObj->restart();
    }
}
