// Mii selection layout (RPMiiSelectLyt, playerSelect.arc). Unit 0x801D660C-0x801D80B0.
// Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPMiiSelect.h>

#include <Pack/RPKernel/RPSysFrameCtrl.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysLytWindow.h>
#include <Pack/RPKernel/RPSysStringUtility.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

namespace {

inline bool IsAnmPlaying(RPSysLytAnmObj* pAnmObj) {
    return pAnmObj->mFrameCtrl.mIsEnabled == 1;
}

inline void PlayFromEnd(RPSysLytAnmObj* pAnmObj) {
    pAnmObj->play(pAnmObj->getFrameMax() - 1.0f);
}

void PlaySceneInAnim(RPMiiSelectLyt* pLyt);
void ResetFaceAllAnim(RPMiiSelectLyt* pLyt);
void ResetArrowAnims(RPMiiSelectLyt* pLyt);
void ApplyBaseLineColor(RPMiiSelectLyt* pLyt, u32 player);

} // namespace

RPMiiSelectLyt::RPMiiSelectLyt() {
    for (s32 i = 0; i < ITEM_NUM; i++) {
        mItemStates[i] = 6;
    }
    mpLayout = NULL;
    for (s32 i = 0; i < ANIM_NUM; i++) {
        mpAnmObjs[i] = NULL;
    }
}

void RPMiiSelectLyt::Load(EGG::Archive* pArchive) {
    RPSysLytResAccessor* pAccessor = RPSysLytResAccessor::create(NULL);
    pAccessor->mountArchive(
        RPSysResourceManager::GetFileFromArchive(pArchive, "playerSelect.arc", NULL), NULL);
    mpLayout = RPSysLayout::create(NULL, pAccessor, "playerSelect_00.brlyt");

    const char* anmNames[ANIM_NUM] = {
        "playerSelect_00_inFrame_00.brlan",       "playerSelect_00_outFrame_00.brlan",
        "playerSelect_00_activeArrow_00.brlan",   "playerSelect_00_activeArrow_00.brlan",
        "playerSelect_00_activePlayer_00.brlan",  "playerSelect_00_activePlayer_00.brlan",
        "playerSelect_00_activePlayer_00.brlan",  "playerSelect_00_activePlayer_00.brlan",
        "playerSelect_00_activePlayer_00.brlan",  "playerSelect_00_activePlayer_00.brlan",
        "playerSelect_00_activePlayer_00.brlan",  "playerSelect_00_activePlayer_00.brlan",
        "playerSelect_00_activePlayer_00.brlan",  "playerSelect_00_activePlayer_00.brlan",
        "playerSelect_00_pushArrow_00.brlan",     "playerSelect_00_pushArrow_00.brlan",
        "playerSelect_00_pushPlayer_00.brlan",    "playerSelect_00_pushPlayer_00.brlan",
        "playerSelect_00_pushPlayer_00.brlan",    "playerSelect_00_pushPlayer_00.brlan",
        "playerSelect_00_pushPlayer_00.brlan",    "playerSelect_00_pushPlayer_00.brlan",
        "playerSelect_00_pushPlayer_00.brlan",    "playerSelect_00_pushPlayer_00.brlan",
        "playerSelect_00_pushPlayer_00.brlan",    "playerSelect_00_pushPlayer_00.brlan",
        "playerSelect_00_moveArrow_00.brlan",     "playerSelect_00_moveArrow_00.brlan",
        "playerSelect_00_inoutArrow_00.brlan",    "playerSelect_00_inoutArrow_00.brlan",
        "playerSelect_00_slidePlayer_00.brlan",   "playerSelect_00_activeButton_00.brlan",
        "playerSelect_00_activeButton_00.brlan",  "playerSelect_00_activeButton_00.brlan",
        "playerSelect_00_pushButton_00.brlan",    "playerSelect_00_pushButton_00.brlan",
        "playerSelect_00_pushButton_00.brlan",    "playerSelect_00_activeButton_01.brlan",
        "playerSelect_00_activeButton_01.brlan",  "playerSelect_00_activeButton_01.brlan",
        "playerSelect_00_pushButton_01.brlan",    "playerSelect_00_pushButton_01.brlan",
        "playerSelect_00_pushButton_01.brlan",    "playerSelect_00_inoutText_00.brlan",
        "playerSelect_00_moveText_00.brlan",      "playerSelect_00_loopMark_00.brlan",
    };
    for (s32 i = 0; i < ANIM_NUM; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, pAccessor, anmNames[i]);
    }
}

