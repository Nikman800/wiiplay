#include <Pack/RPKernel.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPKernel/RPSysPauseMenu.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".
// Wii Play: the menu is rewritten (three buttons, D-pad cursor, pointer selection).
#include <Pack/RPAudio.h>
#include <Pack/RPParty/RPPartyCommon/RPCPadCursor.h>
#include <Pack/RPSystem.h>

#include <egg/core.h>

/**
 * @brief Message IDs of the button captions (group, index)
 */
static const u32 sButtonMessages[3][2] = {
    {750, 0},
    {751, 0},
    {752, 0},
};

/**
 * @brief Layout names
 */
struct RPSysPauseMenuNames {
    const char* anims[2][3];   // per-button animations
    const char* nullPanes[3];  // per-button null panes
    const char* boundPanes[3]; // per-button bounding panes
    const char* textBoxes[3];  // per-button text boxes
};

static const char* sAnmNames0[3] = {"pause_00_Offon_00.brlan", "pause_00_Offon_00.brlan", "pause_00_push_00.brlan"};
static const char* sAnmNames1[3] = {"pause_00_active_00.brlan", "pause_00_active_00.brlan", "pause_00_push_00.brlan"};
static const char* sNullNames[3] = {"N_choicesF_00", "N_choicesS_00", "N_choicesT_00"};
static const char* sBoundNames[3] = {"B_choicesF_00", "B_choicesS_00", "B_choicesT_00"};
static const char* sTextNames[3] = {"T_choicesF_00", "T_choicesS_00", "T_choicesT_00"};

/**
 * @brief Moves an animation to its last frame
 */
static inline void PlayLastFrame(RPSysLytAnmObj* pAnmObj) {
    pAnmObj->play(pAnmObj->getFrameMax() - 1.0f);
}

/**
 * @brief Starts a button's highlight or unhighlight animation
 */
static inline void StartHighlightAnm(RPSysLayout* pLayout,
                                     RPSysLytAnmObj*& rAnmObj, u8 on) {
    pLayout->unbindAnmObj(rAnmObj);

    if (on == 0) {
        rAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
        PlayLastFrame(rAnmObj);
    } else {
        rAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
        rAnmObj->restart();
    }
}


/**
 * @brief Tests which button the pointer is on
 */
inline bool RPSysPauseMenu::isPointing() {
    EGG::Vector2f pos =
        RP_GET_INSTANCE(RPSysCursorDrawMgr)
            ->getPlayerCursorPos(
                static_cast<RPSysCursorDrawMgr::EPlayer>(mUNK_0x80));

    for (u8 i = 0; i < 3; i++) {
        if (mLayout->isInsideBounding(mButtons[i].mBounding, pos)) {
            mUNK_0x7D = i;
            return true;
        }
    }

    return false;
}

/**
 * @brief Reads the pointer and the D-pad
 */
inline void RPSysPauseMenu::updateInput() {
    mUNK_0x7A = mUNK_0x79;

    if (mUNK_0x84 == 0) {
        mUNK_0x79 = isPointing();
    } else {
        mUNK_0x79 = mUNK_0x7A;
    }

    bool moved;
    if (RP_GET_CORE_CTRL(mUNK_0x80)->getCursorHold() & 0x10008) {
        mUNK_0x7E = (mUNK_0x7F + 2) % 3;
        moved = true;
    } else if (RP_GET_CORE_CTRL(mUNK_0x80)->getCursorHold() & 0x20004) {
        mUNK_0x7E = (mUNK_0x7F + 4) % 3;
        moved = true;
    } else if (RP_GET_CORE_CTRL(mUNK_0x80)->getCursorHold() & 0x40001) {
        moved = true;
    } else {
        moved = RP_GET_CORE_CTRL(mUNK_0x80)->getCursorHold() & 0x80002;
    }
    mUNK_0x7B = moved;
}

