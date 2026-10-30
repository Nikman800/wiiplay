#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPMiiSelect.h>
#include <Pack/RPParty/RPPartyCommon/RPPlayerConfirm.h>
#include <Pack/RPParty/RPPartyCommon/RPPlayerNumSelect.h>
#include <Pack/RPSystem/RPSysPlayer.h>

#include <Pack/RPSystem/RPSysPlayerSelectScene.h>
#include <Pack/RPParty/RPPartyCommon/RPCPadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPKokeshiMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPMenuBgModel.h>
#include <Pack/RPParty/RPPartyCommon/RPMenuFrame.h>
#include <Pack/RPParty/RPPartyCommon/RPPlayerSettingPanel.h>
#include <Pack/RPParty/RPPartyCommon/RPPtMenuBg.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpDrawPathXluSnap.h>
#include <Pack/RPGraphics/RPGrpModelManager.h>
#include <Pack/RPGraphics/RPGrpModelResManager.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPSystem/RPSysPlayerMgr.h>
#include <Pack/RPSystem/RPSysProjectLocal.h>
#include <Pack/RPSystem/RPSysQueuedScene.h>
#include <Pack/RPSystem/RPSysSceneCreator.h>
#include <Pack/RPSystem/RPSysSceneMgr.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPSystem/RPSysSaveDataMgr.h>
#include <Pack/RPKernel/RPSysController.h>
#include <Pack/RPKernel/RPSysCursorDrawMgr.h>
#include <Pack/RPKernel/RPSysHomeMenuMgr.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPKernel/RPSysSystem.h>
#include <Pack/RPKernel/RPSysSystemWinMgr.h>
#include <revolution/OS.h>

extern "C" {
u8 Lyt_getCurrentViewNo();
s32 Lyt_getDrawStep();
}

// The adjustment thunks are generated from the virtual overrides below.
#pragma push
#pragma auto_inline off
template bool RPStateMachine<RPPlayerConfirm>::IsStateSettled(s32) const;
#pragma pop

namespace {

// Wii Play: RPSndAudioMgr::startStream by name, converting the name once for the check and
// again for the start (RPTitleLyt)
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

// Wii Play: RPSndAudioMgr::startBgm by name, testing the fanfare flag before the conversion
inline void StartBgmByName(RPSndAudioMgr* pAudioMgr, const char* pName) {
    if (pAudioMgr->isDisableFanfare()) {
        return;
    }

    u32 id = pAudioMgr->changeNameToIdRP(pName);
    if (pAudioMgr->getBgmHandle().IsAttachedSound() && pAudioMgr->getBgmHandle().GetId() == id) {
        return;
    }

    pAudioMgr->getBgmHandle().Stop(0);
    pAudioMgr->startSound(&pAudioMgr->getBgmHandle(), id);

    if (RPSysProjectLocal::instance()->getFrameRate() == RPSysProjectLocal::EFrameRate_50Hz &&
        pAudioMgr->getSoundArchive().GetSoundType(id) == nw4r::snd::SOUND_TYPE_SEQ &&
        pAudioMgr->getBgmHandle().IsAttachedSound() &&
        (pAudioMgr->getSoundArchive().GetSoundUserParam(id) & 0x80000000)) {
        nw4r::snd::SeqSoundHandle handle(&pAudioMgr->getBgmHandle());
        handle.SetTempoRatio(50.0f / 60.0f);
    }
}

inline RPSportsPlayerData* GetSportsPlayerData(s32 player) {
    return RPSysPlayerMgr::instance()->getPlayer(player).mpSportsPlayerData;
}

inline u32 GetTimeSeconds() {
    return OS_TICKS_TO_SEC(OSGetTime());
}

} // namespace

// Seconds at the last game start; the confirm offers advice again after 15 minutes.
static u32 sLastStartTime;

static RPSysLayout* CreateBgLayout(EGG::Archive* pArchive);

RPSysPlayerSelectScene::~RPSysPlayerSelectScene() {}

