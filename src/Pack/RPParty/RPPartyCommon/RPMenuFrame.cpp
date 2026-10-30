// Menu frame layout (RPMenuFrame, menuCommon.arc / menuFrame_00.brlyt): the frame, back button,
// caption, player name, training and party-game panels of the Party menus.
// Unit 0x801CE684-0x801D151C. Functions in reverse address order; the state machine
// instantiation follows at the end of the file.
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>
#include <Pack/RPParty/RPPartyCommon/RPMenuFrame.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysController.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPKernel/RPSysTagProcessor.h>
#include <Pack/RPParty/RPPartyCommon/RPInterpCamera.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <egg/core.h>
#include <egg/gfxe/eggScreen.h>

#include <cstdio>

extern "C" {
// Still named by its ledger placeholder (its unit is not linked yet)
EGG::Vector2f Lyt_getPaneGlobalPos(const nw4r::lyt::Pane* pPane);
void Lyt_setPaneColorFromTable(nw4r::lyt::Pane* pPane, u32 player);
}

namespace {
struct RPMenuFrameStateFuncs {
    RPMenuFrame::StateMachine::StateFunc mEnter; // at 0x0
    RPMenuFrame::StateMachine::StateFunc mCalc;  // at 0xC
};

const char* sAnchorPaneNames[2] = {"N_frameUp_00", "N_frameDown_00"};
f32 sFrameOffset = 80.0f;
const char* sBackButtonPaneName = "N_backButton_00";
const char* sBackTextPaneName = "T_caption_01";
const char* sCaptionPaneName = "N_caption_00";
const char* sNamePaneName = "N_name_00";
const char* sTrainingPaneName = "N_training_00";
const char* sPtGamePaneName = "N_ptGame_00";

} // namespace

RPMenuFrame::RPMenuFrame(s32 mode) {
    mMode = mode;

    mpFrameState = new StateMachine(this, 4);
    mpBackState = new StateMachine(this, 7);
    mpBackTextState = new StateMachine(this, 2);
    mpCaptionState = new StateMachine(this, 2);
    mpNameState = new StateMachine(this, 4);
    mpTrainingState = new StateMachine(this, 5);
    mpPtGameState = new StateMachine(this, 5);

    {
        RPMenuFrameStateFuncs funcs[4] = {
            {&RPMenuFrame::EnterFrameHidden, NULL},
            {&RPMenuFrame::EnterFrameIn, &RPMenuFrame::CalcFrameIn},
            {&RPMenuFrame::EnterFrameShown, &RPMenuFrame::CalcFrameShown},
            {&RPMenuFrame::EnterFrameOut, &RPMenuFrame::CalcFrameOut},
        };
        for (s32 i = 0; i < 4; i++) {
            mpFrameState->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
        }
    }

    {
        RPMenuFrameStateFuncs funcs[7] = {
            {NULL, NULL},
            {&RPMenuFrame::EnterBackIn, &RPMenuFrame::CalcBackIn},
            {&RPMenuFrame::EnterBackIdle, &RPMenuFrame::CalcBackIdle},
            {&RPMenuFrame::EnterBackActivate, &RPMenuFrame::CalcBackActivate},
            {&RPMenuFrame::EnterBackDeactivate, &RPMenuFrame::CalcBackDeactivate},
            {&RPMenuFrame::EnterBackPush, &RPMenuFrame::CalcBackPush},
            {&RPMenuFrame::EnterBackOut, &RPMenuFrame::CalcBackOut},
        };
        for (s32 i = 0; i < 7; i++) {
            mpBackState->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
        }
    }

    {
        RPMenuFrameStateFuncs funcs[7] = {
            {&RPMenuFrame::EnterBackTextIdle, &RPMenuFrame::CalcBackTextIdle},
            {&RPMenuFrame::EnterBackTextChange, &RPMenuFrame::CalcBackTextChange},
        };
        for (s32 i = 0; i < 2; i++) {
            mpBackTextState->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
        }
    }

    {
        RPMenuFrameStateFuncs funcs[2] = {
            {&RPMenuFrame::EnterCaptionIdle, &RPMenuFrame::CalcCaptionIdle},
            {&RPMenuFrame::EnterCaptionChange, &RPMenuFrame::CalcCaptionChange},
        };
        for (s32 i = 0; i < 2; i++) {
            mpCaptionState->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
        }
    }

    {
        RPMenuFrameStateFuncs funcs[4] = {
            {&RPMenuFrame::EnterNameHidden, &RPMenuFrame::CalcNameHidden},
            {&RPMenuFrame::EnterNameIn, &RPMenuFrame::CalcNameIn},
            {&RPMenuFrame::EnterNameShown, &RPMenuFrame::CalcNameShown},
            {&RPMenuFrame::EnterNameOut, &RPMenuFrame::CalcNameOut},
        };
        for (s32 i = 0; i < 4; i++) {
            mpNameState->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
        }
    }

    {
        RPMenuFrameStateFuncs funcs[5] = {
            {&RPMenuFrame::EnterTrainingHidden, &RPMenuFrame::CalcTrainingHidden},
            {&RPMenuFrame::EnterTrainingIn, &RPMenuFrame::CalcTrainingIn},
            {&RPMenuFrame::EnterTrainingShown, &RPMenuFrame::CalcTrainingShown},
            {&RPMenuFrame::EnterTrainingChange, &RPMenuFrame::CalcTrainingChange},
            {&RPMenuFrame::EnterTrainingOut, &RPMenuFrame::CalcTrainingOut},
        };
        for (s32 i = 0; i < 5; i++) {
            mpTrainingState->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
        }
    }

    {
        RPMenuFrameStateFuncs funcs[5] = {
            {&RPMenuFrame::EnterPtGameHidden, &RPMenuFrame::CalcPtGameHidden},
            {&RPMenuFrame::EnterPtGameIn, &RPMenuFrame::CalcPtGameIn},
            {&RPMenuFrame::EnterPtGameShown, &RPMenuFrame::CalcPtGameShown},
            {&RPMenuFrame::EnterPtGameChange, &RPMenuFrame::CalcPtGameChange},
            {&RPMenuFrame::EnterPtGameOut, &RPMenuFrame::CalcPtGameOut},
        };
        for (s32 i = 0; i < 5; i++) {
            mpPtGameState->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
        }
    }

    mpTagProcessor = RPSysTagProcessor::instance();
}

