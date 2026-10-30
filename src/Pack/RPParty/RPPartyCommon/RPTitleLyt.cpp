// Wii Play title screen layouts (RPTitleLyt, ptTitle.arc).
// Unit 0x801FB7D0-0x801FD6D4. Functions in reverse address order; the state machine
// instantiation follows at the end of the file.
// The stream start below reads RPSndAudioMgr's stream handle directly.
#include <Pack/RPAudio/RPSndAudioMgr.h>

#include <Pack/RPParty/RPPartyCommon/RPTitleLyt.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPSystem/RPSysProjectLocal.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPSystem/RPSysScene.h>
#include <Pack/RPSystem/RPSysSceneMgr.h>
#include <Pack/RPUtility/RPUtlRandom.h>

#include <egg/gfxe/eggScreen.h>

namespace {
struct RPTitleLytStateFuncs {
    RPTitleSeq::StateFunc mEnter; // at 0x0
    RPTitleSeq::StateFunc mCalc;  // at 0xC
};

// Frames between two bubbles of a group
s32 sBubbleInterval = 5;
// Frames in the closed state before it ends
s32 sClosedFrames = 10;

inline void ResetAnmObjs(RPSysLytAnmObj** ppAnmObjs, s32 num) {
    for (s32 i = 0; i < num; i++) {
        ppAnmObjs[i]->resetFrame();
        ppAnmObjs[i]->stop();
        ppAnmObjs[i]->setFrame(0.0f);
    }
}

inline void SetGroupVisible(RPSysLayout* pLayout, const char* pName, bool visible) {
    nw4r::lyt::Group* pGroup = pLayout->findGroup(pName);
    for (nw4r::lyt::detail::PaneLinkList::Iterator it = pGroup->GetPaneList().GetBeginIter();
         it != pGroup->GetPaneList().GetEndIter(); ++it) {
        it->mTarget->SetVisible(visible);
    }
}

// Wii Play: moves a pane right (the non-Japanese text is wider)
inline void ShiftPaneX(nw4r::lyt::Pane* pPane, nw4r::math::VEC3& rTrans, f32 dx) {
    rTrans.x = pPane->GetTranslate().x;
    rTrans.y = pPane->GetTranslate().y;
    rTrans.z = pPane->GetTranslate().z;
    rTrans.x += dx;
    pPane->SetTranslate(rTrans);
}
// Wii Play: RPSndAudioMgr::startStream by name, converting the name once for the check and
// again for the start
inline void StartStreamByName(RPSndAudioMgr* pAudioMgr, const char* pName) {
    u32 id = pAudioMgr->changeNameToIdRP(pName);
    if (pAudioMgr->mHandleStream.IsAttachedSound() && pAudioMgr->mHandleStream.GetId() == id) {
        return;
    }

    if (pAudioMgr->mIsDisableFanfare) {
        return;
    }

    pAudioMgr->mHandleStream.Stop(0);
    pAudioMgr->unk940 = OSGetTick();
    pAudioMgr->mStreamPlayTick = 0;
    pAudioMgr->startSound(&pAudioMgr->mHandleStream, pAudioMgr->changeNameToIdRP(pName));
}
} // namespace

inline void RPTitleLyt::StartBubble(s32 group) {
    if (mBubbleTimers[group] != 0) {
        return;
    }

    for (s32 i = 0; i < BUBBLE_NUM; i++) {
        if (mpBubbleStates[group][i]->IsStateSettled(0)) {
            mpBubbleStates[group][i]->ChangeState(1);
            mBubbleTimers[group] = sBubbleInterval;
            return;
        }
    }
}

RPTitleLyt::~RPTitleLyt() {}