void RPMiiSelectLyt::Reset() {
    mpLayout->unbindAllAnmObj();
    PlaySceneInAnim(this);
    ResetFaceAllAnim(this);

    for (s32 i = 0; i < ITEM_NUM; i++) {
        mItemStates[i] = -1;
        SetItemState(i, 0, false);
    }

    ResetArrowAnims(this);
    SetArrowState(0, 0, true);
    SetArrowState(1, 0, true);
    SetArrowBackVisible(0);
}

void RPMiiSelectLyt::Calculate() {
    mpLayout->calc();
}

void RPMiiSelectLyt::Draw() {
    SetArrowBackVisible(2);
    mpLayout->draw();
}

void RPMiiSelectLyt::DrawBackLayer() {
    SetArrowBackVisible(1);
    mpLayout->draw();
    SetArrowBackVisible(0);
}

void RPMiiSelectLyt::SetPlayerTitle(RPSysMessage* pMessage, u32 player, bool cpu) {
    RPSysLytTextBox* pTextBox = mpLayout->findTextBox("T_playerSelect");
    if (cpu) {
        pTextBox->setMessage(pMessage, 600, 1, 0);
    } else {
        wchar_t buffer[8];
        RPSysStringUtility::GetStringHalfSizeNumber(player + 1, buffer, 8, false);
        pTextBox->setMessage(pMessage, 600, 0, 1, buffer);
    }

    RPSysLayout::bindAnmObjToPane(mpAnmObjs[45], mpLayout->findPane("W_baseLine_01"), true);
    mpAnmObjs[45]->restart();

    ApplyBaseLineColor(this, player);
}

void RPMiiSelectLyt::SetButtonTexts(RPSysMessage* pMessage) {
    mpLayout->findTextBox("T_button_00")->setMessage(pMessage, 0x262, 0, 0);
    mpLayout->findTextBox("T_button_01")->setMessage(pMessage, 0x263, 0, 0);
    mpLayout->findTextBox("T_button_02")->setMessage(pMessage, 0x264, 0, 0);
    mpLayout->findTextBox("T_message_00")->setMessage(pMessage, 0x265, 0, 0);
}

void RPMiiSelectLyt::PlaySceneOutAnim() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane("N_scene_00");
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[0], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[1], pPane, true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[1], pPane, false);
    mpAnmObjs[1]->restart();
}

bool RPMiiSelectLyt::IsSceneAnimPlaying() {
    if (IsAnmPlaying(mpAnmObjs[0]) || IsAnmPlaying(mpAnmObjs[1])) {
        return true;
    }
    return false;
}

bool RPMiiSelectLyt::IsPointOnItem(s32 idx, const EGG::Vector2f& rPos) {
    const char* paneNames[ITEM_NUM] = {
        "B_button_00", "B_button_01", "B_button_02", "B_arrowR_00", "B_arrowL_00",
        "B_face_00",   "B_face_01",   "B_face_02",   "B_face_03",   "B_face_04",
        "B_face_05",   "B_face_06",   "B_face_07",   "B_face_08",   "B_face_09",
    };
    return mpLayout->isInsideBounding(
        RPSysLayout::castToBounding(mpLayout->findPane(paneNames[idx])), rPos);
}

void RPMiiSelectLyt::SetItemState(s32 idx, s32 state, bool force) {
    if (idx >= 3) {
        SetFaceState(idx, state);
    } else {
        SetButtonState(idx, state, force);
    }
}

nw4r::lyt::Pane* RPMiiSelectLyt::GetItemPane(s32 idx) {
    const char* paneNames[ITEM_NUM] = {
        "B_button_00", "B_button_01", "B_button_02", "B_arrowR_00", "B_arrowL_00",
        "B_face_00",   "B_face_01",   "B_face_02",   "B_face_03",   "B_face_04",
        "B_face_05",   "B_face_06",   "B_face_07",   "B_face_08",   "B_face_09",
    };
    return mpLayout->findPane(paneNames[idx]);
}