void RPSysPauseMenu::load() {
    if (mResAccessor != NULL) {
        return;
    }

    EGG::Heap* pHeap = RP_GET_INSTANCE(RPSysSystem)->getResourceHeap();
    mResAccessor = RPSysLytResAccessor::create(pHeap);

    EGG::Archive* pArchive =
        RP_GET_INSTANCE(RPSysResourceManager)->GetStaticLocalArchive();

    mResAccessor->mountArchive(
        RPSysResourceManager::GetFileFromArchive(pArchive, "pause.arc"));

    mLayout = RPSysLayout::create(pHeap, mResAccessor, "pause_00.brlyt");
    mPane_0x14 = mLayout->findPane("N_center_00");
    mPane_0x18 = mLayout->findPane("N_bg_00");

    const char** ppNull = sNullNames;
    const char** ppBound = sBoundNames;
    const char** ppText = sTextNames;
    int nullIndex = 0;
    for (int i = 0; i < 3; i++, nullIndex++) {
        mButtons[i].mPane = mLayout->findPane(ppNull[nullIndex]);
        mButtons[i].mBounding =
            RPSysLayout::castToBounding(mLayout->findPane(ppBound[i]));
        mButtons[i].mTextBox = mLayout->findTextBox(ppText[i]);
    }

    mAnmObj_0x04 = mLayout->createAnmObj(pHeap, mResAccessor,
                                         "pause_00_in_00.brlan");
    mAnmObj_0x08 = mLayout->createAnmObj(pHeap, mResAccessor,
                                         "pause_00_out_00.brlan");
    mAnmObj_0x08->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            mButtons[i].mAnmObjs[j] = mLayout->createAnmObj(
                pHeap, mResAccessor, sAnmNames1[j]);
            mButtons[i].mAnmObjs[j]->setPlayMode(
                RPSysFrameCtrl::EPlayMode_OneTime);
        }
    }

    const void* pMsgBin =
        RPSysResourceManager::GetMessageResource("common_message.bmg");
    RPSysMessage* pMessage = new (pHeap) RPSysMessage(pMsgBin, pHeap);

    int messageIndex = 0;
    for (int buttonIndex = 0; buttonIndex < 3; buttonIndex++, messageIndex++) {
        mButtons[buttonIndex].mTextBox->setMessage(pMessage, sButtonMessages[messageIndex][0],
                                         sButtonMessages[messageIndex][1], 0);
    }

    mUtlCursor->LoadWithParent(pArchive, pHeap);
}