RPTitleLyt::RPTitleLyt() {
    mpStateMachine = new RPTitleSeq(this, 7);
    {
        RPTitleLytStateFuncs funcs[7] = {
            {NULL, NULL},
            {&RPTitleLyt::EnterWaitReady, &RPTitleLyt::CalcWaitReady},
            {&RPTitleLyt::EnterIn, &RPTitleLyt::CalcIn},
            {&RPTitleLyt::EnterLoop, &RPTitleLyt::CalcWaitPress},
            {&RPTitleLyt::EnterOut, &RPTitleLyt::CalcOut},
            {&RPTitleLyt::EnterClosed, &RPTitleLyt::CalcClosed},
            {NULL, NULL},
        };
        for (s32 i = 0; i < 7; i++) {
            mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
        }
    }

    for (s32 i = 0; i < BUTTON_NUM; i++) {
        mpButtonStates[i] = new RPTitleSeq(this, 3);

        RPTitleLytStateFuncs funcs[3] = {
            {&RPTitleLyt::EnterButtonIdle, &RPTitleLyt::CalcButtonIdle},
            {&RPTitleLyt::EnterButtonPress, &RPTitleLyt::CalcButtonPress},
            {NULL, NULL},
        };
        for (s32 j = 0; j < 3; j++) {
            mpButtonStates[i]->SetStateFuncs(funcs[j].mEnter, funcs[j].mCalc, j);
        }
    }

    for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
        for (s32 j = 0; j < BUBBLE_NUM; j++) {
            mpBubbleStates[i][j] = new RPTitleSeq(this, 2);

            RPTitleLytStateFuncs funcs[2] = {
                {NULL, NULL},
                {&RPTitleLyt::EnterBubble, &RPTitleLyt::CalcBubble},
            };
            for (s32 k = 0; k < 2; k++) {
                mpBubbleStates[i][j]->SetStateFuncs(funcs[k].mEnter, funcs[k].mCalc, k);
            }
        }
    }

    mpResAccessor = RPSysLytResAccessor::create(NULL);
}