bool RPMiiSelectLyt::AreDecideAnimsDone() {
    for (s32 i = 14; i < 26; i++) {
        if (IsAnmPlaying(mpAnmObjs[i])) {
            return false;
        }
    }
    for (s32 i = 34; i < 37; i++) {
        if (IsAnmPlaying(mpAnmObjs[i])) {
            return false;
        }
    }
    for (s32 i = 40; i < 43; i++) {
        if (IsAnmPlaying(mpAnmObjs[i])) {
            return false;
        }
    }
    return true;
}

bool RPMiiSelectLyt::AreSelectAnimsDone() {
    for (s32 i = 2; i < 14; i++) {
        if (IsAnmPlaying(mpAnmObjs[i])) {
            return false;
        }
    }
    for (s32 i = 31; i < 34; i++) {
        if (IsAnmPlaying(mpAnmObjs[i])) {
            return false;
        }
    }
    for (s32 i = 37; i < 40; i++) {
        if (IsAnmPlaying(mpAnmObjs[i])) {
            return false;
        }
    }
    return true;
}

void RPMiiSelectLyt::SetFaceAllAnimFrame(s32 start) {
    if (start) {
        mpAnmObjs[30]->restart();
        mpAnmObjs[30]->stop();
    } else {
        PlayFromEnd(mpAnmObjs[30]);
        mpAnmObjs[30]->stop();
    }
}

void RPMiiSelectLyt::PlayFaceAllAnim(s32 forward) {
    if (forward) {
        mpAnmObjs[30]->restart();
        mpAnmObjs[30]->setRate(1.0f);
    } else {
        PlayFromEnd(mpAnmObjs[30]);
        mpAnmObjs[30]->setRate(-1.0f);
    }
}

bool RPMiiSelectLyt::IsFaceAllAnimPlaying() {
    return IsAnmPlaying(mpAnmObjs[30]);
}

void RPMiiSelectLyt::SetArrowState(s32 arrow, s32 state, bool force) {
    s32 inout = 29 - arrow;
    f32 frame;
    if (force ||
        (state && (frame = mpAnmObjs[inout]->mFrameCtrl.mFrame,
                   frame > mpAnmObjs[inout]->getFrameMax() - 2.0f)) ||
        (!state && mpAnmObjs[inout]->mFrameCtrl.mFrame < 1.0f)) {
        const char* paneNames[2] = {"P_arrowL_00", "P_arrowR_00"};
        nw4r::lyt::Pane* pPane = mpLayout->findPane(paneNames[arrow]);
        RPSysLayout::unbindAnmObjToPane(mpAnmObjs[3 - arrow], pPane, true);
        RPSysLayout::unbindAnmObjToPane(mpAnmObjs[15 - arrow], pPane, true);
        RPSysLayout::unbindAnmObjToPane(mpAnmObjs[inout], pPane, true);
        RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[inout], pPane, true);

        if (force) {
            if (state) {
                mpAnmObjs[inout]->restart();
                mpAnmObjs[inout]->setRate(-1.0f);
                mpAnmObjs[27 - arrow]->play(-1);
            } else {
                PlayFromEnd(mpAnmObjs[inout]);
                mpAnmObjs[inout]->setRate(1.0f);
            }
        } else {
            if (state) {
                PlayFromEnd(mpAnmObjs[inout]);
                mpAnmObjs[inout]->setRate(-1.0f);
                mpAnmObjs[27 - arrow]->play(-1);
            } else {
                mpAnmObjs[inout]->restart();
                mpAnmObjs[inout]->setRate(1.0f);
            }
        }
        mItemStates[4 - arrow] = 0;
    }
}

void RPMiiSelectLyt::PlayMessageAnim(s32 close) {
    nw4r::lyt::Pane* pPane = mpLayout->findPane("N_message_00");
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[43], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[44], pPane, true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[43], pPane, true);
    if (close) {
        PlayFromEnd(mpAnmObjs[43]);
        mpAnmObjs[43]->setRate(-1.0f);
    } else {
        mpAnmObjs[43]->restart();
        mpAnmObjs[43]->setRate(1.0f);
    }
    mpAnmObjs[44]->stop();
}

void RPMiiSelectLyt::PlayMessageMoveAnim() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane("N_message_00");
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[43], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[44], pPane, true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[44], pPane, true);
    mpAnmObjs[44]->restart();
}

bool RPMiiSelectLyt::IsMessageAnimPlaying() {
    if (IsAnmPlaying(mpAnmObjs[43]) || IsAnmPlaying(mpAnmObjs[44])) {
        return true;
    }
    return false;
}

