#pragma auto_inline off

#include <Pack/RPParty/RPPartyCommon/RPGameInfoLyt.h>
#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>

#include <Pack/RPParty/RPPartyCommon/RPSelectScene.h>

extern "C" s32 Party_gameIndexToMsgIndex(s32 game);

// Score ranges, medal thresholds and game ordering retained in this file's data.
extern const s32 lbl_8031D9C0[64] = {
    -999, 9999, 0, 999, 0, 999, 0, 9999,
    0, 999, -99, 99, -999, 9999, 0, 999,
    0, 999, 200, 320, 450, 600, 20, 40,
    60, 80, 40, 70, 100, 200, 150, 400,
    800, 1200, 3, 7, 10, 14, 15, 27,
    51, 60, 1000, 1600, 2300, 2600, 170, 250,
    300, 325, 11, 32, 84, 125, 0, 1,
    2, 3, 4, 5, 7, 8, 9, 0
};

RPGameInfoLyt::RPGameInfoLyt(s32 playerNum) {
    mpStateMachine = new RPGameInfoSeq(this, 6);
    RPGameInfoSeq::StateFunc funcs[6][2] = {
        {NULL, NULL},
        {&RPGameInfoLyt::EnterIn, &RPGameInfoLyt::CalcIn},
        {&RPGameInfoLyt::EnterIdle, &RPGameInfoLyt::CalcIdle},
        {&RPGameInfoLyt::EnterPushA, &RPGameInfoLyt::CalcPushA},
        {&RPGameInfoLyt::EnterOut, &RPGameInfoLyt::CalcOut},
        {NULL, NULL}
    };
    for (s32 i = 0; i < 6; i++) {
        mpStateMachine->SetStateFuncs(funcs[i][0], funcs[i][1], i);
    }
    mPlayerNum = playerNum;
}

void RPGameInfoLyt::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "ptSelectGame.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "ptSelectGame_01.brlyt");
    const char* remoteNames[9] = {
        "ptG0RemoteP_00.brlyt", "ptG1RemoteC_00.brlyt", "ptG2RemoteM_00.brlyt",
        "ptG3RemoteT_00.brlyt", "ptG4RemoteR_00.brlyt", "ptG5RemoteS_00.brlyt",
        "ptG6RemoteY_00.brlyt", "ptG7RemoteW_00.brlyt", "ptG8RemoteH_00.brlyt"
    };
    for (s32 i = 0; i < 9; i++) {
        mpRemoteLayouts[i] = RPSysLayout::create(NULL, mpResAccessor, remoteNames[i]);
    }
    for (s32 i = 0; i < 9; i++) {
        mpRemotePanes[i] = mpRemoteLayouts[i]->findPane("N_remote_00");
        mpRemoteLayouts[i]->findPane("RootPane")->RemoveChild(mpRemotePanes[i]);
    }
    const char* anmNames[3] = {
        "ptSelectGame_01_in_00.brlan", "ptSelectGame_01_pushA_00.brlan",
        "ptSelectGame_01_out_00.brlan"
    };
    s32 i = 0;
    do {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    } while (++i < 3);
    const char* remoteAnmNames[9] = {
        "ptG0RemoteP_00_remote_00.brlan", "ptG1RemoteC_00_remote_00.brlan",
        "ptG2RemoteM_00_remote_00.brlan", "ptG3RemoteT_00_remote_00.brlan",
        "ptG4RemoteR_00_remote_00.brlan", "ptG5RemoteS_00_remote_00.brlan",
        "ptG6RemoteY_00_remote_00.brlan", "ptG7RemoteW_00_remote_00.brlan",
        "ptG8RemoteH_00_remote_00.brlan"
    };
    for (s32 i = 0; i < 9; i++) {
        mpRemoteAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, remoteAnmNames[i]);
    }
    mpLayout->findTextBox("T_start_00")->setMessage(RPSelectScene::GetInstance()->mpMessage,
                                                 7003, 0, 0);
}

void RPGameInfoLyt::Reset() {
    mpStateMachine->Reset(0);
    mpAnmObjs[0]->resetFrame();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
    mpAnmObjs[1]->resetFrame();
    mpAnmObjs[1]->stop();
    mpAnmObjs[1]->setFrame(0.0f);
    mpAnmObjs[2]->resetFrame();
    mpAnmObjs[2]->stop();
    mpAnmObjs[2]->setFrame(0.0f);
    mpLayout->unbindAllAnmObj();
    mConfirmed = false;
}

void RPGameInfoLyt::Calculate() {
    mpStateMachine->Calculate();
    mpLayout->calc();
}

