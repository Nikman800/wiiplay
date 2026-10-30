#include <Pack/RPParty/RPWlyScene/RPWlyLyt_9.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>

void RPWlyDisplayEndLyt::Init() {
    mState = 0;
    mTimer = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpInLevelAnmObj = NULL;
    mpOutLevelAnmObj = NULL;
    mpInEndAnmObj = NULL;
    mpOutEndAnmObj = NULL;
}

void RPWlyDisplayEndLyt::Create() {
    mpResAccessor = RPCreateLocalLytResAccessor("wally.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "wallyDisplay_00.brlyt");
    mpInLevelAnmObj =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyDisplay_00_inLevel1_00.brlan");
    mpOutLevelAnmObj =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyDisplay_00_outLevel_00.brlan");
    mpInEndAnmObj =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyDisplay_00_inLevel2End_00.brlan");
    mpOutEndAnmObj =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyDisplay_00_outEnd_00.brlan");

    if (RPWlyGameMgr::sInstance->mMode == 0) {
        mpLayout->findPane("N_R_00")->SetVisible(false);
    }

    RPSysMessage* message = RPWlyGameMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_msgEnd_00")->setMessage(message, 0x967, 0, 0);
    mpLayout->findTextBox("T_msgEnd_01")->setMessage(message, 0x967, 0, 0);
}

void RPWlyDisplayEndLyt::Reset() {
    for (int i = 0; i < 4; i++) {
        (&mpInLevelAnmObj)[i]->resetFrame();
        (&mpInLevelAnmObj)[i]->getFrameCtrl()->reinit();
    }
    mpLayout->unbindAllAnmObj();
    if (RPWlyGameMgr::sInstance->mStageNo == 1) {
        mpLayout->bindAnmObj(mpInLevelAnmObj);
    } else {
        mpLayout->bindAnmObj(mpInEndAnmObj);
    }
    mState = 0;
    mTimer = 0;
    mpLayout->findPane("N_msgStart_00")->SetVisible(false);
    mpLayout->findPane("N_msgEnd_00")->SetVisible(false);
    mpLayout->findPane("N_L_00")->SetVisible(false);
    mpLayout->findPane("N_R_00")->SetVisible(false);
}

void RPWlyDisplayEndLyt::Calculate() {
    switch (mState) {
    case 0:
        mTimer = 0;
        mState = 1;
        break;
    case 1:
        break;
    case 4:
        if (mpInLevelAnmObj->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    case 5:
        if (mpOutLevelAnmObj->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    }
    if (RPWlyGameMgr::sInstance->mState == 4) {
        mpLayout->calc();
    }
}

void RPWlyDisplayEndLyt::Destroy() {}

void RPWlyDisplayEndLyt::Draw() {
    if (RPWlyGameMgr::sInstance->mState == 4) {
        mpLayout->draw();
    }
}

void RPWlyDisplayEndLyt::PlayIn() {
    mpLayout->findPane("N_msgEnd_00")->SetVisible(true);
    mState = 4;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpInEndAnmObj);
    mpInEndAnmObj->restart();
}

void RPWlyDisplayEndLyt::PlayOut() {
    mState = 5;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpOutEndAnmObj);
    mpOutEndAnmObj->restart();
}