void RPMenuFrame::Load(EGG::Archive* pArchive) {
    RPLytObjBase::AttachArcFile(pArchive, "menuCommon.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "menuFrame_00.brlyt");

    const char* anmNames[18] = {
        "menuFrame_00_inFrame_00.brlan",        "menuFrame_00_outFrame_00.brlan",
        "menuFrame_00_inBackButton_00.brlan",   "menuFrame_00_outBackButton_00.brlan",
        "menuFrame_00_activeBack_00.brlan",     "menuFrame_00_pushBack_00.brlan",
        "menuFrame_00_changeBack_00.brlan",     "menuFrame_00_changeText_00.brlan",
        "menuFrame_00_inName_00.brlan",         "menuFrame_00_outName_00.brlan",
        "menuFrame_00_inTraining_00.brlan",     "menuFrame_00_outTraining_00.brlan",
        "menuFrame_00_changeTraining_00.brlan", "menuFrame_00_inPtGame_00.brlan",
        "menuFrame_00_outPtGame_00.brlan",      "menuFrame_00_changePtGame_00.brlan",
        "menuFrame_00_tpaSports_00.brlan",      "menuFrame_00_tpaHajimete_00.brlan",
    };
    for (s32 i = 0; i < 18; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    for (s32 i = 0; i < 2; i++) {
        nw4r::lyt::Pane* pPane = mpLayout->findPane(sAnchorPaneNames[i]);
        mAnchorPos[i].x = pPane->GetTranslate().x;
        mAnchorPos[i].y = pPane->GetTranslate().y;
        mAnchorPos[i].z = pPane->GetTranslate().z;
    }

    for (s32 i = 0; i < 2; i++) {
        const char* boundingNames[2] = {"B_backButton_00", "B_backButton_01"};
        mpBackBoundings[i] = mpLayout->findPane(boundingNames[i]);
    }

    void* pMsgRes = RPSysResourceManager::GetMessageResource("common_message.bmg");
    mpMessage = new RPSysMessage(pMsgRes, NULL);
}

void RPMenuFrame::Reset() {
    mpFrameState->Reset(0);
    mpBackState->Reset(0);
    mpBackTextState->Reset(0);
    mpCaptionState->Reset(0);
    mpNameState->Reset(0);
    mpTrainingState->Reset(0);
    mpPtGameState->Reset(0);

    for (s32 i = 0; i < 18; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }

    for (s32 i = 0; i < 2; i++) {
        mpLayout->findPane(sAnchorPaneNames[i])->UnbindAllAnimation(true);
    }
    mpLayout->findPane(sBackButtonPaneName)->UnbindAllAnimation(true);
    mpLayout->findPane(sBackTextPaneName)->UnbindAllAnimation(true);
    mpLayout->findPane(sCaptionPaneName)->UnbindAllAnimation(true);
    mpLayout->findPane(sNamePaneName)->UnbindAllAnimation(true);
    mpLayout->findPane(sTrainingPaneName)->UnbindAllAnimation(true);
    mpLayout->findPane(sPtGamePaneName)->UnbindAllAnimation(true);

    mpLayout->unbindAllAnmObj();
    if (mMode == 0) {
        mpAnmObjs[16]->restart();
    } else {
        mpAnmObjs[17]->restart();
    }

    for (s32 i = 0; i < 2; i++) {
        mpLayout->findPane(sAnchorPaneNames[i])->SetVisible(true);
    }

    mpLogoPane = mpLayout->findPane("N_3Drogo_00");

    mBackActive = false;
    mBackTextRequested = false;
    mBackTextId = -1;
    mpBackTextBox = mpLayout->findTextBox(sBackTextPaneName);
    mpBackTextBox->setMessage(mpMessage, 0x1C2, 0, 0);

    mCaptionRequested = false;
    mCaptionId = -1;
    mpCaptionTextBox = mpLayout->findTextBox("T_caption_00");
    mpCaptionTextBox->setText(L"", 0);

    mNameShowRequested = false;
    mNameHideRequested = false;
    mNameChangeRequested = false;
    mNamePlayer = -1;
    mNameRequested = false;
    mpPlayerTextBox = mpLayout->findTextBox("T_player_00");
    mpPlayerTextBox->setText(L"", 0);
    mpPlayerNameTextBox = mpLayout->findTextBox("T_playerName_00");
    mpPlayerNameTextBox->setText(L"", 0);
    mpModelPane = mpLayout->findPane("N_3dmodel_00");

    mTrainingShowRequested = false;
    mTrainingHideRequested = false;
    mTrainingRequested = false;
    mTrainingId = -1;
    mpTrainingTextBox = mpLayout->findTextBox("T_training_00");
    mpTrainingTextBox->setText(L"", 0);

    mPtGameShowRequested = false;
    mPtGameHideRequested = false;
    mPtGameRequested = false;
    mPtGameId = -1;
    mpPtGameTextBox = mpLayout->findTextBox("T_ptGame_00");
    mpPtGameTextBox->setText(L"", 0);
}

void RPMenuFrame::Calculate() {
    mpFrameState->Calculate();
    mpBackState->Calculate();
    mpBackTextState->Calculate();
    mpCaptionState->Calculate();
    mpNameState->Calculate();
    mpTrainingState->Calculate();
    mpPtGameState->Calculate();

    mpLayout->unbindAnmObj(mpAnmObjs[16]);
    mpLayout->unbindAnmObj(mpAnmObjs[17]);
    if (mMode == 0) {
        mpLayout->bindAnmObj(mpAnmObjs[16]);
    } else {
        mpLayout->bindAnmObj(mpAnmObjs[17]);
    }

    if (mpFrameState->GetState() == 0) {
        mpLayout->findPane("RootPane")->SetVisible(false);
    } else {
        mpLayout->findPane("RootPane")->SetVisible(true);
    }

    if (mpBackState->GetState() == 0) {
        mpLayout->findPane(sBackButtonPaneName)->SetVisible(false);
    } else {
        mpLayout->findPane(sBackButtonPaneName)->SetVisible(true);
    }

    if (mpNameState->GetState() == 0) {
        mpLayout->findPane(sNamePaneName)->SetVisible(false);
    } else {
        mpLayout->findPane(sNamePaneName)->SetVisible(true);
    }

    if (mpTrainingState->GetState() == 0) {
        mpLayout->findPane(sTrainingPaneName)->SetVisible(false);
    } else {
        mpLayout->findPane(sTrainingPaneName)->SetVisible(true);
    }

    if (mpPtGameState->GetState() == 0) {
        mpLayout->findPane(sPtGamePaneName)->SetVisible(false);
    } else {
        mpLayout->findPane(sPtGamePaneName)->SetVisible(true);
    }

    mpLayout->calc();
}

void RPMenuFrame::Draw() {
    if (mpFrameState->GetState() != 0) {
        mpLayout->draw();
    }
}

void RPMenuFrame::EnterFrameHidden() {
    for (s32 i = 0; i < 2; i++) {
        nw4r::lyt::Pane* pPane = mpLayout->findPane(sAnchorPaneNames[i]);
        f32 offset = sFrameOffset;
        offset = i == 0 ? offset : -offset;
        nw4r::math::VEC3 trans(mAnchorPos[i].x, mAnchorPos[i].y + offset, mAnchorPos[i].z);
        pPane->SetTranslate(trans);
    }
}

void RPMenuFrame::EnterFrameIn() {
    mpAnmObjs[0]->restart();
}

void RPMenuFrame::CalcFrameIn() {
    f32 t = mpAnmObjs[0]->getFrameCtrl()->getFrame() /
            mpAnmObjs[0]->getFrameCtrl()->getEndFrame();
    if (mpAnmObjs[0]->isFinished()) {
        mpFrameState->ChangeState(2);
        t = 1.0f;
    }

    for (s32 i = 0; i < 2; i++) {
        nw4r::lyt::Pane* pPane = mpLayout->findPane(sAnchorPaneNames[i]);
        f32 offset = sFrameOffset * (1.0f - RPRange::CalcEase(t, 1));
        offset = i == 0 ? offset : -offset;
        nw4r::math::VEC3 trans(mAnchorPos[i].x, mAnchorPos[i].y + offset, mAnchorPos[i].z);
        pPane->SetTranslate(trans);
    }
}

void RPMenuFrame::EnterFrameShown() {}

void RPMenuFrame::CalcFrameShown() {}

void RPMenuFrame::EnterFrameOut() {
    mpAnmObjs[1]->restart();
}

void RPMenuFrame::CalcFrameOut() {
    f32 t = mpAnmObjs[1]->getFrameCtrl()->getFrame() /
            mpAnmObjs[1]->getFrameCtrl()->getEndFrame();
    if (mpAnmObjs[1]->isFinished()) {
        mpFrameState->ChangeState(0);
        t = 1.0f;
    }

    for (s32 i = 0; i < 2; i++) {
        nw4r::lyt::Pane* pPane = mpLayout->findPane(sAnchorPaneNames[i]);
        f32 offset = sFrameOffset * RPRange::CalcEase(t, 1);
        offset = i == 0 ? offset : -offset;
        nw4r::math::VEC3 trans(mAnchorPos[i].x, mAnchorPos[i].y + offset, mAnchorPos[i].z);
        pPane->SetTranslate(trans);
    }
}

BOOL RPMenuFrame::ShowFrame() {
    if (mpFrameState->IsStateSettled(2) || mpFrameState->IsStateOrNext(1)) {
        return TRUE;
    }

    if (mpFrameState->IsStateSettled(0)) {
        mpFrameState->ChangeState(1);
        return TRUE;
    }

    return FALSE;
}

BOOL RPMenuFrame::IsFrameNotEntering() const {
    if (mpFrameState->GetState() == 1 ||
        (mpFrameState->mNextState == 1 && mpFrameState->IsChanging())) {
        return FALSE;
    }
    return TRUE;
}

void RPMenuFrame::SetFramePaneVisible(s32 idx, bool visible) {
    mpLayout->findPane(sAnchorPaneNames[idx])->SetVisible(visible);
}

EGG::Vector2f RPMenuFrame::GetLogoPos() const {
    if (mpLayout->findPane("RootPane")->IsVisible() &&
        mpLayout->findPane(sAnchorPaneNames[0])->IsVisible()) {
        return EGG::Vector2f(mpLogoPane->GetGlobalMtx()._03, mpLogoPane->GetGlobalMtx()._13);
    }
    return EGG::Vector2f(4649.0f, 4649.0f);
}

void RPMenuFrame::EnterBackIn() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sBackButtonPaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[2], pPane, true);
    mpAnmObjs[2]->restart();
    mBackActive = false;
}