void RPGameInfoLyt::EnterIn() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    s32 remoteIndices[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    mRemoteIdx = remoteIndices[mGame];
    nw4r::lyt::Pane* pParent = mpLayout->findPane("N_remoteP_00");
    nw4r::lyt::Pane* pRemote = mpLayout->findPane("N_remote_00");
    if (pRemote != NULL) {
        pParent->RemoveChild(pRemote);
    }
    pParent->AppendChild(mpRemotePanes[mRemoteIdx]);
    mpRemotePanes[mRemoteIdx]->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpRemoteAnmObjs[mRemoteIdx], mpRemotePanes[mRemoteIdx], true);
    mpRemoteAnmObjs[mRemoteIdx]->restart();
    mpRemoteAnmObjs[mRemoteIdx]->setRate(0.0f);
}

void RPGameInfoLyt::CalcIn() {
    if (mpAnmObjs[0]->isFinished()) {
        mpStateMachine->ChangeState(2);
    }
}

void RPGameInfoLyt::EnterIdle() {
    mpRemoteAnmObjs[mRemoteIdx]->setRate(1.0f);
}

void RPGameInfoLyt::CalcIdle() {}

void RPGameInfoLyt::EnterPushA() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
    unk18 = static_cast<s32>(mpAnmObjs[1]->getFrameMax()) - 20;
    RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_A_01_CTSP", mConfirmArg);
    RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_A_01_TVSP", mConfirmArg);
}

void RPGameInfoLyt::CalcPushA() {
    if (mpAnmObjs[1]->isFinished()) {
        mpStateMachine->ChangeState(2);
    }
}

void RPGameInfoLyt::EnterOut() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[2]);
    mpAnmObjs[2]->restart();
}

void RPGameInfoLyt::CalcOut() {
    if (mpAnmObjs[2]->isFinished()) {
        mpStateMachine->ChangeState(5);
    }
}

void RPGameInfoLyt::Draw() {
    s32 state = mpStateMachine->GetState();
    if (state == 0) {
        return;
    }
    if (state == 5) {
        return;
    }
    mpLayout->draw();
}

void RPGameInfoLyt::Open(s32 game) {
    if (mpStateMachine->IsStateSettled(0) || mpStateMachine->IsStateOrNext(5)) {
        mGame = game;
        mpStateMachine->ChangeState(1);
        RPSysMessage* pMessage = RPSelectScene::GetInstance()->mpMessage;
        s32 messageIndex = Party_gameIndexToMsgIndex(game);
        mpLayout->findTextBox("T_gameTitle_00")->setMessage(pMessage, 7000, messageIndex, 0);
        if (RPSelectScene::GetInstance()->mPlayerNum == 1) {
            mpLayout->findTextBox("T_comment_00")->setMessage(pMessage, 7001, messageIndex, 0);
        } else {
            mpLayout->findTextBox("T_comment_00")->setMessage(pMessage, 7002, messageIndex, 0);
        }
        mpLayout->findTextBox("T_step_00")->setMessage(pMessage, 7004, messageIndex, 0);
        mpLayout->findTextBox("T_step_01")->setMessage(pMessage, 7004, messageIndex, 0);
        mConfirmed = false;
    }
}

bool RPGameInfoLyt::Confirm(u32 arg) {
    if (!mpStateMachine->IsStateSettled(2)) {
        return false;
    }
    if (mpStateMachine->IsStateOrNext(3) || mpStateMachine->IsStateOrNext(5)) {
        return true;
    }
    mConfirmArg = arg;
    mConfirmed = true;
    unk18 = 0xFFFF;
    mpStateMachine->ChangeState(3);
    return true;
}

void RPGameInfoLyt::Close() {
    if (mpStateMachine->IsStateSettled(4) || mpStateMachine->IsStateOrNext(5)) {
        return;
    }
    mpStateMachine->ChangeState(4);
}

#pragma defer_codegen off

template RPStateMachine<RPGameInfoLyt>::RPStateMachine(RPGameInfoLyt*, s32);
template void RPStateMachine<RPGameInfoLyt>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template void RPStateMachine<RPGameInfoLyt>::Reset(s32);
template void RPStateMachine<RPGameInfoLyt>::ChangeState(s32);
template void RPStateMachine<RPGameInfoLyt>::Calculate();
template void RPStateMachine<RPGameInfoLyt>::CallExit();
template void RPStateMachine<RPGameInfoLyt>::CallCalc();
template void RPStateMachine<RPGameInfoLyt>::CallEnter();
template bool RPStateMachine<RPGameInfoLyt>::IsStateOrNext(s32) const;