void RPTitleLyt::Load() {
    RPLytObjBase::AttachArcFile(RPSysResourceManager::LoadGameLocalArchive(), "ptTitle.arc",
                                mpResAccessor);

    const char* lytNames[3] = {"ptTitleBg_00.brlyt", "ptTitleLogo_00.brlyt",
                               "ptTitleInfo_00.brlyt"};
    for (s32 i = 0; i < 3; i++) {
        mpLayouts[i] = RPSysLayout::create(NULL, mpResAccessor, lytNames[i]);
    }

    for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
        for (s32 j = 0; j < BUBBLE_NUM; j++) {
            mpBubbleLayouts[i][j] = RPSysLayout::create(NULL, mpResAccessor, "ptTitleBub_00.brlyt");
        }
    }

    const char* anmNames[7] = {
        "ptTitleBg_00_inBg_00.brlan",       "ptTitleBg_00_loopBg_00.brlan",
        "ptTitleLogo_00_inLogo_00.brlan",   "ptTitleLogo_00_loopLogo_00.brlan",
        "ptTitleInfo_00_inInfo_00.brlan",   "ptTitleInfo_00_waitInfo_00.brlan",
        "ptTitleInfo_00_actOthers_00.brlan",
    };
    for (s32 i = 0; i <= 1; i++) {
        mpAnmObjs[i] = mpLayouts[0]->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }
    for (s32 i = 2; i <= 3; i++) {
        mpAnmObjs[i] = mpLayouts[1]->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }
    for (s32 i = 4; i <= 6; i++) {
        mpAnmObjs[i] = mpLayouts[2]->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    const char* buttonAnmNames[1] = {"ptTitleInfo_00_actABButton_00.brlan"};
    for (s32 i = 0; i < BUTTON_NUM; i++) {
        for (s32 j = 0; j < 1; j++) {
            mpButtonAnmObjs[i] =
                mpLayouts[0]->createAnmObj(NULL, mpResAccessor, buttonAnmNames[j]);
        }
    }

    const char* bubbleAnmNames[3] = {"ptTitleBub_00_bubble_00.brlan",
                                     "ptTitleBub_00_bubble_01.brlan",
                                     "ptTitleBub_00_bubble_02.brlan"};
    for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
        for (s32 j = 0; j < BUBBLE_NUM; j++) {
            for (s32 k = 0; k < 3; k++) {
                mpBubbleAnmObjs[i][j][k] =
                    mpBubbleLayouts[i][j]->createAnmObj(NULL, mpResAccessor, bubbleAnmNames[k]);
            }
        }
    }

    s32 notJapan = 0;
    if (RPSysProjectLocal::instance()->getRegion() != RPSysProjectLocal::ERegion_NTSC_J) {
        notJapan = 1;
    }

    const char* groupNames[2] = {"japan_00", "notJapan_00"};
    for (s32 g = 0; g < 2; g++) {
        for (s32 i = 0; i < 3; i++) {
            SetGroupVisible(mpLayouts[i], groupNames[g], notJapan == g);
        }
        for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
            for (s32 j = 0; j < BUBBLE_NUM; j++) {
                SetGroupVisible(mpBubbleLayouts[i][j], groupNames[g], notJapan == g);
            }
        }
    }

    if (!notJapan) {
        nw4r::math::VEC3 trans;
        nw4r::lyt::Pane* pPane = mpLayouts[2]->findPane("N_infoTextCnt_00");
        ShiftPaneX(pPane, trans, 20.0f);
        for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
            for (s32 j = 0; j < BUBBLE_NUM; j++) {
                pPane = mpBubbleLayouts[i][j]->findPane("N_bubble_00");
                ShiftPaneX(pPane, trans, 20.0f);
            }
        }
    }

    nw4r::lyt::Pane* pWidePane = mpLayouts[1]->findPane("N_wipeWide_00");
    nw4r::lyt::Pane* pSmallPane = mpLayouts[1]->findPane("N_wipeSmall_00");
    if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
        pWidePane->SetVisible(true);
        pSmallPane->SetVisible(false);
    } else {
        pWidePane->SetVisible(false);
        pSmallPane->SetVisible(true);
    }

    const char* circleNames[2][BUTTON_NUM] = {
        {"N_circleACJ_00", "N_circleBCJ_00"},
        {"N_circleACE_00", "N_circleBCE_00"},
    };
    for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
        for (s32 j = 0; j < BUBBLE_NUM; j++) {
            for (s32 k = 0; k < BUTTON_NUM; k++) {
                nw4r::lyt::Pane* pPane = mpBubbleLayouts[i][j]->findPane(circleNames[notJapan][k]);
                if (k == i) {
                    pPane->SetVisible(true);
                } else {
                    pPane->SetVisible(false);
                }
            }
        }
    }

    const char* buttonNames[2][BUTTON_NUM] = {
        {"N_AButtonJ_00", "N_BButtonJ_00"},
        {"N_AButtonE_00", "N_BButtonE_00"},
    };
    for (s32 i = 0; i < BUTTON_NUM; i++) {
        mpButtonPanes[i] = mpLayouts[2]->findPane(buttonNames[notJapan][i]);
    }

    void* pMsgRes = RPSysResourceManager::GetMessageResource("party_message.bmg");
    mpMessage = new RPSysMessage(pMsgRes, NULL);

    mpLayouts[2]->findTextBox("T_textPush_00")->setMessage(mpMessage, 0x1771, 0, 0);
    mpLayouts[2]->findTextBox("T_textPush_01")->setMessage(mpMessage, 0x1771, 0, 0);
}

void RPTitleLyt::Reset() {
    mpStateMachine->Reset(0);

    for (s32 i = 0; i < BUTTON_NUM; i++) {
        mpButtonStates[i]->Reset(0);
    }

    for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
        for (s32 j = 0; j < BUBBLE_NUM; j++) {
            mpBubbleStates[i][j]->Reset(0);
        }
    }

    ResetAnmObjs(mpAnmObjs, 7);

    // Wii Play: seven animations from each button's (only two exist; the rest are bubbles)
    for (s32 i = 0; i < BUTTON_NUM; i++) {
        ResetAnmObjs(&mpButtonAnmObjs[i], 7);
    }

    for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
        for (s32 j = 0; j < BUBBLE_NUM; j++) {
            for (s16 k = 0; k < 3; k++) {
                mpBubbleAnmObjs[i][j][k]->resetFrame();
                mpBubbleAnmObjs[i][j][k]->stop();
                mpBubbleAnmObjs[i][j][k]->setFrame(0.0f);
            }
        }
    }

    for (s32 i = 0; i < 3; i++) {
        mpLayouts[i]->unbindAllAnmObj();
    }

    for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
        for (s32 j = 0; j < BUBBLE_NUM; j++) {
            mpBubbleLayouts[i][j]->unbindAllAnmObj();
        }
    }

    mButtonsReady = false;
    mBubbleTimers[0] = 0;
    mBubbleTimers[1] = 0;
}

