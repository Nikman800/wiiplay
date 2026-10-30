#include <Pack/RPParty/RPPartyCommon/RPRankCompleteLyt.h>

#include <Pack/RPKernel.h>
#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <egg/math.h>

#include <Pack/RPParty/RPPartyCommon/RPSelectScene.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

class RPSelectScene;

inline RPSysMessage* GetSelectSceneMessage() {
    return RPSelectScene::GetInstance()->mpMessage;
}

RPRankCompleteLyt::RPRankCompleteLyt() {
    mpStateMachine = new RPStateMachine<RPRankCompleteLyt>(this, 5);

    void (RPRankCompleteLyt::*stateFuncs[5][2])() = {
        {NULL, NULL},
        {&RPRankCompleteLyt::EnterIn, &RPRankCompleteLyt::CalcIn},
        {&RPRankCompleteLyt::EnterShow, &RPRankCompleteLyt::CalcShow},
        {&RPRankCompleteLyt::EnterOut, &RPRankCompleteLyt::CalcOut},
        {NULL, NULL},
    };

    for (s32 state = 0; state < 5; state++) {
        mpStateMachine->SetStateFuncs(stateFuncs[state][0], stateFuncs[state][1], state);
    }

    const char* effectNames[] = {
        "Bsm_com_fubuki",
        "Bsm_com_fireworks_fubuki",
        "Bsm_com_fireworks_slv",
        "Bsm_com_fireworks_gld",
        "Bsm_com_fireworks_pla",
    };
    for (s32 effect = 0; effect < EFFECT_NUM; effect++) {
        mpEffects[effect] = new RPSysEffect(effectNames[effect], 2, 0);
    }
}

void RPRankCompleteLyt::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "ptSelectGame.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "rankComplete_00.brlyt");

    const char* anmNames[] = {
        "rankComplete_00_in_00.brlan",
        "rankComplete_00_out_00.brlan",
    };
    for (s32 i = 0; i < 2; i++) {
        (&mpInAnmObj)[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    RPSysMessage* pMessage = GetSelectSceneMessage();
    mpLayout->findTextBox("T_congrats_00")->setMessage(pMessage, 0x1B81, 0, 0);
    mpLayout->findTextBox("T_congrats_01")->setMessage(pMessage, 0x1B81, 0, 0);
}

void RPRankCompleteLyt::Reset() {
    mpStateMachine->Reset(0);
    mpInAnmObj->resetFrame();
    mpInAnmObj->stop();
    mpInAnmObj->setFrame(0.0f);
    mpOutAnmObj->resetFrame();
    mpOutAnmObj->stop();
    mpOutAnmObj->setFrame(0.0f);
    mpLayout->unbindAllAnmObj();
}

void RPRankCompleteLyt::Calculate() {
    mpStateMachine->Calculate();
    s32 state = mpStateMachine->GetState();
    if (state != 0 && state != 4) {
        mpLayout->calc();
    }

    for (s32 i = 0; i < EFFECT_NUM; i++) {
        mpEffects[i]->update();
    }
}

void RPRankCompleteLyt::EnterIn() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpInAnmObj);
    mpInAnmObj->restart();
}

void RPRankCompleteLyt::CalcIn() {
    if (mpStateMachine->GetTimer() == 1) {
        mpEffects[0]->create();
        mpEffects[0]->setPos(EGG::Vector3f::zero);
        mpEffects[1]->create();
        mpEffects[1]->setPos(EGG::Vector3f::zero);

        s32 first;
        s32 last;
        switch (mRank) {
        case 1:
            first = 0;
            last = -1;
            break;
        case 2:
            first = 2;
            last = 2;
            break;
        case 3:
            first = 3;
            last = 3;
            break;
        case 4:
            first = 4;
            last = 4;
            break;
        }

        for (s32 effect = first; effect <= last; effect++) {
            mpEffects[effect]->create();
            mpEffects[effect]->setPos(EGG::Vector3f::zero);
        }
    }

    if (mpInAnmObj->isFinished()) {
        mpStateMachine->ChangeState(2);
    }
}

void RPRankCompleteLyt::EnterShow() {
    RPSndAudioMgr::instance()->startFanfare("RP_COMMON_BGM_RANK_COMPLETE");
}

void RPRankCompleteLyt::CalcShow() {}

void RPRankCompleteLyt::EnterOut() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpOutAnmObj);
    mpOutAnmObj->restart();
    mpEffects[1]->fade();
}

void RPRankCompleteLyt::CalcOut() {
    if (mpOutAnmObj->isFinished()) {
        mpStateMachine->ChangeState(4);
    }
}

void RPRankCompleteLyt::Draw() {
    s32 state = mpStateMachine->GetState();
    if (state != 0 && state != 4) {
        mpLayout->draw();
    }
}

void RPRankCompleteLyt::Open(s32 rank) {
    if (mpStateMachine->IsStateSettled(0)) {
        mRank = rank;
        mpStateMachine->ChangeState(1);
        mpLayout->findTextBox("T_rank_00")->setMessage(GetSelectSceneMessage(), 0x1B80,
                                                       rank - 1, 0);
    }
}

void RPRankCompleteLyt::Close() {
    if (mpStateMachine->IsStateOrNext(3) || mpStateMachine->IsStateOrNext(4)) {
        return;
    }
    mpStateMachine->ChangeState(3);
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPRankCompleteLyt>::RPStateMachine(RPRankCompleteLyt*, s32);
template void RPStateMachine<RPRankCompleteLyt>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template void RPStateMachine<RPRankCompleteLyt>::Reset(s32);
template void RPStateMachine<RPRankCompleteLyt>::ChangeState(s32);
template void RPStateMachine<RPRankCompleteLyt>::Calculate();
template void RPStateMachine<RPRankCompleteLyt>::CallExit();
template void RPStateMachine<RPRankCompleteLyt>::CallCalc();
template void RPStateMachine<RPRankCompleteLyt>::CallEnter();
template bool RPStateMachine<RPRankCompleteLyt>::IsStateSettled(s32) const;
template bool RPStateMachine<RPRankCompleteLyt>::IsStateOrNext(s32) const;