void RPMenuFrame::CalcBackIn() {
    if (mpAnmObjs[2]->isFinished()) {
        mpBackState->ChangeState(2);
    }
}

void RPMenuFrame::EnterBackIdle() {}

void RPMenuFrame::CalcBackIdle() {}

void RPMenuFrame::EnterBackActivate() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sBackButtonPaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[4], pPane, true);
    mpAnmObjs[4]->play(mpAnmObjs[4]->getFrameCtrl()->getFrame());
    mpAnmObjs[4]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    mpAnmObjs[4]->setRate(1.0f);
    mBackActive = true;

    if (mBackPlayer >= 0 && mBackPlayer < 4) {
        EGG::Vector2f pos = Lyt_getPaneGlobalPos(mpBackBoundings[0]);
        pos.x /= static_cast<f32>(static_cast<u16>(EGG::Screen::GetSizeXMax() / 2));
        pos.y /= static_cast<f32>(static_cast<u16>(EGG::Screen::GetSizeYMax() / 2));
        RPSndAudioMgr::instance()->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_01", pos, 0.5f,
                                                      0);
        static_cast<RPSysCoreController*>(
            EGG::CoreControllerMgr::instance()->getNthController(mBackPlayer))
            ->rumble(RPSysCoreController::ERumblePattern_Cursor);
    }
}

