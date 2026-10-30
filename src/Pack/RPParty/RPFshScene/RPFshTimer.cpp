// Fishing time limit (RPFshTimer): counts the frames down, shows the last seconds
// (fishingTimeUp_00.brlyt) with their sounds.
// Unit 0x8020E1F0-0x8020F1A8 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshTimer.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPFshScene/RPFshSound.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPSystem.h>

RPFshTimer* RPFshTimer::sInstance;

inline RPFshTimer::RPFshTimer() {
    mpStateMachine = new RPStateMachine<RPFshTimer>(this, STATE_NUM);

    RPFshStateFuncs<RPFshTimer> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPFshTimer::IdleEnter, &RPFshTimer::IdleCalc},
        {&RPFshTimer::CountEnter, &RPFshTimer::CountCalc},
        {&RPFshTimer::CountdownEnter, &RPFshTimer::CountdownCalc},
        {&RPFshTimer::LastSecondEnter, &RPFshTimer::LastSecondCalc},
        {&RPFshTimer::EndEnter, &RPFshTimer::EndCalc},
        {NULL, NULL},
    };
    for (int i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    mpResAccessor = RPSysLytResAccessor::create(NULL);
    RPGrpRenderer::GetCurrent()->AppendDrawObject(this);
}

void RPFshTimer::CreateInstance() {
    sInstance = new RPFshTimer();
}

void RPFshTimer::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshTimer::~RPFshTimer() {}

void RPFshTimer::Load() {
    RPLytObjBase::AttachArcFile(RPSysResourceManager::LoadGameLocalArchive(), "fishing.arc",
                                mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "fishingTimeUp_00.brlyt");

    const char* anmNames[2] = {"fishingTimeUp_00_count_00.brlan",
                               "fishingTimeUp_00_count_01.brlan"};
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    for (int i = 0; i < 2; i++) {
        const char* textNames[2] = {"T_number_00", "T_number_01"};
        mpNumberTexts[i] = mpLayout->findTextBox(textNames[i]);
    }
}

void RPFshTimer::Reset() {
    mpStateMachine->Reset(STATE_NONE);
    mInWindow = false;
    mWindowEntered = false;
    mWindowStart = -1;
    mWindowEnd = -1;
}

void RPFshTimer::Calculate() {
    if (mWindowEntered) {
        mWindowEntered = false;
    }

    mpStateMachine->Calculate();

    if (mpStateMachine->GetState() != STATE_NONE && mpStateMachine->GetState() != STATE_DONE) {
        mpLayout->calc();
    }
}

inline void RPFshTimer::CountFrame() {
    if (mFrame < mFrameLimit) {
        mFrame++;

        s32 rest = mTotalFrame - mFrame;
        if (rest % 60 == 59) {
            wchar_t text[10];
            RPSysStringUtility::GetStringFullSizeNumber(rest / 60, text, 10, false);
            for (int i = 0; i < 2; i++) {
                mpNumberTexts[i]->setText(text, 0);
            }
        }
    }
}

inline void RPFshTimer::UpdateWindow() {
    if (mFrame >= mWindowStart && mFrame <= mWindowEnd) {
        mInWindow = true;
        if (mFrame == mWindowStart) {
            mWindowEntered = true;
        }
    } else {
        mInWindow = false;
    }
}

void RPFshTimer::IdleEnter() {
    mpLayout->getRootPane()->SetVisible(false);
}

void RPFshTimer::IdleCalc() {
    mpStateMachine->ChangeState(STATE_COUNT);
}

void RPFshTimer::CountEnter() {
    mpLayout->getRootPane()->SetVisible(false);
}

void RPFshTimer::CountCalc() {
    CountFrame();

    if (mFrame >= mFrameLimit) {
        mpStateMachine->ChangeState(STATE_END);
    } else if (mTotalFrame - mFrame <= 60) {
        mpStateMachine->ChangeState(STATE_LAST_SECOND);
    } else if (mTotalFrame - mFrame <= 360) {
        mpStateMachine->ChangeState(STATE_COUNTDOWN);
    } else {
        UpdateWindow();
    }
}

void RPFshTimer::CountdownEnter() {}

void RPFshTimer::CountdownCalc() {
    CountFrame();

    if (mFrame >= mFrameLimit) {
        mpStateMachine->ChangeState(STATE_END);
    } else if (mTotalFrame - mFrame <= 60) {
        mpStateMachine->ChangeState(STATE_LAST_SECOND);
    } else {
        if ((mTotalFrame - mFrame) % 60 == 59) {
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[0]);
            mpAnmObjs[0]->restart();
            mpLayout->getRootPane()->SetVisible(true);

            switch ((mTotalFrame - mFrame - 59) / 60) {
            case 5:
                RPSndAudioMgr::instance()->startSystemSe(0x32U);
                break;
            case 4:
                RPSndAudioMgr::instance()->startSystemSe(0x33U);
                break;
            case 3:
                RPSndAudioMgr::instance()->startSystemSe(0x34U);
                break;
            case 2:
                RPSndAudioMgr::instance()->startSystemSe(0x35U);
                break;
            case 1:
                RPSndAudioMgr::instance()->startSystemSe(0x36U);
                break;
            }
        }

        UpdateWindow();
    }
}

void RPFshTimer::LastSecondEnter() {
    RPFshStopAmbientSound();
}

void RPFshTimer::LastSecondCalc() {
    CountFrame();

    if (mFrame >= mFrameLimit) {
        mpStateMachine->ChangeState(STATE_END);
    } else {
        if (mTotalFrame - mFrame == 59) {
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[1]);
            mpAnmObjs[1]->restart();
            mpLayout->getRootPane()->SetVisible(true);

            RPSndAudioMgr::instance()->startSystemSe(0x37U);
            RPSndAudioMgr::instance()->stopBgm(RPFshGetBgmFadeFrames(0x68));
        }

        UpdateWindow();
    }
}

void RPFshTimer::EndEnter() {
    mpLayout->getRootPane()->SetVisible(false);
}

void RPFshTimer::EndCalc() {
    if (mFrame >= mFrameLimit) {
        mpStateMachine->ChangeState(STATE_DONE);
    }
}

void RPFshTimer::Start(s32 frames) {
    mpStateMachine->Reset(STATE_NONE);
    mInWindow = false;
    mWindowEntered = false;
    mWindowStart = -1;
    mWindowEnd = -1;
    mFrame = 0;
    mFrameLimit = frames;
    mTotalFrame = frames;

    wchar_t text[10];
    RPSysStringUtility::GetStringFullSizeNumber(frames / 60, text, 10, false);
    for (int i = 0; i < 2; i++) {
        mpNumberTexts[i]->setText(text, 0);
    }

    mpStateMachine->ChangeState(STATE_IDLE);
}

s32 RPFshTimer::GetTimeLimitSec() {
    return 60;
}

void RPFshTimer::UserDraw() {
    if (mpStateMachine->GetState() != STATE_NONE && mpStateMachine->GetState() != STATE_DONE &&
        RPGrpRenderer::GetCurrentViewNo() == 4 && RPGrpRenderer::GetDrawStep() == 5) {
        mpLayout->draw();
    }
}
