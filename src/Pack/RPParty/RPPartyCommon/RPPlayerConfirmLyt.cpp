#include <Pack/RPParty/RPPartyCommon/RPPlayerConfirmLyt.h>

#include <Pack/RPParty/RPPartyCommon/RPPlayerConfirm.h>
#include <Pack/RPParty/RPPartyCommon/RPPlayerNameTag.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <egg/gfxe/eggScreen.h>

#include <cstdio>


#pragma auto_inline off

RPPlayerConfirmLyt::RPPlayerConfirmLyt() {
    typedef RPStateMachine<RPPlayerConfirmLyt> StateMachine;
    typedef StateMachine::StateFunc StateFunc;
    mpStateMachine = new StateMachine(this, 6);
    StateFunc mainFuncs[6][2] = {
        {NULL, NULL},
        {&RPPlayerConfirmLyt::MainInEnter, &RPPlayerConfirmLyt::MainInCalc},
        {&RPPlayerConfirmLyt::MainIdleEnter, &RPPlayerConfirmLyt::MainIdleCalc},
        {&RPPlayerConfirmLyt::MainDecideEnter, &RPPlayerConfirmLyt::MainDecideCalc},
        {&RPPlayerConfirmLyt::MainOutEnter, &RPPlayerConfirmLyt::MainOutCalc},
        {NULL, NULL},
    };
    for (s32 i = 0; i < 6; i++) {
        mpStateMachine->SetStateFuncs(mainFuncs[i][0], mainFuncs[i][1], i);
    }
    for (s32 i = 0; i < BUTTON_NUM; i++) {
        mpButtonStateMachines[i] = new StateMachine(this, 6);
        StateFunc buttonFuncs[6][2] = {
            {&RPPlayerConfirmLyt::ButtonActiveEnter, &RPPlayerConfirmLyt::ButtonActiveCalc},
            {&RPPlayerConfirmLyt::ButtonActivateEnter, &RPPlayerConfirmLyt::ButtonActivateCalc},
            {&RPPlayerConfirmLyt::ButtonInactiveEnter, &RPPlayerConfirmLyt::ButtonInactiveCalc},
            {&RPPlayerConfirmLyt::ButtonDeactivateEnter,
             &RPPlayerConfirmLyt::ButtonDeactivateCalc},
            {&RPPlayerConfirmLyt::ButtonPushEnter, &RPPlayerConfirmLyt::ButtonPushCalc},
            {NULL, NULL},
        };
        for (s32 j = 0; j < 6; j++) {
            mpButtonStateMachines[i]->SetStateFuncs(buttonFuncs[j][0], buttonFuncs[j][1], j);
        }
    }
    s32 player = 0;
    do {
        mpNameTags[player] = new RPPlayerNameTag(player);
    } while (++player < PLAYER_NUM);
}

// The button texts set in a file-local inline: pMessage gets a new virtual register and the
// original's r28 (written in Load: r25, the last loop's counter). The reference keeps the reloads.
static inline void Menu_SetButtonMessages(RPSysLayout*& rpLayout) {
    RPSysMessage* pMessage = RPPlayerConfirm::sInstance->mpMessage;
    rpLayout->findTextBox("T_button_00")->setMessage(pMessage, 0x28A, 0, 0);
    rpLayout->findTextBox("T_button_01")->setMessage(pMessage, 0x28B, 0, 0);
}

void RPPlayerConfirmLyt::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "playerSelect.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "playerSelect_02.brlyt");

    const char* frameNames[FRAME_ANM_NUM] = {"playerSelect_02_inFrame_00.brlan",
                                             "playerSelect_02_outFrame_00.brlan"};
    const char* buttonNames[BUTTON_ANM_NUM] = {"playerSelect_02_activeButton_00.brlan",
                                               "playerSelect_02_pushButton_00.brlan"};
    for (s32 i = 0; i < FRAME_ANM_NUM; i++) {
        mpFrameAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, frameNames[i]);
    }
    for (s32 i = 0; i < BUTTON_ANM_NUM; i++) {
        for (s32 j = 0; j < BUTTON_NUM; j++) {
            mpButtonAnmObjs[i][j] = mpLayout->createAnmObj(NULL, mpResAccessor, buttonNames[i]);
        }
    }

    char name[32];
    for (s32 i = 0; i < BUTTON_NUM; i++) {
        std::snprintf(name, sizeof(name), "P_button_%02u", i);
        mpButtonPanes[i] = mpLayout->findPane(name);
    }
    for (s32 i = 0; i < BUTTON_NUM; i++) {
        std::snprintf(name, sizeof(name), "B_button_%02u", i);
        mpButtonBoundings[i] = mpLayout->findPane(name);
    }

    Menu_SetButtonMessages(mpLayout);
    for (s32 i = 0; i < PLAYER_NUM; i++) {
        mpNameTags[i]->Load(pArchive);
    }
}