void RPMenuFrame::CalcBackActivate() {
    if (mpAnmObjs[4]->isFinished()) {
        mpBackState->ChangeState(2);
    }
}

void RPMenuFrame::EnterBackDeactivate() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sBackButtonPaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[4], pPane, true);
    mpAnmObjs[4]->play(mpAnmObjs[4]->getFrameCtrl()->getFrame());
    mpAnmObjs[4]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
    mpAnmObjs[4]->setRate(-1.0f);
    mBackActive = false;
}

void RPMenuFrame::CalcBackDeactivate() {
    if (mpAnmObjs[4]->isFinished()) {
        mpBackState->ChangeState(2);
    }
}

void RPMenuFrame::EnterBackPush() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sBackButtonPaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[5], pPane, true);
    mpAnmObjs[5]->restart();
    RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_BACK_01_CTSP",
                                                       mBackPushRemote);
    RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_BACK_01_TVSP",
                                                       mBackPushRemote);
}

void RPMenuFrame::CalcBackPush() {
    if (mpAnmObjs[5]->isFinished()) {
        mpBackState->ChangeState(2);
        mpAnmObjs[4]->play(0);
        mpAnmObjs[4]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
        mpAnmObjs[4]->setRate(-1.0f);
        mBackActive = false;
    }
}