void RPTitleLyt::Calculate() {
    mpStateMachine->Calculate();

    for (s32 i = 0; i < BUTTON_NUM; i++) {
        mCurIdx = i;
        mpButtonStates[i]->Calculate();
    }

    for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
        for (s32 j = 0; j < BUBBLE_NUM; j++) {
            mCurIdx = i;
            mCurBubble = j;
            mpBubbleStates[i][j]->Calculate();
        }
    }

    bool wasReady = mButtonsReady;
    mButtonsReady = true;
    for (s32 i = 0; i < BUTTON_NUM; i++) {
        if (mpButtonStates[i]->IsStateSettled(0)) {
            mButtonsReady = false;
            break;
        }
    }

    if (mButtonsReady && !wasReady) {
        RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_KEY_AB_01");
    }

    for (s32 i = 0; i < BUTTON_NUM; i++) {
        if (mpButtonStates[i]->GetState() != 0) {
            bool busy = false;
            for (s32 j = 0; j < BUBBLE_NUM; j++) {
                if (!mpBubbleStates[i][j]->IsStateSettled(0)) {
                    busy = true;
                }
            }

            if (!busy) {
                StartBubble(i);
            }
        }
    }

    for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
        if (mBubbleTimers[i] > 0) {
            mBubbleTimers[i]--;
        }
    }

    if (mpStateMachine->GetState() != 0) {
        for (s32 i = 0; i < 3; i++) {
            mpLayouts[i]->calc();
        }

        for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
            for (s32 j = 0; j < BUBBLE_NUM; j++) {
                if (mpBubbleStates[i][j]->GetState() != 0) {
                    mpBubbleLayouts[i][j]->calc();
                }
            }
        }
    }
}

void RPTitleLyt::EnterWaitReady() {
    mpLayouts[0]->unbindAllAnmObj();
    mpLayouts[0]->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mpAnmObjs[0]->setRate(0.0f);

    mpLayouts[1]->unbindAllAnmObj();
    mpLayouts[1]->bindAnmObj(mpAnmObjs[2]);
    mpAnmObjs[2]->restart();
    mpAnmObjs[2]->setRate(0.0f);

    mpLayouts[2]->unbindAllAnmObj();
    mpLayouts[2]->bindAnmObj(mpAnmObjs[4]);
    mpAnmObjs[4]->restart();
    mpAnmObjs[4]->setRate(0.0f);
}

void RPTitleLyt::CalcWaitReady() {
    if (RPSysSceneMgr::instance()->getCurrentSceneRP()->isSceneDisplay()) {
        mpStateMachine->ChangeState(2);
    }
}

void RPTitleLyt::EnterIn() {
    mpAnmObjs[0]->setRate(1.0f);
    mpAnmObjs[2]->setRate(1.0f);
    mpAnmObjs[4]->setRate(1.0f);

    StartStreamByName(RPSndAudioMgr::instance(), "RP_COMMON_BGM_PARTY_01");
}

void RPTitleLyt::CalcIn() {
    if (mpAnmObjs[0]->isFinished() && mpAnmObjs[2]->isFinished() &&
        mpAnmObjs[4]->isFinished()) {
        mpStateMachine->ChangeState(3);
    }
}

void RPTitleLyt::EnterLoop() {
    mpLayouts[0]->unbindAllAnmObj();
    mpLayouts[0]->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();

    mpLayouts[1]->unbindAllAnmObj();
    mpLayouts[1]->bindAnmObj(mpAnmObjs[3]);
    mpAnmObjs[3]->restart();

    mpLayouts[2]->unbindAllAnmObj();
    mpLayouts[2]->bindAnmObj(mpAnmObjs[5]);
    mpAnmObjs[5]->restart();
}

void RPTitleLyt::CalcWaitPress() {
    if (mButtonsReady) {
        mpStateMachine->ChangeState(4);
    }
}

void RPTitleLyt::EnterOut() {
    mpLayouts[2]->unbindAllAnmObj();
    mpLayouts[2]->bindAnmObj(mpAnmObjs[6]);
    mpAnmObjs[6]->restart();
}