void RPSysPlayerSelectScene::Configure() {
    RPGrpModelResManager::GetCurrent()->CreateResourceList(0x100);
    RPGrpModelManager* models = RPGrpModelManager::GetCurrent();
    models->CreateModelScene(0, 0, 1, NULL, NULL);
    models->CreateModelScene(1, 5, 1, NULL, NULL);
    models->GetDrawPathManager(1)->GetDrawPathXluSnap()->GetXluSnap()->onFlag(1);
    models->CreateModelScene(2, 0, 1, NULL, NULL);
    models->CreateModelScene(3, 0, 1, NULL, NULL);

    mState = 0;
    mWindowShown = false;
    mSaved = false;
    mLoadOldMii[0] = true;
    mLoadOldMii[1] = true;
    mLoadOldMii[2] = true;
    mLoadOldMii[3] = true;
    mOldMiiLoaded = false;
    mFrameShown = false;
    mPlayer = 0;
    mMenuMode = RPSysProjectLocal::instance()->getPack();
    mPauseTimer = 0;
    mAdviceMode = -1;
    mpMessage = NULL;
    mpBgModel = NULL;
    mpLogoModel = NULL;
    if (mMenuMode == 0) {
        u8 startFrame = 0;
        if (RPSysQueuedScene::instance()->getNextSceneID() == 11) {
            startFrame = 1;
        } else if (RPSysQueuedScene::instance()->getNextSceneID() == 10) {
            startFrame = 2;
        }
        mpBgModel = new RPMenuBgModel(0, 0, startFrame);
        mpLogoModel = new RPLogoModel(3, 2);
    } else {
        RPPtMenuBg::CreateInstance(0, 0, false);
    }
    mpBgLayout = NULL;
    mpFrame = new RPMenuFrame(mMenuMode);
    mpCPadCursor = new RPCPadCursor;
    mpWCursor = new RPWCursor;
    mpPlayerNumSelect = new RPPlayerNumSelect(mpFrame, mpCPadCursor);
    mpKokeshiMgr = new RPKokeshiMgr(2, 1);
    mpMiiSelect = new RPMiiSelect(mpFrame, mpCPadCursor, mpWCursor, mpKokeshiMgr);
    RPPlayerSettingPanel::CreateInstance();
    RPPlayerSettingPanel::instance()->Init();
    RPPlayerConfirm::CreateInstance(mMenuMode);

    RPGrpScreen* screen = new RPGrpScreen;
    screen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    RPGrpRenderer::GetCurrent()->CreateView2D(1, screen);
    screen = new RPGrpScreen;
    screen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    RPGrpRenderer::GetCurrent()->CreateView2D(4, screen);
    RPGrpRenderer::GetCurrent()->GetView(4)->SetDrawScene(3);
    RPGrpRenderer::GetCurrent()->LinkViews();
}

void RPSysPlayerSelectScene::LoadResource() {
    EGG::Archive* staticLocal;
    EGG::Archive* local = RPSysResourceManager::LoadGameLocalArchive(-1, NULL);
    staticLocal = RPSysResourceManager::instance()->GetStaticLocalArchive();
    EGG::Archive* common = RPSysResourceManager::LoadGameCommonArchive(-1, NULL);
    void* message = RPSysResourceManager::GetMessageResource("common_message.bmg");
    mpMessage = new RPSysMessage(message, NULL);
    if (mpBgModel != NULL) {
        mpBgModel->Load(common);
        mpLogoModel->Load(common);
    } else if (RPPtMenuBg::sInstance != NULL) {
        RPPtMenuBg::sInstance->Load(common);
    }
    if (mMenuMode == 0) {
        mpBgLayout = CreateBgLayout(local);
    }
    mpFrame->Load(local);
    mpCPadCursor->Load(staticLocal);
    mpWCursor->Load(staticLocal);
    mpPlayerNumSelect->Load(local, mpMessage);
    mpMiiSelect->Load(local, mpMessage);
    mpKokeshiMgr->Load(common);
    RPPlayerSettingPanel::instance()->Load(local, staticLocal);
    RPPlayerConfirm::sInstance->Load();
    void* effect = RPSysResourceManager::GetFileFromArchive(common, "pack_sequence.breff", NULL);
    void* texture = RPSysResourceManager::GetFileFromArchive(common, "pack_sequence.breft", NULL);
    getEffectCreator()->setEffectResource(effect, texture);
}