void RPMenuFrame::EnterBackOut() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sBackButtonPaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[3], pPane, true);
    mpAnmObjs[3]->restart();
}

void RPMenuFrame::CalcBackOut() {
    if (mpAnmObjs[3]->isFinished()) {
        mpBackState->ChangeState(0);
    }
}

BOOL RPMenuFrame::IsBackButtonIdle() const {
    BOOL idle = FALSE;
    if (mpBackState->GetState() == 2 && !mpBackState->IsChanging()) {
        idle = TRUE;
    }
    return idle;
}

BOOL RPMenuFrame::IsPointOnBackButton(const EGG::Vector2f& rPos) const {
    for (s32 i = 0; i < 2; i++) {
        if (mpLayout->isInsideBounding(RPSysLayout::castToBounding(mpBackBoundings[i]), rPos)) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL RPMenuFrame::SetBackButtonActive(bool active, s32 player) {
    if (mpBackState->IsStateSettled(2) || (active && mpBackState->IsStateSettled(4)) ||
        (!active && mpBackState->IsStateSettled(3))) {
        if (active != mBackActive) {
            if (active) {
                mBackPlayer = player;
                mpBackState->ChangeState(3);
            } else {
                mpBackState->ChangeState(4);
            }
        }
        return TRUE;
    }
    return FALSE;
}

void RPMenuFrame::PushBackButton(u32 remote) {
    mBackPushRemote = remote;
    mpBackState->ChangeState(5);
}

BOOL RPMenuFrame::IsBackPushDone() const {
    return !mpBackState->IsStateOrNext(5);
}

BOOL RPMenuFrame::ShowBackButton() {
    if (!mpBackState->IsStateOrNext(6) && !mpBackState->IsStateOrNext(0)) {
        return TRUE;
    }

    if (mpBackState->IsStateSettled(0)) {
        mpBackState->ChangeState(1);
        return TRUE;
    }

    return FALSE;
}

BOOL RPMenuFrame::HideBackButton() {
    if (mpBackState->IsStateSettled(0) || mpBackState->IsStateOrNext(6)) {
        return TRUE;
    }

    if (mpBackState->IsStateSettled(2)) {
        mpBackState->ChangeState(6);
        return TRUE;
    }

    return FALSE;
}

void RPMenuFrame::EnterBackTextIdle() {}

void RPMenuFrame::CalcBackTextIdle() {
    if (mBackTextRequested) {
        mpBackTextState->ChangeState(1);
    }
}

void RPMenuFrame::EnterBackTextChange() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sBackTextPaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[6], pPane, true);
    mpAnmObjs[6]->restart();
}

