// Tanks "Mission complete" message after the last stage (tnkMsg_04.brlyt).
// Unit 0x80272A68-0x80272EF4 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkLyt.h>

#include <Pack/RPKernel.h>

RPTnkCompleteMsg::RPTnkCompleteMsg() {}

RPTnkCompleteMsg::~RPTnkCompleteMsg() {}

void RPTnkCompleteMsg::Load(EGG::Archive* pArchive) {
    const char* anmNames[2] = {"tnkMsg_04_in_00.brlan", "tnkMsg_04_out_00.brlan"};
    AttachArcFile(pArchive, "tank.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "tnkMsg_04.brlyt");
    s32 i = 0;
    do {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    } while (++i < 2);
}

void RPTnkCompleteMsg::Reset() {
    mState = 1;
    mTimer = 180;
    mActive = false;
    mpLayout->reset();
    s32 i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 2);

    RPSysMessage* pMsg = RPTnkLayoutManager::sInstance->GetMessage();
    mpLayout->findTextBox("T_complete00_00")->setMessage(pMsg, 0xB58, 0, 0);
    mpLayout->findTextBox("T_complete00_01")->setMessage(pMsg, 0xB58, 0, 0);
    mpLayout->findTextBox("T_complete00_02")->setMessage(pMsg, 0xB58, 0, 0);
    mpLayout->findTextBox("T_complete01_00")->setMessage(pMsg, 0xB58, 0, 0);

    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
}

void RPTnkCompleteMsg::Calculate() {
    if (mState == 2) {
        if (mpAnmObjs[0]->isFinished()) {
            mState = 0;
        }
    } else if (mState == 3) {
        if (mpAnmObjs[1]->isFinished()) {
            mState = 1;
            mActive = true;
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

void RPTnkCompleteMsg::Draw() {
    mpLayout->draw();
}

void RPTnkCompleteMsg::VF_0x18() {}

void RPTnkCompleteMsg::Show() {
    if (mState != 0) {
        mTimer = 180;
        mActive = false;
        mpLayout->unbindAnmObj(mpAnmObjs[0]);
        mpLayout->bindAnmObj(mpAnmObjs[0]);
        mpAnmObjs[0]->restart();
        mState = 2;
    }
}
