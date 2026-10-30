// Game-select screen (RPSelectScene): the board, game info, rank complete and menu frame layouts,
// the player dolls and the crowd. Functions in reverse address order (-inline deferred); the
// game -> message index helper is defined last so that it comes first, then the state machine
// instantiations follow in address order.
#include <Pack/RPParty/RPPartyCommon/RPSelectScene.h>
#include <Pack/RPParty/RPPartyCommon/RPCPadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPCrowdMii.h>
#include <Pack/RPParty/RPPartyCommon/RPDoll.h>
#include <Pack/RPParty/RPPartyCommon/RPGameInfoLyt.h>
#include <Pack/RPParty/RPPartyCommon/RPMenuFrame.h>
#include <Pack/RPParty/RPPartyCommon/RPMsgBoard.h>
#include <Pack/RPParty/RPPartyCommon/RPPtMenuBg.h>
#include <Pack/RPParty/RPPartyCommon/RPRankCompleteLyt.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPSelectGameLyt.h>
#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPSystem/RPSysPlayer.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPSystem.h>
#include <Pack/RPUtility/RPUtlRandom.h>

extern "C" {
// Still named by their ledger placeholders
EGG::Vector2f EGG_Vector2f_operator_minus(const EGG::Vector2f*, const EGG::Vector2f*);
s32 Party_gameIndexToMsgIndex(s32 game);
}

// Game index -> message index: its initializer comes first in .rodata
inline s32 GameIndexToMsgIndex(s32 game) {
    s32 msgIdx[RPSelectGameLyt::GAME_NUM] = {0, 1, 2, 3, 4, 5, 7, 8, 9};
    return msgIdx[game];
}

namespace {

inline void ChangeState(RPSelectSceneSeq* pSeq, s32 state) {
    pSeq->ChangeState(state);
}

// Starts the menu BGM unless fanfares are disabled (checked before the name is converted)
inline bool StartBgmChecked(RPSndAudioMgr* pSnd, const char* pName) {
    if (pSnd->mIsDisableFanfare) {
        return false;
    }

    u32 id = pSnd->changeNameToIdRP(pName);
    if (pSnd->mHandleBgm.IsAttachedSound() && pSnd->mHandleBgm.GetId() == id) {
        return false;
    }

    pSnd->mHandleBgm.Stop(0);
    bool success = pSnd->startSound(&pSnd->mHandleBgm, id);

    if (RPSysProjectLocal::instance()->getFrameRate() == RPSysProjectLocal::EFrameRate_50Hz &&
        pSnd->getSoundArchive().GetSoundType(id) == nw4r::snd::SOUND_TYPE_SEQ &&
        pSnd->mHandleBgm.IsAttachedSound() &&
        (pSnd->getSoundArchive().GetSoundUserParam(id) & 0x80000000)) {
        nw4r::snd::SeqSoundHandle handle(&pSnd->mHandleBgm);
        handle.SetTempoRatio(50.0f / 60.0f);
    }

    return success;
}

inline bool StartWaveSoundChecked(RPSndAudioMgr* pSnd, const char* pName) {
    if (pSnd->mIsDisableFanfare) {
        if (pName[0] == 'B' && pName[1] == 'g' && pName[2] == 'm') {
            return false;
        }
    }

    return pSnd->startSound(&pSnd->mHandleWaveSound, pName);
}

inline void StopHandle(nw4r::snd::SoundHandle& rHandle, int frames) {
    if (rHandle.IsAttachedSound()) {
        rHandle.Stop(frames);
    }
}

inline void SetHandleVolume(nw4r::snd::SoundHandle& rHandle, f32 volume, int frames) {
    if (rHandle.IsAttachedSound()) {
        rHandle.SetVolume(volume, frames);
    }
}

inline bool IsGameInfoConfirmDone(const RPGameInfoLyt* pLyt) {
    bool done = false;
    if (pLyt->mConfirmed && pLyt->mpStateMachine->GetTimer() > static_cast<s32>(pLyt->unk18)) {
        done = true;
    }
    return done;
}

} // namespace

void RPSelectScene::CreateInstance() {
    sInstance = new RPSelectScene();
}

void RPSelectScene::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPSelectScene::~RPSelectScene() {
    RPDollMgr::DestroyInstance();
    RPCrowdMgr::DestroyInstance();
    RPMsgBoard::DestroyInstance();
}