void RPSysPlayerSelectScene::Reset() {
    RPSysCoreController::sceneReset();
    RPSysCursorDrawMgr::instance()->startDpdCheck();
    mWindowShown = false;
    mSaved = false;
    mLoadOldMii[0] = true;
    mLoadOldMii[1] = true;
    mLoadOldMii[2] = true;
    mLoadOldMii[3] = true;
    mOldMiiLoaded = false;
    mFrameShown = false;
    mPlayer = 0;
    bool measure;
    if (mMenuMode == 0 && RPSysQueuedScene::instance()->getNextSceneID() == 11) {
        measure = true;
    } else {
        measure = false;
    }
    if (measure) {
        mState = 2;
    } else {
        mState = 0;
    }
    mNextState = -1;
    mPauseTimer = 0;
    mAdviceMode = -1;
    if (mpBgModel != NULL) {
        mpBgModel->Reset();
        mpLogoModel->Reset();
    } else if (RPPtMenuBg::sInstance != NULL) {
        RPPtMenuBg::sInstance->Reset(9);
    }
    mpFrame->Reset();
    mpFrame->ShowBackButton();
    if (mState == 0) {
        SetTitleMessage(0x191);
    } else {
        SetTitleMessage(0x190);
    }
    mpCPadCursor->Reset();
    mpWCursor->Reset();
    RPPlayerSettingPanel::instance()->Reset(mpFrame);
    RPPlayerConfirm::sInstance->Reset(mpFrame);
    RPSysCoreController::disableConnection();
    if (mMenuMode == 0) {
        if (RPSysQueuedScene::instance()->getNextSceneID() == 10) {
            StartStreamByName(RPSndAudioMgr::instance(), "STRM_Training_Select");
        } else if (RPSysQueuedScene::instance()->getNextSceneID() == 11) {
            StartStreamByName(RPSndAudioMgr::instance(), "STRM_Measurement_Start");
        } else {
            StartStreamByName(RPSndAudioMgr::instance(), "STRM_SportsPackSelect");
        }
    } else {
        StartBgmByName(RPSndAudioMgr::instance(), "RP_COMMON_BGM_PARTY_MENU");
    }
    RPSysHomeMenuMgr::instance()->disable(true);
}

static RPSysLayout* CreateBgLayout(EGG::Archive* pArchive) {
    RPSysLytResAccessor* accessor = RPSysLytResAccessor::create(NULL);
    void* archive = RPSysResourceManager::GetFileFromArchive(pArchive, "menuCommon.arc", NULL);
    accessor->mountArchive(archive, NULL);
    return RPSysLayout::create(NULL, accessor, "menuBg_00.brlyt");
}

// The confirm screen's Kokeshi attached in a file-local inline: its locals' new virtual registers
// put the first state-11 loop's pointer in r27 as in the original (written out in Calculate: r29).
// The reference keeps the member reloads.
static inline void Menu_AttachConfirmKokeshi(RPKokeshiMgr*& rpMgr) {
    RPPlayerConfirm* pConfirm;
    for (s32 i = 0; i < RPSysPlayerMgr::instance()->getPlayerNum(); i++) {
        pConfirm = RPPlayerConfirm::sInstance;
        rpMgr->AttachKokeshi(pConfirm->GetPlayerModelPane(i), NULL,
                             rpMgr->GetPlayer(i)->mSource,
                             rpMgr->GetPlayer(i)->mIndex);
    }
}

