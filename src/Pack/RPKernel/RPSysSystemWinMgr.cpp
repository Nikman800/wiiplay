#include <Pack/RPKernel/RPSysSystemWinMgr.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysCursorDrawMgr.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytBounding.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPKernel/RPSysSystem.h>
#include <Pack/RPParty/RPPartyCommon/RPCPadCursor.h>
#include <Pack/RPKernel/RPSysController.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <egg/core.h>

#include <cstdarg>

// Unused (dead-stripped) table; its strings stay at the head of the string pool
static const char* Lyt_systemWindowUnusedAnimNames[] = {
    "systemWindow_00_Offon_00.brlan",
    "systemWindow_00_push_00.brlan",
};

const char* RPSysSystemWinMgr::sButtonAnimNames[] = {
    "systemWindow_00_activeButton_00.brlan",
    "systemWindow_00_activeButton_00.brlan",
    "systemWindow_00_pushButton_00.brlan",
};

const char* RPSysSystemWinMgr::sAButtonAnimNames[] = {
    "systemWindow_00_inAButton_00.brlan",
    "systemWindow_00_blinkAButton_00.brlan",
};

const char* RPSysSystemWinMgr::sButtonPaneNames[] = {
    "N_choicesL_00",
    "N_choicesR_00",
};

const char* RPSysSystemWinMgr::sButtonBoundingNames[] = {
    "B_choicesL_00",
    "B_choicesR_00",
};

const char* RPSysSystemWinMgr::sButtonTextNames[] = {
    "T_choicesL_00",
    "T_choicesR_00",
};

static inline void Lyt_playLastFrame(RPSysLytAnmObj* pAnmObj) {
    pAnmObj->play(pAnmObj->getFrameMax() - 1.0f);
}

RPSysSystemWinMgr* RPSysSystemWinMgr::spInstance = NULL;

RPSysSystemWinMgr* RPSysSystemWinMgr::CreateInstance(EGG::Heap* pHeap) {
    if (spInstance == NULL) {
        spInstance = new (pHeap, 4) RPSysSystemWinMgr(pHeap);
        return spInstance;
    }
    return NULL;
}

inline void RPSysSystemWinMgr::resetButtons() {
    for (u8 i = 0; i < 2; i++) {
        mButtons[i].mSelected = false;
        if (mButtons[i].mAnmState != 0) {
            mpLayout->unbindAnmObj(mButtons[i].mpAnmObjs[0]);
            mButtons[i].mpAnmObjs[0]->resetFrame();
            mButtons[i].mpAnmObjs[0]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
            Lyt_playLastFrame(mButtons[i].mpAnmObjs[0]);
            RPSysLayout::bindAnmObjToPane(mButtons[i].mpAnmObjs[0], mButtons[i].mpPane, true);
            mFocus = mButtons[i].mpAnmObjs[0]->getFrameCtrl()->getEndFrame() -
                     mButtons[i].mpAnmObjs[0]->getFrameCtrl()->getStartFrame();
            mButtons[i].mAnmState = 0;
        }
    }
}

