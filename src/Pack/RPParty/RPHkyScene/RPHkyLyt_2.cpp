// Laser Hockey remaining time layout (airHockeyTime_00.brlyt), single player only.
// Unit 0x8022150C-0x80221D6C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>

#include <cwchar>

void RPHkyTimeLyt::SetNumber(s32 seconds) {
    if (seconds != -1) {
        std::swprintf(mNumberText, 4, L"%d", seconds);
        for (int i = 0; i < 4; i++) {
            if (mNumberText[i] == 0) {
                break;
            }

            mNumberText[i] += 0xFEE0;
        }
    } else {
        mNumberText[0] = 0;
    }

    mpLayout->findTextBox("T_number_00")->setText(mNumberText, 0);
}

void RPHkyTimeLyt::Create() {
    mState = 0;
    mTimer = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
    mpAnmObjs[ANM_SWITCH_BG] = NULL;
    mpAnmObjs[ANM_SWITCH_NUM] = NULL;
    mpAnmObjs[ANM_LAST_NUM] = NULL;
}

void RPHkyTimeLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("airHockey.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "airHockeyTime_00.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyTime_00_inAll_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyTime_00_outAll_00.brlan");
    mpAnmObjs[ANM_SWITCH_BG] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyTime_00_switchBG_00.brlan");
    mpAnmObjs[ANM_SWITCH_NUM] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyTime_00_switchNum_00.brlan");
    mpAnmObjs[ANM_LAST_NUM] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyTime_00_lastNum_00.brlan");
}

void RPHkyTimeLyt::Reset() {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mState = 0;
    mTimer = 0;
    mSeconds = -1;

    if (mpLayout != NULL) {
        SetNumber(120);
    }
}

void RPHkyTimeLyt::Calculate() {
    switch (mState) {
    case 0:
        mTimer = 0;
        mState = 1;
        break;
    case 1:
        break;
    case 2:
        if (mpAnmObjs[ANM_IN]->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    case 3:
        if (mpAnmObjs[ANM_OUT]->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    }

    s32 timer = RPHkyGame::GetInstance()->GetTimer();
    s32 seconds;
    if (timer <= -20) {
        seconds = -1;
    } else if (timer <= 0) {
        seconds = 0;
    } else {
        seconds = (timer - 1) / 60 + 1;
    }

    SetNumber(seconds);

    if (mSeconds != seconds) {
        if (seconds >= 1) {
            if (seconds < 6) {
                RPSndAudioMgr::instance()->startSystemSe(0xF);
            } else if (seconds < 11) {
                RPSndAudioMgr::instance()->startSystemSe(0xE);
            } else if (seconds < 21) {
                RPSndAudioMgr::instance()->startSystemSe(0xD);
            } else if (seconds < 31) {
                RPSndAudioMgr::instance()->startSystemSe(0xC);
            }
        }

        mSeconds = seconds;
    }

    switch (timer) {
    case 600:
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObjToPane(mpAnmObjs[ANM_SWITCH_BG], "W_timeBg_00", true);
        mpLayout->bindAnmObjToPane(mpAnmObjs[ANM_SWITCH_BG], "P_timerIcon_00", true);
        mpAnmObjs[ANM_SWITCH_BG]->restart();
        mpLayout->bindAnmObjToPane(mpAnmObjs[ANM_SWITCH_NUM], "T_number_00", true);
        mpAnmObjs[ANM_SWITCH_NUM]->restart();
        break;
    case 0:
    case 60:
    case 120:
    case 180:
    case 240:
    case 300:
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObjToPane(mpAnmObjs[ANM_SWITCH_BG], "W_timeBg_00", true);
        mpLayout->bindAnmObjToPane(mpAnmObjs[ANM_SWITCH_BG], "P_timerIcon_00", true);
        mpAnmObjs[ANM_SWITCH_BG]->restart();
        mpLayout->bindAnmObjToPane(mpAnmObjs[ANM_LAST_NUM], "T_number_00", true);
        mpAnmObjs[ANM_LAST_NUM]->restart();
        break;
    }

    mpLayout->calc();
}

void RPHkyTimeLyt::Exit() {}

void RPHkyTimeLyt::Draw() {
    mpLayout->draw();
}

void RPHkyTimeLyt::PlayIn() {
    mpLayout->findTextBox("T_number_00")->SetVisible(true);
    mState = 2;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpAnmObjs[ANM_IN]->restart();
}

void RPHkyTimeLyt::PlayOut() {
    mpLayout->findTextBox("T_number_00")->SetVisible(false);
    mState = 3;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
    mpAnmObjs[ANM_OUT]->restart();
}