void RPSysPauseMenu::open() {
    mUNK_0x79 = false;
    mUNK_0x7B = false;
    mUNK_0x7C = true;
    mUNK_0x84 = 0;

    u8 i;
    u8 pointIndex;
    for (i = 0; i < 3; i++) {
        mButtons[i].mUNK_0x0 = false;
        mButtons[i].mUNK_0x1 = false;
    }

    mUNK_0x7D = 0;
    mUNK_0x7E = 0;
    mUNK_0x7F = 0;
    mUNK_0x74 = 1;

    mState = mAnmObj_0x04->getFrameCtrl()->getEndFrame() -
             mAnmObj_0x04->getFrameCtrl()->getStartFrame();

    mLayout->unbindAnmObj(mAnmObj_0x04);
    mAnmObj_0x04->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    mAnmObj_0x04->restart();
    mLayout->bindAnmObjToPane(mAnmObj_0x04, mPane_0x14, true);

    if (RP_GET_INSTANCE(RPSysProjectLocal)->getPack() == 0) {
        mPane_0x18->SetVisible(true);
    } else if (RP_GET_INSTANCE(RPSysProjectLocal)->getPack() == 1) {
        mPane_0x18->SetVisible(false);
    }

    mUNK_0x78 = true;
    mUtlCursor->Reset();

    for (i = 0; i < 3; i++) {
        // NONMATCHING-DEBT: the natural form is mButtons[i].<field> (same instructions: the
        // address is this + 28 * i with the 0x1C folded into each offset), but its address is a CSE
        // temporary numbered above i (mwcc_trace: @ r38 vs i r36), so i and the address swap
        // r26/r27 (194/212). Here the named pointer is numbered below i. Tried 2026-10-29: index
        // form, Button&/Button* locals (propagated away: 171/212), block-scope int/u8/u16/u32/s32
        // counters (197-199/212), an inline member taking Button& (194/212); the whole loop as an
        // inline member with a u8 counter (207/212: this loop matches, but i then exists only in the
        // unrolled first loop, so pointIndex takes r26 instead of sharing i's r27).
        RPSysPauseMenu* pButtonMenu = reinterpret_cast<RPSysPauseMenu*>(reinterpret_cast<u8*>(this) + sizeof(Button) * i);
        pButtonMenu->mButtons[0].mUNK_0x0 = false;
        pButtonMenu->mButtons[0].mUNK_0x1 = false;

        mLayout->unbindAnmObj(pButtonMenu->mButtons[0].mAnmObjs[0]);
        pButtonMenu->mButtons[0].mAnmObjs[0]->resetFrame();
        pButtonMenu->mButtons[0].mAnmObjs[0]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
        pButtonMenu->mButtons[0].mAnmObjs[0]->restart();
        mLayout->bindAnmObjToPane(pButtonMenu->mButtons[0].mAnmObjs[0], pButtonMenu->mButtons[0].mPane,
                                  true);
    }

    mUtlCursor->SetActive(0);
    {
    mUNK_0x7A = mUNK_0x79;
    
        if (mUNK_0x84 == 0) {
    
        EGG::Vector2f pos =
            RP_GET_INSTANCE(RPSysCursorDrawMgr)
                ->getPlayerCursorPos(
                    static_cast<RPSysCursorDrawMgr::EPlayer>(mUNK_0x80));
        bool pointing;
        for (pointIndex = 0; pointIndex < 3; pointIndex++) {
            if (mLayout->isInsideBounding(mButtons[pointIndex].mBounding, pos)) {
                mUNK_0x7D = pointIndex;
                pointing = true;
                goto pointing_done;
            }
        }
        pointing = false;
    pointing_done:
        mUNK_0x79 = pointing;
        } else {
            mUNK_0x79 = mUNK_0x7A;
        }
    
        bool moved;
        if (RP_GET_CORE_CTRL(mUNK_0x80)->getCursorHold() & 0x10008) {
            mUNK_0x7E = (mUNK_0x7F + 2) % 3;
            moved = true;
        } else if (RP_GET_CORE_CTRL(mUNK_0x80)->getCursorHold() & 0x20004) {
            mUNK_0x7E = (mUNK_0x7F + 4) % 3;
            moved = true;
        } else if (RP_GET_CORE_CTRL(mUNK_0x80)->getCursorHold() & 0x40001) {
            moved = true;
        } else {
            moved = RP_GET_CORE_CTRL(mUNK_0x80)->getCursorHold() & 0x80002;
        }
        mUNK_0x7B = moved;
    }
}

u8 RPSysPauseMenu::calc() {
    if (mUNK_0x78) {
        mUNK_0x78 = false;
        RP_GET_INSTANCE(RPSysCursorDrawMgr)->startDpdCheck();
        RP_GET_INSTANCE(RPSysCursorDrawMgr)
            ->createPlayerCursor(
                static_cast<RPSysCursorDrawMgr::EPlayer>(mUNK_0x80));
    }

    u8 result = 3;

    if (mUNK_0x84 > 0) {
        mUNK_0x84--;
    }

    switch (mUNK_0x74) {
    case 1: {
        if (mState == 0) {
            mUNK_0x74 = 2;
            updateInput();
            updateButtonHighlight();
            mUtlCursor->SetActive(mUNK_0x7C);
        } else {
            mState--;
        }
        break;
    }

    case 2: {
        updateSelect();
        break;
    }

    case 4: {
        if (mState == 0) {
            mUNK_0x74 = 8;
            RP_GET_INSTANCE(RPSysCursorDrawMgr)
                ->setPlayerCursorOutOfScreen(
                    static_cast<RPSysCursorDrawMgr::EPlayer>(mUNK_0x80));
            RP_GET_INSTANCE(RPSysCursorDrawMgr)->finishDpdCheck();

            if (mUNK_0x7F != 0 &&
                RP_GET_INSTANCE(RPSysProjectLocal)->getPack() == 0) {
                mState = mAnmObj_0x08->getFrameCtrl()->getEndFrame() -
                         mAnmObj_0x08->getFrameCtrl()->getStartFrame();
            } else {
                mLayout->unbindAnmObj(mAnmObj_0x08);
                mAnmObj_0x08->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
                mAnmObj_0x08->restart();
                mLayout->bindAnmObjToPane(mAnmObj_0x08, mPane_0x14, true);
                mState = mAnmObj_0x08->getFrameCtrl()->getEndFrame() -
                         mAnmObj_0x08->getFrameCtrl()->getStartFrame();
            }

            resetButtonAnims();
        } else {
            mState--;
        }
        break;
    }

    case 8: {
        if (mState == 0) {
            mState = -1;
            result = mUNK_0x7F;
        } else if (mState < 0) {
            result = mUNK_0x7F;
        } else {
            mState--;
        }
        break;
    }
    }

    mLayout->calc();
    mUtlCursor->SetTarget(
        mButtons[mUNK_0x7F].mBounding);
    mUtlCursor->Calculate();

    return result;
}

