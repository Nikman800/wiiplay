#include <Pack/RPParty/RPBilScene/RPBilHud.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysKokeshiGenInfo.h>
#include <Pack/RPKernel/RPSysKokeshiIcon.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysStringUtility.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPParty/RPBilScene/RPBilGameMgr.h>
#include <Pack/RPParty/RPBilScene/RPBilPlayer.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>

RPBilHud* RPBilHud::sInstance;
// Wii Play: all zero, but in .sdata (explicitly initialized), not .sbss
#pragma explicit_zero_data on
wchar_t RPBilHud::sEmptyText[4] = L"";
#pragma explicit_zero_data reset

void RPBilHud::CreateInstance() {
    sInstance = new RPBilHud();
}

void RPBilHud::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilHud::~RPBilHud() {}

RPBilHud::RPBilHud() {
    mpPromptStateMachine = new RPBilHudSeq(this, STATE_NUM);
    mpTurnPanelStateMachine = new RPBilHudSeq(this, STATE_NUM);

    RPBilStateFuncs<RPBilHud> promptStateFuncs[STATE_NUM] = {
        {&RPBilHud::PromptIdleEnter, &RPBilHud::PromptIdleCalc},
        {&RPBilHud::PromptInEnter, &RPBilHud::PromptInCalc},
        {&RPBilHud::PromptShownEnter, &RPBilHud::PromptShownCalc},
        {&RPBilHud::PromptOutEnter, &RPBilHud::PromptOutCalc},
    };
    for (s32 i = 0; i < STATE_NUM; i++) {
        mpPromptStateMachine->SetStateFuncs(promptStateFuncs[i].mEnter,
                                            promptStateFuncs[i].mCalc, i);
    }

    RPBilStateFuncs<RPBilHud> turnPanelStateFuncs[STATE_NUM] = {
        {&RPBilHud::TurnPanelIdleEnter, &RPBilHud::TurnPanelIdleCalc},
        {&RPBilHud::TurnPanelInEnter, &RPBilHud::TurnPanelInCalc},
        {&RPBilHud::TurnPanelShownEnter, &RPBilHud::TurnPanelShownCalc},
        {&RPBilHud::TurnPanelOutEnter, &RPBilHud::TurnPanelOutCalc},
    };
    for (s32 i = 0; i < STATE_NUM; i++) {
        mpTurnPanelStateMachine->SetStateFuncs(turnPanelStateFuncs[i].mEnter,
                                               turnPanelStateFuncs[i].mCalc, i);
    }

    mPlayerNum = RPBilGameMgr::sPlayerNum;
    for (s32 i = 0; i < mPlayerNum; i++) {
        RPSysKokeshiGenInfo genInfo;
        genInfo.SetGenType(RPSysKokeshiManager::GenType_Player);
        genInfo.SetPlayerNo(i);
        genInfo.SetIconWidth(0x60);
        genInfo.SetIconHeight(0x60);
        genInfo.SetIconBGType(RFLIconBG_Direct);
        genInfo.SetIconBGColor((GXColor){0, 0, 0, 0});
        genInfo.SetIconMipMap(true);
        mpKokeshiIcons[i] = new RPSysKokeshiIcon(&genInfo);
    }

    RPGrpRenderer::GetCurrent()->AppendDrawObject(static_cast<IRPGrpDrawObject&>(*this));
}