void RPSysSystemWinMgr::setSystemWindow(E_WINDOW_TYPE type, u32 group,
                                        RPSysMessage* pMessage, int argc, ...) {
    va_list argv;
    va_list argv2;
    va_start(argv, argc);
    va_start(argv2, argc);

    mResult = RESULT_WAIT;
    mState = 1;
    mWindowType = type;
    mGroup = group;
    mPage = 1;
    mSeType[0] = 0;
    mSeType[1] = 0;
    mNextButton = 0;
    mSelButton = 0;
    mHoverButton[0] = 0;
    mHoverButton[1] = 0;
    mHoverButton[2] = 0;
    mHoverButton[3] = 0;

    switch (type) {
    case E_WINDOW_TYPE_TEXT:
        RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_MESS_10");
        mpLayout->changeFontColor(mpTextBoxes[0], 0);
        mpLayout->changeFontColor(mpTextBoxes[1], 0);
        mpLayout->changeFontColor(mpTextBoxes[2], 0);
        mpLayout->changeFontColor(mpTextBoxes[3], 0);
        break;
    case E_WINDOW_TYPE_ERROR:
        RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_MESS_20");
        mpLayout->changeFontColor(mpTextBoxes[0], 1);
        mpLayout->changeFontColor(mpTextBoxes[1], 1);
        mpLayout->changeFontColor(mpTextBoxes[2], 1);
        mpLayout->changeFontColor(mpTextBoxes[3], 1);
        break;
    }

    if (pMessage == NULL) {
        pMessage = mpMessage;
    }

    mpCursor->SetActive(0);

    switch (pMessage->GetAttribute(group, 0)) {
    case 999:
        mpTextBoxes[2]->setMessage(pMessage, group, 0, argc, argv);
        mpTextBoxes[3]->setMessage(pMessage, group, 0, argc, argv2);
        mMessageId = 999;
        mButtons[0].mpTextBox->setMessage(mpMessage, 700, 0, 0);
        mButtons[1].mpTextBox->setMessage(mpMessage, 701, 0, 0);
        break;
    case 998:
        mpTextBoxes[2]->setMessage(pMessage, group, 0, argc, argv);
        mpTextBoxes[3]->setMessage(pMessage, group, 0, argc, argv2);
        mMessageId = 998;
        mButtons[0].mpTextBox->setMessage(mpMessage, 701, 0, 0);
        mButtons[1].mpTextBox->setMessage(mpMessage, 700, 0, 0);
        break;
    case 0:
        mpTextBoxes[2]->setMessage(pMessage, group, 0, argc, argv);
        mpTextBoxes[3]->setMessage(pMessage, group, 0, argc, argv2);
        mMessageId = 0;
        break;
    case 2:
        mpTextBoxes[2]->setMessage(pMessage, group, 0, argc, argv);
        mpTextBoxes[3]->setMessage(pMessage, group, 0, argc, argv2);
        mMessageId = 2;
        mButtons[0].mpTextBox->setMessage(pMessage, group, 1, 0);
        mButtons[1].mpTextBox->setMessage(pMessage, group, 2, 0);
        break;
    case 888:
        mpTextBoxes[0]->setMessage(pMessage, group, 0, argc, argv);
        mpTextBoxes[1]->setMessage(pMessage, group, 0, argc, argv2);
        mpTextBoxes[2]->setText(L"", 0);
        mpTextBoxes[3]->setText(L"", 0);
        mMessageId = 888;
        break;
    }

    switch (mMessageId) {
    case 2:
    case 998:
    case 999:
        mpTextBoxes[0]->SetVisible(false);
        mpTextBoxes[1]->SetVisible(false);
        for (int i = 0; i < 2; i++) {
            mpTextBoxes[i + 2]->SetVisible(mPage == i);
        }
        mButtons[0].mpPane->SetVisible(true);
        mButtons[1].mpPane->SetVisible(true);
        mpAButtonPane->SetVisible(false);
        break;
    case 0:
        mpTextBoxes[0]->SetVisible(false);
        mpTextBoxes[1]->SetVisible(false);
        for (int i = 0; i < 2; i++) {
            mpTextBoxes[i + 2]->SetVisible(mPage == i);
        }
        mButtons[0].mpPane->SetVisible(false);
        mButtons[1].mpPane->SetVisible(false);
        mpAButtonPane->SetVisible(true);
        break;
    case 888:
        for (int i = 0; i < 2; i++) {
            mpTextBoxes[i]->SetVisible(mPage == i);
        }
        mpTextBoxes[2]->SetVisible(false);
        mpTextBoxes[3]->SetVisible(false);
        mButtons[0].mpPane->SetVisible(false);
        mButtons[1].mpPane->SetVisible(false);
        mpAButtonPane->SetVisible(false);
        break;
    }

    resetInputState();

    mTimer = mpInAnmObj->getFrameCtrl()->getEndFrame() -
             mpInAnmObj->getFrameCtrl()->getStartFrame();
    mpLayout->unbindAnmObj(mpInAnmObj);
    mpInAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    mpInAnmObj->restart();
    RPSysLayout::bindAnmObjToPane(mpInAnmObj, mpCenterPane, true);
    mState = 1;

    resetButtons();

    va_end(argv2);
    va_end(argv);
}