void RPSysPauseMenu::draw() {
    if (!mUNK_0x78) {
        mLayout->draw();
        mUtlCursor->Draw();
    }
}

void RPSysPauseMenu::updateSelect() {
    {
    mUNK_0x7A = mUNK_0x79;
    
        if (mUNK_0x84 == 0) {
            mUNK_0x79 = isPointing();
        } else {
            mUNK_0x79 = mUNK_0x7A;
        }
    
        bool moved;
        if (RP_GET_CORE_CTRL(mUNK_0x80)->getCursorHold() & 0x10008) {
            mUNK_0x7E = (mUNK_0x7F + 2) % 3;
            moved = true;
        } else if (RP_GET_CORE_CTRL(mUNK_0x80)->getCursorHold() & 0x20004) {
            mUNK_0x7E = (mUNK_0x7F + 4) % 3;
            moved = true;
        } else if (RP_GET_CORE_CTRL(mUNK_0x80)->getCursorHold() & 0x40001) {
            moved = true;
        } else {
            moved = RP_GET_CORE_CTRL(mUNK_0x80)->getCursorHold() & 0x80002;
        }
        mUNK_0x7B = moved;
    }
    updateButtonHighlight();

    if (RP_GET_CORE_CTRL(mUNK_0x80)->downTrigger(WPAD_BUTTON_A) &&
        !RP_GET_CORE_CTRL(mUNK_0x80)->downTrigger(WPAD_BUTTON_PLUS)) {
        if (mUNK_0x79 || mUNK_0x7C) {
            mUNK_0x74 = 4;

            u8 sel = mUNK_0x7F;
            mLayout->unbindAnmObj(mButtons[sel].mAnmObjs[2]);
            mButtons[sel].mAnmObjs[2]->setPlayMode(
                RPSysFrameCtrl::EPlayMode_OneTime);
            mButtons[sel].mAnmObjs[2]->restart();
            mLayout->bindAnmObjToPane(mButtons[sel].mAnmObjs[2],
                                      mButtons[sel].mPane, true);
            mUNK_0x84 = mButtons[sel].mAnmObjs[2]->getFrameCtrl()->getEndFrame() -
                        mButtons[sel].mAnmObjs[2]->getFrameCtrl()->getStartFrame();

            s32 player = mUNK_0x80;
            switch (mUNK_0x7F) {
            case 0: {
                RP_GET_INSTANCE(RPSndAudioMgr)
                    ->startPauseSe("RP_COM_GAME_SE_PAUSE_KEY_A_01_CTSP",
                                   "RP_COM_GAME_SE_PAUSE_KEY_A_01_TVSP", false,
                                   player);
                break;
            }
            case 1: {
                RP_GET_INSTANCE(RPSndAudioMgr)
                    ->startPauseSe("RP_COM_GAME_SE_PAUSE_KEY_A_02_CTSP",
                                   "RP_COM_GAME_SE_PAUSE_KEY_A_02_TVSP", false,
                                   player);
                RP_GET_INSTANCE(RPSndAudioMgr)->stopAllSoundOnPauseOff();
                break;
            }
            case 2: {
                RP_GET_INSTANCE(RPSndAudioMgr)
                    ->startPauseSe("RP_COM_GAME_SE_PAUSE_KEY_A_03_CTSP",
                                   "RP_COM_GAME_SE_PAUSE_KEY_A_03_TVSP", false,
                                   player);
                RP_GET_INSTANCE(RPSndAudioMgr)->stopAllSoundOnPauseOff();
                break;
            }
            }

            mUNK_0x7C = false;
            mState =
                mButtons[mUNK_0x7F].mAnmObjs[2]->getFrameCtrl()->getEndFrame() -
                mButtons[mUNK_0x7F].mAnmObjs[2]->getFrameCtrl()->getStartFrame();
        }
    }

    if (RP_GET_CORE_CTRL(mUNK_0x80)->downTrigger(WPAD_BUTTON_PLUS)) {
        u8 sel = mUNK_0x7F;
        if (sel != 0) {
            mLayout->unbindAnmObj(mButtons[sel].mAnmObjs[0]);
            mButtons[sel].mAnmObjs[0]->setPlayMode(
                RPSysFrameCtrl::EPlayMode_Reset);
            PlayLastFrame(mButtons[sel].mAnmObjs[0]);
            mLayout->bindAnmObjToPane(mButtons[sel].mAnmObjs[0],
                                      mButtons[sel].mPane, true);
            mUNK_0x84 =
                mButtons[sel].mAnmObjs[0]->getFrameCtrl()->getEndFrame() -
                mButtons[sel].mAnmObjs[0]->getFrameCtrl()->getStartFrame();
            mUNK_0x7F = 0;
        }

        mUNK_0x74 = 4;

        sel = mUNK_0x7F;
        mLayout->unbindAnmObj(mButtons[sel].mAnmObjs[2]);
        mButtons[sel].mAnmObjs[2]->setPlayMode(
            RPSysFrameCtrl::EPlayMode_OneTime);
        mButtons[sel].mAnmObjs[2]->restart();
        mLayout->bindAnmObjToPane(mButtons[sel].mAnmObjs[2],
                                  mButtons[sel].mPane, true);
        mUNK_0x84 = mButtons[sel].mAnmObjs[2]->getFrameCtrl()->getEndFrame() -
                    mButtons[sel].mAnmObjs[2]->getFrameCtrl()->getStartFrame();

        s32 player = mUNK_0x80;
        switch (mUNK_0x7F) {
        case 0: {
            RP_GET_INSTANCE(RPSndAudioMgr)
                ->startPauseSe("RP_COM_GAME_SE_PAUSE_KEY_A_01_CTSP",
                               "RP_COM_GAME_SE_PAUSE_KEY_A_01_TVSP", false,
                               player);
            break;
        }
        case 1: {
            RP_GET_INSTANCE(RPSndAudioMgr)
                ->startPauseSe("RP_COM_GAME_SE_PAUSE_KEY_A_02_CTSP",
                               "RP_COM_GAME_SE_PAUSE_KEY_A_02_TVSP", false,
                               player);
            RP_GET_INSTANCE(RPSndAudioMgr)->stopAllSoundOnPauseOff();
            break;
        }
        case 2: {
            RP_GET_INSTANCE(RPSndAudioMgr)
                ->startPauseSe("RP_COM_GAME_SE_PAUSE_KEY_A_03_CTSP",
                               "RP_COM_GAME_SE_PAUSE_KEY_A_03_TVSP", false,
                               player);
            RP_GET_INSTANCE(RPSndAudioMgr)->stopAllSoundOnPauseOff();
            break;
        }
        }

        mUNK_0x7C = false;
        mState =
            mButtons[mUNK_0x7F].mAnmObjs[2]->getFrameCtrl()->getEndFrame() -
            mButtons[mUNK_0x7F].mAnmObjs[2]->getFrameCtrl()->getStartFrame();
    }

    mUtlCursor->SetTarget(
        mButtons[mUNK_0x7F].mBounding);
    mUtlCursor->SetActive(mUNK_0x7C);
}