RPSelectScene::RPSelectScene() {
    mpStateMachine = new RPSelectSceneSeq(this, 16);
    RPSelectSceneSeq::StateFunc funcs[16][2] = {
        {NULL, NULL},
        {&RPSelectScene::EnterStart, &RPSelectScene::CalcWaitSaveLoad},
        {&RPSelectScene::EnterDialog, &RPSelectScene::CalcWaitDialog},
        {&RPSelectScene::EnterUnlockGame, &RPSelectScene::CalcWaitUnlockMsg},
        {&RPSelectScene::EnterRankComplete, &RPSelectScene::CalcRankComplete},
        {&RPSelectScene::EnterOpenSelect, &RPSelectScene::CalcWaitSelectOpen},
        {&RPSelectScene::EnterSelect, &RPSelectScene::CalcSelect},
        {&RPSelectScene::EnterDecideGame, &RPSelectScene::CalcDecideGame},
        {&RPSelectScene::EnterBackPress, &RPSelectScene::CalcBackHighlight},
        {&RPSelectScene::EnterShowGameInfo, &RPSelectScene::CalcShowGameInfo},
        {&RPSelectScene::EnterWaitInfoOpen, &RPSelectScene::CalcWaitInfoOpen},
        {&RPSelectScene::EnterGameInfo, &RPSelectScene::CalcGameInfo},
        {&RPSelectScene::EnterInfoConfirm, &RPSelectScene::CalcWaitInfoConfirm},
        {&RPSelectScene::EnterBackButtonPress, &RPSelectScene::CalcWaitBackButton},
        {&RPSelectScene::EnterReturnToSelect, &RPSelectScene::CalcGotoSelect},
        {&RPSelectScene::EnterFinish, NULL}
    };
    for (s32 i = 0; i < 16; i++) {
        mpStateMachine->SetStateFuncs(funcs[i][0], funcs[i][1], i);
    }

    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));
    mPlayerNum = RPSceneInfo::spInstance->GetPlayerNum();
    RPDollMgr::CreateInstance(mPlayerNum);
    RPCrowdMgr::CreateInstance();
    mpSelectGameLyt = new RPSelectGameLyt(mPlayerNum);
    mpGameInfoLyt = new RPGameInfoLyt(mPlayerNum);
    mpRankCompleteLyt = new RPRankCompleteLyt();
    mpMenuFrame = new RPMenuFrame(1);
    mpCursor = new RPCPadCursor();
    RPMsgBoard::CreateInstance();
}

void RPSelectScene::Load() {
    RPSysResourceManager::LoadGameCommonArchive();
    EGG::Archive* pStaticArchive = RPSysResourceManager::instance()->GetStaticLocalArchive();
    EGG::Archive* pLocalArchive = RPSysResourceManager::LoadGameLocalArchive();
    void* pMsgData = RPSysResourceManager::GetMessageResource("party_message.bmg");
    mpMessage = new RPSysMessage(pMsgData, NULL);
    RPDollMgr::GetInstance()->Load();
    RPCrowdMgr::GetInstance()->Load();
    mpSelectGameLyt->Load(pLocalArchive);
    mpGameInfoLyt->Load(pLocalArchive);
    mpRankCompleteLyt->Load(pLocalArchive);
    mpMenuFrame->Load(pLocalArchive);
    mpCursor->Load(pStaticArchive);
    RPMsgBoard::GetInstance()->LoadMessages();
}

void RPSelectScene::Reset() {
    mpStateMachine->Reset(0);
    unk68 = -1;
    s32 lastGame = RPSceneInfo::spInstance->unk28;
    mCursor = lastGame;
    mLastGame = lastGame;
    mPointerPos = EGG::Vector2f::zero;
    mPadCursor = 9;
    unk7C = 9;
    mSpeakerFlag = RPSysPlayerMgr::instance()->getSpeakerFlag(-1);
    RPDollMgr::GetInstance()->Reset();
    RPCrowdMgr::GetInstance()->Reset();
    mpSelectGameLyt->Reset();
    mpGameInfoLyt->Reset();
    mpRankCompleteLyt->Reset();
    mpMenuFrame->Reset();
    mpCursor->Reset();
    mpCursor->SelectStyle(0);
    mpCursor->SetActive(0);
    unkDC = true;
}

