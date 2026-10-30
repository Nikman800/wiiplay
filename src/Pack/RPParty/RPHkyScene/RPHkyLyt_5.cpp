// Laser Hockey start countdown layout "3, 2, 1, start" (airHockeyMsg_00.brlyt).
// Unit 0x80223060-0x80223670 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPHkyScene/RPHkyBgmCtrl.h>
#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPHkyScene/RPHkyHud.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>

void RPHkyStartMsgLyt::Create() {
    mState = 0;
    mTimer = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_COUNT_3] = NULL;
    mpAnmObjs[ANM_COUNT_2] = NULL;
    mpAnmObjs[ANM_COUNT_1] = NULL;
    mpAnmObjs[ANM_STAR] = NULL;
}

void RPHkyStartMsgLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("airHockey.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "airHockeyMsg_00.brlyt");
    mpAnmObjs[ANM_COUNT_3] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyMsg_00_count_00.brlan");
    mpAnmObjs[ANM_COUNT_2] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyMsg_00_count_01.brlan");
    mpAnmObjs[ANM_COUNT_1] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyMsg_00_count_02.brlan");
    mpAnmObjs[ANM_STAR] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyMsg_00_star_00.brlan");
    mpLayout->findTextBox("T_startText_00")
        ->setMessage(RPHkyGame::GetInstance()->GetMessage(), 0xA2A, 0, 0);
}

void RPHkyStartMsgLyt::Reset() {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_COUNT_3]);
    mState = 0;
}

void RPHkyStartMsgLyt::Calculate() {
    switch (mState) {
    case 0:
        mTimer = 0;
        mState = 1;
        break;
    case 1:
        break;
    case 2:
        if (mpAnmObjs[ANM_COUNT_3]->isFinished()) {
            mState = 3;
            mTimer = 0;
            RPSndAudioMgr::instance()->startSystemSe(8);
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[ANM_COUNT_2]);
            mpAnmObjs[ANM_COUNT_2]->restart();
        }
        break;
    case 3:
        if (mpAnmObjs[ANM_COUNT_2]->isFinished()) {
            mState = 4;
            mTimer = 0;
            RPSndAudioMgr::instance()->startSystemSe(8);
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[ANM_COUNT_1]);
            mpAnmObjs[ANM_COUNT_1]->restart();
        }
        break;
    case 4:
        if (mpAnmObjs[ANM_COUNT_1]->isFinished()) {
            mState = 5;
            mTimer = 0;
            RPHkyGame::GetInstance()->GetBgmCtrl()->StartGameBgm();
            RPSndAudioMgr::instance()->startSystemSe(9);
            RPHkyHud::GetInstance()->GetBalloonLyt()->RequestOut();
            if (!RPHkyGame::GetInstance()->IsMultiPlayer()) {
                RPHkyHud::GetInstance()->GetTimeLyt()->PlayIn();
            }
            RPHkyHud::GetInstance()->GetScoreLyt()->PlayIn();
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[ANM_STAR]);
            mpAnmObjs[ANM_STAR]->restart();
        }
        break;
    case 5:
        if (mpAnmObjs[ANM_STAR]->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    }

    mpLayout->calc();
}

void RPHkyStartMsgLyt::Exit() {}

void RPHkyStartMsgLyt::Draw() {
    mpLayout->draw();
}

void RPHkyStartMsgLyt::Start() {
    mState = 2;
    RPSndAudioMgr::instance()->startSystemSe(8);
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_COUNT_3]);
    mpAnmObjs[ANM_COUNT_3]->restart();
}

bool RPHkyStartMsgLyt::IsDone() const {
    return mState == 1 && mpAnmObjs[ANM_STAR]->isFinished();
}
