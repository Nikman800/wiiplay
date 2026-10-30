#include <Pack/RPParty/RPWlyScene/RPWlyLyt_2.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>

void RPWlyStartCountLyt::Init() {
    mState = 0;
    mTimer = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpCountAnmObjs[0] = NULL;
    mpCountAnmObjs[1] = NULL;
    mpCountAnmObjs[2] = NULL;
    mpStarAnmObj = NULL;
}

void RPWlyStartCountLyt::Create() {
    mpResAccessor = RPCreateLocalLytResAccessor("wally.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "wallyStartCount_00.brlyt");
    mpCountAnmObjs[0] = mpLayout->createAnmObj(
        NULL, mpResAccessor, "wallyStartCount_00_count_00.brlan");
    mpCountAnmObjs[1] = mpLayout->createAnmObj(
        NULL, mpResAccessor, "wallyStartCount_00_count_01.brlan");
    mpCountAnmObjs[2] = mpLayout->createAnmObj(
        NULL, mpResAccessor, "wallyStartCount_00_count_02.brlan");
    mpStarAnmObj = mpLayout->createAnmObj(
        NULL, mpResAccessor, "wallyStartCount_00_star_00.brlan");

    RPSysMessage* message = RPWlyGameMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_startText_00")->setMessage(message, 0x962, 0, 0);
    mpLayout->findTextBox("T_startText_01")->setMessage(message, 0x962, 0, 0);
}

void RPWlyStartCountLyt::Reset() {
    // The three countdown animations and the star animation occupy consecutive slots.
    for (int i = 0; i < 4; ++i) {
        mpCountAnmObjs[i]->resetFrame();
        mpCountAnmObjs[i]->getFrameCtrl()->reinit();
    }
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpCountAnmObjs[0]);
    mState = 0;
}

void RPWlyStartCountLyt::Calculate() {
    switch (mState) {
    case 0:
        mTimer = 0;
        mState = 1;
        break;
    case 1:
        break;
    case 2:
        if (mpCountAnmObjs[0]->isFinished()) {
            mState = 3;
            mTimer = 0;
            RPSndAudioMgr::instance()->startSystemSe(3);
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpCountAnmObjs[1]);
            mpCountAnmObjs[1]->restart();
        }
        break;
    case 3:
        if (mpCountAnmObjs[1]->isFinished()) {
            mState = 4;
            mTimer = 0;
            RPSndAudioMgr::instance()->startSystemSe(3);
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpCountAnmObjs[2]);
            mpCountAnmObjs[2]->restart();
        }
        break;
    case 4:
        if (mpCountAnmObjs[2]->isFinished()) {
            mState = 5;
            mTimer = 0;
            RPSndAudioMgr::instance()->startSystemSe(4);
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpStarAnmObj);
            mpStarAnmObj->restart();
        }
        break;
    case 5:
        if (mpStarAnmObj->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    }

    if (mState != 1) {
        mpLayout->calc();
    }
}

void RPWlyStartCountLyt::Destroy() {}

void RPWlyStartCountLyt::Draw() {
    if (mState != 1) {
        mpLayout->draw();
    }
}