void RPBilHud::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "billiard.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "billiard_00.brlyt");

    const char* anmNames[ANM_NUM] = {
        "billiard_00_inAll_00.brlan",       "billiard_00_stText_00.brlan",
        "billiard_00_loopText_00.brlan",    "billiard_00_in1pMessage_00.brlan",
        "billiard_00_out1pMessage_00.brlan", "billiard_00_in2pMessage_00.brlan",
        "billiard_00_out2pMessage_00.brlan", "billiard_00_outAll_00.brlan",
    };
    for (s32 i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    const char* groupNames[GROUP_NUM] = {"gCommon_00", "g1Player_00", "g2Players_00"};
    for (s32 i = 0; i < GROUP_NUM; i++) {
        mpGroups[i] = mpLayout->findGroup(groupNames[i]);
    }

    const char* messagePaneNames[PLAYER_MAX] = {"N_1pMessage_00", "N_2pMessage_00"};
    for (s32 i = 0; i < PLAYER_MAX; i++) {
        mpMessagePanes[i] = mpLayout->findPane(messagePaneNames[i]);
    }

    mpTextPane = mpLayout->findPane("N_text_00");

    const char* textBoxNames[PLAYER_MAX] = {"T_text_00", "T_text_01"};
    for (s32 i = 0; i < PLAYER_MAX; i++) {
        mpTextBoxes[i] = mpLayout->findTextBox(textBoxNames[i]);
    }

    const char* facePaneNames[PLAYER_MAX][2] = {
        {"N_1pFacePict_00", "N_1pFacePict_01"},
        {"N_2pFacePict_00", "N_2pFacePict_01"},
    };
    for (s32 i = 0; i < PLAYER_MAX; i++) {
        for (s32 j = 0; j < 2; j++) {
            mpFacePanes[i][j] = mpLayout->findPane(facePaneNames[i][j]);
        }
    }

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();

    const char* textNames[PLAYER_MAX] = {"T_1pText_00", "T_2pText_00"};
    for (s32 i = 0; i < PLAYER_MAX; i++) {
        mpLayout->findTextBox(textNames[i])->setMessage(pMessage, 0x904, i, 0);
    }

    const char* turnTextNames[PLAYER_MAX][2] = {
        {"T_1pMessage_00", "T_1pMessage_01"},
        {"T_2pMessage_00", "T_2pMessage_01"},
    };
    for (s32 i = 0; i < PLAYER_MAX; i++) {
        for (s32 j = 0; j < 2; j++) {
            mpLayout->findTextBox(turnTextNames[i][j])->setMessage(pMessage, 0x8FE, i, 0);
        }
    }

    mpLayout->findTextBox("T_unit_00")->setMessage(pMessage, 0x903, 0, 0);

    for (s32 i = 0; i < mPlayerNum; i++) {
        mpKokeshiIcons[i]->LoadResource(NULL);
    }
}

void RPBilHud::Reset() {
    mpPromptStateMachine->Reset(STATE_IDLE);
    mpTurnPanelStateMachine->Reset(STATE_IDLE);

    for (s32 i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }

    mShowPrompt = false;
    mHidePrompt = false;
    mInstruction = -1;
    mTurnPlayer = -1;
    mHideTurnPanel = false;
    mTurnPlayerShown = -1;

    s32 shownGroup;
    s32 hiddenGroup;
    if (mPlayerNum == 1) {
        shownGroup = 1;
        hiddenGroup = 2;
    } else {
        shownGroup = 2;
        hiddenGroup = 1;
    }
    {
        nw4r::lyt::Group* pGroup = mpGroups[shownGroup];
        for (nw4r::lyt::detail::PaneLinkList::Iterator it = pGroup->GetPaneList().GetBeginIter();
             it != pGroup->GetPaneList().GetEndIter(); ++it) {
            it->mTarget->SetVisible(true);
        }
    }
    {
        nw4r::lyt::Group* pGroup = mpGroups[hiddenGroup];
        for (nw4r::lyt::detail::PaneLinkList::Iterator it = pGroup->GetPaneList().GetBeginIter();
             it != pGroup->GetPaneList().GetEndIter(); ++it) {
            it->mTarget->SetVisible(false);
        }
    }

    mpLayout->unbindAllAnmObj();
    for (s32 i = 0; i < GROUP_NUM; i++) {
        RPSysLayout::unbindAllAnmObjToGroup(mpGroups[i], true);
    }
    for (s32 i = 0; i < PLAYER_MAX; i++) {
        mpMessagePanes[i]->UnbindAllAnimation(true);
    }
    mpTextPane->UnbindAllAnimation(true);
}

void RPBilHud::Calculate() {
    mpPromptStateMachine->Calculate();
    mpTurnPanelStateMachine->Calculate();
    for (s32 i = 0; i < PLAYER_MAX; i++) {
        if (i == mTurnPlayerShown) {
            mpMessagePanes[i]->SetVisible(true);
        } else {
            mpMessagePanes[i]->SetVisible(false);
        }
    }
    if (mpPromptStateMachine->GetState() != STATE_IDLE) {
        mpLayout->calc();
    }
}

void RPBilHud::PromptIdleEnter() {}

void RPBilHud::PromptIdleCalc() {
    if (mShowPrompt) {
        mpPromptStateMachine->ChangeState(STATE_IN);
    }
}

