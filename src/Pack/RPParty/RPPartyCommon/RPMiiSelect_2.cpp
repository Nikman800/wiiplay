// Mii selection (RPMiiSelect): source tabs, pages of Mii faces, pointer and D-pad input.
// Unit 0x801D80B0-0x801DB3B8. Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPMiiSelect.h>

#include <Pack/RPParty/RPPartyCommon/RPMenuFrame.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysController.h>
#include <Pack/RPKernel/RPSysCursorDrawMgr.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPKernel/RPSysSystemWinMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPCPadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPKokeshiMgr.h>
#include <Pack/RPSystem/RPSysPlayerMgr.h>
#include <Pack/RPSystem/RPSysProjectLocal.h>
#include <Pack/RPSystem/RPSysQueuedScene.h>
#include <Pack/RPSystem/RPSysSceneMgr.h>

#include <egg/core.h>

#pragma auto_inline off

namespace {

// The item reached from each item with the D-pad (up, down, right, left), per mode
const s32 sCursorMoves[3][RPMiiSelectLyt::ITEM_NUM][4] = {
    {
        {0, 1, 5, 0},
        {1, 2, 5, 0},
        {2, 2, 5, 1},
        {3, 3, 3, 3},
        {4, 4, 4, 4},
        {0, 6, 10, 4},
        {0, 7, 11, 5},
        {0, 8, 12, 6},
        {0, 9, 13, 7},
        {0, 3, 14, 8},
        {5, 11, 10, 4},
        {6, 12, 11, 10},
        {7, 13, 12, 11},
        {8, 14, 13, 12},
        {9, 3, 14, 13},
    },
    {
        {0, 1, 6, 0},
        {1, 2, 6, 0},
        {2, 2, 6, 1},
        {3, 3, 3, 3},
        {4, 4, 4, 4},
        {1, 6, 10, 5},
        {1, 7, 11, 5},
        {1, 8, 12, 6},
        {1, 9, 13, 7},
        {1, 9, 14, 8},
        {5, 11, 10, 10},
        {6, 12, 11, 10},
        {7, 13, 12, 11},
        {8, 14, 13, 12},
        {9, 14, 14, 13},
    },
    {
        {0, 1, 5, 0},
        {1, 2, 5, 0},
        {2, 2, 5, 1},
        {3, 3, 3, 3},
        {4, 4, 4, 4},
        {2, 6, 10, 5},
        {2, 7, 11, 5},
        {2, 8, 12, 6},
        {2, 9, 13, 7},
        {2, 9, 14, 8},
        {5, 11, 10, 10},
        {6, 12, 11, 10},
        {7, 13, 12, 11},
        {8, 14, 13, 12},
        {9, 14, 14, 13},
    },
};


inline void ResetPage(RPMiiSelect::Page* pPages, s32 i) {
    pPages[i].mLoaded = false;
    pPages[i].mFirst = 0;
    pPages[i].mTotal = 0;
    pPages[i].mPage = 0;
    pPages[i].mPageNum = 1;
    pPages[i].mCount = 0;
}

// The speaker flag goes through an inline: a direct getSpeakerFlag argument swaps the flag and
// sound-id registers of the inlined startSystemSeWithRemote
inline u32 SpeakerFlag(s32 player) {
    return RPSysPlayerMgr::instance()->getSpeakerFlag(player);
}

} // namespace

inline s32 RPMiiSelect::GetRemoteSource() {
    return RP_GET_CORE_CTRL(mChannel)->getChannelID() + 2;
}

inline s32 RPMiiSelect::GetSource() {
    switch (mMode) {
    case EMode_Plaza:
        return 0;
    case EMode_Guest:
        return 1;
    case EMode_Remote:
        return GetRemoteSource();
    }
    return 1;
}

inline bool RPMiiSelect::IsFirstPage() {
    if (mMode == EMode_Plaza) {
        return mpPages[mMode].mPage == 0;
    }
    return true;
}

inline bool RPMiiSelect::IsLastPage() {
    if (mMode == EMode_Plaza) {
        return mpPages[mMode].mPage >= mpPages[mMode].mPageNum - 1;
    }
    return true;
}

inline bool RPMiiSelect::IsMiiTaken(s32 item) {
    if (item >= 5 && item < 15) {
        s32 source = GetSource();
        s32 idx = GetMiiIndexForItem(source);
        return mpKokeshiMgr->IsInPlayers(source, idx, (u16)mPlayer);
    }
    return false;
}

// File-local, so its literal joins the file's wide-string pool and is shared with CalcInput's L""
// (a literal in an inline member gets its own weak @STRING@ object)
static inline const wchar_t* MiiSelect_EmptyName() {
    return L"";
}

