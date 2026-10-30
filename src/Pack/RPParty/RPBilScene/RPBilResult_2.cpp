// Billiards result layout (RPBilResultLyt, billiard_02.brlyt): the end-of-game score sheet for
// one or two players, one state machine per screen half.
// Unit 0x802CED28-0x802D032C. Functions in reverse address order; the state machine
// instantiation follows at the end of the file.
#include <Pack/RPParty/RPBilScene/RPBilResult_2.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPBilScene/RPBilHud.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>

static inline int GetOutAnm(int side) {
    int anm = 4;
    if (side == 0) {
        anm = 3;
    }
    return anm;
}

RPBilResultLyt::RPBilResultLyt() {
    for (int i = 0; i < SIDE_NUM; i++) {
        mpStateMachines[i] = new RPStateMachine<RPBilResultLyt>(this, STATE_NUM);
    }

    RPBilStateFuncs<RPBilResultLyt> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPBilResultLyt::ShowEnter, &RPBilResultLyt::ShowCalc},
        {&RPBilResultLyt::ShownEnter, &RPBilResultLyt::ShownCalc},
        {&RPBilResultLyt::HideEnter, &RPBilResultLyt::HideCalc},
        {NULL, NULL},
    };
    for (int i = 0; i < SIDE_NUM; i++) {
        for (int j = 0; j < STATE_NUM; j++) {
            mpStateMachines[i]->SetStateFuncs(funcs[j].mEnter, funcs[j].mCalc, j);
        }
    }
}

void RPBilResultLyt::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "billiard.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "billiard_02.brlyt");

    const char* anmNames[ANM_NUM] = {
        "billiard_02_inAll_00.brlan",        "billiard_02_switch1pSum_00.brlan",
        "billiard_02_switch2pSum_00.brlan",  "billiard_02_outUpperDisp_00.brlan",
        "billiard_02_outDownDisp_00.brlan",
    };
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    const char* nomName = "N_nomDisplay_00";
    const char* displayNames[2] = {"N_1pDisplay_00", "N_2pDisplay_00"};
    mpNomDisplayPane = mpLayout->findPane(nomName);
    for (int i = 0; i < 2; i++) {
        mpDisplayPanes[i] = mpLayout->findPane(displayNames[i]);
    }

    const char* nomSideNames[SIDE_NUM] = {"N_nomDisplayU_00", "N_nomDisplayD_00"};
    const char* sideNames[2][SIDE_NUM] = {
        {"N_1pDisplayU_00", "N_1pDisplayD_00"},
        {"N_2pDisplayU_00", "N_2pDisplayD_00"},
    };
    for (int i = 0; i < SIDE_NUM; i++) {
        mpNomSidePanes[i] = mpLayout->findPane(nomSideNames[i]);
        for (int j = 0; j < 2; j++) {
            mpSidePanes[j][i] = mpLayout->findPane(sideNames[j][i]);
        }
    }

    const char* nomFaceNames[2] = {"N_nomFacePict_00", "N_nomFacePict_01"};
    const char* faceNames[2][2] = {
        {"N_1pFacePict_00", "N_1pFacePict_01"},
        {"N_2pFacePict_00", "N_2pFacePict_01"},
    };
    for (int i = 0; i < 2; i++) {
        mpNomFacePanes[i] = mpLayout->findPane(nomFaceNames[i]);
        for (int j = 0; j < 2; j++) {
            mpFacePanes[i][j] = mpLayout->findPane(faceNames[i][j]);
        }
    }

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_nomText_00")->setMessage(pMessage, 0x908, 0, 0);
    mpLayout->findTextBox("T_nomText_01")->setMessage(pMessage, 0x908, 0, 0);
    mpLayout->findTextBox("T_1pText_00")->setMessage(pMessage, 0x908, 0, 0);
    mpLayout->findTextBox("T_1pText_01")->setMessage(pMessage, 0x908, 0, 0);
    mpLayout->findTextBox("T_2pText_00")->setMessage(pMessage, 0x908, 1, 0);
    mpLayout->findTextBox("T_2pText_01")->setMessage(pMessage, 0x908, 1, 0);
    mpLayout->findTextBox("T_1pScoreText_00")->setMessage(pMessage, 0x900, 0, 0);
    mpLayout->findTextBox("T_2pScoreText_00")->setMessage(pMessage, 0x900, 0, 0);
    mpLayout->findTextBox("T_nomScoText_01")->setMessage(pMessage, 0x900, 1, 0);
    mpLayout->findTextBox("T_nomScoText_02")->setMessage(pMessage, 0x900, 2, 0);
    mpLayout->findTextBox("T_1pScoreText_01")->setMessage(pMessage, 0x900, 2, 0);
    mpLayout->findTextBox("T_2pScoreText_01")->setMessage(pMessage, 0x900, 2, 0);
    mpLayout->findTextBox("T_nomSumText_00")->setMessage(pMessage, 0x900, 4, 0);
    mpLayout->findTextBox("T_1pSumText_00")->setMessage(pMessage, 0x900, 4, 0);
    mpLayout->findTextBox("T_2pSumText_00")->setMessage(pMessage, 0x900, 4, 0);
}