void RPSysSystemWinMgr::setMessagePage(s32 page) {
    mPage = page;

    for (int i = 0; i < 2; i++) {
        switch (mMessageId) {
        case 888:
            mpTextBoxes[i]->SetVisible(mPage == i);
            break;
        default:
            mpTextBoxes[i + 2]->SetVisible(mPage == i);
            break;
        }
    }
}

void RPSysSystemWinMgr::closeSystemWindow() {
    if (mState == 0) {
        return;
    }

    mState = 5;
    for (u8 i = 0; i < mInputPlayerNum; i++) {
        if (mUseActiveCursor) {
            RPSysCursorDrawMgr::instance()->fadeActiveCursor();
        } else {
            RPSysCursorDrawMgr::instance()->setPlayerCursorOutOfScreen(
                static_cast<RPSysCursorDrawMgr::EPlayer>(mInputPlayers[i].mPlayer));
        }
    }
    RPSysCursorDrawMgr::instance()->finishDpdCheck();

    mpLayout->unbindAnmObj(mpOutAnmObj);
    mpOutAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    mpOutAnmObj->restart();
    RPSysLayout::bindAnmObjToPane(mpOutAnmObj, mpCenterPane, true);
    mTimer = mpOutAnmObj->getFrameCtrl()->getEndFrame() -
             mpOutAnmObj->getFrameCtrl()->getStartFrame();
    resetButtonAnims();
}

void RPSysSystemWinMgr::enableCursors() {
    RPSysCursorDrawMgr::instance()->startDpdCheck();
    if (!mInputSet) {
        setCursors();
    }

    for (u8 i = 0; i < mInputPlayerNum; i++) {
        if (mUseActiveCursor) {
            RPSysCursorDrawMgr::instance()->createActiveCursor();
            if (mInputPlayers[i].mChannel < 0) {
                setCursors();
            }
        } else {
            RPSysCursorDrawMgr::instance()->createPlayerCursor(
                static_cast<RPSysCursorDrawMgr::EPlayer>(mInputPlayers[i].mPlayer));
            if (mInputPlayers[i].mChannel < 0) {
                mInputPlayers[i].mChannel = RPSysCursorDrawMgr::instance()->getPlayerCursorChannel(
                    static_cast<RPSysCursorDrawMgr::EPlayer>(mInputPlayers[i].mPlayer));
            }
        }
    }
}

void RPSysSystemWinMgr::resetInputState() {
    mButtons[0].mAnmState = 0;
    mButtons[0].mSelected = false;
    mButtons[1].mAnmState = 0;
    mButtons[1].mSelected = false;
    mFocus = 0;
    mCloseDelay = 0;
    mUseActiveCursor = true;
    mInputSet = false;
    mInputPlayerNum = 0;
    mpCursor->Reset();

    if (mMessageId != 0 && mMessageId != 888) {
        mUseCPadCursor = true;
    } else {
        mUseCPadCursor = false;
    }

    mCursorMoved = false;
    mCursorPos.x = 0.0f;
    mCursorPos.y = 0.0f;
}