void RPMenuFrame::CalcBackTextChange() {
    if (mpAnmObjs[6]->getFrameCtrl()->getFrame() ==
        mpAnmObjs[6]->getFrameCtrl()->getEndFrame() / 2) {
        mpBackTextBox->setMessage(mBackTextReq.pMessage, mBackTextReq.group, mBackTextReq.index,
                                  mBackText);
        mBackTextId = mBackTextReq.id;
        mBackTextRequested = false;
    }

    if (mpAnmObjs[6]->isFinished()) {
        mpBackTextState->ChangeState(0);
    }
}

void RPMenuFrame::EnterCaptionIdle() {}

void RPMenuFrame::CalcCaptionIdle() {
    if (mCaptionRequested) {
        mpCaptionState->ChangeState(1);
    }
}

void RPMenuFrame::EnterCaptionChange() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sCaptionPaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[7], pPane, true);
    mpAnmObjs[7]->restart();
}

void RPMenuFrame::CalcCaptionChange() {
    if (mpAnmObjs[7]->getFrameCtrl()->getFrame() ==
        mpAnmObjs[7]->getFrameCtrl()->getEndFrame() / 2) {
        mpCaptionTextBox->setMessage(mCaptionReq.pMessage, mCaptionReq.group, mCaptionReq.index,
                                     mCaption);
        mCaptionId = mCaptionReq.id;
        mCaptionRequested = false;
    }

    if (mpAnmObjs[7]->isFinished()) {
        mpCaptionState->ChangeState(0);
    }
}

void RPMenuFrame::RequestCaptionChange(u32 id, BOOL force, RPSysMessage* pMessage, u32 group,
                                       u32 index, int argc, ...) {
    bool change = false;
    if (mCaptionRequested && (force || id != mCaptionReq.id)) {
        change = true;
    }
    if (!mCaptionRequested && (force || id != mCaptionId)) {
        change = true;
    }

    if (change) {
        va_list args;
        va_start(args, argc);
        mpTagProcessor->PreProcessEx(pMessage->GetMsg(group, index), mCaption, TEXT_LEN, NULL,
                                     argc, args);
        va_end(args);

        mCaptionReq.id = id;
        mCaptionRequested = true;
        mCaptionReq.pMessage = pMessage;
        mCaptionReq.group = group;
        mCaptionReq.index = index;
    }
}

void RPMenuFrame::SetCaption(s32 id, RPSysMessage* pMessage, u32 group, u32 index, int argc,
                             ...) {
    mCaptionId = id;
    mCaptionReq.id = id;
    mCaptionRequested = false;

    va_list args;
    va_start(args, argc);
    mpCaptionTextBox->setMessage(pMessage, group, index, argc, args);
    va_end(args);
}

void RPMenuFrame::EnterNameHidden() {}

void RPMenuFrame::CalcNameHidden() {
    if (mNameShowRequested) {
        mpNameState->ChangeState(1);
    }

    if (mNameChangeRequested) {
        mNamePlayer = mNextNamePlayer;
        if (mpNameState->GetState() == 0) {
            mNamePlayer = mNextNamePlayer;
            mNameShowRequested = true;
            mNameHideRequested = false;
        }
        mNameChangeRequested = false;
    }

    if (mNameRequested) {
        mpPlayerNameTextBox->setMessage(mNameReq.pMessage, mNameReq.group, mNameReq.index,
                                        mPlayerName);
        mNameRequested = false;
    }
}