inline void RPMiiSelect::UpdateCursors() {
    if (mCursor >= 0 && mCursor <= 2) {
        mpWCursor->SetActive(1);
    } else {
        mpWCursor->SetActive(0);
    }

    if (mCursor >= 5 && mCursor <= 14) {
        s32 source = GetSource();
        mpFrame->SetPlayerNameText(mpMessage, 0x1C7, 0,
                                   mpKokeshiMgr->GetMiiName(source, GetMiiIndexForItem(source)));
    } else {
        mpFrame->SetPlayerNameText(mpMessage, 0x1C7, 0, MiiSelect_EmptyName());
    }
}

inline void RPMiiSelect::ClearItems() {
    for (s32 i = 0; i < RPMiiSelectLyt::ITEM_NUM; i++) {
        mpLayout->SetItemState(i, 0, false);
    }
}

RPMiiSelect::RPMiiSelect(RPMenuFrame* pFrame, RPCPadCursor* pCPadCursor, RPWCursor* pWCursor,
                         RPKokeshiMgr* pKokeshiMgr) {
    mpMessage = NULL;
    mState = -1;
    mpKokeshiMgr = pKokeshiMgr;
    mpFrame = pFrame;
    mpCPadCursor = pCPadCursor;
    mpWCursor = pWCursor;
    mpPages = new Page[6];
    mpLayout = new RPMiiSelectLyt();
    InitCategoryPages(0, 0);
}

void RPMiiSelect::Load(EGG::Archive* pArchive, RPSysMessage* pMessage) {
    mpMessage = pMessage;
    mpLayout->Load(pArchive);
}

// The Plaza page set up in an inline: its locals' new virtual registers give the original's r6/r7
// (written in Start, the source index and its *0x18 offset swap). The references keep the member
// reloads.
static inline void MiiSelect_InitPlazaPage(RPMiiSelect::Page*& rpPages,
                                           RPKokeshiMgr*& rpKokeshiMgr) {
    RPMiiSelect::EMode source = RPMiiSelect::EMode_Plaza;
    rpPages[source].mLoaded = true;
    rpPages[source].mFirst = 0;
    rpPages[source].mTotal = rpKokeshiMgr->mHeadNum[source];
    rpPages[source].mPage = 0;
    rpPages[source].mPageNum = (rpPages[source].mTotal - 1) / 10 + 1;
    rpPages[source].mCount = rpPages[source].mTotal < 10 ? rpPages[source].mTotal : 10;
}

void RPMiiSelect::Start(s32 player) {
    mPlayer = player;
    mChannel = RPSysPlayerMgr::instance()->getChannel(player);

    MiiSelect_InitPlazaPage(mpPages, mpKokeshiMgr);

    EMode source;
    source = EMode_Guest;
    mpPages[source].mLoaded = true;
    mpPages[source].mFirst = 0;
    mpPages[source].mTotal = mpKokeshiMgr->mHeadNum[source];
    mpPages[source].mPage = 0;
    mpPages[source].mPageNum = 1;
    mpPages[source].mCount = mpKokeshiMgr->mHeadNum[source];

    InitCategory();
    mState = 0;
    mMessageType = 0;
    mLastPointed = 15;
    mpFrame->SetBackButtonActive(false, -1);
    mpCPadCursor->Reset();
    mpWCursor->Reset();
    mpLayout->Reset();
    mpLayout->SetPlayerTitle(mpMessage, mPlayer, RPSysPlayerMgr::instance()->getPlayerNum() == 1);
    mpLayout->SetItemState(mMode, 2, true);
    if (mpPages[GetSource()].mPageNum > 1) {
        mpLayout->SetArrowState(1, 1, false);
    }
    mpLayout->SetButtonTexts(mpMessage);
    AttachPageKokeshi(0);
    mpKokeshiMgr->SetVisible(true);
    mpKokeshiMgr->ApplyPlayerHighlights(mPlayer);
}