void RPBilResultLyt::Reset() {
    for (int i = 0; i < SIDE_NUM; i++) {
        mpStateMachines[i]->Reset(STATE_NONE);
    }

    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }

    mpLayout->unbindAllAnmObj();
    mpNomDisplayPane->UnbindAllAnimation(true);
    for (int i = 0; i < 2; i++) {
        mpDisplayPanes[i]->UnbindAllAnimation(true);
    }
    for (int i = 0; i < SIDE_NUM; i++) {
        mpNomSidePanes[i]->UnbindAllAnimation(true);
        for (int j = 0; j < 2; j++) {
            mpSidePanes[j][i]->UnbindAllAnimation(true);
        }
    }
}

void RPBilResultLyt::Calculate() {
    for (int i = 0; i < SIDE_NUM; i++) {
        mSide = i;
        mpStateMachines[i]->Calculate();
    }

    bool active = false;
    for (int i = 0; i < SIDE_NUM; i++) {
        s32 state = mpStateMachines[i]->GetState();
        if (state != STATE_NONE || state != STATE_DONE) {
            active = true;
        }
    }
    if (active) {
        mpLayout->calc();
    }
}

void RPBilResultLyt::ShowEnter() {
    mpNomSidePanes[mSide]->UnbindAllAnimation(true);
    mpLayout->bindAnmObjToPane(mpAnmObjs[0], mpNomSidePanes[mSide], true);
    for (int i = 0; i < 2; i++) {
        mpSidePanes[i][mSide]->UnbindAllAnimation(true);
        mpLayout->bindAnmObjToPane(mpAnmObjs[0], mpSidePanes[i][mSide], true);
    }
    mpAnmObjs[0]->restart();

    s32 winner = RPBilResult::sInstance->GetWinner();
    if (mSide == 1) {
        for (int i = 0; i < 2; i++) {
            if (i == winner) {
                if (i == 0) {
                    mpLayout->bindAnmObjToPane(mpAnmObjs[1], mpNomSidePanes[1], true);
                    mpLayout->bindAnmObjToPane(mpAnmObjs[1], mpSidePanes[i][1], true);
                    mpAnmObjs[1]->restart();
                } else {
                    mpLayout->bindAnmObjToPane(mpAnmObjs[2], mpSidePanes[i][1], true);
                    mpAnmObjs[2]->restart();
                }
            }
        }
    }
}

void RPBilResultLyt::ShowCalc() {
    if (mpAnmObjs[0]->isFinished()) {
        mpStateMachines[mSide]->ChangeState(STATE_SHOWN);
    }
}

void RPBilResultLyt::ShownEnter() {}

void RPBilResultLyt::ShownCalc() {}

void RPBilResultLyt::HideEnter() {
    int anm = GetOutAnm(mSide);
    mpNomSidePanes[mSide]->UnbindAllAnimation(true);
    mpLayout->bindAnmObjToPane(mpAnmObjs[anm], mpNomSidePanes[mSide], true);
    for (int i = 0; i < 2; i++) {
        mpSidePanes[i][mSide]->UnbindAllAnimation(true);
        mpLayout->bindAnmObjToPane(mpAnmObjs[anm], mpSidePanes[i][mSide], true);
    }
    mpAnmObjs[anm]->restart();
}

void RPBilResultLyt::HideCalc() {
    int anm = GetOutAnm(mSide);
    if (mpAnmObjs[anm]->isFinished()) {
        mpNomSidePanes[mSide]->SetVisible(false);
        for (int i = 0; i < 2; i++) {
            mpSidePanes[i][mSide]->SetVisible(false);
        }
        mpStateMachines[mSide]->ChangeState(STATE_DONE);
    }
}

void RPBilResultLyt::Draw() {
    bool active = false;
    for (int i = 0; i < SIDE_NUM; i++) {
        s32 state = mpStateMachines[i]->GetState();
        if (state != STATE_NONE || state != STATE_DONE) {
            active = true;
        }
    }
    if (active) {
        mpLayout->draw();
        for (int i = 0; i < mPlayerNum; i++) {
            for (int j = 1; j >= 0; j--) {
                bool black = false;
                if (j == 1) {
                    black = true;
                }
                RPSysKokeshiIcon* pIcon = RPBilHud::sInstance->GetKokeshiIcon(i);
                if (mPlayerNum == 1) {
                    RPDrawKokeshiIcon(pIcon, mpNomFacePanes[j], black, NULL);
                } else {
                    RPDrawKokeshiIcon(pIcon, mpFacePanes[i][j], black, NULL);
                }
            }
        }
    }
}