void RPMenuFrame::EnterNameIn() {
    mNameShowRequested = false;
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sNamePaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[8], pPane, true);
    mpAnmObjs[8]->restart();

    Lyt_setPaneColorFromTable(mpLayout->findPicture("P_nameBg_01"), mNamePlayer);
    Lyt_setPaneColorFromTable(mpLayout->findPicture("P_nameBg_02"), mNamePlayer);

    if (mMode == 1) {
        if (mNamePlayer < 2) {
            mpPlayerTextBox->setMessage(mpMessage, mNamePlayer + 0x1C3, 0, 0);
        }
    } else {
        mpPlayerTextBox->SetVisible(false);
    }
}

void RPMenuFrame::CalcNameIn() {
    if (mpAnmObjs[8]->isFinished()) {
        mpNameState->ChangeState(2);
    }
}

void RPMenuFrame::EnterNameShown() {}

void RPMenuFrame::CalcNameShown() {
    if (mNameHideRequested) {
        mpNameState->ChangeState(3);
    }
}

void RPMenuFrame::EnterNameOut() {
    mNameHideRequested = false;
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sNamePaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[9], pPane, true);
    mpAnmObjs[9]->restart();
}

void RPMenuFrame::CalcNameOut() {
    if (mpAnmObjs[9]->isFinished()) {
        mpNameState->ChangeState(0);
    }
}

void RPMenuFrame::ShowName(s32 player) {
    if (mpNameState->GetState() == 0) {
        mNamePlayer = player;
        mNameShowRequested = true;
        mNameHideRequested = false;
        return;
    }

    if (mNamePlayer != player) {
        mNameChangeRequested = true;
        mNextNamePlayer = player;
        mNameHideRequested = true;
        mNameShowRequested = false;
    }
}

void RPMenuFrame::HideName() {
    mNameHideRequested = true;
    mNameShowRequested = false;
}

void RPMenuFrame::SetPlayerNameText(RPSysMessage* pMessage, u32 group, u32 index,
                                    const wchar_t* pName) {
    if (mpNameState->IsStateOrNext(3)) {
        swprintf(mPlayerName, TEXT_LEN, pName);
        mNameReq.pMessage = pMessage;
        mNameRequested = true;
        mNameReq.group = group;
        mNameReq.index = index;
    } else {
        mpPlayerNameTextBox->setMessage(pMessage, group, index, pName);
    }
}

nw4r::lyt::Pane* RPMenuFrame::GetModelPane() const {
    return mpModelPane;
}

void RPMenuFrame::EnterTrainingHidden() {}

void RPMenuFrame::CalcTrainingHidden() {
    if (mTrainingShowRequested) {
        mpTrainingState->ChangeState(1);
    }

    if (mTrainingRequested) {
        mpTrainingTextBox->setMessage(mTrainingReq.pMessage, mTrainingReq.group,
                                      mTrainingReq.index, mTrainingText);
        mTrainingShowRequested = true;
        mTrainingHideRequested = false;
        mTrainingId = mTrainingReq.id;
        mTrainingRequested = false;
    }
}

void RPMenuFrame::EnterTrainingIn() {
    mTrainingShowRequested = false;
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sTrainingPaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[10], pPane, true);
    mpAnmObjs[10]->restart();
}

void RPMenuFrame::CalcTrainingIn() {
    if (mpAnmObjs[10]->isFinished()) {
        mpTrainingState->ChangeState(2);
    }
}

void RPMenuFrame::EnterTrainingShown() {}

void RPMenuFrame::CalcTrainingShown() {
    if (mTrainingHideRequested) {
        mpTrainingState->ChangeState(4);
        return;
    }
    if (mTrainingRequested) {
        mpTrainingState->ChangeState(3);
    }
}

void RPMenuFrame::EnterTrainingChange() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sTrainingPaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[12], pPane, true);
    mpAnmObjs[12]->restart();
}

void RPMenuFrame::CalcTrainingChange() {
    if (mpAnmObjs[12]->getFrameCtrl()->getFrame() ==
        mpAnmObjs[12]->getFrameCtrl()->getEndFrame() / 2) {
        mpTrainingTextBox->setMessage(mTrainingReq.pMessage, mTrainingReq.group,
                                      mTrainingReq.index, mTrainingText);
        mTrainingId = mTrainingReq.id;
        mTrainingRequested = false;
    }

    if (mpAnmObjs[12]->isFinished()) {
        mpTrainingState->ChangeState(2);
    }
}

void RPMenuFrame::EnterTrainingOut() {
    mTrainingHideRequested = false;
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sTrainingPaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[11], pPane, true);
    mpAnmObjs[11]->restart();
}