void RPSysPauseMenu::updateButtonHighlight() {
    for (int i = 0; i < 3; i++) {
        mButtons[i].mUNK_0x0 = false;
    }

    if (mUNK_0x79) {
        mUNK_0x7F = mUNK_0x7D;
        mButtons[mUNK_0x7D].mUNK_0x0 = true;

        if (mButtons[mUNK_0x7F].mUNK_0x1 != true) {
            RP_GET_CORE_CTRL(mUNK_0x80)
                ->rumble(RPSysCoreController::ERumblePattern_Cursor);
            RP_GET_INSTANCE(RPSndAudioMgr)
                ->startSystemSe("RP_COM_GAME_SE_PAUSE_Cursor_01");
        }

        mUNK_0x7C = false;
    } else if (mUNK_0x7B) {
        if (mUNK_0x7C) {
            if (mUNK_0x7F != mUNK_0x7E) {
                mUNK_0x7F = mUNK_0x7E;
                RP_GET_INSTANCE(RPSndAudioMgr)
                ->startSystemSe("RP_COM_GAME_SE_PAUSE_Cursor_01");
            }
        } else {
            mUNK_0x7C = true;
            RP_GET_INSTANCE(RPSndAudioMgr)
                ->startSystemSe("RP_COM_GAME_SE_PAUSE_Cursor_01");
        }
    }

    if (mUNK_0x7C) {
        mButtons[mUNK_0x7F].mUNK_0x0 = true;
    }

    for (u8 i = 0; i < 3; i++) {
        if (mButtons[i].mUNK_0x1 != mButtons[i].mUNK_0x0) {
            u8 on = mButtons[i].mUNK_0x0;
            StartHighlightAnm(mLayout, mButtons[i].mAnmObjs[on], on);

            mLayout->bindAnmObjToPane(mButtons[i].mAnmObjs[on],
                                      mButtons[i].mPane, true);
            mUNK_0x84 =
                mButtons[i].mAnmObjs[on]->getFrameCtrl()->getEndFrame() -
                mButtons[i].mAnmObjs[on]->getFrameCtrl()->getStartFrame();
        }

        mButtons[i].mUNK_0x1 = mButtons[i].mUNK_0x0;
    }
}