void RPSysPlayerSelectScene::Calculate() {
    if (!mFrameShown && isSceneDisplay()) {
        mFrameShown = true;
        mpFrame->ShowFrame();
    }
    mpFrame->Calculate();
    if (mpBgModel != NULL) {
        mpBgModel->Calculate();
        EGG::Vector2f logoPos = mpFrame->GetLogoPos();
        mpLogoModel->SetPosFromScreen(logoPos);
        mpLogoModel->Calculate();
    } else if (RPPtMenuBg::sInstance != NULL) {
        RPPtMenuBg::sInstance->Calculate();
    }

    if (RPSysCoreController::isDisconnectOccurred()) {
        CalcPaused();
    } else if (mState == 0) {
        RPSysCursorDrawMgr::instance()->createActiveCursor();
        mpPlayerNumSelect->Reset();
        RPSysPlayerMgr::instance()->reset();
        mState = 1;
    } else if (mState == 1) {
        CalcPlayerCountSelect();
    } else if (mState == 2) {
        mpPlayerNumSelect->Reset();
        RPSysPlayerMgr::instance()->reset();
        RPSysPlayerMgr::instance()->setPlayerNum(1);
        RPSysPlayerMgr::instance()->setRemoteNum(1);
        mPlayer = 0;
        mOldMiiLoaded = LoadPlayerMiis();
        if (mOldMiiLoaded) {
            mState = 10;
        } else {
            mState = 3;
        }
        SetTitleMessage(0x190);
        bool changed = mpKokeshiMgr->IsRecordsChanged();
        mpKokeshiMgr->ClearRecordsChanged();
        if (changed) {
            CommitPlayerMii(mState, 0);
        }
    } else if (static_cast<u32>(mState - 3) <= 2) {
        StartMiiSelect();
    } else if (mState == 6) {
        CalcMiiSelect();
    } else if (mState == 10) {
        CalcWaitMiiLoad();
    } else if (static_cast<u32>(mState - 7) <= 1) {
        StartPlayerSettings();
    } else if (mState == 9) {
        CalcPlayerSettings();
    } else if (mState == 11) {
        mSaved = false;
        RPSysCursorDrawMgr::instance()->createActiveCursor();
        s32 i;
        RPPlayerConfirm::Params params;
        params.playerNum = RPSysPlayerMgr::instance()->getPlayerNum();
        for (i = 0; i < params.playerNum; i++) {
            params.pHeads[i] = mpKokeshiMgr->GetPlayer(i)->mpHead;
            params.controllers[i] = RPSysPlayerMgr::instance()->getChannel(i);
        }
        if (mMenuMode == 0) {
            if (sLastStartTime == 0) {
                mAdviceMode = 1;
            } else if (RPSysQueuedScene::instance()->getNextSceneID() == 11) {
                mAdviceMode = 1;
            } else if (GetTimeSeconds() - sLastStartTime < 900) {
                mAdviceMode = 1;
            } else {
                mAdviceMode = 0;
            }
        } else {
            mAdviceMode = -1;
        }
        params.adviceId = mAdviceMode;
        bool checkABPress;
        if (mMenuMode == 0 && (RPSysQueuedScene::instance()->getNextSceneID() == 6 ||
                               RPSysQueuedScene::instance()->getNextSceneID() == 3 ||
                               RPSysQueuedScene::instance()->getNextSceneID() == 7 ||
                               RPSysQueuedScene::instance()->getNextSceneID() == 11)) {
            checkABPress = true;
        } else {
            checkABPress = false;
        }
        params.checkABPress = checkABPress;
        RPPlayerConfirm::sInstance->Start(params);
        mpKokeshiMgr->ResetAll();
        mpKokeshiMgr->SetViewMode(2, RPSysPlayerMgr::instance()->getPlayerNum());
        mpKokeshiMgr->SetVisible(true);
        Menu_AttachConfirmKokeshi(mpKokeshiMgr);
        mpFrame->HideName();
        mState = 12;
    } else if (mState == 12) {
        CalcConfirm();
    } else if (mState == 13) {
        CalcStartGame();
    } else if (mState == 14) {
        if (isSceneChangeEnable()) {
            if (mMenuMode == 0) {
                if (static_cast<u32>(RPSysQueuedScene::instance()->getNextSceneID() - 10) <= 1) {
                    RPSndAudioMgr::instance()->stopStream(30);
                }
                RPSysSceneCreator::instance()->changeSceneAfterFade(9, false);
            } else {
                RPSndAudioMgr::instance()->stopBgm(35);
                RPSysSceneCreator::instance()->changeSceneAfterFade(25, false);
            }
        }
    } else if (mState == 15) {
        mPlayer = 0;
        mState = 3;
        PlayControllerChangeSe(0, TRUE, FALSE);
    } else if (mState == 16) {
        if (RPSysSystemWinMgr::instance()->getResult() == RPSysSystemWinMgr::RESULT_CONTINUE) {
            if (mNextState == 11) {
                PlayControllerChangeSe(mPlayer, TRUE, TRUE);
            }
            mState = mNextState;
        }
        mpKokeshiMgr->Calculate();
    }

    if (mpBgLayout != NULL) {
        mpBgLayout->calc();
    }
    mpCPadCursor->Calculate();
    mpWCursor->Calculate();
}

void RPSysPlayerSelectScene::UserDraw() {
    if (Lyt_getCurrentViewNo() == 1) {
        if (Lyt_getDrawStep() == 5) {
            if (mpBgLayout != NULL) {
                mpBgLayout->draw();
            }
            mpPlayerNumSelect->DrawLayout();
            mpMiiSelect->DrawLayout();
            if (mState == 9) {
                RPPlayerSettingPanel::instance()->Draw();
            }
            RPPlayerConfirm::sInstance->DrawLayout();
            mpFrame->Draw();
            mpPlayerNumSelect->DrawCursor();
            if (mState == 9) {
                RPPlayerSettingPanel::instance()->DrawCursors();
            }
            RPPlayerConfirm::sInstance->DrawCursor();
        }
    } else if (Lyt_getCurrentViewNo() == 4 && Lyt_getDrawStep() == 5) {
        mpMiiSelect->DrawOverlay();
        RPPlayerConfirm::sInstance->DrawOverlay();
    }
}

void RPSysPlayerSelectScene::Exit() {
    RPPlayerSettingPanel::instance()->Exit();
    RPPlayerSettingPanel::DestroyInstance();
    RPPlayerConfirm::DestroyInstance();
    if (RPPtMenuBg::sInstance != NULL) {
        RPPtMenuBg::DestroyInstance();
    }
    RPSysCoreController::disableConnection();
}

