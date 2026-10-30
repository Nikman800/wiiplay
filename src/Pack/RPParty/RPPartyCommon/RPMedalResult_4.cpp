#include <Pack/RPParty/RPPartyCommon/RPMedalResult.h>
#include <egg/math/eggVector.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysStringUtility.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>

const char* RPRecordLyt::sNewRecPaneNames[RECORD_NUM] = {
    "N_newRec_00", "N_newRec_01", "N_newRec_02", "N_newRec_03", "N_newRec_04"
};

const char* RPRecordLyt::sRecordPaneNames[RECORD_NUM] = {
    "N_tRecord_00", "N_tRecord_01", "N_tRecord_02", "N_tRecord_03", "N_tRecord_04"
};

const char* RPRecordLyt::sCenterPaneNames[2] = {
    "N_center_00", "N_center_01"
};

// Keep calls to the state-machine members out of line. Their explicit
// instantiations below reproduce the order at the end of the original unit.
#pragma auto_inline off

RPRecordLyt::RPRecordLyt() {
    mpStateMachine = new RPRecordSeq(this, 7);

    RPRecordSeq::StateFunc funcs[7][2] = {
        {NULL, NULL},
        {&RPRecordLyt::EnterIn, &RPRecordLyt::CalcIn},
        {&RPRecordLyt::EnterBlink, &RPRecordLyt::CalcBlink},
        {&RPRecordLyt::EnterNewRec, &RPRecordLyt::CalcNewRec},
        {&RPRecordLyt::EnterShow, &RPRecordLyt::CalcShow},
        {&RPRecordLyt::EnterOut, &RPRecordLyt::CalcOut},
        {NULL, NULL}
    };
    RPRecordSeq::StateFunc nullFunc = NULL;
    funcs[0][0] = nullFunc;
    funcs[0][1] = nullFunc;
    funcs[6][0] = nullFunc;
    funcs[6][1] = nullFunc;

    for (s32 i = 0; i < 7; i++) {
        mpStateMachine->SetStateFuncs(funcs[i][0], funcs[i][1], i);
    }
}

void RPRecordLyt::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "myRecord.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "myRecord_00.brlyt");

    const char* anmNames[5] = {
        "myRecord_00_in_00.brlan",
        "myRecord_00_newRec_00.brlan",
        "myRecord_00_outRec_00.brlan",
        "myRecord_00_outNewRec_00.brlan",
        "myRecord_00_blink_00.brlan"
    };
    for (s32 i = 0; i < 5; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    mpLayout->findTextBox("T_header_00")
        ->setMessage(RPCommon::GetInstance()->GetMessage(), 0x1F40, 0, 0);
    mpPlayerPosPane = mpLayout->findPane("N_pPosition_00");
}

void RPRecordLyt::Reset() {
    mpStateMachine->Reset(0);
    mPanePosLatched = false;
    unk35 = false;

    for (s32 i = 0; i < 5; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }

    mpLayout->unbindAllAnmObj();
    for (s32 i = 0; i < RECORD_NUM; i++) {
        mpLayout->findPane(sRecordPaneNames[i])->UnbindAllAnimation(true);
    }
}

void RPRecordLyt::Calculate() {
    mpStateMachine->Calculate();

    s32 i = 0;
    do {
        mpLayout->findPane(sCenterPaneNames[i])->SetVisible(true);
        i++;
    } while (i < 2);

    s32 state = mpStateMachine->GetState();
    if (state != 0 && state != 6) {
        mpLayout->calc();
    }
}

void RPRecordLyt::EnterIn() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();

    for (s32 i = 0; i < RECORD_NUM; i++) {
        nw4r::lyt::Pane* pPane = mpLayout->findPane(sNewRecPaneNames[i]);
        if (i == mRank) {
            pPane->SetVisible(true);
        } else {
            pPane->SetVisible(false);
        }
    }

    RPCommon* pCommon = RPCommon::GetInstance();
    RPSysLytTextBox* pScoreText = mpLayout->findTextBox("T_nMyRec_00");
    wchar_t scoreBuffer[20];
    RPSysStringUtility::GetStringFullSizeNumber(mScore, scoreBuffer, 20, false);
    s32 gameIdx;
    s32 plural;
    pCommon->GetGameMsgIndex(mScore, &gameIdx, &plural);
    pScoreText->setMessage(pCommon->GetMessage(), gameIdx + 0x2008, plural, 1,
                           scoreBuffer);

    const char* recordNames[RECORD_NUM] = {
        "T_nRecord_00", "T_nRecord_01", "T_nRecord_02", "T_nRecord_03", "T_nRecord_04"
    };
    wchar_t recordBuffer[20];
    for (s32 i = 0; i < RECORD_NUM; i++) {
        RPSysStringUtility::GetStringFullSizeNumber(mRecords[i], recordBuffer, 20,
                                                  false);
        mpLayout->findTextBox(recordNames[i])->setText(recordBuffer, 0);
    }

    const char* unitNames[RECORD_NUM] = {
        "T_uRecord_00", "T_uRecord_01", "T_uRecord_02", "T_uRecord_03", "T_uRecord_04"
    };
    for (s32 i = 0; i < RECORD_NUM; i++) {
        RPSysLytTextBox* pText = mpLayout->findTextBox(unitNames[i]);
        pCommon->GetGameMsgIndex(mRecords[i], &gameIdx, &plural);
        pText->setMessage(pCommon->GetMessage(), gameIdx + 0x1FA4, plural, 0);
    }

    RPSndAudioMgr::instance()->startSystemSe("RP_COM_GAME_PARTY_SE_Win_Open_01");
    RPSndAudioMgr::instance()->startSystemSe("RP_COM_GAME_PARTY_SE_LANK_01");
}