void RPSysSystemWinMgr::load() {
    if (mpResAccessor != NULL) {
        return;
    }

    EGG::Heap* pHeap = RPSysSystem::instance()->getResourceHeap();
    mpResAccessor = RPSysLytResAccessor::create(pHeap);

    EGG::Archive* pArchive = RPSysResourceManager::instance()->GetStaticLocalArchive();
    mpResAccessor->mountArchive(
        RPSysResourceManager::GetFileFromArchive(pArchive, "messageWindow.arc", NULL), NULL);

    mpLayout = RPSysLayout::create(pHeap, mpResAccessor, "systemWindow_00.brlyt");
    mpCenterPane = mpLayout->findPane("N_center_00");
    mpTextBoxes[0] = mpLayout->findTextBox("T_message_02");
    mpTextBoxes[1] = mpLayout->findTextBox("T_message_00");
    mpTextBoxes[2] = mpLayout->findTextBox("T_message_03");
    mpTextBoxes[3] = mpLayout->findTextBox("T_message_01");

    for (u8 i = 0; i < 2; i++) {
        mButtons[i].mpPane = mpLayout->findPane(sButtonPaneNames[i]);
        mButtons[i].mpBounding = mpLayout->findBounding(sButtonBoundingNames[i]);
        mButtons[i].mpTextBox = mpLayout->findTextBox(sButtonTextNames[i]);
    }

    mpAButtonPane = mpLayout->findPane("N_AButton_00");
    unk4C = 0;
    unk50 = 0;

    mpInAnmObj = mpLayout->createAnmObj(pHeap, mpResAccessor, "systemWindow_00_in_00.brlan");
    mpOutAnmObj = mpLayout->createAnmObj(pHeap, mpResAccessor, "systemWindow_00_out_00.brlan");
    mpOutAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);

    for (u8 i = 0; i < 2; i++) {
        for (u8 j = 0; j < 3; j++) {
            mButtons[i].mpAnmObjs[j] =
                mpLayout->createAnmObj(pHeap, mpResAccessor, sButtonAnimNames[j]);
            mButtons[i].mpAnmObjs[j]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
            RPSysLayout::bindAnmObjToPane(mButtons[i].mpAnmObjs[j], mButtons[i].mpPane, true);
        }
    }

    for (u8 i = 0; i < 2; i++) {
        mpAButtonAnmObjs[i] =
            mpLayout->createAnmObj(pHeap, mpResAccessor, sAButtonAnimNames[i]);
        mpAButtonAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
        RPSysLayout::bindAnmObjToPane(mpAButtonAnmObjs[i], mpAButtonPane, true);
    }

    void* pMessageRes = RPSysResourceManager::GetMessageResource("common_message.bmg");
    mpMessage = new (pHeap, 4) RPSysMessage(pMessageRes, pHeap);

    mpCursor->LoadWithParent(pArchive, pHeap);
}

void RPSysSystemWinMgr::reset() {
    mState = 0;
    mTimer = 0;
    mMessageId = 0;
    mResult = RESULT_NONE;
    mGroup = 0;
    mInputPlayerNum = 0;
    mPointerHit = false;
    mDpadInput = false;
    mUseCPadCursor = false;
    resetInputState();
}

inline void RPSysSystemWinMgr::updateButtons() {
    updateFocus();

    for (u8 i = 0; i < 2; i++) {
        if (mButtons[i].mAnmState != mButtons[i].mSelected) {
            mCursorMoved = true;
            if (mButtons[i].mSelected) {
                if (mButtons[i].mAnmState != 1) {
                    mpLayout->unbindAnmObj(mButtons[i].mpAnmObjs[1]);
                    mButtons[i].mpAnmObjs[1]->resetFrame();
                    mButtons[i].mpAnmObjs[1]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
                    mButtons[i].mpAnmObjs[1]->restart();
                    RPSysLayout::bindAnmObjToPane(mButtons[i].mpAnmObjs[1], mButtons[i].mpPane,
                                                  true);
                    mFocus = mButtons[i].mpAnmObjs[1]->getFrameCtrl()->getEndFrame() -
                             mButtons[i].mpAnmObjs[1]->getFrameCtrl()->getStartFrame();
                    mButtons[i].mAnmState = 1;
                }
            } else if (mButtons[i].mAnmState != 0) {
                mpLayout->unbindAnmObj(mButtons[i].mpAnmObjs[0]);
                mButtons[i].mpAnmObjs[0]->resetFrame();
                mButtons[i].mpAnmObjs[0]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
                Lyt_playLastFrame(mButtons[i].mpAnmObjs[0]);
                RPSysLayout::bindAnmObjToPane(mButtons[i].mpAnmObjs[0], mButtons[i].mpPane, true);
                mFocus = mButtons[i].mpAnmObjs[0]->getFrameCtrl()->getEndFrame() -
                         mButtons[i].mpAnmObjs[0]->getFrameCtrl()->getStartFrame();
                mButtons[i].mAnmState = 0;
            }
        }
    }
}