void RPMenuFrame::CalcTrainingOut() {
    if (mpAnmObjs[11]->isFinished()) {
        mpTrainingState->ChangeState(0);
    }
}

void RPMenuFrame::EnterPtGameHidden() {}

void RPMenuFrame::CalcPtGameHidden() {
    if (mPtGameShowRequested) {
        mpPtGameState->ChangeState(1);
    }

    if (mPtGameRequested) {
        mpPtGameTextBox->setMessage(mPtGameReq.pMessage, mPtGameReq.group, mPtGameReq.index,
                                    mPtGameText);
        mPtGameShowRequested = true;
        mPtGameId = mPtGameReq.id;
        mPtGameRequested = false;
    }
}

void RPMenuFrame::EnterPtGameIn() {
    mPtGameShowRequested = false;
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sPtGamePaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[13], pPane, true);
    mpAnmObjs[13]->restart();
}

void RPMenuFrame::CalcPtGameIn() {
    if (mpAnmObjs[13]->isFinished()) {
        mpPtGameState->ChangeState(2);
    }
}

void RPMenuFrame::EnterPtGameShown() {}

void RPMenuFrame::CalcPtGameShown() {
    if (mPtGameHideRequested) {
        mpPtGameState->ChangeState(4);
        return;
    }
    if (mPtGameRequested) {
        mpPtGameState->ChangeState(3);
    }
}

void RPMenuFrame::EnterPtGameChange() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sPtGamePaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[15], pPane, true);
    mpAnmObjs[15]->restart();
}

void RPMenuFrame::CalcPtGameChange() {
    if (mpAnmObjs[15]->getFrameCtrl()->getFrame() ==
        mpAnmObjs[15]->getFrameCtrl()->getEndFrame() / 2) {
        mpPtGameTextBox->setMessage(mPtGameReq.pMessage, mPtGameReq.group, mPtGameReq.index,
                                    mPtGameText);
        mPtGameId = mPtGameReq.id;
        mPtGameRequested = false;
    }

    if (mpAnmObjs[15]->isFinished()) {
        mpPtGameState->ChangeState(2);
    }
}

void RPMenuFrame::EnterPtGameOut() {
    mPtGameHideRequested = false;
    nw4r::lyt::Pane* pPane = mpLayout->findPane(sPtGamePaneName);
    pPane->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[14], pPane, true);
    mpAnmObjs[14]->restart();
}

void RPMenuFrame::CalcPtGameOut() {
    if (mpAnmObjs[14]->isFinished()) {
        mpPtGameState->ChangeState(0);
    }
}

void RPMenuFrame::ShowPtGame() {
    mPtGameShowRequested = true;
}

void RPMenuFrame::HidePtGame() {
    mPtGameHideRequested = true;
}

void RPMenuFrame::RequestPtGameText(u32 id, BOOL force, RPSysMessage* pMessage, u32 group,
                                    u32 index, int argc, ...) {
    bool change = false;
    if (mPtGameRequested && (force || id != mPtGameReq.id)) {
        change = true;
    }
    if (!mPtGameRequested && (force || id != mPtGameId)) {
        change = true;
    }

    if (change) {
        va_list args;
        va_start(args, argc);
        mpTagProcessor->PreProcessEx(pMessage->GetMsg(group, index), mPtGameText, TEXT_LEN, NULL,
                                     argc, args);
        va_end(args);

        mPtGameReq.id = id;
        mPtGameRequested = true;
        mPtGameReq.pMessage = pMessage;
        mPtGameReq.group = group;
        mPtGameReq.index = index;
    }
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPMenuFrame>::RPStateMachine(RPMenuFrame*, s32);
template void RPStateMachine<RPMenuFrame>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template void RPStateMachine<RPMenuFrame>::Reset(s32);
template void RPStateMachine<RPMenuFrame>::ChangeState(s32);
template void RPStateMachine<RPMenuFrame>::Calculate();
template bool RPStateMachine<RPMenuFrame>::IsStateOrNext(s32) const;
template bool RPStateMachine<RPMenuFrame>::IsStateSettled(s32) const;
template void RPStateMachine<RPMenuFrame>::CallEnter();
template void RPStateMachine<RPMenuFrame>::CallCalc();
template void RPStateMachine<RPMenuFrame>::CallExit();