void RPMiiSelect::Calculate(bool active) {
    if (mState == 0) {
        CalcEnter(active);
    } else if (active) {
        if (mState == 1) {
            bool noHit[2];
            noHit[0] = true;
            noHit[1] = false;
            CalcPointer(&noHit[0], &noHit[1]);
            CalcInput(noHit[0], noHit[1]);
        } else if (mState == 2) {
            if (mpLayout->AreSelectAnimsDone()) {
                mpLayout->SetItemState(mCursor, 2, false);
                if (mCursor == 3 || mCursor == 4) {
                    PlayItemSe(mCursor, NULL);
                }
                mState = 3;
            }
        } else if (mState == 3) {
            StartPageScroll();
        } else if (mState == 4) {
            CalcTabChange();
        } else if (mState == 5) {
            if (mpLayout->AreDecideAnimsDone()) {
                mState = 1;
            }
        } else if (mState == 6) {
            if (mpLayout->AreDecideAnimsDone()) {
                mpKokeshiMgr->SetVisible(false);
                if (mpPages[GetSource()].mPageNum == 1) {
                    mpLayout->SetArrowState(1, 0, true);
                }
                if (mpPages[GetSource()].mPageNum == 1) {
                    mpLayout->SetArrowState(1, 0, true);
                }
                mpLayout->SetArrowState(1, 0, false);
                mpLayout->SetArrowState(0, 0, false);
                mState = 7;
            }
        } else if (mState == 7) {
            CalcFadeOut();
        } else if (mState == 8) {
            if (!mpLayout->IsMessageAnimPlaying()) {
                mpLayout->PlayMessageMoveAnim();
                if (RP_GET_CORE_CTRL(mChannel)->isCoreBaseDevice()) {
                    mpKokeshiMgr->ReleaseRemoteMii(RP_GET_CORE_CTRL(mChannel)->getChannelID());
                }
                mState = 9;
            }
        } else if (mState == 9) {
            s32 chan = RP_GET_CORE_CTRL(mChannel)->getChannelID();
            if (!RP_GET_CORE_CTRL(mChannel)->isCoreBaseDevice() ||
                mpKokeshiMgr->IsRemoteMiiReady(chan)) {
                mpLayout->PlayMessageAnim(0);
                mState = 10;
            }
        } else if (mState == 10) {
            if (!mpLayout->IsMessageAnimPlaying()) {
                s32 chan = RP_GET_CORE_CTRL(mChannel)->getChannelID();
                if (RP_GET_CORE_CTRL(mChannel)->isCoreBaseDevice() &&
                    !mpKokeshiMgr->IsRemoteLoadError(chan)) {
                    InitCategoryPages(1, chan);
                    s32 source = chan + 2;
                    if (mpPages[source].mTotal > 0) {
                        mState = 12;
                    } else {
                        ShowMessage(2);
                    }
                } else {
                    ShowMessage(4);
                }
            }
        } else if (mState == 12) {
            StartPage();
        } else if (mState == 13) {
            CalcFadeIn();
        } else if (mState == 14) {
            if (mpLayout->AreDecideAnimsDone()) {
                mpKokeshiMgr->SetVisible(false);
                mpFrame->HideName();
                mpLayout->PlaySceneOutAnim();
                mState = 15;
            }
        } else if (mState == 15) {
            if (!mpLayout->IsSceneAnimPlaying()) {
                mState = 16;
            }
        } else if (mState == 16) {
        } else if (mState == 17) {
            if (mpFrame->IsBackPushDone()) {
                mpKokeshiMgr->SetVisible(false);
                mpFrame->HideName();
                mpLayout->PlaySceneOutAnim();
                mState = 18;
            }
        } else if (mState == 18) {
            if (!mpLayout->IsSceneAnimPlaying()) {
                mState = 19;
            }
        } else if (mState == 19) {
        } else if (mState == 20) {
            s32 result = RPSysSystemWinMgr::instance()->getResult();
            if (mMessageType == 2 || mMessageType == 4) {
                if (result == 6) {
                    OnMessageResult(true);
                }
            } else if (result == 4 || result == 2) {
                OnMessageResult(true);
            } else if (result == 5 || result == 3) {
                OnMessageResult(false);
            }
        }
    }

    mpLayout->Calculate();
    if (mLastFace >= 0 && mLastFace < 15) {
        mpCPadCursor->SetTarget(mpLayout->GetItemPane(mLastFace));
    }
    if (mCursor >= 0 && mCursor <= 2) {
        mpWCursor->SetTarget(mpLayout->GetItemPane(mCursor));
    }
}

void RPMiiSelect::DrawLayout() {
    if (mState >= 0 && mState != 16 && mState != 19) {
        mpLayout->Draw();
    }
}

void RPMiiSelect::DrawOverlay() {
    if (mState >= 0 && mState != 16 && mState != 19) {
        mpLayout->DrawBackLayer();
        mpWCursor->Draw();
        mpCPadCursor->Draw();
    }
}

RPMiiSelect::Selection RPMiiSelect::GetSelectedMii() {
    Selection sel;
    sel.mSource = GetSource();
    sel.mIndex = GetMiiIndexForItem(sel.mSource);
    return sel;
}

