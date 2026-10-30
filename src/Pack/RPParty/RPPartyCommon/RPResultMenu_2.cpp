// Two-button play-again / quit menu (RPChoiceLyt, ptGOMenu_00.brlyt).
// Unit 0x801EA4A8-0x801EB2CC. Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPResultMenu_2.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysController.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPSystem/RPSysPlayerMgr.h>

RPChoiceLyt::RPChoiceLyt() {
    for (int i = 0; i < BUTTON_NUM; i++) {
        mpButtonPanes[i] = NULL;
    }
}

void RPChoiceLyt::Load(EGG::Archive* pArchive) {
    RPLytObjBase::AttachArcFile(pArchive, "ptVsResult.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "ptGOMenu_00.brlyt");

    const char* anmNames[2] = {
        "ptGOMenu_00_in_00.brlan",
        "ptGOMenu_00_out_00.brlan",
    };
    const char* buttonAnmNames[2] = {
        "ptGOMenu_00_active_00.brlan",
        "ptGOMenu_00_push_00.brlan",
    };
    for (int i = 0; i < 2; i++) {
        (&mpInAnmObj)[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }
    for (int i = 0; i < BUTTON_NUM; i++) {
        for (int j = 0; j < 2; j++) {
            mpButtonAnmObjs[i][j] =
                mpLayout->createAnmObj(NULL, mpResAccessor, buttonAnmNames[j]);
        }
    }

    RPSysMessage* pMessage = RPCommon::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_choicesF_00")
        ->setMessage(pMessage, 0x1F55, 0, 0);
    mpLayout->findTextBox("T_choicesS_00")
        ->setMessage(pMessage, 0x1F55, 1, 0);

    mpChoicesPane = mpLayout->findPane("N_choices_00");
    const char* choiceNames[2] = {"N_choicesF_00", "N_choicesS_00"};
    for (int i = 0; i < BUTTON_NUM; i++) {
        mpChoicePanes[i] = mpLayout->findPane(choiceNames[i]);
    }

    const char* buttonNames[2] = {"B_choicesF_00", "B_choicesS_00"};
    for (int i = 0; i < BUTTON_NUM; i++) {
        mpButtonPanes[i] = mpLayout->findPane(buttonNames[i]);
        mpBoundings[i] = RPSysLayout::castToBounding(mpButtonPanes[i]);
    }
}

void RPChoiceLyt::Reset() {
    mState = 0;
    mNextState = 0;
    mIsChanging = false;
    mStateTimer = 0;
    mCursor = -1;

    mpInAnmObj->resetFrame();
    mpInAnmObj->stop();
    mpInAnmObj->setFrame(0.0f);
    mpOutAnmObj->resetFrame();
    mpOutAnmObj->stop();
    mpOutAnmObj->setFrame(0.0f);
    for (int i = 0; i < BUTTON_NUM; i++) {
        for (int j = 0; j < 2; j++) {
            mpButtonAnmObjs[i][j]->resetFrame();
            mpButtonAnmObjs[i][j]->stop();
            mpButtonAnmObjs[i][j]->setFrame(0.0f);
        }
    }

    mpChoicesPane->UnbindAllAnimation(true);
    int i = 0;
    do {
        mpChoicePanes[i]->UnbindAllAnimation(true);
    } while (++i < BUTTON_NUM);
}

void RPChoiceLyt::Calculate() {
    typedef void (RPChoiceLyt::*StateFunc)();
    struct StateFuncs {
        StateFunc mEnter;
        StateFunc mCalc;
    };
    StateFuncs funcs[6] = {
        {NULL, NULL},
        {&RPChoiceLyt::EnterIn, &RPChoiceLyt::CalcIn},
        {NULL, NULL},
        {&RPChoiceLyt::EnterPush, &RPChoiceLyt::CalcPush},
        {&RPChoiceLyt::EnterOut, &RPChoiceLyt::CalcOut},
        {NULL, NULL},
    };

    if (mIsChanging) {
        mState = mNextState;
        mIsChanging = false;
        mStateTimer = 0;

        StateFunc enter = funcs[mState].mEnter;
        if (enter != NULL) {
            (this->*enter)();
        }
    }

    StateFunc calc = funcs[mState].mCalc;
    if (calc != NULL) {
        (this->*calc)();
    }

    if (mState != 0) {
        mpLayout->calc();
    }
}

void RPChoiceLyt::EnterIn() {
    mpInAnmObj->restart();
    int i = 0;
    do {
        mpChoicePanes[i]->UnbindAllAnimation(true);
        RPSysLayout::bindAnmObjToPane(
            mpButtonAnmObjs[i][0], mpChoicePanes[i], true);
        mpButtonAnmObjs[i][0]->play(0);
        mpButtonAnmObjs[i][0]->setRate(0.0f);
    } while (++i < BUTTON_NUM);
}

void RPChoiceLyt::CalcIn() {
    int alpha = static_cast<int>(
        mpInAnmObj->getFrameCtrl()->getFrame() * 255.0f /
        static_cast<f32>(mpInAnmObj->getFrameCtrl()->getEndFrame()));
    if (alpha < 0) {
        alpha = 0;
    } else if (alpha > 255) {
        alpha = 255;
    }
    mpChoicesPane->SetAlpha(static_cast<u8>(alpha));

    if (mpInAnmObj->isFinished()) {
        mNextState = 2;
        mIsChanging = true;
    }
}

void RPChoiceLyt::EnterPush() {
    RPSysLayout::bindAnmObjToPane(mpButtonAnmObjs[mCursor][1], mpChoicePanes[mCursor], true);
    mpButtonAnmObjs[mCursor][1]->restart();

    u32 speakerFlag = RPSysPlayerMgr::instance()->getSpeakerFlag(-1);
    if (mCursor == 0) {
        RPSndAudioMgr::instance()->startSystemSeWithRemote(
            "RP_COM_GAME_PARTY_SE_RESULT_KEY_A_01_TVSP", speakerFlag);
        RPSndAudioMgr::instance()->startSystemSeWithRemote(
            "RP_COM_GAME_PARTY_SE_RESULT_KEY_A_01_CTSP", speakerFlag);
    } else {
        RPSndAudioMgr::instance()->startSystemSeWithRemote(
            "RP_COM_GAME_PARTY_SE_RESULT_KEY_A_02_TVSP", speakerFlag);
        RPSndAudioMgr::instance()->startSystemSeWithRemote(
            "RP_COM_GAME_PARTY_SE_RESULT_KEY_A_02_CTSP", speakerFlag);
    }
}

void RPChoiceLyt::CalcPush() {
    if (mpButtonAnmObjs[mCursor][1]->isFinished()) {
        mNextState = 5;
        mIsChanging = true;
    }
}

void RPChoiceLyt::EnterOut() {
    RPSysLytAnmObj* pAnmObj = mpButtonAnmObjs[mCursor][1];
    RPSysLayout::unbindAnmObjToPane(pAnmObj, mpChoicePanes[mCursor], true);
    mpOutAnmObj->restart();
}

void RPChoiceLyt::CalcOut() {
    int alpha = static_cast<int>(
        255.0f -
        mpOutAnmObj->getFrameCtrl()->getFrame() * 255.0f /
            static_cast<f32>(mpOutAnmObj->getFrameCtrl()->getEndFrame()));
    if (alpha < 0) {
        alpha = 0;
    } else if (alpha > 255) {
        alpha = 255;
    }
    mpChoicesPane->SetAlpha(static_cast<u8>(alpha));

    if (mpOutAnmObj->isFinished()) {
        mNextState = 5;
        mIsChanging = true;
    }
}

void RPChoiceLyt::DrawLayer1A() {}

void RPChoiceLyt::DrawLayer1C() {
    if (mState == 0) {
        return;
    }
    mpLayout->draw();
}

void RPChoiceLyt::Open() {
    if (mState != 0 || mIsChanging) {
        return;
    }
    mNextState = 1;
    mIsChanging = true;
}

void RPChoiceLyt::Decide() {
    if (mState != 2) {
        return;
    }
    if (GetState() != 2 || mIsChanging) {
        return;
    }
    mNextState = 3;
    mIsChanging = true;
}

bool RPChoiceLyt::SetCursor(s32 button) {
    if ((mState == 0 && mIsChanging) || mState == 1 ||
        (mState == 2 && !mIsChanging)) {
        mPrevCursor = mCursor;
        mCursor = button;

        for (int i = 0; i < BUTTON_NUM; i++) {
            if (i == mCursor && i != mPrevCursor) {
                mpChoicePanes[i]->UnbindAllAnimation(true);
                RPSysLayout::bindAnmObjToPane(
                    mpButtonAnmObjs[i][0], mpChoicePanes[i], true);
                mpButtonAnmObjs[i][0]->play(
                    static_cast<s16>(mpButtonAnmObjs[i][0]
                                         ->getFrameCtrl()
                                         ->getFrame()));
                mpButtonAnmObjs[i][0]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
                mpButtonAnmObjs[i][0]->setRate(1.0f);
            }
            if (i != mCursor && i == mPrevCursor) {
                mpChoicePanes[i]->UnbindAllAnimation(true);
                RPSysLayout::bindAnmObjToPane(
                    mpButtonAnmObjs[i][0], mpChoicePanes[i], true);
                mpButtonAnmObjs[i][0]->play(
                    static_cast<s16>(mpButtonAnmObjs[i][0]
                                         ->getFrameCtrl()
                                         ->getFrame()));
                mpButtonAnmObjs[i][0]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
                mpButtonAnmObjs[i][0]->setRate(-1.0f);
            }
        }
        return true;
    }
    return false;
}

s32 RPChoiceLyt::HitTest(const nw4r::math::VEC2& rPos) const {
    for (int i = 0; i < BUTTON_NUM; i++) {
        if (mpLayout->isInsideBounding(mpBoundings[i], static_cast<const EGG::Vector2f&>(rPos))) {
            return i;
        }
    }
    return -1;
}

nw4r::lyt::Pane* RPChoiceLyt::GetButtonPane(s32 button) const {
    return mpButtonPanes[button];
}

void RPChoiceLyt::PlayCursorSe(bool rumble) {
    RPSndAudioMgr::instance()->startSystemSe(
        "RP_COM_GAME_PARTY_SE_Cursor_01");
    if (rumble) {
        RPSysCoreController::getActiveController()
            ->rumble(RPSysCoreController::ERumblePattern_Cursor);
    }
}