void RPRecordLyt::CalcIn() {
    if (mpAnmObjs[0]->isFinished()) {
        if (mRank != -1) {
            unk35 = true;
        }

        if (mRank != -1 && mHasRecords) {
            mpStateMachine->ChangeState(3);
        } else if (mRank != -1) {
            mpStateMachine->ChangeState(2);
        } else {
            mpStateMachine->ChangeState(4);
        }
    }
}

void RPRecordLyt::EnterBlink() {
    mpLayout->bindAnmObjToPane(mpAnmObjs[4], sRecordPaneNames[mRank], true);
    mpAnmObjs[4]->restart();
    unk35 = false;
}

void RPRecordLyt::CalcBlink() {
    if (mpStateMachine->GetTimer() >= 180) {
        mpStateMachine->ChangeState(4);
    }
}

void RPRecordLyt::EnterNewRec() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();

    RPCommon* pCommon = RPCommon::GetInstance();
    if (mRank == 0) {
        mpLayout->findTextBox("T_newRec_00")
            ->setMessage(pCommon->GetMessage(), 0x1F45, 0, 0);
    } else {
        mpLayout->findTextBox("T_newRec_00")
            ->setMessage(pCommon->GetMessage(), 0x1F44, 0, 0);
    }

    mpLayout->bindAnmObjToPane(mpAnmObjs[4], sRecordPaneNames[mRank], true);
    mpAnmObjs[4]->restart();
    unk35 = false;
}

void RPRecordLyt::CalcNewRec() {
    if (mpAnmObjs[1]->isFinished()) {
        mpStateMachine->ChangeState(4);
    }
}

void RPRecordLyt::EnterShow() {}

void RPRecordLyt::CalcShow() {}

void RPRecordLyt::EnterOut() {
    mpLayout->unbindAllAnmObj();
    if (mRank != -1) {
        mpLayout->findPane(sRecordPaneNames[mRank])->UnbindAllAnimation(true);
    }

    if (mRank != -1 && mHasRecords) {
        mpLayout->bindAnmObj(mpAnmObjs[3]);
        mpAnmObjs[3]->restart();
    } else {
        mpLayout->bindAnmObj(mpAnmObjs[2]);
        mpAnmObjs[2]->restart();
    }

    RPSndAudioMgr::instance()->startSystemSe("RP_COM_GAME_PARTY_SE_Win_Close_01");
}

void RPRecordLyt::CalcOut() {
    if (mRank != -1 && mHasRecords) {
        if (mpAnmObjs[3]->isFinished()) {
            mpStateMachine->ChangeState(6);
        }
    } else {
        if (mpAnmObjs[2]->isFinished()) {
            mpStateMachine->ChangeState(6);
        }
    }
}

void RPRecordLyt::DrawLayer1A() {
    s32 state = mpStateMachine->GetState();
    if (state != 0 && state != 6) {
        nw4r::lyt::Pane* pFirst = mpLayout->findPane(sCenterPaneNames[0]);
        nw4r::lyt::Pane* pSecond = mpLayout->findPane(sCenterPaneNames[1]);
        pFirst->SetVisible(true);
        pSecond->SetVisible(false);
        mpLayout->draw();
    }
}

void RPRecordLyt::DrawLayer1C() {
    s32 state = mpStateMachine->GetState();
    if (state != 0 && state != 6) {
        nw4r::lyt::Pane* pFirst = mpLayout->findPane(sCenterPaneNames[0]);
        nw4r::lyt::Pane* pSecond = mpLayout->findPane(sCenterPaneNames[1]);
        pFirst->SetVisible(false);
        pSecond->SetVisible(true);
        mpLayout->draw();
    }
}

void RPRecordLyt::Open(s32 score, s32 rank, const s32* pRecords, bool hasRecords) {
    if (mpStateMachine->IsStateSettled(0)) {
        s32 rec4, rec3, rec2, rec1, rec0;
        rec0 = pRecords[0];
        rec1 = pRecords[1];
        rec2 = pRecords[2];
        rec3 = pRecords[3];
        rec4 = pRecords[4];
        mScore = score;
        mHasRecords = hasRecords;
        mRecords[0] = rec0;
        mRecords[1] = rec1;
        mRecords[2] = rec2;
        mRecords[3] = rec3;
        mRecords[4] = rec4;
        mRank = rank;
        mpStateMachine->ChangeState(1);
    }
}

void RPRecordLyt::Close() {
    if (mpStateMachine->IsStateSettled(4)) {
        mpStateMachine->ChangeState(5);
    }
}

void RPRecordLyt::LatchPanePos() {
    EGG::Vector2f pos = GetPanePos();
    mPanePos.x = pos.x;
    mPanePos.y = pos.y;
    mPanePosLatched = true;
}

#pragma auto_inline on

EGG::Vector2f RPRecordLyt::GetPanePos() const {
    if (mPanePosLatched) {
        return mPanePos;
    }
    return EGG::Vector2f(mpPlayerPosPane->GetGlobalMtx()._03, mpPlayerPosPane->GetGlobalMtx()._13);
}

#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPRecordLyt>::RPStateMachine(RPRecordLyt*, s32);
template void RPStateMachine<RPRecordLyt>::SetStateFuncs(const StateFunc&,
                                                       const StateFunc&, s32);
template void RPStateMachine<RPRecordLyt>::Reset(s32);
template void RPStateMachine<RPRecordLyt>::ChangeState(s32);
template void RPStateMachine<RPRecordLyt>::Calculate();
template void RPStateMachine<RPRecordLyt>::CallExit();
template void RPStateMachine<RPRecordLyt>::CallCalc();
template void RPStateMachine<RPRecordLyt>::CallEnter();
template bool RPStateMachine<RPRecordLyt>::IsStateSettled(s32) const;