void RPSelectScene::Calculate() {
    mPrevPointerPos = mPointerPos;
    mPointerPos = RPSysCursorDrawMgr::instance()->getActiveCursorPos();
    unk7C = mPadCursor;
    mPadCursor = mpSelectGameLyt->HitTestGame(mPointerPos);
    if (mCursor != 9) {
        mLastGame = mCursor;
    }

    mpStateMachine->Calculate();

    if (mPlayerNum == 1) {
        RPDollMgr::GetInstance()->SetDollScreenPos(mpSelectGameLyt->GetPlayerPanePos(1, 0), 0);
    } else {
        for (s32 i = 0; i < 2; i++) {
            RPDollMgr::GetInstance()->SetDollScreenPos(mpSelectGameLyt->GetPlayerPanePos(2, i),
                                                        i);
        }
    }

    bool active = true;
    if (!RPSysCoreController::getActiveController()->down(0xF0F)) {
        bool moved = false;
        if (RPSysCursorDrawMgr::instance()->getActiveCursorType() > 0) {
            f32 dist = EGG_Vector2f_operator_minus(&mPrevPointerPos, &mPointerPos).length();
            if (dist > 2.0f) {
                moved = true;
            }
        }
        if (!moved) {
            active = false;
        }
    }

    if (active) {
        RPDollMgr::GetInstance()->SetPointerActive(true);
    } else {
        RPDollMgr::GetInstance()->SetPointerActive(false);
    }

    RPDollMgr::GetInstance()->Calculate();
    RPCrowdMgr::GetInstance()->Calculate();

    bool showGame = false;
    if (mpStateMachine->GetState() < 9 && RPSysSaveDataMgr::instance()->isAsyncFinish()) {
        RPPartySystemData data = RPSysSaveDataMgr::instance()->getPartySystemData();
        if (mCursor != 9 && data.isGameOpen(mCursor)) {
            showGame = true;
        }
    }

    if (showGame) {
        mpMenuFrame->RequestPtGameText(mCursor, FALSE, mpMessage, 7010,
                                       GameIndexToMsgIndex(mCursor), 0);
        mpSelectGameLyt->RequestExpChange(mCursor);
    } else if (mpStateMachine->GetState() < 9) {
        mpMenuFrame->RequestPtGameText(9, FALSE, mpMessage, 1000, 0, 0);
        mpSelectGameLyt->RequestExpChange(9);
    }

    mpSelectGameLyt->Calculate();
    mpGameInfoLyt->Calculate();
    mpRankCompleteLyt->Calculate();
    mpMenuFrame->Calculate();
    mpCursor->Calculate();
    RPPtMenuBg::sInstance->SetMode(mCursor);
}

void RPSelectScene::EnterStart() {
    RPSndAudioMgr* pSnd = RPSndAudioMgr::instance();
    StartBgmChecked(pSnd, "RP_COMMON_BGM_PARTY_MENU");
    mpMenuFrame->ShowBackButton();

    bool save = false;
    mCompleteRank = 0;
    RPPartySystemData data = RPSysSaveDataMgr::instance()->getPartySystemData();
    RPMsgBoard* pBoard = RPMsgBoard::GetInstance();

    RPPartyPlayerData* pData;
    for (s32 i = 0; i < mPlayerNum; i++) {
        pData = RPSysPlayerMgr::instance()->getPlayer(i).mpPartyPlayerData;
        if (pData->isFirstPlay()) {
            if (pBoard->CanPost(RPDollMgr::GetInstance()->GetAvatar(i), data)) {
                pBoard->PostUnlockMessage(RPDollMgr::GetInstance()->GetAvatar(i));
                pBoard->RecordPost(RPDollMgr::GetInstance()->GetAvatar(i), data);
            }
            pData->setFirstPlay();
            save = true;
        }

        if (mPlayerNum == 1) {
            for (s32 medal = 4; medal > 0; medal--) {
                if (pData->isMedalDemo(medal)) {
                    continue;
                }

                bool complete = true;
                for (s32 game = 0; game < 9; game++) {
                    if (pData->getMedal(static_cast<RPPartyPlayerData::EGame>(game)) < medal) {
                        complete = false;
                    }
                }

                if (complete) {
                    if (pBoard->CanPost(RPDollMgr::GetInstance()->GetAvatar(i), data)) {
                        pBoard->PostAllMedalsMessage(RPDollMgr::GetInstance()->GetAvatar(i), medal);
                        pBoard->RecordPost(RPDollMgr::GetInstance()->GetAvatar(i), data);
                    }
                    for (s32 m = medal; m > 0; m--) {
                        pData->setMedalDemo(m);
                    }
                    mCompleteRank = medal;
                    save = true;
                    break;
                }
            }
        }
    }

    if (save) {
        RPSysSaveDataMgr::instance()->setPartySystemData(data);
        RPSysPlayerMgr::instance()->saveAllData();
        RPSysSaveDataMgr::instance()->saveAsync();
    }
}