void RPPlayerConfirmLyt::Reset() {
    mpStateMachine->Reset(0);
    s32 i = 0;
    do {
        mpButtonStateMachines[i]->Reset(2);
    } while (++i < BUTTON_NUM);
    mpFrameAnmObjs[FRAME_ANM_IN]->resetFrame();
    mpFrameAnmObjs[FRAME_ANM_IN]->stop();
    mpFrameAnmObjs[FRAME_ANM_IN]->setFrame(0.0f);
    mpFrameAnmObjs[FRAME_ANM_OUT]->resetFrame();
    mpFrameAnmObjs[FRAME_ANM_OUT]->stop();
    mpFrameAnmObjs[FRAME_ANM_OUT]->setFrame(0.0f);
    mpButtonAnmObjs[BUTTON_ANM_ACTIVE][0]->resetFrame();
    mpButtonAnmObjs[BUTTON_ANM_ACTIVE][0]->stop();
    mpButtonAnmObjs[BUTTON_ANM_ACTIVE][0]->setFrame(0.0f);
    mpButtonAnmObjs[BUTTON_ANM_ACTIVE][1]->resetFrame();
    mpButtonAnmObjs[BUTTON_ANM_ACTIVE][1]->stop();
    mpButtonAnmObjs[BUTTON_ANM_ACTIVE][1]->setFrame(0.0f);
    mpButtonAnmObjs[BUTTON_ANM_PUSH][0]->resetFrame();
    mpButtonAnmObjs[BUTTON_ANM_PUSH][0]->stop();
    mpButtonAnmObjs[BUTTON_ANM_PUSH][0]->setFrame(0.0f);
    mpButtonAnmObjs[BUTTON_ANM_PUSH][1]->resetFrame();
    mpButtonAnmObjs[BUTTON_ANM_PUSH][1]->stop();
    mpButtonAnmObjs[BUTTON_ANM_PUSH][1]->setFrame(0.0f);
    mpLayout->unbindAllAnmObj();
    i = 0;
    do {
        mpButtonPanes[i]->UnbindAllAnimation(true);
    } while (++i < BUTTON_NUM);
    mPushedButton = -1;
    s32 player = 0;
    do {
        mpNameTags[player]->Reset();
    } while (++player < PLAYER_NUM);
}

void RPPlayerConfirmLyt::Calculate() {
    mpStateMachine->Calculate();
    s32 i = 0;
    do {
        mActiveButton = i;
        mpButtonStateMachines[i]->Calculate();
    } while (++i < BUTTON_NUM);
    if (mpStateMachine->GetState() != 0 && mpStateMachine->GetState() != 5) {
        mpLayout->calc();
        for (s32 player = 0; player < RPPlayerConfirm::sInstance->mParams.playerNum; player++) {
            const nw4r::math::MTX34& rMtx = mpPlayerPanes[player]->GetGlobalMtx();
            nw4r::math::VEC2 pos(rMtx._03, rMtx._13);
            RPPlayerNameTag* pNameTag = mpNameTags[player];
            pNameTag->mNamePosX = pos.x;
            pNameTag->mNamePosY = pos.y;
            mpNameTags[player]->Calculate();
        }
    }
}

void RPPlayerConfirmLyt::MainInEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpFrameAnmObjs[FRAME_ANM_IN]);
    mpFrameAnmObjs[FRAME_ANM_IN]->restart();
}

void RPPlayerConfirmLyt::MainInCalc() {
    if (mpFrameAnmObjs[FRAME_ANM_IN]->isFinished()) {
        mpStateMachine->ChangeState(2);
    }
}

void RPPlayerConfirmLyt::MainIdleEnter() {}

void RPPlayerConfirmLyt::MainIdleCalc() {}

void RPPlayerConfirmLyt::MainDecideEnter() {}

void RPPlayerConfirmLyt::MainDecideCalc() {
    if (mpButtonStateMachines[mPushedButton]->GetState() == 5) {
        mpStateMachine->ChangeState(2);
    }
}

void RPPlayerConfirmLyt::MainOutEnter() {
    mpLayout->unbindAllAnmObj();
    RPSysLayout::bindAnmObjToPane(mpFrameAnmObjs[FRAME_ANM_OUT], mpLayout->findPane("N_scene_00"),
                                  false);
    mpFrameAnmObjs[FRAME_ANM_OUT]->restart();
}