void RPTitleLyt::CalcOut() {
    if (mpAnmObjs[6]->isFinished()) {
        mpStateMachine->ChangeState(5);
    }
}

void RPTitleLyt::EnterClosed() {}

void RPTitleLyt::CalcClosed() {
    if (mpStateMachine->GetTimer() > sClosedFrames) {
        mpStateMachine->ChangeState(6);
    }
}

void RPTitleLyt::EnterButtonIdle() {
    mpButtonPanes[mCurIdx]->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpButtonAnmObjs[mCurIdx], mpButtonPanes[mCurIdx], true);
    mpButtonAnmObjs[mCurIdx]->restart();
    mpButtonAnmObjs[mCurIdx]->setRate(0.0f);
}

void RPTitleLyt::CalcButtonIdle() {}

void RPTitleLyt::EnterButtonPress() {
    mpButtonPanes[mCurIdx]->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpButtonAnmObjs[mCurIdx], mpButtonPanes[mCurIdx], true);
    mpButtonAnmObjs[mCurIdx]->restart();
    mpButtonAnmObjs[mCurIdx]->setRate(1.0f);
}

void RPTitleLyt::CalcButtonPress() {
    if (mpButtonAnmObjs[mCurIdx]->isFinished()) {
        mpButtonStates[mCurIdx]->ChangeState(2);
    }
}

void RPTitleLyt::EnterBubble() {
    mpBubbleLayouts[mCurIdx][mCurBubble]->unbindAllAnmObj();
    mBubbleAnmIdx[mCurIdx][mCurBubble] = RPUtlRandom::getU32(3);
    mpBubbleLayouts[mCurIdx][mCurBubble]->bindAnmObj(
        mpBubbleAnmObjs[mCurIdx][mCurBubble][mBubbleAnmIdx[mCurIdx][mCurBubble]]);
    mpBubbleAnmObjs[mCurIdx][mCurBubble][mBubbleAnmIdx[mCurIdx][mCurBubble]]->restart();
}

void RPTitleLyt::CalcBubble() {
    if (mpBubbleAnmObjs[mCurIdx][mCurBubble][mBubbleAnmIdx[mCurIdx][mCurBubble]]->isFinished()) {
        mpBubbleStates[mCurIdx][mCurBubble]->ChangeState(0);
    }
}

void RPTitleLyt::Draw() {
    if (mpStateMachine->GetState() != 0) {
        mpLayouts[0]->draw();

        for (s32 i = 0; i < BUBBLE_GROUP_NUM; i++) {
            for (s32 j = 0; j < BUBBLE_NUM; j++) {
                if (mpBubbleStates[i][j]->GetState() != 0) {
                    mpBubbleLayouts[i][j]->draw();
                }
            }
        }

        mpLayouts[1]->draw();
        mpLayouts[2]->draw();
    }
}

void RPTitleLyt::Open() {
    if (mpStateMachine->IsStateSettled(0) || mpStateMachine->IsStateOrNext(6)) {
        mpStateMachine->ChangeState(1);
    }
}

void RPTitleLyt::OnButtonPress(s32 button, bool pressed) {
    if (mButtonsReady) {
        return;
    }

    if (mpStateMachine->GetState() != 3) {
        return;
    }

    if (!pressed) {
        mpButtonStates[button]->ChangeState(0);
    } else if (mpButtonStates[button]->IsStateSettled(0)) {
        mpButtonStates[button]->ChangeState(1);

        StartBubble(button);
    }
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template bool RPStateMachine<RPTitleLyt>::IsStateOrNext(s32) const;
template void RPStateMachine<RPTitleLyt>::Calculate();
template void RPStateMachine<RPTitleLyt>::ChangeState(s32);
template void RPStateMachine<RPTitleLyt>::Reset(s32);
template void RPStateMachine<RPTitleLyt>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template RPStateMachine<RPTitleLyt>::RPStateMachine(RPTitleLyt*, s32);
template void RPStateMachine<RPTitleLyt>::CallEnter();
template void RPStateMachine<RPTitleLyt>::CallCalc();
template void RPStateMachine<RPTitleLyt>::CallExit();