void RPSelectScene::CalcWaitSaveLoad() {
    if (RPSysSceneMgr::instance()->getCurrentSceneRP()->isSceneDisplay() &&
        RPSysSaveDataMgr::instance()->isAsyncFinish()) {
        RPPartySystemData data = RPSysSaveDataMgr::instance()->getPartySystemData();
        mUnlockGame = -1;
        s32 openNum = -1;
        for (s32 i = 0; i <= 9; i++) {
            if (data.isGameOpen(i)) {
                openNum++;
            }
        }
        if (!data.isMsgFlag(openNum)) {
            mUnlockGame = openNum;
        }

        if (mUnlockGame != -1) {
            ChangeState(mpStateMachine, 3);
        } else if (mCompleteRank != 0) {
            ChangeState(mpStateMachine, 4);
        } else {
            ChangeState(mpStateMachine, 5);
        }
    }
}

void RPSelectScene::EnterDialog() {
    RPSysSystemWinMgr* pWinMgr = RPSysSystemWinMgr::instance();
    if (mPlayerNum == 1) {
        pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 7020, mpMessage, 0);
    } else {
        pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 7021, mpMessage, 0);
    }
    pWinMgr->setCursors();
}

void RPSelectScene::CalcWaitDialog() {
    RPSysSystemWinMgr* pWinMgr = RPSysSystemWinMgr::instance();
    if (pWinMgr->getResult() == RPSysSystemWinMgr::RESULT_4) {
        ChangeState(mpStateMachine, 15);
    } else if (pWinMgr->getResult() == RPSysSystemWinMgr::RESULT_5) {
        ChangeState(mpStateMachine, 6);
        mCursor = mDecidedGame;
        if (mCursor != 9) {
            mUsePad = true;
        }
    }
}

void RPSelectScene::EnterUnlockGame() {
    RPSysSystemWinMgr* pWinMgr = RPSysSystemWinMgr::instance();
    RPPartySystemData data = RPSysSaveDataMgr::instance()->getPartySystemData();
    data.setMsgFlag(mUnlockGame, true);
    RPSysSaveDataMgr::instance()->setPartySystemData(data);
    s32 game = mUnlockGame;
    RPSceneInfo* pInfo = RPSceneInfo::spInstance;
    if (game < 9) {
        pInfo->unk28 = game;
        mCursor = game;
        mLastGame = game;
    }
    pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT,
                             static_cast<u16>(mUnlockGame + 7030), mpMessage, 0);
    pWinMgr->setCursors();
    RPSysSaveDataMgr::instance()->setPartySystemData(data);
    RPSysSaveDataMgr::instance()->saveAsync();
}

void RPSelectScene::CalcWaitUnlockMsg() {
    if (RPSysSystemWinMgr::instance()->getResult() == RPSysSystemWinMgr::RESULT_CONTINUE &&
        RPSysSaveDataMgr::instance()->isAsyncFinish()) {
        if (mCompleteRank != 0) {
            ChangeState(mpStateMachine, 4);
        } else {
            ChangeState(mpStateMachine, 5);
        }
    }
}

void RPSelectScene::EnterRankComplete() {
    mpRankCompleteLyt->Open(mCompleteRank);
    RPDollMgr::GetInstance()->SetIdleExitAllowed(false);
    RPDollMgr::GetInstance()->RequestReaction(0, -1);
}

