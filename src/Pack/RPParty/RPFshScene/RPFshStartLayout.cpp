// Fishing start sequence (RPFshStartLayout): camera intro, waiting for every player to cast,
// the 3-2-1 countdown (fishingStart_00.brlyt), then the timer starts.
// Unit 0x8021172C-0x802126DC (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshStartLayout.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPFshScene/RPFshBonusFish.h>
#include <Pack/RPParty/RPFshScene/RPFshCamera.h>
#include <Pack/RPParty/RPFshScene/RPFshFishMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshMsg.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayer.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayerMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshScoreLayout.h>
#include <Pack/RPParty/RPFshScene/RPFshSound.h>
#include <Pack/RPParty/RPFshScene/RPFshTimer.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPPauseText.h>
#include <Pack/RPSystem.h>

// Wii Play: the original names the enter dispatcher CallExit and vice versa.
template <> void RPStateMachine<RPFshStartLayout>::Calculate();
template <> void RPStateMachine<RPFshStartLayout>::CallExit();
template <> void RPStateMachine<RPFshStartLayout>::CallEnter();

f32 RPFshStartLayout::sReadyDistFar = 2.7f;
f32 RPFshStartLayout::sReadyDistNear = 2.5f;

RPFshStartLayout* RPFshStartLayout::sInstance;

void RPFshStartLayout::CreateInstance() {
    sInstance = new RPFshStartLayout();
}

void RPFshStartLayout::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshStartLayout::~RPFshStartLayout() {}

RPFshStartLayout::RPFshStartLayout() {
    mpStateMachine = new RPStateMachine<RPFshStartLayout>(this, STATE_NUM);

    RPFshStateFuncs<RPFshStartLayout> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPFshStartLayout::IntroEnter, &RPFshStartLayout::IntroCalc},
        {&RPFshStartLayout::WaitReadyEnter, &RPFshStartLayout::WaitReadyCalc},
        {&RPFshStartLayout::CountEnter, &RPFshStartLayout::CountCalc},
        {&RPFshStartLayout::PlayEnter, NULL},
    };
    for (int i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    mpResAccessor = RPSysLytResAccessor::create(NULL);
    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));

    mpCountSnd = new RPSndRangeTable();
    mpCountSnd->setInfo(RPFshGetStartSoundTable());
}

// The compiler names the original lbl_8031FEE8 string pool @stringBase0;
// the relocation names differ in status, but the linked bytes are identical.
void RPFshStartLayout::Create() {
    RPLytObjBase::AttachArcFile(RPSysResourceManager::LoadGameLocalArchive(), "fishing.arc",
                                mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "fishingStart_00.brlyt");

    const char* anmNames[4] = {"fishingStart_00_count_00.brlan", "fishingStart_00_count_01.brlan",
                               "fishingStart_00_count_02.brlan", "fishingStart_00_star_00.brlan"};
    for (int i = 0; i < 4; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    for (int i = 0; i < 3; i++) {
        const char* textNames[3][2] = {{"T_countdown1_00", "T_countdown1_01"},
                                       {"T_countdown2_00", "T_countdown2_01"},
                                       {"T_countdown3_00", "T_countdown3_01"}};
        wchar_t text[3];
        RPSysStringUtility::GetStringFullSizeNumber(i + 1, text, 3, false);
        for (int j = 0; j < 2; j++) {
            mpLayout->findTextBox(textNames[i][j]);
            mpLayout->findTextBox(textNames[i][j])->setText(text, 0);
        }
    }

    mpLayout->findTextBox("T_startText_00")->setMessage(pMessage, 0x7D4, 0, 0);
    mpLayout->findTextBox("T_startText_01")->setMessage(pMessage, 0x7D4, 0, 0);
}

void RPFshStartLayout::Reset() {
    mpStateMachine->Reset(STATE_INTRO);
    mTimeLimit = RPFshTimer::GetTimeLimitSec() * 101 - 1;
    mTimerWindowStart = RPFshTimer::GetTimeLimitSec() * 71 - 1;
    mFarMsgShown = false;

    mpLayout->unbindAllAnmObj();
    for (int i = 0; i < 4; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }
}

void RPFshStartLayout::Exit() {}

void RPFshStartLayout::Calculate() {
    mpStateMachine->Calculate();

    if (mpStateMachine->GetState() != STATE_NONE) {
        mpLayout->calc();
    }
}

void RPFshStartLayout::IntroEnter() {
    RPSndAudioMgr::instance()->startBgm(0x67U);
    RPFshFishMgr::GetInstance()->SpawnInitialFish();
    RPFshCameraMgr::GetInstance()->GetCamera()->StartIntroMove();
}

void RPFshStartLayout::IntroCalc() {
    if (!RPFshCameraMgr::GetInstance()->GetCamera()->IsIntroMoving()) {
        mpStateMachine->ChangeState(STATE_WAIT_READY);
        RPFshCameraMgr::GetInstance()->GetCamera()->SetDefault();
        RPFshPlayerMgr::GetInstance()->InitPlayers();
        RPFshStartAmbientSound();
    }
}

void RPFshStartLayout::WaitReadyEnter() {
    mReady = false;
}

void RPFshStartLayout::WaitReadyCalc() {
    bool allCast = true;
    bool anyCast = false;
    RPFshPlayerMgr* pPlayerMgr = RPFshPlayerMgr::GetInstance();

    for (int i = 0; i < pPlayerMgr->GetPlayerNum(); i++) {
        s32 state = pPlayerMgr->GetPlayer(i)->GetState();
        bool cast = state != RPFshPlayer::STATE_NONE && state != RPFshPlayer::STATE_AIM;
        if (cast) {
            anyCast = true;
        }
        if (!cast) {
            allCast = false;
        }
    }

    if (allCast) {
        RPFshMsg00::GetInstance()->Close();
        RPFshMsg01::GetInstance()->Close();
        mReady = true;
    }

    if (mReady && RPFshMsg00::GetInstance()->IsClosed() && RPFshMsg01::GetInstance()->IsClosed()) {
        mpStateMachine->ChangeState(STATE_COUNT);
    }

    if (!anyCast) {
        mpStateMachine->ResetTimer();
    }

    if (!mReady) {
        if (!mFarMsgShown) {
            bool far = false;
            for (int i = 0; i < pPlayerMgr->GetPlayerNum(); i++) {
                if (pPlayerMgr->GetPlayer(i)->GetRemoteDist() > sReadyDistFar) {
                    far = true;
                }
            }

            if (far) {
                RPFshMsg00::GetInstance()->Close();
                RPFshMsg01::GetInstance()->Open();
                mFarMsgShown = true;
            } else {
                RPFshMsg00::GetInstance()->Open();
                RPFshMsg01::GetInstance()->Close();
            }
        } else {
            bool near = true;
            for (int i = 0; i < pPlayerMgr->GetPlayerNum(); i++) {
                if (pPlayerMgr->GetPlayer(i)->GetRemoteDist() > sReadyDistNear) {
                    near = false;
                }
            }

            if (near) {
                RPFshMsg00::GetInstance()->Open();
                RPFshMsg01::GetInstance()->Close();
                mFarMsgShown = false;
            } else {
                RPFshMsg00::GetInstance()->Close();
                RPFshMsg01::GetInstance()->Open();
            }
        }
    }
}

void RPFshStartLayout::CountEnter() {
    mCount = 0;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[mCount]);
    mpAnmObjs[mCount]->restart();
    mpCountSnd->startSound(3 - mCount);
}