inline void RPSysSystemWinMgr::startAButtonIn() {
    mpLayout->unbindAnmObj(mpAButtonAnmObjs[0]);
    mpAButtonAnmObjs[0]->resetFrame();
    mpAButtonAnmObjs[0]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    mpAButtonAnmObjs[0]->restart();
    RPSysLayout::bindAnmObjToPane(mpAButtonAnmObjs[0], mpAButtonPane, true);
}

void RPSysSystemWinMgr::calc() {
    if (mState == 0 || mpResAccessor == NULL) {
        return;
    }

    mResult = RESULT_WAIT;
    if (mFocus > 0 && mMessageId != 0) {
        mFocus--;
    }

    if (mTimer == 0) {
        switch (mState) {
        case 1:
            if (mCloseDelay == 0) {
                mState = 3;
                if (mMessageId == 0) {
                    startAButtonIn();
                    mFocus = 0;
                }
            } else {
                mTimer = mCloseDelay;
                mState = 2;
            }

            if (mMessageId == 999 || mMessageId == 998 || mMessageId == 2) {
                if (mInputPlayers[0].mChannel < 0) {
                    setCursors();
                }
            }
            break;
        case 2:
            mState = 3;
            if (mMessageId == 0) {
                startAButtonIn();
                mFocus = 0;
            }
            break;
        case 3:
            switch (mMessageId) {
            case 999:
                updateButtons();
                checkAPress();
                break;
            case 998:
                updateButtons();
                checkAPress();
                break;
            case 2:
                updateButtons();
                checkAPress();
                break;
            case 0:
                updateAButtonOnly();
                break;
            case 1:
            case 888:
                break;
            }
            break;
        case 4:
            closeSystemWindow();
            resetButtonAnims();
            break;
        case 5:
            switch (mMessageId) {
            case 2:
                switch (mSelButton) {
                case 0:
                    mResult = RESULT_4;
                    break;
                case 1:
                    mResult = RESULT_5;
                    break;
                default:
                    mResult = RESULT_CONTINUE;
                    break;
                }
                break;
            case 999:
                switch (mSelButton) {
                case 0:
                    mResult = RESULT_YES;
                    break;
                case 1:
                    mResult = RESULT_NO;
                    break;
                default:
                    mResult = RESULT_CONTINUE;
                    break;
                }
                break;
            case 998:
                switch (mSelButton) {
                case 0:
                    mResult = RESULT_NO;
                    break;
                case 1:
                    mResult = RESULT_YES;
                    break;
                default:
                    mResult = RESULT_CONTINUE;
                    break;
                }
                break;
            case 0:
                mResult = RESULT_CONTINUE;
                break;
            case 888:
                mResult = RESULT_CONTINUE;
                break;
            case 1:
                break;
            }
            mpCursor->SetActive(0);
            mState = 0;
            break;
        }
    } else {
        mTimer--;
        if (mMessageId == 0 && mState == 2 && !mpAButtonPane->IsVisible() &&
            mTimer <= mpAButtonAnmObjs[0]->getFrameCtrl()->getEndFrame() -
                          mpAButtonAnmObjs[0]->getFrameCtrl()->getStartFrame()) {
            startAButtonIn();
        }
    }

    mpLayout->calc();

    if (mState == 1 && mTimer == 3) {
        enableCursors();
    }

    if (mState == 1 && mTimer == 0 &&
        (mMessageId == 999 || mMessageId == 998 || mMessageId == 2)) {
        updateButtons();
    }

    if (mState >= 3) {
        mpCursor->SetActive(mUseCPadCursor);
    }

    mpCursor->SetTarget(mButtons[mSelButton].mpBounding);
    mpCursor->Calculate();

    if (mCursorMoved) {
        if (mUseCPadCursor) {
            EGG::Vector2f pos;
            RP_getPointerScreenPos(pos, *mpCursor);
            mCursorPos.x = pos.x;
            mCursorPos.y = pos.y;
        }
        mCursorMoved = false;
        RPSndAudioMgr::instance()->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_01",
                                                      mCursorPos, 0.5f, 0);
    }
}