void RPBilHud::PromptInEnter() {
    mShowPrompt = false;
    RPSysLayout::unbindAnmObjToGroup(mpAnmObjs[0], mpGroups[0], true);
    RPSysLayout::unbindAnmObjToGroup(mpAnmObjs[7], mpGroups[0], true);
    RPSysLayout::unbindAnmObjToGroup(mpAnmObjs[0], mpGroups[1], true);
    RPSysLayout::unbindAnmObjToGroup(mpAnmObjs[7], mpGroups[1], true);
    RPSysLayout::unbindAnmObjToGroup(mpAnmObjs[0], mpGroups[2], true);
    RPSysLayout::unbindAnmObjToGroup(mpAnmObjs[7], mpGroups[2], true);
    RPSysLayout::bindAnmObjToGroup(mpAnmObjs[0], mpGroups[0], true);
    if (mPlayerNum == 1) {
        RPSysLayout::bindAnmObjToGroup(mpAnmObjs[0], mpGroups[1], true);
    } else {
        RPSysLayout::bindAnmObjToGroup(mpAnmObjs[0], mpGroups[2], true);
    }
    mpAnmObjs[0]->restart();
}

void RPBilHud::PromptInCalc() {
    if (mpAnmObjs[0]->isFinished()) {
        mpPromptStateMachine->ChangeState(STATE_SHOWN);
    }
}

void RPBilHud::PromptShownEnter() {
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[1], mpTextPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[2], mpTextPane, true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[1], mpTextPane, true);
    mpAnmObjs[1]->restart();
    mLoopPrompt = false;
}

void RPBilHud::PromptShownCalc() {
    if (!mLoopPrompt && mpAnmObjs[1]->isFinished()) {
        RPSysLayout::unbindAnmObjToPane(mpAnmObjs[1], mpTextPane, true);
        RPSysLayout::unbindAnmObjToPane(mpAnmObjs[2], mpTextPane, true);
        RPSysLayout::bindAnmObjToPane(mpAnmObjs[2], mpTextPane, true);
        mpAnmObjs[2]->restart();
        mLoopPrompt = true;
    }
    if (mHidePrompt) {
        mpPromptStateMachine->ChangeState(STATE_OUT);
    }
}

void RPBilHud::PromptOutEnter() {
    mHidePrompt = false;
    RPSysLayout::unbindAnmObjToGroup(mpAnmObjs[0], mpGroups[0], true);
    RPSysLayout::unbindAnmObjToGroup(mpAnmObjs[7], mpGroups[0], true);
    RPSysLayout::unbindAnmObjToGroup(mpAnmObjs[0], mpGroups[1], true);
    RPSysLayout::unbindAnmObjToGroup(mpAnmObjs[7], mpGroups[1], true);
    RPSysLayout::unbindAnmObjToGroup(mpAnmObjs[0], mpGroups[2], true);
    RPSysLayout::unbindAnmObjToGroup(mpAnmObjs[7], mpGroups[2], true);
    RPSysLayout::bindAnmObjToGroup(mpAnmObjs[7], mpGroups[0], true);
    if (mPlayerNum == 1) {
        RPSysLayout::bindAnmObjToGroup(mpAnmObjs[7], mpGroups[1], true);
    } else {
        RPSysLayout::bindAnmObjToGroup(mpAnmObjs[7], mpGroups[2], true);
    }
    mpAnmObjs[7]->restart();
}

void RPBilHud::PromptOutCalc() {
    if (mpAnmObjs[7]->isFinished()) {
        mpPromptStateMachine->ChangeState(STATE_IDLE);
    }
}

void RPBilHud::ShowPrompt() {
    mShowPrompt = true;
    mHidePrompt = false;
}

void RPBilHud::HidePrompt() {
    mHidePrompt = true;
    mShowPrompt = false;
}

void RPBilHud::TurnPanelIdleEnter() {}

void RPBilHud::TurnPanelIdleCalc() {
    if (mTurnPlayer != -1) {
        mpTurnPanelStateMachine->ChangeState(STATE_IN);
    }
}

void RPBilHud::TurnPanelInEnter() {
    mTurnPlayerShown = mTurnPlayer;
    mTurnPlayer = -1;
    if (mTurnPlayerShown == 0) {
        mpTurnAnmObj = mpAnmObjs[3];
    } else {
        mpTurnAnmObj = mpAnmObjs[5];
    }
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[3], mpMessagePanes[mTurnPlayerShown], true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[4], mpMessagePanes[mTurnPlayerShown], true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[5], mpMessagePanes[mTurnPlayerShown], true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[6], mpMessagePanes[mTurnPlayerShown], true);
    RPSysLayout::bindAnmObjToPane(mpTurnAnmObj, mpMessagePanes[mTurnPlayerShown], true);
    mpTurnAnmObj->restart();

    if (mTurnPlayerShown == 0) {
        RPSndAudioMgr::instance()->startSystemSe(0x1AU);
        RPSndAudioMgr::instance()->startSystemSeWithRemote(0x22U, 1);
    } else {
        RPSndAudioMgr::instance()->startSystemSe(0x1BU);
        RPSndAudioMgr::instance()->startSystemSeWithRemote(0x23U, 2);
    }
    RPBilPlayerMgr::sInstance->GetPlayer(mTurnPlayerShown)->Rumble();
}

