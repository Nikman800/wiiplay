#include <Pack/RPParty/RPWlyScene/RPWlyLyt_6.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLevelTable.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>

void RPWlyMessageLyt::Init() {
    mState = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpInAnmObj = NULL;
    mpWaitAnmObj = NULL;
    mpOutAnmObj = NULL;
}

void RPWlyMessageLyt::Create() {
    mpResAccessor = RPCreateLocalLytResAccessor("wally.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "wallyMsg_00.brlyt");
    mpInAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor, "wallyMsg_00_inMsg_00.brlan");
    mpWaitAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor, "wallyMsg_00_waitMsg_00.brlan");
    mpOutAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor, "wallyMsg_00_outMsg_00.brlan");

    RPSysMessage* message = RPWlyGameMgr::sInstance->mpMessage;
    RPWlyLevelData* level = RPWlyLevelTable::sInstance->GetCurrentLevel();
    if (RPWlyGameMgr::sInstance->mStageNo == 1 || level->mQuestionType == 0 ||
        level->mQuestionType == 8) {
        mpLayout->findTextBox("T_msgText_00")->setMessage(message, 0x961, 1, 0);
        mpLayout->findTextBox("T_msgText_01")->setMessage(message, 0x961, 1, 0);
    } else if (RPWlyLevelTable::sInstance->HasAltMessage()) {
        mpLayout->findTextBox("T_msgText_00")->setMessage(message, 0x961, 0, 0);
        mpLayout->findTextBox("T_msgText_01")->setMessage(message, 0x961, 0, 0);
    } else {
        mpLayout->findPane("RootPane")->SetVisible(false);
    }
}

void RPWlyMessageLyt::Reset() {
    mState = 0;
    mTimer = 0;
    for (int i = 0; i < 3; i++) {
        (&mpInAnmObj)[i]->resetFrame();
        (&mpInAnmObj)[i]->getFrameCtrl()->reinit();
    }
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpInAnmObj);
}

void RPWlyMessageLyt::Calculate() {
    switch (mState) {
    case 0:
        mState = 1;
        break;
    case 2:
        if (mpInAnmObj->isFinished()) {
            mState = 3;
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpWaitAnmObj);
            mpWaitAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
            mpWaitAnmObj->restart();
        }
        break;
    case 3:
        if (mpWaitAnmObj->isFinished()) {
            if (mTimer++ == 5 || RPWlyGameMgr::sInstance->mState == 4) {
                mState = 4;
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObj(mpOutAnmObj);
                mpOutAnmObj->restart();
            } else {
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObj(mpWaitAnmObj);
                mpWaitAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
                mpWaitAnmObj->restart();
            }
        }
        break;
    case 4:
        if (mpOutAnmObj->isFinished()) {
            mState = 1;
        }
        break;
    }
    mpLayout->calc();
}

void RPWlyMessageLyt::Destroy() {}

void RPWlyMessageLyt::Draw() {
    mpLayout->draw();
}

void RPWlyMessageLyt::Show() {
    mState = 2;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpInAnmObj);
    mpInAnmObj->restart();
}