void RPPlayerConfirmLyt::MainOutCalc() {
    if (mpFrameAnmObjs[FRAME_ANM_OUT]->isFinished()) {
        mpStateMachine->ChangeState(5);
    }
}

void RPPlayerConfirmLyt::ButtonActiveEnter() {}

void RPPlayerConfirmLyt::ButtonActiveCalc() {}

void RPPlayerConfirmLyt::ButtonActivateEnter() {
    mpButtonPanes[mActiveButton]->UnbindAllAnimation(true);
    RPSysLayout::forceBindAnmObjToPane(mpButtonAnmObjs[BUTTON_ANM_ACTIVE][mActiveButton],
                                       mpButtonPanes[mActiveButton], true);
    mpButtonAnmObjs[BUTTON_ANM_ACTIVE][mActiveButton]->play(
        mpButtonAnmObjs[BUTTON_ANM_ACTIVE][mActiveButton]->getFrameCtrl()->getFrame());
    mpButtonAnmObjs[BUTTON_ANM_ACTIVE][mActiveButton]->setPlayMode(
        RPSysFrameCtrl::EPlayMode_OneTime);
    mpButtonAnmObjs[BUTTON_ANM_ACTIVE][mActiveButton]->setRate(1.0f);
}

void RPPlayerConfirmLyt::ButtonActivateCalc() {
    if (mpButtonAnmObjs[BUTTON_ANM_ACTIVE][mActiveButton]->isFinished()) {
        mpButtonStateMachines[mActiveButton]->ChangeState(0);
    }
}

void RPPlayerConfirmLyt::ButtonInactiveEnter() {}

void RPPlayerConfirmLyt::ButtonInactiveCalc() {}

void RPPlayerConfirmLyt::ButtonDeactivateEnter() {
    mpButtonPanes[mActiveButton]->UnbindAllAnimation(true);
    RPSysLayout::forceBindAnmObjToPane(mpButtonAnmObjs[BUTTON_ANM_ACTIVE][mActiveButton],
                                       mpButtonPanes[mActiveButton], true);
    mpButtonAnmObjs[BUTTON_ANM_ACTIVE][mActiveButton]->play(
        mpButtonAnmObjs[BUTTON_ANM_ACTIVE][mActiveButton]->getFrameCtrl()->getFrame());
    mpButtonAnmObjs[BUTTON_ANM_ACTIVE][mActiveButton]->setPlayMode(
        RPSysFrameCtrl::EPlayMode_Reset);
    mpButtonAnmObjs[BUTTON_ANM_ACTIVE][mActiveButton]->setRate(-1.0f);
}

void RPPlayerConfirmLyt::ButtonDeactivateCalc() {
    if (mpButtonAnmObjs[BUTTON_ANM_ACTIVE][mActiveButton]->isFinished()) {
        mpButtonStateMachines[mActiveButton]->ChangeState(2);
    }
}

void RPPlayerConfirmLyt::ButtonPushEnter() {
    mpButtonPanes[mActiveButton]->UnbindAllAnimation(true);
    RPSysLayout::forceBindAnmObjToPane(mpButtonAnmObjs[BUTTON_ANM_PUSH][mActiveButton],
                                       mpButtonPanes[mActiveButton], true);
    mpButtonAnmObjs[BUTTON_ANM_PUSH][mActiveButton]->restart();
    if (mPushedButton == 0) {
        RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_BACK_01_CTSP",
                                                           mPushRemote);
        RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_BACK_01_TVSP",
                                                           mPushRemote);
    } else {
        RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_A_02_CTSP",
                                                           mPushRemote);
        RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_A_02_TVSP",
                                                           mPushRemote);
    }
}

void RPPlayerConfirmLyt::ButtonPushCalc() {
    if (mpButtonAnmObjs[BUTTON_ANM_PUSH][mActiveButton]->isFinished()) {
        mpButtonStateMachines[mActiveButton]->ChangeState(5);
    }
}

void RPPlayerConfirmLyt::Draw() {
    if (mpStateMachine->GetState() != 0 && mpStateMachine->GetState() != 5) {
        mpLayout->draw();
        for (s32 i = 0; i < RPPlayerConfirm::sInstance->mParams.playerNum; i++) {
            mpNameTags[i]->Draw();
        }
    }
}