void RPMiiSelect::CalcEnter(bool active) {
    bool noHit = true;
    bool onBack = false;
    CalcPointer(&noHit, &onBack);

    bool showLater = false;
    if (RPSysProjectLocal::instance()->getPack() == 0 &&
        RPSysQueuedScene::instance()->getNextSceneID() == 11) {
        showLater = true;
    } else {
        mpFrame->ShowName(mPlayer);
    }

    if (!mpLayout->IsSceneAnimPlaying() && active) {
        if (noHit && !onBack) {
            mpCPadCursor->SetActive(1);
        }
        if (showLater) {
            mpFrame->ShowName(mPlayer);
        }
        mState = 1;
    }
}

void RPMiiSelect::StartPageScroll() {
    if (!mpLayout->AreDecideAnimsDone()) {
        return;
    }

    bool wasFirst = IsFirstPage();
    bool wasLast = IsLastPage();
    if (mCursor == 3) {
        mpLayout->PlayFaceAllAnim(1);
        AttachPageKokeshi(1);
        if (mpPages[GetSource()].mPageNum > 1) {
            if (wasFirst) {
                mpLayout->SetArrowState(0, 1, false);
            }
            if (IsLastPage()) {
                mpLayout->SetArrowState(1, 0, false);
            }
        }
    } else {
        mpLayout->PlayFaceAllAnim(0);
        AttachPageKokeshi(-1);
        if (mpPages[GetSource()].mPageNum > 1) {
            if (wasLast) {
                mpLayout->SetArrowState(1, 1, false);
            }
            if (IsFirstPage()) {
                mpLayout->SetArrowState(0, 0, false);
            }
        }
    }
    mState = 4;
}

void RPMiiSelect::CalcTabChange() {
    if (mpLayout->IsFaceAllAnimPlaying()) {
        return;
    }

    mpLayout->SetFaceAllAnimFrame(1);
    AttachPageKokeshi(0);
    EGG::Vector2f pos = RPSysCursorDrawMgr::instance()->getPlayerCursorPos(
        static_cast<RPSysCursorDrawMgr::EPlayer>(mPlayer));
    ClearItems();
    mLastFace = mCursor = GetItemAfterPageChange();
    mpCPadCursor->SetActive(1);
    for (s32 i = 0; i < RPMiiSelectLyt::ITEM_NUM; i++) {
        if (IsItemEnabled(i) && mpLayout->IsPointOnItem(i, pos)) {
            mCursor = i;
            mpCPadCursor->SetActive(0);
        }
    }
    if (!IsMiiTaken(mCursor)) {
        mpLayout->SetItemState(mCursor, 1, false);
    }
    if (mCursor != 3 && mCursor != 4) {
        mLastFace = mCursor;
    }
    mState = 1;
}

void RPMiiSelect::CalcFadeOut() {
    if (mpKokeshiMgr->GetAlpha() != 0) {
        return;
    }

    for (s32 i = 0; i < 6; i++) {
        mpKokeshiMgr->DetachSource(i);
    }
    ClearItems();

    if (mCursor == 0) {
        if (mpPages[mCursor].mTotal > 0) {
            mState = 12;
        } else {
            ShowMessage(3);
        }
    } else if (mCursor == 1) {
        mState = 12;
    } else {
        s32 source = GetRemoteSource();
        if (mpPages[source].mLoaded) {
            if (mpPages[source].mTotal > 0) {
                mState = 12;
            } else {
                ShowMessage(2);
            }
        } else {
            mpLayout->PlayMessageAnim(1);
            mState = 8;
        }
    }
}

void RPMiiSelect::StartPage() {
    AttachPageKokeshi(0);
    mpKokeshiMgr->SetVisible(true);
    if (!IsFirstPage()) {
        mpLayout->SetArrowState(0, 1, false);
    }
    if (!IsLastPage()) {
        mpLayout->SetArrowState(1, 1, false);
    }

    if (mpPages[GetSource()].mTotal > 0) {
        mLastFace = mCursor = GetFirstFreeFace();
    } else {
        mLastFace = mCursor = mMode;
        mpLayout->SetItemState(mMode, 1, false);
        for (s32 i = 5; i < RPMiiSelectLyt::ITEM_NUM; i++) {
            mpLayout->SetItemState(i, 0, false);
        }
    }
    mState = 13;
}

void RPMiiSelect::CalcFadeIn() {
    if (mpKokeshiMgr->GetAlpha() != 255) {
        return;
    }

    GetSource();

    EGG::Vector2f pos = RPSysCursorDrawMgr::instance()->getPlayerCursorPos(
        static_cast<RPSysCursorDrawMgr::EPlayer>(mPlayer));
    mpCPadCursor->SetActive(1);
    for (s32 i = 0; i < RPMiiSelectLyt::ITEM_NUM; i++) {
        if (IsItemEnabled(i) && mpLayout->IsPointOnItem(i, pos)) {
            mCursor = i;
            mpCPadCursor->SetActive(0);
        }
    }
    if (!IsMiiTaken(mCursor)) {
        mpLayout->SetItemState(mCursor, 1, false);
    }
    if (mCursor != 3 && mCursor != 4) {
        mLastFace = mCursor;
    }
    mState = 1;
}