void RPSysSystemWinMgr::draw() {
    if (mState != 0 && mpResAccessor != NULL) {
        mpLayout->draw();
        mpCursor->Draw();
    }
}

void RPSysSystemWinMgr::updateAButtonOnly() {
    if (mFocus == 0) {
        mpLayout->unbindAnmObj(mpAButtonAnmObjs[1]);
        mpAButtonAnmObjs[1]->resetFrame();
        mpAButtonAnmObjs[1]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
        mpAButtonAnmObjs[1]->restart();
        RPSysLayout::bindAnmObjToPane(mpAButtonAnmObjs[1], mpAButtonPane, true);
        mFocus = mpAButtonAnmObjs[1]->getFrameCtrl()->getEndFrame() -
                 mpAButtonAnmObjs[1]->getFrameCtrl()->getStartFrame();
    } else {
        mFocus--;
    }

    for (u8 i = 0; i < mInputPlayerNum; i++) {
        if (mInputPlayers[i].mChannel >= 0 &&
            EGG::CoreControllerMgr::instance()
                ->getNthController(mInputPlayers[i].mChannel)
                ->downTrigger(EGG::cCORE_BUTTON_A)) {
            decideButton(mInputPlayers[i].mChannel);
        }
    }
}

void RPSysSystemWinMgr::updateFocus() {
    if (mFocus == 0) {
        mPointerHit = false;
    }

    mDpadInput = false;
    u8 i = 0;
    mButtons[0].mSelected = false;
    mButtons[1].mSelected = false;

    for (; i < mInputPlayerNum; i++) {
        if (mFocus == 0) {
            mPointerHit += hitTestPointer(i);
        }
        mDpadInput += checkDpadInput(i);
    }

    if (mPointerHit) {
        bool first = true;
        mUseCPadCursor = false;
        for (u8 i = 0; i < mInputPlayerNum; i++) {
            if (mHoverButton[i] != 2) {
                if (first) {
                    first = false;
                    mSelButton = mHoverButton[i];
                }
                mButtons[mHoverButton[i]].mSelected = true;
            }
        }
    } else if (mDpadInput) {
        if (mUseCPadCursor) {
            mButtons[mNextButton].mSelected = true;
            mSelButton = mNextButton;
        } else {
            mButtons[mSelButton].mSelected = true;
        }
        mUseCPadCursor = true;
    } else if (mUseCPadCursor) {
        mButtons[mSelButton].mSelected = true;
    }
}

void RPSysSystemWinMgr::checkAPress() {
    for (u8 i = 0; i < mInputPlayerNum; i++) {
        if (mInputPlayers[i].mChannel < 0) {
            return;
        }

        if (EGG::CoreControllerMgr::instance()
                ->getNthController(mInputPlayers[i].mChannel)
                ->downTrigger(EGG::cCORE_BUTTON_A)) {
            if (mHoverButton[i] != 2) {
                mSelButton = mHoverButton[i];
                decideButton(mInputPlayers[i].mChannel);
                return;
            }

            if (mUseCPadCursor) {
                decideButton(mInputPlayers[i].mChannel);
                mpCursor->SetActive(0);
                mUseCPadCursor = false;
                return;
            }
        }
    }
}