void RPSysPlayerSelectScene::CalcPlayerCountSelect() {
    mpPlayerNumSelect->Calculate(isSceneDisplay());
    if (mpPlayerNumSelect->mPhase == 11) {
        RPSysPlayerMgr::instance()->setPlayerNum(mpPlayerNumSelect->mState + 1);
        RPSysPlayerMgr::instance()->setRemoteNum(mpPlayerNumSelect->mPlayerNum);
        mPlayer = 0;
        mOldMiiLoaded = LoadPlayerMiis();
        if (mOldMiiLoaded) {
            mState = 10;
        } else {
            mState = 3;
        }
        bool changed = mpKokeshiMgr->IsRecordsChanged();
        mpKokeshiMgr->ClearRecordsChanged();
        if (changed) {
            CommitPlayerMii(mState, 0);
        }
        if (mState == 3) {
            PlayControllerChangeSe(mPlayer, TRUE, FALSE);
        }
    } else if (mpPlayerNumSelect->mPhase == 13) {
        mState = 14;
    }
}

void RPSysPlayerSelectScene::StartMiiSelect() {
    if (mState != 3) {
        if (mState == 4) {
            RPSysCursorDrawMgr::instance()->setPlayerCursorOutOfScreen(
                static_cast<RPSysCursorDrawMgr::EPlayer>(mPlayer));
            mPlayer++;
            PlayControllerChangeSe(mPlayer, FALSE, FALSE);
        } else if (mState == 5) {
            RPSysCursorDrawMgr::instance()->setPlayerCursorOutOfScreen(
                static_cast<RPSysCursorDrawMgr::EPlayer>(mPlayer));
            mPlayer--;
            PlayControllerChangeSe(mPlayer, FALSE, FALSE);
        }
    }
    RPSysCursorDrawMgr::instance()->createPlayerCursor(
        static_cast<RPSysCursorDrawMgr::EPlayer>(mPlayer));
    mpKokeshiMgr->ResetAll();
    mpKokeshiMgr->SetViewMode(0, 1);
    mpMiiSelect->Start(mPlayer);
    mState = 6;
}

void RPSysPlayerSelectScene::CalcMiiSelect() {
    mpMiiSelect->Calculate(isSceneDisplay());
    mpKokeshiMgr->Calculate();
    if (mpMiiSelect->mState == 16) {
        RPMiiSelect::Selection sel = mpMiiSelect->GetSelectedMii();
        mpKokeshiMgr->SetPlayerSlot(sel.mSource, sel.mIndex, mPlayer);
        RPKokeshiMgr::Player slot = *mpKokeshiMgr->GetPlayer(mPlayer);
        RFLCreateID createID = slot.mpHead->GetAdditionalInfo().createID;
        RPSysPlayerMgr::instance()->setAvatar(slot.mType, slot.mMiiIndex, createID, mPlayer);
        RPSysPlayerMgr::instance()->loadData(mPlayer);
        if (mMenuMode == 0) {
            mState = 7;
        } else if (mPlayer + 1 >= mpPlayerNumSelect->mState + 1) {
            CommitPlayerMii(11, 1);
        } else {
            CommitPlayerMii(4, 1);
        }
    } else if (mpMiiSelect->mState == 19) {
        if (mPlayer == 0) {
            bool measure;
            if (mMenuMode == 0 && RPSysQueuedScene::instance()->getNextSceneID() == 11) {
                measure = true;
            } else {
                measure = false;
            }
            if (measure) {
                mState = 14;
            } else {
                mState = 0;
            }
        } else if (mMenuMode == 0) {
            mState = 8;
        } else {
            mState = 5;
        }
    }
}

void RPSysPlayerSelectScene::CalcWaitMiiLoad() {
    bool ready = true;
    for (s32 player = 0; player < RPSysPlayerMgr::instance()->getPlayerNum(); player++) {
        if (RPSysPlayerMgr::instance()->isCtrlOldData(player)) {
            s32 channel = RPSysPlayerMgr::instance()->getChannel(player);
            if (!mpKokeshiMgr->IsRemoteMiiReady(RP_GET_CORE_CTRL(channel)->getChannelID())) {
                ready = false;
            }
        }
    }
    if (ready) {
        for (s32 player = 0; player < RPSysPlayerMgr::instance()->getPlayerNum(); player++) {
            s8 type;
            u16 index;
            RPSysPlayerMgr::instance()->getOldData(&type, &index, player);
            mpKokeshiMgr->SetPlayerSlotByType(type, index, player);
            RPKokeshiMgr::Player slot = *mpKokeshiMgr->GetPlayer(player);
            RFLCreateID createID = slot.mpHead->GetAdditionalInfo().createID;
            RPSysPlayerMgr::instance()->setAvatar(slot.mType, slot.mMiiIndex, createID, player);
            RPSysPlayerMgr::instance()->loadData(player);
        }
        mPlayer = RPSysPlayerMgr::instance()->getPlayerNum() - 1;
        mState = 11;
    }
}