void RPMiiSelect::ShowMessage(s32 type) {
    RPSysSystemWinMgr* pWinMgr = RPSysSystemWinMgr::instance();
    if (type == 1) {
        if (RPSysProjectLocal::instance()->getPack() == 0) {
            pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 0x104, NULL, 0);
            pWinMgr->setDefaultCursor(static_cast<RPSysSystemWinMgr::E_BUTTON>(0));
        } else {
            pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 0x105, NULL, 0);
            pWinMgr->setDefaultCursor(static_cast<RPSysSystemWinMgr::E_BUTTON>(0));
        }
    } else if (type == 2) {
        pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 0xFB, NULL, 0);
    } else if (type == 3) {
        pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 0xFA, NULL, 0);
        pWinMgr->setDefaultCursor(static_cast<RPSysSystemWinMgr::E_BUTTON>(0));
    } else if (type == 4) {
        pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 0xA1, NULL, 0);
    }
    pWinMgr->setPlayerInput(mPlayer);
    mMessageType = type;
    mState = 20;
}

void RPMiiSelect::OnMessageResult(bool yes) {
    if (mMessageType == 1) {
        if (yes) {
            mpLayout->SetItemState(mCursor, 2, false);
            mpCPadCursor->SetActive(0);
            PlayChoiceEffect(mPlayer, mCursor);
            mState = 14;
        } else {
            mState = 1;
        }
    } else if (mMessageType == 2) {
        mState = 12;
    } else if (mMessageType == 3) {
        if (yes) {
            mState = 12;
        } else {
            RPSysSceneMgr::instance()->returnToMenu(false);
            mState = 21;
        }
    } else if (mMessageType == 4) {
        mState = 12;
    }
}

void RPMiiSelect::CalcPointer(bool* pNoHit, bool* pOnBack) {
    EGG::Vector2f pos = RPSysCursorDrawMgr::instance()->getPlayerCursorPos(
        static_cast<RPSysCursorDrawMgr::EPlayer>(mPlayer));
    s32 pointed = -1;

    for (s32 i = 0; i < RPMiiSelectLyt::ITEM_NUM; i++) {
        if (IsItemEnabled(i)) {
            if (mpLayout->IsPointOnItem(i, pos) && *pNoHit) {
                *pNoHit = false;
                pointed = i;
                mCursor = i;
                if (!IsMiiTaken(i)) {
                    mpLayout->SetItemState(i, 1, false);
                }
            } else if (i == mCursor && mpCPadCursor->IsActive()) {
                if (!IsMiiTaken(mCursor)) {
                    mpLayout->SetItemState(i, 1, false);
                }
            } else {
                mpLayout->SetItemState(i, 0, false);
            }
        } else {
            mpLayout->SetItemState(i, 0, false);
        }
    }

    if (mpFrame->IsPointOnBackButton(pos)) {
        *pNoHit = false;
        pointed = 15;
        *pOnBack = true;
        mpFrame->SetBackButtonActive(true, -1);
    } else {
        mpFrame->SetBackButtonActive(false, -1);
    }

    if (!*pNoHit) {
        mpCPadCursor->SetActive(0);
    }
    if (mCursor != 3 && mCursor != 4) {
        mLastFace = mCursor;
    }

    if (pointed >= 0 && pointed != mLastPointed) {
        s32 chan = RPSysCursorDrawMgr::instance()->getPlayerCursorChannel(
            static_cast<RPSysCursorDrawMgr::EPlayer>(mPlayer));
        RP_GET_CORE_CTRL(chan)->rumble(static_cast<RPSysCoreController::ERumblePattern>(0));
        if (mCursor != 3 && mCursor != 4) {
            PlayItemSe(mCursor, &pos);
        } else {
            PlayItemSe(5, &pos);
        }
    }
    mLastPointed = pointed;
}