bool RPSysSystemWinMgr::hitTestPointer(u16 index) {
    EGG::Vector2f pos;
    s32 channel;

    if (mUseActiveCursor) {
        pos = RPSysCursorDrawMgr::instance()->getActiveCursorPos();
        channel = RPSysCursorDrawMgr::instance()->getActiveCursorPlayer();
    } else {
        pos = RPSysCursorDrawMgr::instance()->getPlayerCursorPos(
            static_cast<RPSysCursorDrawMgr::EPlayer>(mInputPlayers[index].mPlayer));
        channel = RPSysCursorDrawMgr::instance()->getPlayerCursorChannel(
            static_cast<RPSysCursorDrawMgr::EPlayer>(mInputPlayers[index].mPlayer));
    }

    if (channel < 0) {
        return false;
    }

    for (u8 i = 0; i < 2; i++) {
        if (mpLayout->isInsideBounding(mButtons[i].mpBounding, pos)) {
            if (i != mHoverButton[index]) {
                static_cast<RPSysCoreController*>(
                    EGG::CoreControllerMgr::instance()->getNthController(channel))
                    ->rumble(RPSysCoreController::ERumblePattern_Cursor);
                mCursorPos.x = pos.x;
                mCursorPos.y = pos.y;
            }
            mHoverButton[index] = i;
            return true;
        }
    }

    mHoverButton[index] = 2;
    return false;
}

bool RPSysSystemWinMgr::checkDpadInput(u16 index) {
    s32 channel = mInputPlayers[index].mChannel;
    if (channel < 0) {
        return false;
    }

    if (EGG::CoreControllerMgr::instance()->getNthController(channel)->downTrigger(
            EGG::cCORE_BUTTON_LEFT | EGG::cCORE_FSSTICK_LEFT)) {
        if (mSelButton != 0) {
            mNextButton = mSelButton - 1;
        }
        return true;
    }

    if (EGG::CoreControllerMgr::instance()->getNthController(channel)->downTrigger(
            EGG::cCORE_BUTTON_RIGHT | EGG::cCORE_FSSTICK_RIGHT)) {
        if (mSelButton < 1) {
            mNextButton = mSelButton + 1;
        }
        return true;
    }

    if (EGG::CoreControllerMgr::instance()->getNthController(channel)->downTrigger(
            EGG::cCORE_BUTTON_UP | EGG::cCORE_FSSTICK_UP)) {
        return true;
    }

    return EGG::CoreControllerMgr::instance()->getNthController(channel)->downTrigger(
        EGG::cCORE_BUTTON_DOWN | EGG::cCORE_FSSTICK_DOWN);
}

void RPSysSystemWinMgr::resetButtonAnims() {
    for (u8 i = 0; i < 2; i++) {
        u8 sel;
        if (i == mSelButton) {
            mButtons[i].mSelected = true;
            sel = 1;
        } else {
            mButtons[i].mSelected = false;
            sel = 0;
        }

        mpLayout->unbindAnmObj(mButtons[i].mpAnmObjs[sel]);
        mButtons[i].mpAnmObjs[sel]->resetFrame();
        if (sel == 1) {
            mButtons[i].mpAnmObjs[sel]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
            Lyt_playLastFrame(mButtons[i].mpAnmObjs[sel]);
            mButtons[i].mpAnmObjs[sel]->stop();
        } else {
            mButtons[i].mpAnmObjs[sel]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
            mButtons[i].mpAnmObjs[sel]->setFrame(0.0f);
            mButtons[i].mpAnmObjs[sel]->stop();
        }
        RPSysLayout::bindAnmObjToPane(mButtons[i].mpAnmObjs[sel], mButtons[i].mpPane,
                                      true);
        mButtons[i].mAnmState = sel != 0;
    }
}