void RPSysPlayerSelectScene::StartPlayerSettings() {
    if (mState != 7) {
        if (mState == 8) {
            RPSysCursorDrawMgr::instance()->setPlayerCursorOutOfScreen(
                static_cast<RPSysCursorDrawMgr::EPlayer>(mPlayer));
            mPlayer--;
            PlayControllerChangeSe(mPlayer, FALSE, FALSE);
        }
    }
    RPSysCursorDrawMgr::instance()->createPlayerCursor(
        static_cast<RPSysCursorDrawMgr::EPlayer>(mPlayer));
    RPSportsPlayerData* pData = GetSportsPlayerData(mPlayer);
    RPPlayerSettingPanel* pPanel = RPPlayerSettingPanel::instance();
    pPanel->SetOption(0, pData->getHandPrimary(RPSportsPlayerData::ESport_Tns));
    pPanel->SetOption(1, pData->getHandPrimary(RPSportsPlayerData::ESport_Bsb));
    pPanel->SetOption(2, pData->getHandSecondary(RPSportsPlayerData::ESport_Bsb));
    pPanel->SetOption(3, pData->getHandPrimary(RPSportsPlayerData::ESport_Box));
    pPanel->SetOption(4, pData->getHandPrimary(RPSportsPlayerData::ESport_Gol));
    pPanel->SetOption(5, pData->getHandPrimary(RPSportsPlayerData::ESport_Bow));
    pPanel->SetPlayer(RPSysPlayerMgr::instance()->getChannel(mPlayer), mPlayer);
    if (RPSysQueuedScene::instance()->getNextSceneID() == 6) {
        pPanel->SetMode(5);
    } else if (RPSysQueuedScene::instance()->getNextSceneID() == 3) {
        pPanel->SetMode(5);
    } else if (RPSysQueuedScene::instance()->getNextSceneID() == 4) {
        // through a reference: with four identical pPanel->SetMode(5) arms CW folds the last
        // three into one (if (c) A; else A;), the original keeps all four
        RPPlayerSettingPanel& rPanel = *pPanel;
        rPanel.SetMode(5);
    } else {
        pPanel->SetMode(5);
    }
    pPanel->Open();
    mpKokeshiMgr->ResetAll();
    mpKokeshiMgr->SetViewMode(1, 1);
    mpKokeshiMgr->SetVisible(true);
    mpKokeshiMgr->AttachKokeshi(mpFrame->GetModelPane(), NULL,
                                mpKokeshiMgr->GetPlayer(mPlayer)->mSource,
                                mpKokeshiMgr->GetPlayer(mPlayer)->mIndex);
    mpFrame->ShowName(mPlayer);
    mpFrame->SetPlayerNameText(mpMessage, 0x1C7, 0,
                               mpKokeshiMgr->GetPlayer(mPlayer)->mpHead->GetAdditionalInfo().name);
    mState = 9;
}

void RPSysPlayerSelectScene::CalcPlayerSettings() {
    RPPlayerSettingPanel* pPanel = RPPlayerSettingPanel::instance();
    pPanel->Calculate();
    mpFrame->ShowName(mPlayer);
    mpKokeshiMgr->Calculate();
    if (pPanel->IsConfirmed()) {
        RPSportsPlayerData* pData = GetSportsPlayerData(mPlayer);
        pData->setHandPrimary(static_cast<RPSportsPlayerData::EHand>(pPanel->GetOption(0)),
                              RPSportsPlayerData::ESport_Tns);
        pData->setHandPrimary(static_cast<RPSportsPlayerData::EHand>(pPanel->GetOption(1)),
                              RPSportsPlayerData::ESport_Bsb);
        pData->setHandSecondary(static_cast<RPSportsPlayerData::EHand>(pPanel->GetOption(2)),
                                RPSportsPlayerData::ESport_Bsb);
        pData->setHandPrimary(static_cast<RPSportsPlayerData::EHand>(pPanel->GetOption(3)),
                              RPSportsPlayerData::ESport_Box);
        pData->setHandPrimary(static_cast<RPSportsPlayerData::EHand>(pPanel->GetOption(4)),
                              RPSportsPlayerData::ESport_Gol);
        pData->setHandPrimary(static_cast<RPSportsPlayerData::EHand>(pPanel->GetOption(5)),
                              RPSportsPlayerData::ESport_Bow);
        if (mPlayer + 1 >= mpPlayerNumSelect->mState + 1) {
            CommitPlayerMii(11, 1);
        } else {
            CommitPlayerMii(4, 1);
        }
    } else if (pPanel->IsCancelled()) {
        mState = 3;
    }
}