void RPMiiSelect::CalcInput(bool noHit, bool onBack) {
    const u32 buttons[4] = {0x10008, 0x80002, 0x20004, 0x40001};

    s32 i;
    for (i = 0; i < 4; i++) {
        u32 hold = RP_GET_CORE_CTRL(mChannel)->getCursorHold();
        if (hold & buttons[i]) {
            if (mpCPadCursor->IsActive()) {
                s32 item = sCursorMoves[mMode][mCursor][i];
                if (IsItemEnabled(item)) {
                    if (mCursor != item && item != 3 && item != 4) {
                        PlayItemSe(item, NULL);
                    }
                    if (item == 3 || item == 4) {
                        mCursor = item;
                        for (i = 0; i < RPMiiSelectLyt::ITEM_NUM; i++) {
                        mpLayout->SetItemState(i, 0, false);
                    }
                        mpLayout->SetItemState(mCursor, 1, false);
                        mpCPadCursor->SetActive(0);
                        mState = 2;
                    } else if (mCursor < 3 && item > 4) {
                        mLastFace = mCursor = GetFirstFreeFace();
                    } else {
                        mLastFace = mCursor = item;
                    }
                }
            } else {
                mCursor = mLastFace;
                if (noHit) {
                    mpCPadCursor->SetActive(1);
                    for (i = 0; i < RPMiiSelectLyt::ITEM_NUM; i++) {
                        mpLayout->SetItemState(i, 0, false);
                    }
                    mpLayout->SetItemState(mLastFace, 1, false);
                    PlayItemSe(mLastFace, NULL);
                }
            }
            UpdateCursors();
            return;
        }
    }

    if ((!noHit && !onBack) || mpCPadCursor->IsActive()) {
        UpdateCursors();
    } else {
        mpWCursor->SetActive(0);
        mpFrame->SetPlayerNameText(mpMessage, 0x1C7, 0, L"");
    }

    if (RP_GET_CORE_CTRL(mChannel)->downTrigger(0x1400)) {
        if (mpFrame->IsBackButtonIdle()) {
            for (i = 0; i < RPMiiSelectLyt::ITEM_NUM; i++) {
                        mpLayout->SetItemState(i, 0, false);
                    }
            mpFrame->PushBackButton(SpeakerFlag(mPlayer));
            mpCPadCursor->SetActive(0);
            mpWCursor->SetActive(0);
            mState = 17;
        }
        return;
    }

    if (!RP_GET_CORE_CTRL(mChannel)->downTrigger(0x800)) {
        return;
    }

    if (mpCPadCursor->IsActive() || !noHit) {
        if (onBack) {
            if (mpFrame->IsBackButtonIdle()) {
                mpFrame->PushBackButton(SpeakerFlag(mPlayer));
                mpCPadCursor->SetActive(0);
                mState = 17;
            }
        } else if (mCursor < 3) {
            mpLayout->SetItemState(mCursor, 2, false);
            RPSndAudioMgr::instance()->startSystemSeWithRemote(
                "RP_COMMON_SE_KEY_A_03_CTSP", SpeakerFlag(mPlayer));
            RPSndAudioMgr::instance()->startSystemSeWithRemote(
                "RP_COMMON_SE_KEY_A_03_TVSP", SpeakerFlag(mPlayer));
            if (mCursor == mMode) {
                s32 source = GetSource();
                if (mpPages[source].mLoaded) {
                    if (mpPages[source].mTotal == 0) {
                        if (mCursor == 0) {
                            ShowMessage(3);
                        } else {
                            ShowMessage(2);
                        }
                    } else {
                        mState = 5;
                    }
                } else {
                    mpCPadCursor->SetActive(0);
                    mState = 6;
                }
            } else {
                mpLayout->SetItemState(mMode, 0, true);
                mMode = mCursor;
                mpCPadCursor->SetActive(0);
                mState = 6;
            }
        } else if (mCursor < 5) {
            mpLayout->SetItemState(mCursor, 2, false);
            PlayItemSe(mCursor, NULL);
            mState = 3;
        } else if (mCursor < 15) {
            if (mMode == EMode_Plaza) {
                if (IsMiiTaken(mCursor)) {
                    RPSndAudioMgr::instance()->startSystemSeWithRemote(
                        "RP_COMMON_SE_KEY_NG_01_CTSP",
                        SpeakerFlag(mPlayer));
                    RPSndAudioMgr::instance()->startSystemSeWithRemote(
                        "RP_COMMON_SE_KEY_NG_01_TVSP",
                        SpeakerFlag(mPlayer));
                } else {
                    mpLayout->SetItemState(mCursor, 2, false);
                    mpCPadCursor->SetActive(0);
                    PlayChoiceEffect(mPlayer, mCursor);
                    mState = 14;
                }
            } else if (IsMiiTaken(mCursor)) {
                RPSndAudioMgr::instance()->startSystemSeWithRemote(
                    "RP_COMMON_SE_KEY_NG_01_CTSP",
                    SpeakerFlag(mPlayer));
                RPSndAudioMgr::instance()->startSystemSeWithRemote(
                    "RP_COMMON_SE_KEY_NG_01_TVSP",
                    SpeakerFlag(mPlayer));
            } else {
                bool ask;
                if (RPSysProjectLocal::instance()->getPack() == 1 &&
                    RPSysPlayerMgr::instance()->getPlayerNum() == 2 &&
                    RPSysPlayerMgr::instance()->getRemoteNum() == 2) {
                    ask = false;
                } else {
                    ask = true;
                }
                if (ask) {
                    ShowMessage(1);
                } else {
                    mpLayout->SetItemState(mCursor, 2, false);
                    mpCPadCursor->SetActive(0);
                    PlayChoiceEffect(mPlayer, mCursor);
                    mState = 14;
                }
            }
        }
    }

    bool off = onBack || !mpCPadCursor->IsActive();
    if (off) {
        mpWCursor->SetActive(0);
    } else if (mCursor >= 0 && mCursor <= 2) {
        mpWCursor->SetActive(1);
    } else {
        mpWCursor->SetActive(0);
    }
}