void RPFshStartLayout::CountCalc() {
    if (mpAnmObjs[mCount]->isFinished()) {
        if (mCount == 3) {
            mpStateMachine->ChangeState(STATE_PLAY);
            RPFshTimer::GetInstance()->Start(mTimeLimit);
            RPFshTimer::GetInstance()->SetWindow(mTimerWindowStart, mTimeLimit);

            for (int i = 0; i < RPFshPlayerMgr::GetInstance()->GetPlayerNum(); i++) {
                RPFshScoreLayout::GetInstance()->RequestShow(i);
            }

            RPFshBonusFish::GetInstance()->Show(0);

            if (RPFshPlayerMgr::GetInstance()->GetPlayerNum() == 1) {
                RPCommon::GetInstance()->GetPauseText()->SetEnabled(0, true);
            }
        } else {
            mCount++;
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[mCount]);
            mpAnmObjs[mCount]->restart();
            mpCountSnd->startSound(3 - mCount);
        }
    }
}

void RPFshStartLayout::PlayEnter() {
    RPSndAudioMgr::instance()->startBgm(0x68U);
}

void RPFshStartLayout::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() == 4 && RPGrpRenderer::GetDrawStep() == 5 &&
        mpStateMachine->GetState() == STATE_COUNT) {
        mpLayout->draw();
    }
}

// Emit the template members in the original address order, after the owner.
#pragma defer_codegen off
#pragma auto_inline off
template <> void RPStateMachine<RPFshStartLayout>::Calculate() {
    if (mIsChanging) {
        if (mCallExit) {
            CallEnter();
        }

        mPrevState = mState;
        mState = mNextState;
        mIsChanging = false;
        mTimer = 0;
        CallExit();
    }

    CallCalc();

    if (mTimer < 0x7FFFFFFF) {
        mTimer++;
    }
}
template void RPStateMachine<RPFshStartLayout>::ChangeState(s32);
template void RPStateMachine<RPFshStartLayout>::Reset(s32);
template void RPStateMachine<RPFshStartLayout>::SetStateFuncs(const StateFunc&, const StateFunc&,
                                                            s32);
template RPStateMachine<RPFshStartLayout>::RPStateMachine(RPFshStartLayout*, s32);
template bool RPStateMachine<RPFshMsg00>::IsStateSettled(s32) const;
template bool RPStateMachine<RPFshMsg01>::IsStateSettled(s32) const;
template <> void RPStateMachine<RPFshStartLayout>::CallExit() {
    if (mpEnterFuncs[mState] != NULL) {
        (mpOwner->*mpEnterFuncs[mState])();
    }
}
template void RPStateMachine<RPFshStartLayout>::CallCalc();
template <> void RPStateMachine<RPFshStartLayout>::CallEnter() {
    if (mpExitFuncs[mState] != NULL) {
        (mpOwner->*mpExitFuncs[mState])();
    }
}