void RPBilHud::TurnPanelInCalc() {
    if (mpTurnAnmObj->isFinished()) {
        mpTurnPanelStateMachine->ChangeState(STATE_SHOWN);
    }
}

void RPBilHud::TurnPanelShownEnter() {}

void RPBilHud::TurnPanelShownCalc() {
    if (mHideTurnPanel) {
        mpTurnPanelStateMachine->ChangeState(STATE_OUT);
    }
}

void RPBilHud::TurnPanelOutEnter() {
    mHideTurnPanel = false;
    if (mTurnPlayerShown == 0) {
        mpTurnAnmObj = mpAnmObjs[4];
    } else {
        mpTurnAnmObj = mpAnmObjs[6];
    }
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[3], mpMessagePanes[mTurnPlayerShown], true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[4], mpMessagePanes[mTurnPlayerShown], true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[5], mpMessagePanes[mTurnPlayerShown], true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[6], mpMessagePanes[mTurnPlayerShown], true);
    RPSysLayout::bindAnmObjToPane(mpTurnAnmObj, mpMessagePanes[mTurnPlayerShown], true);
    mpTurnAnmObj->restart();
}

void RPBilHud::TurnPanelOutCalc() {
    if (mpTurnAnmObj->isFinished()) {
        mpTurnPanelStateMachine->ChangeState(STATE_IDLE);
    }
}

void RPBilHud::ShowTurnPanel(s32 player) {
    if (mPlayerNum > 1) {
        mTurnPlayer = player;
        mHideTurnPanel = false;
    }
}

void RPBilHud::HideTurnPanel() {
    mHideTurnPanel = true;
    mTurnPlayer = -1;
}

void RPBilHud::Set1pScore(s32 shots) {
    m1pScore = shots;
    wchar_t text[12];
    RPSysStringUtility::GetStringFullSizeNumber(shots, text, 10, false);
    mpLayout->findTextBox("T_only1pScore_00")->setText(text, 0);
}

void RPBilHud::SetPlayerScore(s32 player, s32 score) {
    mScores[player] = score;
    wchar_t text[12];
    RPSysStringUtility::GetStringFullSizeNumber(score, text, 10, false);
    const char* scoreNames[PLAYER_MAX] = {"T_1pScore_00", "T_2pScore_00"};
    mpLayout->findTextBox(scoreNames[player])->setText(text, 0);
}

void RPBilHud::SetInstruction(s32 instruction) {
    mInstruction = instruction;
    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    for (s32 i = 0; i < PLAYER_MAX; i++) {
        switch (instruction) {
        case 0:
            mpTextBoxes[i]->setMessage(pMessage, 0x8FC, 1, 0);
            break;
        case 1:
            mpTextBoxes[i]->setMessage(pMessage, 0x8FC, 4, 0);
            break;
        case 2:
            mpTextBoxes[i]->setMessage(pMessage, 0x902, 0, 0);
            break;
        case -1:
            mpTextBoxes[i]->setText(sEmptyText, 0);
            break;
        }
    }
}

void RPBilHud::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() == 9 && RPGrpRenderer::GetDrawStep() == 5 &&
        mpPromptStateMachine->GetState() != STATE_IDLE) {
        mpLayout->draw();
        for (s32 i = 0; i < mPlayerNum; i++) {
            for (s32 j = 1; j >= 0; j--) {
                BOOL black = false;
                if (j == 1) {
                    black = true;
                }
                RPDrawKokeshiIcon(mpKokeshiIcons[i], mpFacePanes[i][j], black, NULL);
            }
        }
    }
}

#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPBilHud>::RPStateMachine(RPBilHud*, s32);
template void RPStateMachine<RPBilHud>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template void RPStateMachine<RPBilHud>::Reset(s32);
template void RPStateMachine<RPBilHud>::ChangeState(s32);
template void RPStateMachine<RPBilHud>::Calculate();
template void RPStateMachine<RPBilHud>::CallEnter();
template void RPStateMachine<RPBilHud>::CallCalc();
template void RPStateMachine<RPBilHud>::CallExit();