void RPSelectScene::CalcRankComplete() {
    bool shown = false;
    RPStateMachine<RPRankCompleteLyt>* pSeq = mpRankCompleteLyt->mpStateMachine;
    if (pSeq->GetState() == 2 && pSeq->GetTimer() >= 280) {
        shown = true;
    }
    if (shown && RPCommon::IsAnyABPressed(1)) {
        mpRankCompleteLyt->Close();
    }
    if (mpRankCompleteLyt->mpStateMachine->GetState() == 4) {
        ChangeState(mpStateMachine, 5);
        RPDollMgr::GetInstance()->SetIdleExitAllowed(true);
    }
}

void RPSelectScene::EnterOpenSelect() {
    if (mpStateMachine->mPrevState != 14) {
        mpSelectGameLyt->Open();
        mpMenuFrame->SetFramePaneVisible(0, false);
        mpMenuFrame->ShowFrame();
    }
    s32 cursor = mCursor;
    mUsePad = true;
    mPadCursor = cursor;
}

void RPSelectScene::CalcWaitSelectOpen() {
    if (mpSelectGameLyt->mpStateMachine->IsStateSettled(2) && mpMenuFrame->IsFrameNotEntering() &&
        (mpStateMachine->mPrevState != 14 ||
         mpGameInfoLyt->mpStateMachine->IsStateSettled(5))) {
        ChangeState(mpStateMachine, 6);
    }
}

void RPSelectScene::EnterSelect() {
    mBackHoldTimer = 0;
    RPDollMgr::GetInstance()->SetDozeAllowed(true);
}

// The cursor index is read through a reference inside an inline, so CW reloads mCursor for the
// call instead of reusing the compare's load (as the original does)
static inline void Party_PlayCursorSe(RPSelectGameLyt* pLyt, const s32& rCursor, bool usePad) {
    pLyt->PlayCursorSe(rCursor, usePad);
}