nw4r::lyt::Pane* RPMiiSelectLyt::GetModelPane(s32 idx) {
    const char* paneNames[20] = {
        "N_3Dmodel_00", "N_3Dmodel_01", "N_3Dmodel_02", "N_3Dmodel_03", "N_3Dmodel_04",
        "N_3Dmodel_05", "N_3Dmodel_06", "N_3Dmodel_07", "N_3Dmodel_08", "N_3Dmodel_09",
        "N_3Dmodel_10", "N_3Dmodel_11", "N_3Dmodel_12", "N_3Dmodel_13", "N_3Dmodel_14",
        "N_3Dmodel_15", "N_3Dmodel_16", "N_3Dmodel_17", "N_3Dmodel_18", "N_3Dmodel_19",
    };
    return mpLayout->findPane(paneNames[idx]);
}

nw4r::lyt::Pane* RPMiiSelectLyt::GetFacePane(s32 idx) {
    const char* paneNames[20] = {
        "N_face_00", "N_face_01", "N_face_02", "N_face_03", "N_face_04",
        "N_face_05", "N_face_06", "N_face_07", "N_face_08", "N_face_09",
        "N_face_10", "N_face_11", "N_face_12", "N_face_13", "N_face_14",
        "N_face_15", "N_face_16", "N_face_17", "N_face_18", "N_face_19",
    };
    return mpLayout->findPane(paneNames[idx]);
}

EGG::Vector2f RPMiiSelectLyt::GetFacePanePos(s32 idx) {
    const char* paneNames[10] = {
        "N_face_00", "N_face_01", "N_face_02", "N_face_03", "N_face_04",
        "N_face_05", "N_face_06", "N_face_07", "N_face_08", "N_face_09",
    };
    u32 face = idx - 5;
    nw4r::math::MTX34 mtx = mpLayout->findPane(paneNames[face])->GetGlobalMtx();
    return EGG::Vector2f(mtx._03, mtx._13);
}

void RPMiiSelectLyt::SetFaceState(s32 idx, s32 state) {
    if (state == mItemStates[idx]) {
        return;
    }

    s32 i = idx - 3;
    const char* paneNames[12] = {
        "P_arrowR_00", "P_arrowL_00", "N_face_00", "N_face_01", "N_face_02", "N_face_03",
        "N_face_04",   "N_face_05",   "N_face_06", "N_face_07", "N_face_08", "N_face_09",
    };
    s32 anims[3] = {i + 2, i + 2, i + 14};

    nw4r::lyt::Pane* pPane = mpLayout->findPane(paneNames[i]);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anims[0]], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anims[1]], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anims[2]], pPane, true);
    RPSysLytAnmObj** ppAnmObj = &mpAnmObjs[anims[state]];
    RPSysLayout::forceBindAnmObjToPane(*ppAnmObj, pPane, true);

    if (state < 2) {
        if (mItemStates[idx] == 2) {
            (*ppAnmObj)->restart();
        } else {
            (*ppAnmObj)->play(-1);
        }

        if (state == 0) {
            (*ppAnmObj)->setRate(-1.0f);
            if (idx == 3) {
                mpAnmObjs[26]->play(-1);
            } else if (idx == 4) {
                mpAnmObjs[27]->play(-1);
            }
        } else if (state == 1) {
            (*ppAnmObj)->setRate(1.0f);
            if (idx == 3) {
                mpAnmObjs[26]->stop();
            } else if (idx == 4) {
                mpAnmObjs[27]->stop();
            }
        }
    } else {
        (*ppAnmObj)->restart();
    }

    mItemStates[idx] = state;
}