void RPMiiSelect::InitCategoryPages(s32 type, s32 chan) {
    if (type != 0) {
        s32 source = chan + 2;
        if (mpPages[source].mLoaded) {
            return;
        }
        mpPages[source].mLoaded = true;
        mpPages[source].mFirst = 0;
        mpPages[source].mTotal = mpKokeshiMgr->mHeadNum[source];
        mpPages[source].mPage = 0;
        mpPages[source].mPageNum = 1;
        mpPages[source].mCount = mpKokeshiMgr->mHeadNum[source];
    } else {
        for (int i = 2; i < 6; i++) {
            mpPages[i].mLoaded = false;
            mpPages[i].mFirst = 0;
            mpPages[i].mTotal = 0;
            mpPages[i].mPage = 0;
            mpPages[i].mPageNum = 1;
            mpPages[i].mCount = 0;
        }
    }
}

void RPMiiSelect::AttachPageKokeshi(s32 dir) {
    if (mMode == EMode_Plaza) {
        Page* pPage = &mpPages[mMode];
        s32 end;
        s32 slot = 0;
        s32 first = pPage->mFirst;
        s32 count = pPage->mCount;
        s32 total = pPage->mTotal;
        if (dir == -1) {
            end = first + count;
            first -= 10;
            pPage->mFirst = first;
            mpPages[mMode].mCount = 10;
            mpPages[mMode].mPage--;
        } else if (dir == 1) {
            if (first + 20 < total) {
                total = first + 20;
            }
            pPage->mFirst = first + 10;
            end = total;
            mpPages[mMode].mCount = (total - 10) - first;
            mpPages[mMode].mPage++;
        } else {
            end = first + count;
            for (s32 i = 0; i < first; i++) {
                mpKokeshiMgr->DetachKokeshi(mMode, i);
            }
            for (s32 i = end; i < total; i++) {
                mpKokeshiMgr->DetachKokeshi(mMode, i);
            }
        }
        for (; first < end; first++) {
            nw4r::lyt::Pane* pModel = mpLayout->GetModelPane(slot);
            mpKokeshiMgr->AttachKokeshi(pModel, mpLayout->GetFacePane(slot), mMode, first);
            slot++;
        }
    } else if (mMode == EMode_Guest) {
        mpPages[mMode].mFirst = 0;
        s32 idx = 0;
        for (s32 slot = 1; slot < 4; slot++) {
            nw4r::lyt::Pane* pModel = mpLayout->GetModelPane(slot);
            mpKokeshiMgr->AttachKokeshi(pModel, mpLayout->GetFacePane(slot), mMode, idx);
            idx++;
        }
        for (s32 slot = 6; slot < 9; slot++) {
            nw4r::lyt::Pane* pModel = mpLayout->GetModelPane(slot);
            mpKokeshiMgr->AttachKokeshi(pModel, mpLayout->GetFacePane(slot), mMode, idx);
            idx++;
        }
    } else {
        s32 source;
        switch (mMode) {
        case EMode_Plaza:
            source = 0;
            break;
        case EMode_Guest:
            source = 1;
            break;
        case EMode_Remote:
            source = GetRemoteSource();
            break;
        default:
            source = 1;
            break;
        }
        mpPages[source].mFirst = 0;
        for (s32 i = 0; i < mpPages[source].mTotal; i++) {
            nw4r::lyt::Pane* pModel = mpLayout->GetModelPane(i);
            mpKokeshiMgr->AttachKokeshi(pModel, mpLayout->GetFacePane(i), source, i);
        }
    }
}

void RPMiiSelect::InitCategory() {
    s32 free = 0;
    for (s32 i = 0; i < mPlayer; i++) {
        RPKokeshiMgr::Player player = *mpKokeshiMgr->GetPlayer(i);
        if (player.mSource == 0 || player.mType == 0) {
            free++;
        }
    }
    if (mpPages[0].mTotal > free) {
        mMode = EMode_Plaza;
    } else {
        mMode = EMode_Guest;
    }
    mLastFace = mCursor = GetFirstFreeFace();
}