void RPBilResultLyt::ShowScores(s32 playerNum) {
    for (int i = 0; i < SIDE_NUM; i++) {
        if (!mpStateMachines[i]->IsStateSettled(STATE_NONE)) {
            return;
        }
    }

    mPlayerNum = playerNum;
    for (int i = 0; i < SIDE_NUM; i++) {
        mpStateMachines[i]->ChangeState(STATE_SHOW);
    }

    if (mPlayerNum == 1) {
        mpNomDisplayPane->SetVisible(true);
        mpDisplayPanes[0]->SetVisible(false);
        mpDisplayPanes[1]->SetVisible(false);
    } else {
        mpNomDisplayPane->SetVisible(false);
        mpDisplayPanes[0]->SetVisible(true);
        mpDisplayPanes[1]->SetVisible(true);
    }

    for (int i = 0; i < SIDE_NUM; i++) {
        mpNomSidePanes[i]->SetVisible(true);
        for (int j = 0; j < 2; j++) {
            mpSidePanes[j][i]->SetVisible(true);
        }
    }

    RPBilPlayerResult results[2];
    for (int i = 0; i < mPlayerNum; i++) {
        results[i] = *RPBilResult::sInstance->GetPlayerResult(i);
    }

    wchar_t text[12];
    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    for (int i = 0; i < mPlayerNum; i++) {
        if (mPlayerNum == 2) {
            RPSysStringUtility::GetStringFullSizeNumber(results[i].mScore, text, 10, false);
            const char* scoreNames[2] = {"T_1pScoreFig_00", "T_2pScoreFig_00"};
            mpLayout->findTextBox(scoreNames[i])->setText(text, 0);
        }

        if (mPlayerNum == 1) {
            RPSysStringUtility::GetStringFullSizeNumber(results[i].mShotCount, text, 10, false);
            bool plural = false;
            if (!RPSysStringUtility::IsSingular(results[i].mShotCount)) {
                plural = true;
            }
            mpLayout->findTextBox("T_nomScoFigS_00")->setMessage(pMessage, 0x90F, plural, 1, text);
            RPSysStringUtility::GetStringFullSizeNumber(results[i].mShotBonus, text, 10, false);
            mpLayout->findTextBox("T_nomScoFig_01")->setText(text, 0);
        }

        RPSysStringUtility::GetStringFullSizeNumber(results[i].mFoulCount, text, 10, false);
        bool plural = false;
        if (!RPSysStringUtility::IsSingular(results[i].mFoulCount)) {
            plural = true;
        }
        if (mPlayerNum == 1) {
            mpLayout->findTextBox("T_nomScoFigS_01")->setMessage(pMessage, 0x910, plural, 1, text);
        } else {
            const char* foulNames[2] = {"T_1pScoreFigS_00", "T_2pScoreFigS_00"};
            mpLayout->findTextBox(foulNames[i])->setMessage(pMessage, 0x90A, plural, 1, text);
        }

        RPSysStringUtility::GetStringFullSizeNumber(results[i].mFoulPenalty, text, 10, false);
        if (mPlayerNum == 1) {
            mpLayout->findTextBox("T_nomScoFig_02")->setText(text, 0);
        } else {
            const char* penaltyNames[2] = {"T_1pScoreFig_01", "T_2pScoreFig_01"};
            mpLayout->findTextBox(penaltyNames[i])->setText(text, 0);
        }

        RPSysStringUtility::GetStringFullSizeNumber(results[i].mTotal, text, 10, false);
        if (mPlayerNum == 1) {
            mpLayout->findTextBox("T_nomSumFig_00")->setText(text, 0);
        } else {
            const char* sumNames[2] = {"T_1pSumFig_00", "T_2pSumFig_00"};
            mpLayout->findTextBox(sumNames[i])->setText(text, 0);
        }
    }
}

bool RPBilResultLyt::IsShowDone() const {
    for (int i = 0; i < SIDE_NUM; i++) {
        if (mpStateMachines[i]->IsStateOrNext(STATE_NONE) ||
            mpStateMachines[i]->IsStateOrNext(STATE_SHOW)) {
            return false;
        }
    }
    return true;
}

void RPBilResultLyt::HideSide(s32 side) {
    if (!mpStateMachines[side]->IsStateOrNext(STATE_HIDE)) {
        switch (mpStateMachines[side]->IsStateOrNext(STATE_DONE)) {
        case false:
            mpStateMachines[side]->ChangeState(STATE_HIDE);
            break;
        }
    }
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPBilResultLyt>::RPStateMachine(RPBilResultLyt*, s32);
template void RPStateMachine<RPBilResultLyt>::SetStateFuncs(const StateFunc&, const StateFunc&,
                                                            s32);
template void RPStateMachine<RPBilResultLyt>::Reset(s32);
template void RPStateMachine<RPBilResultLyt>::ChangeState(s32);
template void RPStateMachine<RPBilResultLyt>::Calculate();
template void RPStateMachine<RPBilResultLyt>::CallExit();
template void RPStateMachine<RPBilResultLyt>::CallCalc();
template void RPStateMachine<RPBilResultLyt>::CallEnter();
template bool RPStateMachine<RPBilResultLyt>::IsStateSettled(s32) const;
template bool RPStateMachine<RPBilResultLyt>::IsStateOrNext(s32) const;