namespace {

// The confirmation closed (state 7 or 8).
inline bool IsConfirmClosed() {
    bool closed = true;
    RPPlayerConfirm* pConfirm = RPPlayerConfirm::sInstance;
    if (!pConfirm->mpStateMachine->IsStateSettled(7) &&
        !pConfirm->mpStateMachine->IsStateSettled(8)) {
        closed = false;
    }
    return closed;
}

} // namespace

#pragma push
#pragma opt_propagation off
void RPSysPlayerSelectScene::CalcConfirm() {
    RPPlayerConfirm* pConfirm = RPPlayerConfirm::sInstance;
    pConfirm->Calculate();
    if (IsConfirmClosed() && pConfirm->mResult != 0) {
        mpKokeshiMgr->SetVisible(false);
    }
    mpFrame->HideName();
    mpKokeshiMgr->Calculate();
    if (!mSaved && pConfirm->mResult == 0) {
        mSaved = true;
        RPSysPlayerMgr::instance()->saveAllData();
        RPSysPlayerMgr::instance()->calcOldData();
        RPSysPlayerMgr::instance()->saveOldData();
        RPSysSaveDataMgr::instance()->saveAsync();
    } else if (RPPlayerConfirm::sInstance->mpStateMachine->IsStateSettled(8) &&
               RPSysSaveDataMgr::instance()->isAsyncFinish()) {
        s32 result = RPPlayerConfirm::sInstance->mResult;
        if (mSaved && result == 0) {
            if (sLastStartTime == 0 || mAdviceMode == 0) {
                sLastStartTime = GetTimeSeconds();
            }
            mState = 13;
        } else if (result == 1) {
            mOldMiiLoaded = false;
            mState = 15;
        } else if (mOldMiiLoaded) {
            if (mMenuMode == 0) {
                if (RPSysQueuedScene::instance()->getNextSceneID() == 11) {
                    mState = 14;
                } else {
                    mState = 0;
                }
            } else {
                mState = 0;
            }
        } else {
            if (mMenuMode == 0) {
                mState = 7;
            } else {
                mState = 3;
            }
            PlayControllerChangeSe(mPlayer, TRUE, FALSE);
        }
    }
}
#pragma pop

void RPSysPlayerSelectScene::CalcStartGame() {
    mpKokeshiMgr->Calculate();
    if (isSceneChangeEnable()) {
        if (mMenuMode == 0) {
            s32 next = RPSysQueuedScene::instance()->getNextSceneID();
            if (next == 10) {
                RPSysSceneCreator::instance()->changeSceneAfterFade(next, false);
            } else if (next == 11) {
                RPSysSceneCreator::instance()->changeSceneAfterFade(next, false);
            } else if (next == 26) {
                RPSysSceneCreator::instance()->changeSceneAfterFade(26, false);
            } else {
                RPSysSceneCreator::instance()->changeSceneAfterFade(2, false);
            }
        } else {
            RPSysSceneCreator::instance()->changeSceneAfterFade(26, false);
        }
    }
}

void RPSysPlayerSelectScene::CommitPlayerMii(s32 nextState, s32 kind) {
    RPSysSystemWinMgr* pWinMgr = RPSysSystemWinMgr::instance();
    if (kind == 0) {
        if (mMenuMode == 0) {
            pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 0x122, NULL, 0);
        } else {
            pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 0x123, NULL, 0);
        }
        pWinMgr->setCursors();
        mNextState = nextState;
        mState = 16;
    } else if (kind == 1) {
        RPSportsPlayerData* pSportsData;
        RPPartyPlayerData* pPartyData;
        bool registered = false;
        const RPSysPlayer& rPlayer = RPSysPlayerMgr::instance()->getPlayer(mPlayer);
        pSportsData = rPlayer.mpSportsPlayerData;
        pPartyData = rPlayer.mpPartyPlayerData;
        RPKokeshiMgr::Player slot = *mpKokeshiMgr->GetPlayer(mPlayer);
        RFLCreateID createID = slot.mpHead->GetAdditionalInfo().createID;
        if (pSportsData != NULL) {
            registered = pSportsData->isRegistered();
            pSportsData->setRegistered(true);
            pSportsData->setDebutTime(RPSysSystem::instance()->getCalendarTime32());
            pSportsData->setCreateID(&createID);
        } else if (pPartyData != NULL) {
            registered = pPartyData->isRegistered();
            pPartyData->setRegistered(true);
            pPartyData->setDebutTime(RPSysSystem::instance()->getCalendarTime32());
            pPartyData->setCreateID(&createID);
        }
        if (!registered && slot.mSource == 0) {
            if (mMenuMode == 0) {
                pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 0x118, NULL, 1,
                                         slot.mpHead->GetAdditionalInfo().name);
            } else {
                pWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 0x119, NULL, 1,
                                         slot.mpHead->GetAdditionalInfo().name);
            }
            pWinMgr->setPlayerInput(mPlayer);
            mNextState = nextState;
            mState = 16;
        } else {
            if (nextState == 11) {
                PlayControllerChangeSe(mPlayer, TRUE, TRUE);
            }
            mState = nextState;
        }
    }
}