bool RPMiiSelect::IsItemEnabled(s32 item) {
    if (item == 0) {
        return true;
    }
    if (item == 1) {
        return true;
    }
    if (item == 2) {
        return true;
    }
    if (item < 3) {
        return true;
    }
    if (item == 3) {
        return !IsLastPage();
    }
    if (item == 4) {
        return !IsFirstPage();
    }
    if (mMode == EMode_Plaza) {
        return mpPages[0].mCount + 5 - item > 0;
    }
    if (mMode == EMode_Guest) {
        bool enabled = false;
        if ((item >= 6 && item < 9) || (item >= 11 && item < 14)) {
            enabled = true;
        }
        return enabled;
    }
    return item - 5 < mpPages[GetRemoteSource()].mCount;
}

s32 RPMiiSelect::GetMiiIndexForItem(s32 source) {
    if (source == 1) {
        if (mCursor > 5 && mCursor < 9) {
            return mCursor - 6;
        }
        if (mCursor > 10 && mCursor < 14) {
            return mCursor - 8;
        }
        return -1;
    }
    return mCursor + mpPages[source].mPage * 10 - 5;
}

s32 RPMiiSelect::GetFirstFreeFace() {
    s32 source = GetSource();
    for (s32 i = 0; i < 4; i++) {
        s32 idx = i + mpPages[source].mPage * 10;
        if (i < mpPages[source].mCount && !mpKokeshiMgr->IsInPlayers(source, idx, (u16)mPlayer)) {
            if (source == 1) {
                if (i >= 3) {
                    return 11;
                }
                return i + 6;
            }
            return i + 5;
        }
    }
    s32 face = 5;
    if (source == 1) {
        face = 6;
    }
    return face;
}

s32 RPMiiSelect::GetItemAfterPageChange() {
    if (mCursor == 4) {
        if (mLastFace >= 10 && mLastFace < 14) {
            return 14;
        }
        return 9;
    }
    if (mCursor == 3) {
        if (mLastFace >= 10 && mLastFace <= 14) {
            return mpPages[GetSource()].mCount <= 5 ? 5 : 10;
        }
        return 5;
    }
    return 5;
}

void RPMiiSelect::PlayChoiceEffect(s32 player, s32 item) {
    const char* effectNames[4] = {"Seq_choice_mii_blue", "Seq_choice_mii_red",
                                  "Seq_choice_mii_green", "Seq_choice_mii_yellow"};
    RPSysEffect effect(effectNames[player], 3, 0);
    effect.create();
    EGG::Vector2f pos = mpLayout->GetFacePanePos(item);
    effect.setPos(nw4r::math::VEC3(pos.x, pos.y, 0.0f));
    effect.update();
    RPSndAudioMgr::instance()->startSystemSeWithRemote(
        "RP_COMMON_SE_KEY_A_02_CTSP", SpeakerFlag(mPlayer));
    RPSndAudioMgr::instance()->startSystemSeWithRemote(
        "RP_COMMON_SE_KEY_A_02_TVSP", SpeakerFlag(mPlayer));
}

void RPMiiSelect::PlayItemSe(s32 item, const EGG::Vector2f* pPos) {
    nw4r::math::VEC2 pos;
    if (pPos != NULL) {
        pos.x = pPos->x / (f32)(u16)(EGG::Screen::GetSizeXMax() / 2);
        pos.y = pPos->y / (f32)(u16)(EGG::Screen::GetSizeYMax() / 2);
    } else {
        nw4r::lyt::Pane* pPane = mpLayout->GetItemPane(item);
        // the quotients are named locals stored afterwards (the original's y f2, x f0)
        f32 y = pPane->GetGlobalMtx()._13 / (f32)(u16)(EGG::Screen::GetSizeYMax() / 2);
        f32 x = pPane->GetGlobalMtx()._03 / (f32)(u16)(EGG::Screen::GetSizeXMax() / 2);
        pos.y = y;
        pos.x = x;
    }

    if (item < 3) {
        RPSndAudioMgr::instance()->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_02", pos, 0.5f, 0);
    } else if (item < 5) {
        RPSndAudioMgr::instance()->startSystemSeWithRemote(
            "RP_COMMON_SE_KEY_A_04_CTSP", SpeakerFlag(mPlayer));
        RPSndAudioMgr::instance()->startSystemSeWithRemote(
            "RP_COMMON_SE_KEY_A_04_TVSP", SpeakerFlag(mPlayer));
    } else {
        RPSndAudioMgr::instance()->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_05", pos, 0.5f, 0);
    }
}