void RPSelectScene::CalcSelect() {
    s32 prevCursor = mCursor;
    bool prevUsePad = mUsePad;
    RPPartySystemData data = RPSysSaveDataMgr::instance()->getPartySystemData();

    if (mPadCursor != 9) {
        mCursor = mPadCursor;
        mUsePad = false;
    } else if (!mUsePad) {
        if (RPSysCoreController::getActiveController()->downTrigger(0xF000F)) {
            s32 lastGame = mLastGame;
            mUsePad = true;
            mCursor = lastGame;
        } else {
            mCursor = 9;
        }
    } else {
        u32 dirs[4] = {0x10008, 0x20004, 0x40001, 0x80002};
        for (s32 i = 0; i < 4; i++) {
            u32 hold = RPSysCoreController::getActiveController()->getCursorHold();
            if (hold & dirs[i]) {
                s32 next = mpSelectGameLyt->GetNeighborGame(mCursor, dirs[i]);
                while (next != mCursor) {
                    if (next == 9) {
                        break;
                    }
                    mCursor = next;
                    i = 4;
                    break;
                }
            }
        }
    }

    if (mpMenuFrame->IsPointOnBackButton(mPointerPos)) {
        mpMenuFrame->SetBackButtonActive(true,
                                         RPSysCoreController::getActiveController()->getPlayerNo());
        mCursor = 9;
        mUsePad = false;
    } else {
        mpMenuFrame->SetBackButtonActive(false, -1);
    }

    if (mPadCursor == 9) {
        for (s32 i = 0; i < mPlayerNum; i++) {
            if (RPDollMgr::GetInstance()->HitTestDoll(mPointerPos, i) != -1) {
                mCursor = 9;
                mUsePad = false;
            }
        }
    }

    if (mCursor != 9 && RPCommon::IsAPressed(mPlayerNum)) {
        if (data.isGameOpen(mCursor)) {
            ChangeState(mpStateMachine, 7);
        } else {
            RPSndAudioMgr* pSnd = RPSndAudioMgr::instance();
            pSnd->startSystemSeWithRemote("RP_COMMON_SE_KEY_NG_01_CTSP", 1);
            pSnd->startSystemSe("RP_COMMON_SE_KEY_NG_01_TVSP");
        }
    } else if (mpMenuFrame->IsBackButtonIdle() &&
               (RPSysCoreController::getActiveController()->downTrigger(0x1400) ||
                (mpMenuFrame->IsPointOnBackButton(mPointerPos) && RPCommon::IsAPressed(mPlayerNum)))) {
        ChangeState(mpStateMachine, 8);
        mDecidedGame = mCursor;
        mCursor = 9;
        mUsePad = false;
    }

    if (mCursor != 9 && data.isGameOpen(mCursor)) {
        mpSelectGameLyt->HighlightGame(mCursor);
    } else {
        mpSelectGameLyt->UnhighlightAll();
    }

    if (mUsePad) {
        mpCursor->SetActive(1);
        if (data.isGameOpen(mCursor)) {
            mpCursor->SetTarget(mpSelectGameLyt->GetGameIconPane(mCursor, 1));
        } else {
            mpCursor->SetTarget(mpSelectGameLyt->GetGameIconPane(mCursor, 3));
        }
    } else {
        mpCursor->SetActive(0);
    }

    if (mCursor != 9 &&
        ((mpStateMachine->mPrevState == 5 && mpStateMachine->GetTimer() == 0) ||
         mCursor != prevCursor || (prevUsePad && !mUsePad))) {
        Party_PlayCursorSe(mpSelectGameLyt, mCursor, !mUsePad);
    }

    for (s32 i = 0; i < 9; i++) {
        if (i == mCursor && !data.isGameOpen(i)) {
            mpSelectGameLyt->RequestLockedAnm(i);
        } else {
            mpSelectGameLyt->StopLockedAnm(i);
        }
    }

    if (mCursor == prevCursor) {
        if (mBackHoldTimer < 60) {
            mBackHoldTimer++;
        }
    } else {
        mBackHoldTimer = 0;
    }

    if (RPSysCursorDrawMgr::instance()->getActiveCursorType() > 0) {
        RPDollMgr::GetInstance()->LookAt(mPointerPos, -1);
    } else if (mUsePad && mBackHoldTimer < 60) {
        RPDollMgr::GetInstance()->LookAt(mpSelectGameLyt->GetGameIconPos(mCursor), -1);
    } else if (RPUtlRandom::getU32(100) == 0) {
        RPDollMgr::GetInstance()->ClearLook(RPUtlRandom::getU32(mPlayerNum));
    }

    if (mPadCursor == 9) {
        HandleDollClicks(0);
    }
    if (mPlayerNum == 2) {
        HandleDollClicks(1);
    }

    if (mpStateMachine->IsChanging()) {
        RPDollMgr::GetInstance()->SetDozeAllowed(false);
    }

    bool wasAwake = unkDC;
    RPSndAudioMgr* pSnd = RPSndAudioMgr::instance();
    if (RPDollMgr::GetInstance()->AreAllAsleep()) {
        unkDC = false;
        if (wasAwake) {
            SetHandleVolume(pSnd->mHandleBgm, 0.0f, 480);
            StartWaveSoundChecked(pSnd, "RP_COMMON_BGM_PARTY_MENU_SLEEP");
        }
    } else {
        unkDC = true;
        if (!wasAwake) {
            StopHandle(pSnd->mHandleWaveSound, 240);
            StartBgmChecked(pSnd, "RP_COMMON_BGM_PARTY_MENU");
            SetHandleVolume(pSnd->mHandleBgm, 1.0f, 120);
        }
    }
}

void RPSelectScene::EnterDecideGame() {
    mpSelectGameLyt->DecideGame(mCursor, mSpeakerFlag);
    mUsePad = false;
    mpCursor->SetActive(0);
}

void RPSelectScene::CalcDecideGame() {
    mpSelectGameLyt->UnhighlightOthers(mCursor);
    if (mpSelectGameLyt->IsSelectable()) {
        ChangeState(mpStateMachine, 9);
    }
}

void RPSelectScene::EnterBackPress() {
    mpMenuFrame->PushBackButton(mSpeakerFlag);
    mUsePad = false;
    mpCursor->SetActive(0);
}

void RPSelectScene::CalcBackHighlight() {
    if (mCursor != 9) {
        mpSelectGameLyt->HighlightGame(mCursor);
    } else {
        mpSelectGameLyt->UnhighlightAll();
    }
    if (mpMenuFrame->IsBackPushDone()) {
        ChangeState(mpStateMachine, 2);
    }
}