// Stand-in (dead-stripped by the linker) placing the decide sounds before the cursor sound in the string pool
extern "C" void PauseMenu_poolOrderDecideSe() {
    RP_GET_INSTANCE(RPSndAudioMgr)
        ->startPauseSe("RP_COM_GAME_SE_PAUSE_KEY_A_01_CTSP",
                       "RP_COM_GAME_SE_PAUSE_KEY_A_01_TVSP", false, 0);
    RP_GET_INSTANCE(RPSndAudioMgr)
        ->startPauseSe("RP_COM_GAME_SE_PAUSE_KEY_A_02_CTSP",
                       "RP_COM_GAME_SE_PAUSE_KEY_A_02_TVSP", false, 0);
    RP_GET_INSTANCE(RPSndAudioMgr)
        ->startPauseSe("RP_COM_GAME_SE_PAUSE_KEY_A_03_CTSP",
                       "RP_COM_GAME_SE_PAUSE_KEY_A_03_TVSP", false, 0);
}

void RPSysPauseMenu::resetButtonAnims() {
    for (u8 i = 0; i < 3; i++) {
        u8 on = i == mUNK_0x7F;
        mButtons[i].mUNK_0x0 = on;

        mLayout->unbindAnmObj(mButtons[i].mAnmObjs[on]);
        mButtons[i].mAnmObjs[on]->resetFrame();

        if (on == true) {
            mButtons[i].mAnmObjs[on]->setPlayMode(
                RPSysFrameCtrl::EPlayMode_OneTime);
            PlayLastFrame(mButtons[i].mAnmObjs[on]);
            mButtons[i].mAnmObjs[on]->stop();
        } else {
            mButtons[i].mAnmObjs[on]->setPlayMode(
                RPSysFrameCtrl::EPlayMode_Reset);
            mButtons[i].mAnmObjs[on]->setFrame(0.0f);
            mButtons[i].mAnmObjs[on]->stop();
        }

        mLayout->bindAnmObjToPane(mButtons[i].mAnmObjs[on], mButtons[i].mPane,
                                  true);
    }
}

RPSysPauseMenu::RPSysPauseMenu() {
    mUNK_0x74 = 0;
    mResAccessor = NULL;
    mLayout = NULL;
    mPane_0x14 = NULL;

    for (u8 i = 0; i < 3; i++) {
        mButtons[i].mPane = NULL;
        mButtons[i].mBounding = NULL;
        mButtons[i].mTextBox = NULL;

        for (u8 j = 0; j < 3; j++) {
            mButtons[i].mAnmObjs[j] = NULL;
        }
    }

    mUtlCursor = new RPCPadCursor();
}

RPSysPauseMenu::~RPSysPauseMenu() {}

// Stand-in (dead-stripped by the linker) creating the int-to-float constant before 1.0f and 0.0f
extern "C" f32 PauseMenu_poolOrderS16(s16 x) {
    return x;
}