void RPSysPlayerSelectScene::CalcPaused() {
    if (!RPSysSceneMgr::instance()->isShutDownReserved() && mPauseTimer > 30) {
        RPSysSystemWinMgr* window = RPSysSystemWinMgr::instance();
        if (!mWindowShown) {
            window->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 0x10F, NULL, 0);
            window->setCursors();
            if ((mState >= 3 && mState <= 10) ||
                (mState == 16 && mNextState >= 3 && mNextState <= 11)) {
                PlayControllerChangeSe(mPlayer, TRUE, TRUE);
            }
        } else if (window->getResult() == RPSysSystemWinMgr::RESULT_CONTINUE) {
            RPSysSceneCreator::instance()->changeSceneAfterFade(-1, true);
        }
        mWindowShown = RPSysCoreController::isDisconnectOccurred();
    }
    mpKokeshiMgr->Calculate();
    mPauseTimer++;
}

bool RPSysPlayerSelectScene::LoadPlayerMiis() {
    bool loaded = false;
    s32 playerNum = RPSysPlayerMgr::instance()->getPlayerNum();
    s32 index = playerNum - 1;
    if (mLoadOldMii[index]) {
        RPSysPlayerMgr::instance()->loadOldData();
        loaded = RPSysPlayerMgr::instance()->isValidOldData();
        if (loaded) {
            for (s32 i = 0; i < playerNum; i++) {
                if (RPSysPlayerMgr::instance()->isCtrlOldData(i)) {
                    s32 channel = RPSysPlayerMgr::instance()->getChannel(i);
                    if (RP_GET_CORE_CTRL(channel)->isCoreBaseDevice()) {
                        mpKokeshiMgr->RequestRemoteMii(RP_GET_CORE_CTRL(channel)->getChannelID());
                    } else {
                        loaded = false;
                    }
                }
            }
        }
    }
    if (loaded) {
        mLoadOldMii[index] = false;
    }
    return loaded;
}

void RPSysPlayerSelectScene::SetTitleMessage(u32 msgId) {
    if (mMenuMode == 0) {
        if (RPSysQueuedScene::instance()->getNextSceneID() == 6) {
            mpFrame->SetCaption(0x192, mpMessage, 0x192, 0, 0);
        } else if (RPSysQueuedScene::instance()->getNextSceneID() == 3) {
            mpFrame->SetCaption(0x193, mpMessage, 0x193, 0, 0);
        } else if (RPSysQueuedScene::instance()->getNextSceneID() == 4) {
            mpFrame->SetCaption(0x194, mpMessage, 0x194, 0, 0);
        } else if (RPSysQueuedScene::instance()->getNextSceneID() == 14) {
            mpFrame->SetCaption(0x195, mpMessage, 0x195, 0, 0);
        } else if (RPSysQueuedScene::instance()->getNextSceneID() == 7) {
            mpFrame->SetCaption(0x196, mpMessage, 0x196, 0, 0);
        } else if (RPSysQueuedScene::instance()->getNextSceneID() == 10) {
            mpFrame->SetCaption(0x197, mpMessage, 0x197, 0, 0);
        } else if (RPSysQueuedScene::instance()->getNextSceneID() == 11) {
            mpFrame->SetCaption(0x198, mpMessage, 0x198, 0, 0);
        }
    } else {
        mpFrame->SetCaption(msgId, mpMessage, msgId, 0, 0);
    }
}

void RPSysPlayerSelectScene::PlayControllerChangeSe(s32 player, BOOL ignoreActive, BOOL useActive) {
    s32 remotes = RPSysPlayerMgr::instance()->getRemoteNum();
    s32 channel = RPSysPlayerMgr::instance()->getChannel(player);
    s32 active = RPSysCoreController::getActiveController()->getPlayerNo();
    if (remotes > 1 && (!ignoreActive || channel != active)) {
        if (useActive) {
            channel = active;
        }
        switch (channel) {
        case 0:
            RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_CON_Change_01", 1);
            break;
        case 1:
            RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_CON_Change_01", 2);
            break;
        case 2:
            RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_CON_Change_01", 4);
            break;
        case 3:
            RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_CON_Change_01", 8);
            break;
        }
        RP_GET_CORE_CTRL(channel)->rumble(RPSysCoreController::ERumblePattern_Change);
    }
}