void RPSelectScene::EnterShowGameInfo() {
    mpSelectGameLyt->Close();
    if (mpStateMachine->mPrevState == 7) {
        mpGameInfoLyt->Open(mCursor);
        s32 step = -static_cast<s32>(255.0f / mpSelectGameLyt->GetAnmLength(1));
        RPDollMgr::GetInstance()->mAlphaStep = step;
    }
    mpMenuFrame->HidePtGame();
}

void RPSelectScene::CalcShowGameInfo() {
    ChangeState(mpStateMachine, 10);
}

void RPSelectScene::EnterWaitInfoOpen() {}

void RPSelectScene::CalcWaitInfoOpen() {
    if (mpGameInfoLyt->mpStateMachine->IsStateSettled(2) &&
        mpSelectGameLyt->mpStateMachine->IsStateSettled(5)) {
        ChangeState(mpStateMachine, 11);
    }
}

void RPSelectScene::EnterGameInfo() {}

void RPSelectScene::CalcGameInfo() {
    if (mpMenuFrame->IsPointOnBackButton(mPointerPos)) {
        mpMenuFrame->SetBackButtonActive(true,
                                         RPSysCoreController::getActiveController()->getPlayerNo());
    } else {
        mpMenuFrame->SetBackButtonActive(false, -1);
    }

    if (!mpMenuFrame->IsPointOnBackButton(mPointerPos) && RPCommon::IsAPressed(mPlayerNum)) {
        ChangeState(mpStateMachine, 12);
    } else if (mpMenuFrame->IsBackButtonIdle() &&
               (RPSysCoreController::getActiveController()->downTrigger(0x1400) ||
                (mpMenuFrame->IsPointOnBackButton(mPointerPos) && RPCommon::IsAPressed(mPlayerNum)))) {
        ChangeState(mpStateMachine, 13);
    }
}

void RPSelectScene::EnterInfoConfirm() {
    mpGameInfoLyt->Confirm(mSpeakerFlag);
}

void RPSelectScene::CalcWaitInfoConfirm() {
    if (IsGameInfoConfirmDone(mpGameInfoLyt)) {
        ChangeState(mpStateMachine, 15);
    }
}

void RPSelectScene::EnterBackButtonPress() {
    mpMenuFrame->PushBackButton(mSpeakerFlag);
}

void RPSelectScene::CalcWaitBackButton() {
    if (mpMenuFrame->IsBackPushDone()) {
        ChangeState(mpStateMachine, 14);
    }
}

void RPSelectScene::EnterReturnToSelect() {
    mpGameInfoLyt->Close();
    mpSelectGameLyt->Open();
    mpMenuFrame->ShowPtGame();
    s32 step = static_cast<s32>(255.0f / mpSelectGameLyt->GetAnmLength(0));
    RPDollMgr::GetInstance()->mAlphaStep = step;
}

void RPSelectScene::CalcGotoSelect() {
    ChangeState(mpStateMachine, 5);
}

void RPSelectScene::EnterFinish() {
    RPSndAudioMgr* pSnd = RPSndAudioMgr::instance();
    if (mpStateMachine->mPrevState == 12) {
        unk68 = 0;
        RPSceneInfo::spInstance->unk28 = mCursor;
        StopHandle(pSnd->mHandleBgm, 35);
    } else {
        unk68 = 1;
        StopHandle(pSnd->mHandleWaveSound, 240);
        StartBgmChecked(pSnd, "RP_COMMON_BGM_PARTY_MENU");
        SetHandleVolume(pSnd->mHandleBgm, 1.0f, 120);
    }
}