void RPSysSystemWinMgr::decideButton(s32 channel) {
    u32 remote;

    mState = 4;
    switch (channel) {
    case 0:
        remote = 1;
        break;
    case 1:
        remote = 2;
        break;
    case 2:
        remote = 4;
        break;
    case 3:
        remote = 8;
        break;
    }

    if (mMessageId == 0) {
        mpAButtonAnmObjs[1]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
        switch (mSeType[0]) {
        case 1:
            RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_BACK_01_CTSP",
                                                               remote);
            RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_BACK_01_TVSP",
                                                               remote);
            break;
        case 0:
            RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_A_04_CTSP",
                                                               remote);
            RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_A_04_TVSP",
                                                               remote);
            break;
        }
    } else {
        u8 sel = mSelButton;
        if (mButtons[sel].mAnmState != 2) {
            mpLayout->unbindAnmObj(mButtons[sel].mpAnmObjs[2]);
            mButtons[sel].mpAnmObjs[2]->resetFrame();
            mButtons[sel].mpAnmObjs[2]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
            mButtons[sel].mpAnmObjs[2]->restart();
            RPSysLayout::bindAnmObjToPane(mButtons[sel].mpAnmObjs[2], mButtons[sel].mpPane, true);
            mFocus = mButtons[sel].mpAnmObjs[2]->getFrameCtrl()->getEndFrame() -
                     mButtons[sel].mpAnmObjs[2]->getFrameCtrl()->getStartFrame();
            mButtons[sel].mAnmState = 1;
        }
        switch (mSeType[mSelButton]) {
        case 1:
            RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_BACK_01_CTSP",
                                                               remote);
            RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_BACK_01_TVSP",
                                                               remote);
            break;
        case 0:
            RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_A_04_CTSP",
                                                               remote);
            RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_A_04_TVSP",
                                                               remote);
            break;
        }
    }

    mTimer = mButtons[mSelButton].mpAnmObjs[2]->getFrameCtrl()->getEndFrame() -
             mButtons[mSelButton].mpAnmObjs[2]->getFrameCtrl()->getStartFrame();
}

bool RPSysSystemWinMgr::setCursors() {
    mInputSet = true;
    mUseActiveCursor = true;
    mInputPlayers[0].mPlayer = 4;
    mInputPlayers[0].mChannel = RPSysCursorDrawMgr::instance()->getActiveCursorPlayer();
    mInputPlayerNum = 1;
    return true;
}

BOOL RPSysSystemWinMgr::setPlayerInput(s32 player) {
    mInputSet = true;
    mUseActiveCursor = false;
    mInputPlayers[0].mPlayer = player;
    mInputPlayers[0].mChannel = RPSysCursorDrawMgr::instance()->getPlayerCursorChannel(
        static_cast<RPSysCursorDrawMgr::EPlayer>(player));
    mInputPlayerNum = 1;
    return TRUE;
}

RPSysSystemWinMgr::RPSysSystemWinMgr(EGG::Heap* pHeap) : mpParentHeap(pHeap) {
    mpResAccessor = NULL;
    mpMessage = NULL;
    mpLayout = NULL;
    mpInAnmObj = NULL;
    mpOutAnmObj = NULL;
    mpCenterPane = NULL;

    for (u8 i = 0; i < 2; i++) {
        mButtons[i].mpPane = NULL;
        mButtons[i].mpBounding = NULL;
        mButtons[i].mpTextBox = NULL;
        for (u8 j = 0; j < 3; j++) {
            mButtons[i].mpAnmObjs[j] = NULL;
        }
        mButtons[i].mSelected = false;
    }

    mpAButtonPane = NULL;
    unk4C = 0;
    unk50 = 0;
    mpAButtonAnmObjs[0] = NULL;
    mpAButtonAnmObjs[1] = NULL;
    unk5C = 0;

    mpCursor = new RPCPadCursor();
}

RPSysSystemWinMgr::~RPSysSystemWinMgr() {}
