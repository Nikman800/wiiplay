// Player count selection and remote entry (RPPlayerNumSelect). Unit 0x801D4AA8-0x801D660C.
// Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPPlayerNumSelect.h>
#include <Pack/RPKernel/RPSysSystemWinMgr.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysController.h>
#include <Pack/RPKernel/RPSysCursorDrawMgr.h>
#include <Pack/RPKernel/RPSysStringUtility.h>
#include <Pack/RPParty/RPPartyCommon/RPCPadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPMenuFrame.h>
#include <Pack/RPSystem/RPSysPlayerMgr.h>
#include <Pack/RPSystem/RPSysProjectLocal.h>
#include <Pack/RPSystem/RPSysQueuedScene.h>

#include <egg/core.h>
#include <egg/gfxe/eggScreen.h>

namespace {

// The button reached from each button with the D-pad (up, down, left, right), per mode
const s32 sCursorMoves[2][6][4] = {
    {
        {0, 0, 1, 0},
        {0, 4, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {4, 4, 4, 1},
        {5, 5, 5, 5},
    },
    {
        {0, 1, 2, 0},
        {1, 1, 3, 0},
        {0, 3, 2, 2},
        {1, 4, 3, 2},
        {4, 4, 4, 3},
        {5, 5, 5, 5},
    },
};
inline bool IsPlayerNumBelowMin(const RPPlayerNumSelect* pSelect) {
    if (RPSysProjectLocal::instance()->getPack() == 0) {
        if (RPSysQueuedScene::instance()->getNextSceneID() == 3 ||
            RPSysQueuedScene::instance()->getNextSceneID() == 7) {
            return pSelect->mPlayerNum < 2;
        } else {
            return pSelect->mPlayerNum < 4;
        }
    } else {
        return pSelect->mPlayerNum < 2;
    }
}

inline bool IsSelectedButtonEnabled(const RPPlayerNumSelect* pSelect) {
    if (pSelect->mState == 4) {
        return IsPlayerNumBelowMin(pSelect);
    }
    return true;
}

inline bool IsButtonEnabled(const RPPlayerNumSelect* pSelect, s32 button) {
    if (button == 4) {
        return IsPlayerNumBelowMin(pSelect);
    }
    return true;
}

enum {
    SE_DECIDE,
    SE_NG,
    SE_WIN_CLOSE,
    SE_SEARCH,
    SE_WIN_OPEN,
};

// The speaker flag goes through an inline: a direct getSpeakerFlag argument swaps the flag and
// sound-id registers of the inlined startSystemSeWithRemote
inline u32 SpeakerFlag() {
    return RPSysPlayerMgr::instance()->getSpeakerFlag(-1);
}

inline void PlaySe(s32 se) {
    switch (se) {
    case SE_DECIDE:
        RPSndAudioMgr::instance()->startSystemSeWithRemote(
            "RP_COMMON_SE_KEY_A_02_CTSP", SpeakerFlag());
        RPSndAudioMgr::instance()->startSystemSeWithRemote(
            "RP_COMMON_SE_KEY_A_02_TVSP", SpeakerFlag());
        break;
    case SE_NG:
        RPSndAudioMgr::instance()->startSystemSeWithRemote(
            "RP_COMMON_SE_KEY_NG_01_CTSP", SpeakerFlag());
        RPSndAudioMgr::instance()->startSystemSeWithRemote(
            "RP_COMMON_SE_KEY_NG_01_TVSP", SpeakerFlag());
        break;
    case SE_WIN_CLOSE:
        RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_WIN_CLOSE_01");
        break;
    case SE_SEARCH:
        RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_SEARCH_01");
        break;
    case SE_WIN_OPEN:
        RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_WIN_OPEN_01");
        break;
    }
}

inline void PlayButtonSe(const nw4r::lyt::Pane* pPane, bool second) {
    nw4r::math::VEC2 pos(pPane->GetGlobalMtx()._03 / (f32)(u16)(EGG::Screen::GetSizeXMax() / 2),
                         pPane->GetGlobalMtx()._13 / (f32)(u16)(EGG::Screen::GetSizeYMax() / 2));
    if (second) {
        RPSndAudioMgr::instance()->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_02", pos, 0.5f, 0);
    } else {
        RPSndAudioMgr::instance()->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_01", pos, 0.5f, 0);
    }
}

inline void PlayCursorSe(s32 button, const nw4r::math::VEC2& rPos) {
    if (button == 4) {
        RPSndAudioMgr::instance()->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_02", rPos, 0.5f, 0);
    } else {
        RPSndAudioMgr::instance()->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_01", rPos, 0.5f, 0);
    }
}

} // namespace

RPPlayerNumSelect::RPPlayerNumSelect(RPMenuFrame* pFrame, RPCPadCursor* pCursor) {
    mPhase = -1;
    mpMessage = NULL;
    mpFrame = pFrame;
    mpCursor = pCursor;

    u8 entryMode;
    if (RPSysProjectLocal::instance()->getPack() == 0 &&
        RPSysQueuedScene::instance()->getNextSceneID() != 3 &&
        RPSysQueuedScene::instance()->getNextSceneID() != 7) {
        entryMode = true;
    } else {
        entryMode = false;
    }
    mEntryMode = entryMode;

    mpLayout = new RPRimoconEntryLyt(mEntryMode);
}

void RPPlayerNumSelect::Load(EGG::Archive* pArchive, RPSysMessage* pMessage) {
    mpMessage = pMessage;
    mpLayout->Load(pArchive);
}

void RPPlayerNumSelect::Reset() {
    mPhase = 0;
    unk8 = 1;
    mPlayerNum = 1;
    mState = 0;
    unk14 = 6;
    mpFrame->SetBackButtonActive(false, -1);
    SetTitle(401);
    mpCursor->Reset();
    mpLayout->Reset();
    mpLayout->SetTexts(mpMessage);
}

// Waits for the "fewer remotes than players" window to close, then decides
inline void CalcFewerConfirm(RPPlayerNumSelect* pSelect) {
    if (RPSysSystemWinMgr::instance()->getResult() == RPSysSystemWinMgr::RESULT_CONTINUE) {
        pSelect->mpCursor->SetActive(0);
        pSelect->mpLayout->SetButtonState(pSelect->mState, 2);
        PlaySe(SE_DECIDE);
        pSelect->mPhase = 9;
    }
}

// Counts the connected remotes in a file-local inline: its locals' new virtual registers give
// StartEntry's loop counter the original's r30 (written in UpdatePlayerNum: r29)
static inline s32 NumSelect_CountRemotes(RPRimoconEntryLyt*& rpLayout) {
    s32 num = 0;
    for (s32 i = 0; i < 4; i++) {
        if (RP_GET_CORE_CTRL(i)->isCoreBaseDevice()) {
            rpLayout->SetRemoteState(i, 1);
            num++;
        } else {
            rpLayout->SetRemoteState(i, 0);
        }
    }
    return num;
}

// Switches from the player count buttons to the remote entry
inline void StartEntry(RPPlayerNumSelect* pSelect) {
    if (pSelect->mpLayout->IsSceneAnimPlaying()) {
        return;
    }

    EGG::Vector2f pos = RPSysCursorDrawMgr::instance()->getActiveCursorPos();
    for (s32 i = 0; i < 5; i++) {
        pSelect->mpLayout->SetButtonState(i, 0);
    }
    pSelect->mState = 5;
    pSelect->mpLayout->SetButtonState(5, 1);
    pSelect->mpCursor->SetActive(!pSelect->mpLayout->IsPointOnButton(pSelect->mState, pos));
    RPSysCoreController::enableConnection();
    pSelect->mPhase = 4;
}

// Counts the connected remotes and passes their states to the layout
inline void UpdatePlayerNum(RPPlayerNumSelect* pSelect) {
    pSelect->unk8 = pSelect->mPlayerNum;
    s32 num = NumSelect_CountRemotes(pSelect->mpLayout);
    pSelect->mPlayerNum = num;
}

void RPPlayerNumSelect::Calculate(bool active) {
    s32 phase = mPhase;
    if (phase != 8 && phase != 9 && phase != 10 && phase != 11) {
        UpdatePlayerNum(this);
    }

    if (mPhase == 0) {
        if (!mpLayout->IsEntryAnimPlaying() && active) {
            bool onBack;
            bool cursorFree;
            cursorFree = true;
            onBack = false;
            CalcPointer(&cursorFree, &onBack);
            if (cursorFree && !onBack) {
                mpCursor->SetActive(1);
            }
            mPhase = 1;
        }
    } else if (active) {
        if (mPhase == 1) {
            bool onBack;
            bool cursorFree;
            cursorFree = true;
            onBack = false;
            CalcPointer(&cursorFree, &onBack);
            CalcCountInput(cursorFree, onBack);
        } else if (mPhase == 2) {
            if (mpLayout->AreButtonAnimsDone()) {
                mpFrame->HideBackButton();
                mpLayout->PlaySceneAnim(1);
                PlaySe(SE_WIN_OPEN);
                mPhase = 3;
            }
        } else if (mPhase == 3) {
            StartEntry(this);
        } else if (mPhase == 4) {
            CalcEntryPointer();
        } else if (mPhase == 5) {
            if (mpLayout->AreButtonAnimsDone()) {
                mpFrame->ShowBackButton();
                mpLayout->PlaySceneAnim(0);
                PlaySe(SE_WIN_CLOSE);
                mPhase = 6;
            }
        } else if (mPhase == 6) {
            StartCountSelect();
        } else if (mPhase == 7) {
            if (RPSysSystemWinMgr::instance()->getResult() == RPSysSystemWinMgr::RESULT_CONTINUE) {
                mPhase = 1;
            }
        } else if (mPhase == 8) {
            CalcFewerConfirm(this);
        } else if (mPhase == 9) {
            if (mpLayout->AreButtonAnimsDone()) {
                mpLayout->PlayEntryOutAnim();
                SetTitle(400);
                mPhase = 10;
            }
        } else if (mPhase == 10) {
            if (!mpLayout->IsEntryAnimPlaying()) {
                mPhase = 11;
            }
        } else if (mPhase == 11) {
        } else if (mPhase == 12) {
            if (mpFrame->IsBackPushDone()) {
                mPhase = 13;
            }
        }
    }

    if (!IsButtonEnabled(this, 4)) {
        mpLayout->ResetButton6Anim();
    }

    mpLayout->Calculate();
    mpCursor->SetTarget(mpLayout->GetButtonPane(mState));
}

void RPPlayerNumSelect::DrawLayout() {
    if (mPhase < 0) {
        return;
    }
    if (mPhase == 11) {
        return;
    }
    mpLayout->Draw();
}

void RPPlayerNumSelect::DrawCursor() {
    if (mPhase < 0) {
        return;
    }
    if (mPhase == 11) {
        return;
    }
    mpCursor->Draw();
}

void RPPlayerNumSelect::CalcEntryPointer() {
    EGG::Vector2f pos = RPSysCursorDrawMgr::instance()->getActiveCursorPos();
    s32 target = -1;

    if (mpLayout->IsPointOnButton(5, pos)) {
        target = 5;
        mpLayout->SetButtonState(5, 1);
        mpCursor->SetActive(0);
    } else if (mpCursor->IsActive()) {
        mpLayout->SetButtonState(5, 1);
    } else {
        mpLayout->SetButtonState(5, 0);
    }

    if (target >= 0 && unk14 != target) {
        RPSysCoreController::getActiveController()->rumble(RPSysCoreController::ERumblePattern(0));
        nw4r::math::VEC2 sePos;
        sePos.x = pos.x / (f32)(u16)(EGG::Screen::GetSizeXMax() / 2);
        sePos.y = pos.y / (f32)(u16)(EGG::Screen::GetSizeYMax() / 2);
        PlayCursorSe(5, sePos);
    }
    unk14 = target;

    if (mPlayerNum > unk8) {
        PlaySe(SE_SEARCH);
    }

    CalcEntryButtons(target >= 0);
}

void RPPlayerNumSelect::StartCountSelect() {
    if (mpLayout->IsSceneAnimPlaying()) {
        return;
    }

    EGG::Vector2f pos = RPSysCursorDrawMgr::instance()->getActiveCursorPos();
    mpLayout->SetButtonState(5, 0);
    mState = 4;
    mpCursor->SetActive(1);
    for (s32 i = 0; i < 5; i++) {
        if (mpLayout->IsPointOnButton(i, pos)) {
            mState = i;
            mpCursor->SetActive(0);
        }
    }

    if (IsSelectedButtonEnabled(this)) {
        mpLayout->SetButtonState(mState, 1);
    }

    mPhase = 1;
}

void RPPlayerNumSelect::CalcPointer(bool* pCursorFree, bool* pOnBack) {
    EGG::Vector2f pos = RPSysCursorDrawMgr::instance()->getActiveCursorPos();
    s32 target = -1;

    for (s32 i = 0; i < 5; i++) {
        if (mpLayout->IsPointOnButton(i, pos) && *pCursorFree) {
            *pCursorFree = false;
            target = i;
            mState = i;
            if (IsButtonEnabled(this, i)) {
                mpLayout->SetButtonState(i, 1);
            }
        } else if (i == mState && mpCursor->IsActive()) {
            if (IsButtonEnabled(this, i)) {
                mpLayout->SetButtonState(i, 1);
            }
        } else {
            mpLayout->SetButtonState(i, 0);
        }
    }

    if (mpFrame->IsPointOnBackButton(pos)) {
        *pCursorFree = false;
        target = 6;
        *pOnBack = true;
        mpFrame->SetBackButtonActive(true, -1);
    } else {
        mpFrame->SetBackButtonActive(false, -1);
    }

    if (!*pCursorFree) {
        mpCursor->SetActive(0);
    }

    if (target >= 0 && target != unk14) {
        RPSysCoreController::getActiveController()->rumble(RPSysCoreController::ERumblePattern(0));
        nw4r::math::VEC2 sePos;
        sePos.x = pos.x / (f32)(u16)(EGG::Screen::GetSizeXMax() / 2);
        sePos.y = pos.y / (f32)(u16)(EGG::Screen::GetSizeYMax() / 2);
        PlayCursorSe(target, sePos);
    }
    unk14 = target;
}

void RPPlayerNumSelect::CalcCountInput(bool cursorFree, bool onBack) {
    const u32 buttons[4] = {0x10008, 0x80002, 0x20004, 0x40001};
    RPSysCoreController* pController = RPSysCoreController::getActiveController();

    for (s32 i = 0; i < 4; i++) {
        u32 hold = pController->getCursorHold();
        if (hold & buttons[i]) {
            if (mpCursor->IsActive()) {
                s32 next = sCursorMoves[mEntryMode][mState][i];
                if (mState != next) {
                    PlayButtonSe(mpLayout->GetButtonPane(mState), next == 4);
                }
                mState = next;
            } else if (cursorFree) {
                mpCursor->SetActive(1);
                if (IsButtonEnabled(this, mState)) {
                    mpLayout->SetButtonState(mState, 1);
                }
                PlayButtonSe(mpLayout->GetButtonPane(mState),
                             mState == 4 ? true : false);
            }
            return;
        }
    }

    if (pController->downTrigger(0x1400)) {
        if (mpFrame->IsBackButtonIdle()) {
            for (s32 i = 0; i < 6; i++) {
                mpLayout->SetButtonState(i, 0);
            }
            mpFrame->PushBackButton(SpeakerFlag());
            mpCursor->SetActive(0);
            mPhase = 12;
        }
    } else if (pController->downTrigger(0x800)) {
        if (!mpCursor->IsActive() && cursorFree) {
            return;
        }

        if (onBack) {
            if (mpFrame->IsBackButtonIdle()) {
                mpFrame->PushBackButton(SpeakerFlag());
                mPhase = 12;
            }
        } else if (mState == 4) {
            if (IsButtonEnabled(this, mState)) {
                mpCursor->SetActive(0);
                mpLayout->SetButtonState(mState, 2);
                PlaySe(SE_DECIDE);
                mPhase = 2;
            } else {
                PlaySe(SE_NG);
            }
        } else if (mState < 4) {
            bool lacking;
            if (RPSysProjectLocal::instance()->getPack() == 0 &&
                (RPSysQueuedScene::instance()->getNextSceneID() == 14 ||
                 RPSysQueuedScene::instance()->getNextSceneID() == 4 ||
                 RPSysQueuedScene::instance()->getNextSceneID() == 10)) {
                lacking = mPlayerNum == 0;
            } else {
                lacking = mPlayerNum < mState + 1;
            }

            if (lacking) {
                RPSysSystemWinMgr* pWinMgr = RPSysSystemWinMgr::instance();
                wchar_t num[8];
                RPSysStringUtility::GetStringHalfSizeNumber(mState + 1, num, 8, false);
                pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 200, NULL, 1, num);
                pWinMgr->setCursors();
                mPhase = 7;
            } else {
                bool fewer;
                if (RPSysProjectLocal::instance()->getPack() == 0) {
                    fewer = mPlayerNum < mState + 1;
                } else {
                    fewer = false;
                }

                if (fewer) {
                    RPSysSystemWinMgr* pWinMgr = RPSysSystemWinMgr::instance();
                    wchar_t selected[8];
                    wchar_t connected[8];
                    RPSysStringUtility::GetStringHalfSizeNumber(mState + 1, selected, 8,
                                                                false);
                    RPSysStringUtility::GetStringHalfSizeNumber(mPlayerNum, connected, 8,
                                                                false);
                    pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT,
                                             (u16)(210 + (mPlayerNum > 1)), NULL, 2,
                                             connected, selected);
                    pWinMgr->setCursors();
                    mPhase = 8;
                } else {
                    mpCursor->SetActive(0);
                    mpLayout->SetButtonState(mState, 2);
                    PlaySe(SE_DECIDE);
                    mPhase = 9;
                }
            }
        }
    }
}