void RPMiiSelectLyt::SetButtonState(s32 idx, s32 state, bool force) {
    if (!force) {
        s32 cur = mItemStates[idx];
        if (state == cur) {
            return;
        }
        if (state + 3 == cur) {
            return;
        }
    }

    bool rewind = false;
    bool toEnd = false;
    if (force && state == 2) {
        rewind = true;
        state = 3;
    } else if (force && state == 0) {
        rewind = true;
        state = 0;
    } else if (state == 0 && mItemStates[idx] == 2) {
        rewind = true;
        state = 3;
    } else if (state == 1 && mItemStates[idx] == 2) {
        toEnd = true;
        state = 4;
    } else if (state == 1 && mItemStates[idx] == 5) {
        state += 3;
        toEnd = true;
    } else if (mItemStates[idx] >= 3) {
        state += 3;
    }
    mItemStates[idx] = state;

    const char* paneNames[3] = {"P_button_00", "P_button_01", "P_button_02"};
    s32 anims[6] = {idx + 31, idx + 31, idx + 34, idx + 37, idx + 37, idx + 40};

    nw4r::lyt::Pane* pPane = mpLayout->findPane(paneNames[idx]);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anims[0]], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anims[1]], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anims[2]], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anims[3]], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anims[4]], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anims[5]], pPane, true);
    RPSysLytAnmObj** ppAnmObj = &mpAnmObjs[anims[state]];
    RPSysLayout::forceBindAnmObjToPane(*ppAnmObj, pPane, true);

    if (state != 2 && state != 5) {
        (*ppAnmObj)->play(-1);
        if (state == 0 || state == 3) {
            (*ppAnmObj)->setRate(-1.0f);
        } else if (state == 1 || state == 4) {
            (*ppAnmObj)->setRate(1.0f);
        }
    } else {
        (*ppAnmObj)->restart();
    }

    if (rewind) {
        (*ppAnmObj)->restart();
        (*ppAnmObj)->setRate(-1.0f);
    } else if (toEnd) {
        PlayFromEnd(*ppAnmObj);
        (*ppAnmObj)->setRate(1.0f);
    }
}

void RPMiiSelectLyt::SetArrowBackVisible(s32 mode) {
    const char* groupNames[2] = {"G_arrow_00", "G_back_00"};
    bool visible = true;
    if (mode == 1) {
        visible = false;
    }
    for (s32 i = 0; i < 2; i++) {
        if (mode != 0) {
            visible = visible ^ 1;
        }
        nw4r::lyt::Group* pGroup = mpLayout->findGroup(groupNames[i]);
        if (pGroup != NULL) {
            for (nw4r::lyt::detail::PaneLinkList::Iterator it = pGroup->GetPaneList().GetBeginIter();
                 it != pGroup->GetPaneList().GetEndIter(); ++it) {
                it->mTarget->SetVisible(visible);
            }
        }
    }
}

namespace {

void PlaySceneInAnim(RPMiiSelectLyt* pLyt) {
    nw4r::lyt::Pane* pScene = pLyt->mpLayout->findPane("N_scene_00");
    RPSysLayout::unbindAnmObjToPane(pLyt->mpAnmObjs[0], pScene, true);
    RPSysLayout::unbindAnmObjToPane(pLyt->mpAnmObjs[1], pScene, true);
    RPSysLayout::bindAnmObjToPane(pLyt->mpAnmObjs[0], pScene, true);
    pLyt->mpAnmObjs[0]->restart();
}

void ResetFaceAllAnim(RPMiiSelectLyt* pLyt) {
    RPSysLayout::bindAnmObjToPane(pLyt->mpAnmObjs[30], pLyt->mpLayout->findPane("N_faceAll_00"), true);
    RPSysLayout::bindAnmObjToPane(pLyt->mpAnmObjs[30], pLyt->mpLayout->findPane("N_faceAll_01"), true);
    pLyt->mpAnmObjs[30]->restart();
    pLyt->mpAnmObjs[30]->stop();
}

void ResetArrowAnims(RPMiiSelectLyt* pLyt) {
    RPSysLayout::forceBindAnmObjToPane(pLyt->mpAnmObjs[26], pLyt->mpLayout->findPane("N_arrowR_00"), false);
    RPSysLayout::forceBindAnmObjToPane(pLyt->mpAnmObjs[27], pLyt->mpLayout->findPane("N_arrowL_00"), false);
    pLyt->mpAnmObjs[26]->restart();
    pLyt->mpAnmObjs[27]->restart();
    pLyt->mpAnmObjs[26]->setPlayMode(RPSysFrameCtrl::EPlayMode_Loop);
    pLyt->mpAnmObjs[27]->setPlayMode(RPSysFrameCtrl::EPlayMode_Loop);
}

void ApplyBaseLineColor(RPMiiSelectLyt* pLyt, u32 player) {
    const char* lineNames[3] = {"W_baseLine_01", "W_baseLine_02", "W_baseLine_03"};
    for (s32 i = 0; i < 3; i++) {
        pLyt->mpLayout->findWindow(lineNames[i])->applyPlayerColor(player, false);
    }
}

} // namespace