void RPPlayerConfirmLyt::Show() {
    if (mpStateMachine->IsStateSettled(0) || mpStateMachine->IsStateOrNext(5)) {
        const char* paneNames[PLAYER_NUM][PLAYER_NUM] = {
            {"N_1p_01", NULL, NULL, NULL},
            {"N_2p_01", "N_2p_02", NULL, NULL},
            {"N_3p_01", "N_3p_02", "N_3p_03", NULL},
            {"N_4p_01", "N_4p_02", "N_4p_03", "N_4p_04"},
        };
        mPlayerNum = RPPlayerConfirm::sInstance->mParams.playerNum;
        for (s32 i = 0; i < PLAYER_NUM; i++) {
            const char* pName = paneNames[mPlayerNum - 1][i];
            if (pName != NULL) {
                mpPlayerPanes[i] = mpLayout->findPane(pName);
            } else {
                mpPlayerPanes[i] = NULL;
            }
        }
        mpStateMachine->ChangeState(1);
        for (s32 i = 0; i < PLAYER_NUM; i++) {
            if (i < mPlayerNum) {
                mpNameTags[i]->Show(true);
            } else {
                mpNameTags[i]->Show(false);
            }
        }
        mPushedButton = -1;
        RPSysMessage* pMessage = RPPlayerConfirm::sInstance->mpMessage;
        if (mPlayerNum == 1) {
            mpLayout->findTextBox("T_playerAll_00")->setMessage(pMessage, 0x26C, 0, 0);
        } else {
            mpLayout->findTextBox("T_playerAll_00")->setMessage(pMessage, 0x26C, 1, 0);
        }
    }
}

s32 RPPlayerConfirmLyt::HitTestButtons(const nw4r::math::VEC2& rPos) const {
    s32 i = 0;
    do {
        if (mpLayout->isInsideBounding(RPSysLayout::castToBounding(mpButtonBoundings[i]),
                                      static_cast<const EGG::Vector2f&>(rPos))) {
            return i;
        }
    } while (++i < BUTTON_NUM);
    return -1;
}

bool RPPlayerConfirmLyt::ActivateButton(s32 button) {
    DeactivateOthers(button);
    if (!mpStateMachine->IsStateSettled(2)) {
        return false;
    }
    if (!IsButtonStateOrNext(button, 0) && !IsButtonStateOrNext(button, 1)) {
        mpButtonStateMachines[button]->ChangeState(1);
    }
    return true;
}

void RPPlayerConfirmLyt::DeactivateButtons() {
    for (s32 i = 0; i < BUTTON_NUM; i++) {
        DeactivateButton(i);
    }
}

bool RPPlayerConfirmLyt::PushButton(s32 button, u32 remote) {
    if (!mpStateMachine->IsStateSettled(2)) {
        return false;
    }
    DeactivateOthers(button);
    if (mpStateMachine->IsStateOrNext(3) || mpStateMachine->IsStateOrNext(5)) {
        return true;
    }
    mPushedButton = button;
    mPushRemote = remote;
    mpStateMachine->ChangeState(3);
    mpButtonStateMachines[button]->ChangeState(4);
    return true;
}

void RPPlayerConfirmLyt::Hide() {
    if (mpStateMachine->IsStateSettled(4) || mpStateMachine->IsStateOrNext(5)) {
        return;
    }
    mpStateMachine->ChangeState(4);
}

nw4r::lyt::Pane* RPPlayerConfirmLyt::GetNameTagModelPane(s32 player) const {
    if (player < RPPlayerConfirm::sInstance->mParams.playerNum) {
        return mpNameTags[player]->unk30;
    }
    return NULL;
}

void RPPlayerConfirmLyt::PlayCursorSe(s32 button) {
    const nw4r::math::MTX34& rMtx = mpButtonBoundings[button]->GetGlobalMtx();
    nw4r::math::VEC2 pos(rMtx._03, rMtx._13);
    pos.x /= static_cast<f32>(static_cast<u16>(EGG::Screen::GetSizeXMax() / 2));
    pos.y /= static_cast<f32>(static_cast<u16>(EGG::Screen::GetSizeYMax() / 2));
    RPSndAudioMgr::instance()->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_01", pos, 0.5f, 0);
}

#pragma defer_codegen off
#pragma auto_inline off

template RPStateMachine<RPPlayerConfirmLyt>::RPStateMachine(RPPlayerConfirmLyt*, s32);
template void RPStateMachine<RPPlayerConfirmLyt>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template void RPStateMachine<RPPlayerConfirmLyt>::Reset(s32);
template void RPStateMachine<RPPlayerConfirmLyt>::ChangeState(s32);
template void RPStateMachine<RPPlayerConfirmLyt>::Calculate();
template void RPStateMachine<RPPlayerConfirmLyt>::CallExit();
template void RPStateMachine<RPPlayerConfirmLyt>::CallCalc();
template void RPStateMachine<RPPlayerConfirmLyt>::CallEnter();
template bool RPStateMachine<RPPlayerConfirmLyt>::IsStateSettled(s32) const;
template bool RPStateMachine<RPPlayerConfirmLyt>::IsStateOrNext(s32) const;