void RPPlayerNumSelect::CalcEntryButtons(bool onButton) {
    const u32 buttons[4] = {0x10008, 0x80002, 0x20004, 0x40001};
    RPSysCoreController* pController = RPSysCoreController::getActiveController();

    for (s32 i = 0; i < 4; i++) {
        u32 hold = pController->getCursorHold();
        if (hold & buttons[i]) {
            if (!onButton && !mpCursor->IsActive()) {
                mpCursor->SetActive(1);
                PlayButtonSe(mpLayout->GetButtonPane(mState), false);
            }
            return;
        }
    }

    if (pController->downTrigger(0x800) &&
        (onButton || mpCursor->IsActive())) {
        mpCursor->SetActive(0);
        mpLayout->SetButtonState(5, 2);
        RPSysCoreController::disableConnection();
        PlaySe(SE_DECIDE);
        mPhase = 5;
    }
}

void RPPlayerNumSelect::SetTitle(u32 msgId) {
    if (RPSysProjectLocal::instance()->getPack() == 0) {
        s32 scene = RPSysQueuedScene::instance()->getNextSceneID();
        if (scene == 6) {
            mpFrame->RequestCaptionChange(0x192, FALSE, mpMessage, 0x192, 0, 0);
        } else if (scene == 3) {
            mpFrame->RequestCaptionChange(0x193, FALSE, mpMessage, 0x193, 0, 0);
        } else if (scene == 4) {
            mpFrame->RequestCaptionChange(0x194, FALSE, mpMessage, 0x194, 0, 0);
        } else if (scene == 14) {
            mpFrame->RequestCaptionChange(0x195, FALSE, mpMessage, 0x195, 0, 0);
        } else if (scene == 7) {
            mpFrame->RequestCaptionChange(0x196, FALSE, mpMessage, 0x196, 0, 0);
        } else if (scene == 10) {
            mpFrame->RequestCaptionChange(0x197, FALSE, mpMessage, 0x197, 0, 0);
        } else if (scene == 11) {
            mpFrame->RequestCaptionChange(0x198, FALSE, mpMessage, 0x198, 0, 0);
        }
    } else {
        mpFrame->RequestCaptionChange(msgId, FALSE, mpMessage, msgId, 0, 0);
    }
}

// STAND-IN: dead-stripped; compiled first (defined last), it orders the nine SE-name strings in
// .rodata (0x80319E58..0x80319F47: KEY_CURSOR_02, KEY_CURSOR_01, ..., WIN_OPEN_01) as in the
// original. Without it the strings are reordered and five Calc* functions' code changes.
namespace {
void SePoolOrder(nw4r::math::VEC2& rPos) {
    RPSndAudioMgr::instance()->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_02", rPos, 0.5f, 0);
    RPSndAudioMgr::instance()->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_01", rPos, 0.5f, 0);
    RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_KEY_A_02_CTSP");
    RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_KEY_A_02_TVSP");
    RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_KEY_NG_01_CTSP");
    RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_KEY_NG_01_TVSP");
    RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_WIN_CLOSE_01");
    RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_SEARCH_01");
    RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_WIN_OPEN_01");
}
} // namespace