void RPSelectScene::HandleDollClicks(s32 player) {
    EGG::Vector2f pos(mPointerPos);
    u32 playerFlag = 1;
    if (player == 1) {
        playerFlag = 2;
        pos = RPSysCursorDrawMgr::instance()->getPlayerChannelCursorPos(
            RPSysCursorDrawMgr::EPlayer_P2);
    }

    for (s32 i = 0; i < mPlayerNum; i++) {
        RPDollMgr* pDollMgr = RPDollMgr::GetInstance();
        s32 hit = pDollMgr->HitTestDoll(pos, i);
        if (hit == -1 ||
            !EGG::CoreControllerMgr::instance()->getNthController(player)->downTrigger(
                0x800)) {
            continue;
        }

        RPSndAudioMgr* pSnd = RPSndAudioMgr::instance();
        if (hit == 0) {
            if (pDollMgr->RequestReaction(1, i)) {
                pSnd->startSystemSeWithRemote("RP_COMMON_SE_Party_DOLL_CLICK_01_CTSP", playerFlag);
                pSnd->startSystemSeWithRemote("RP_COMMON_SE_Party_DOLL_CLICK_01_TVSP", playerFlag);
            }
        } else if (hit == 1) {
            if (pDollMgr->RequestReaction(2, i)) {
                pSnd->startSystemSeWithRemote("RP_COMMON_SE_Party_DOLL_CLICK_02_CTSP", playerFlag);
                pSnd->startSystemSeWithRemote("RP_COMMON_SE_Party_DOLL_CLICK_02_TVSP", playerFlag);
            }
        } else if (hit == 2) {
            if (pDollMgr->RequestReaction(3, i)) {
                pSnd->startSystemSeWithRemote("RP_COMMON_SE_Party_DOLL_CLICK_HL_CTSP", playerFlag);
                pSnd->startSystemSeWithRemote("RP_COMMON_SE_Party_DOLL_CLICK_HL_TVSP", playerFlag);
            }
        } else if (hit == 3) {
            if (pDollMgr->RequestReaction(4, i)) {
                pSnd->startSystemSeWithRemote("RP_COMMON_SE_Party_DOLL_CLICK_HR_CTSP", playerFlag);
                pSnd->startSystemSeWithRemote("RP_COMMON_SE_Party_DOLL_CLICK_HR_TVSP", playerFlag);
            }
        } else if (hit == 4) {
            if (pDollMgr->RequestReaction(5, i)) {
                pSnd->startSystemSeWithRemote("RP_COMMON_SE_Party_DOLL_CLICK_FL_CTSP", playerFlag);
                pSnd->startSystemSeWithRemote("RP_COMMON_SE_Party_DOLL_CLICK_FL_TVSP", playerFlag);
            }
        } else if (hit == 5) {
            if (pDollMgr->RequestReaction(6, i)) {
                pSnd->startSystemSeWithRemote("RP_COMMON_SE_Party_DOLL_CLICK_FR_CTSP", playerFlag);
                pSnd->startSystemSeWithRemote("RP_COMMON_SE_Party_DOLL_CLICK_FR_TVSP", playerFlag);
            }
        }
    }
}

void RPSelectScene::UserDraw() {
    if (mpStateMachine->GetState() != 0 && RPGrpRenderer::GetCurrentViewNo() == 3 &&
        RPGrpRenderer::GetDrawStep() == 5) {
        mpSelectGameLyt->Draw();
        mpCursor->Draw();
        mpGameInfoLyt->Draw();
        mpMenuFrame->Draw();
        mpRankCompleteLyt->Draw();
    }
}

s32 RPSelectScene::GetResourceNum() {
    return RPDollMgr::GetResourceNum();
}

RPSysKokeshi* RPSelectScene::GetDollAvatar(s32 player) const {
    return RPDollMgr::GetInstance()->GetAvatar(player);
}

// Game index -> message index (RPGameInfoLyt calls it). Defined last, so it comes first in .text
// (0x801EF328); it shares the inline helper's initializer (0x8031D5B8) with Calculate.
s32 Party_gameIndexToMsgIndex(s32 game) {
    return GameIndexToMsgIndex(game);
}

// The state machine instantiations in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template void RPStateMachine<RPSelectScene>::Calculate();
template void RPStateMachine<RPSelectScene>::Reset(s32);
template void RPStateMachine<RPSelectScene>::SetStateFuncs(const StateFunc&, const StateFunc&,
                                                           s32);
template RPStateMachine<RPSelectScene>::RPStateMachine(RPSelectScene*, s32);
// RPStateMachine<RPGameInfoLyt>::IsStateSettled is instantiated here in the original (between the
// constructor and CallEnter); RPGameInfoLyt's own copy is dropped by the linker.
template bool RPStateMachine<RPGameInfoLyt>::IsStateSettled(s32) const;
template void RPStateMachine<RPSelectScene>::CallEnter();
template void RPStateMachine<RPSelectScene>::CallCalc();
template void RPStateMachine<RPSelectScene>::CallExit();
