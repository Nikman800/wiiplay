#include <Pack/RPParty/RPPartyCommon/RPSportsMessageScene.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPGraphics/RPGrpModelResManager.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPKernel/RPSysCursorDrawMgr.h>
#include <Pack/RPKernel/RPSysKokeshiGenInfo.h>
#include <Pack/RPKernel/RPSysKokeshiIcon.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPKernel/RPSysSystemWinMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPFrameDown.h>
#include <Pack/RPSystem/RPSysPlayerMgr.h>
#include <Pack/RPSystem/RPSysProjectLocal.h>
#include <Pack/RPSystem/RPSysQueuedScene.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPSystem/RPSysSceneCreator.h>

#include <egg/core/eggController.h>

extern "C" void Lyt_setPaneColorFromTable(nw4r::lyt::Pane* pPane, u32 player);

/**
 * @brief Sports Pack selection (Wii Sports leftover, never set in Wii Play)
 */
struct RPSysNunchukCheckSportInfo {
    u32 unk0;     // at 0x0
    s32 mSportID; // at 0x4 RPSysSceneCreator::ESportID
    s32 mMode;    // at 0x8 0: training, 1: fitness test, 2: free play
};

// Defined by a later file (.sbss 8043C6B8 lies between RPPlayerSettingPanel::spInstance and
// RPPlayerConfirm::sInstance, after this file in link order); this file only reads it

static const char* sArcName = "nunchukCheck.arc";
static const char* sLayoutName = "nunchukCheck_00.brlyt";
static const char* sNunchukPaneNames[2] = {"N_nunchuk_01", "N_nunchuk_00"};
static const char* sNunchukTextNames[2] = {"T_nunchuk_00", "T_nunchuk_01"};
static const u16 sNunchukMsgIDs[2] = {0x352, 0x353};

// Wii Sports leftovers: the scene's own back button (RPFrameDown replaced it)
static const char* sBackButtonPaneName = "N_backButton_00";
static const char* sBackButtonAnmNames[2] = {"nunchukCheck_00_activeBack_00.brlan",
                                             "nunchukCheck_00_pushBack_00.brlan"};
static const char* sBackButtonBoundingName = "B_backButton_00";
static const char* sCaptionName = "T_caption_01";

const char* RPSportsMsgPanel::sPaneNames[4][7] = {
    {
        "N_player_00", "N_ok_00", "N_LED_00", "P_P1LED_00",
        "P_P2LED_00", "P_P3LED_00", "P_P4LED_00",
    }, {
        "N_player_01", "N_ok_01", "N_LED_01", "P_P1LED_01",
        "P_P2LED_01", "P_P3LED_01", "P_P4LED_01",
    }, {
        "N_player_02", "N_ok_02", "N_LED_02", "P_P1LED_02",
        "P_P2LED_02", "P_P3LED_02", "P_P4LED_02",
    }, {
        "N_player_03", "N_ok_03", "N_LED_03", "P_P1LED_03",
        "P_P2LED_03", "P_P3LED_03", "P_P4LED_03",
    }
};

static const char* sAnimationNames[8] = {
    "nunchukCheck_00_inOK_00.brlan", "nunchukCheck_00_inOK_00.brlan",
    "nunchukCheck_00_outOK_00.brlan", "nunchukCheck_00_outOK_00.brlan",
    "nunchukCheck_00_LED_00.brlan", "nunchukCheck_00_LED_00.brlan",
    "nunchukCheck_00_LED_00.brlan", "nunchukCheck_00_LED_00.brlan",
};

static const char* sColorNames[4][2] = {
    {"P_pColor1_00", "P_pColor2_00"},
    {"P_pColor1_01", "P_pColor2_01"},
    {"P_pColor1_02", "P_pColor2_02"},
    {"P_pColor1_03", "P_pColor2_03"},
};

static const char* sPlayerNames[4] = {
    "T_pName_00", "T_pName_01", "T_pName_02", "T_pName_03",
};

static const char* sOkNames[4] = {
    "T_ok_00", "T_ok_01", "T_ok_02", "T_ok_03",
};

// Wii Sports leftovers: back button sounds
static const char* sBackSoundName = "RP_COMMON_SE_KEY_BACK_01";
static const char* sCursorSoundName = "RP_COMMON_SE_KEY_CURSOR_01";

/******************************************************************************
 *
 * RPSportsMsgPanel (inline parts)
 *
 ******************************************************************************/

inline void RPSportsMsgPanel::SkipIn(u32 anm, u32 pane) {
    RPSysLytAnmObj* pAnmObj = mpAnmObjs[anm];
    nw4r::lyt::Pane* pPane = mpPanes[pane];
    mpLayout->unbindAnmObj(pAnmObj);
    pAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    pAnmObj->play(pAnmObj->getFrameMax() - 1.0f);
    RPSysLayout::forceBindAnmObjToPane(pAnmObj, pPane, true);
}

inline s32 RPSportsMsgPanel::PlayIn(u32 anm, u32 pane) {
    // Unused; puts 1.0f ahead of the int-to-float constant in .sdata2 (Calculate is compiled
    // before Start, which is the only real user)
    f32 lastFrameOffset = 1.0f;
    RPSysLytAnmObj* pAnmObj = mpAnmObjs[anm];
    nw4r::lyt::Pane* pPane = mpPanes[pane];
    mpLayout->unbindAnmObj(pAnmObj);
    pAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    pAnmObj->restart();
    RPSysLayout::forceBindAnmObjToPane(pAnmObj, pPane, true);
    return pAnmObj->getFrameCtrl()->getEndFrame() - pAnmObj->getFrameCtrl()->getStartFrame();
}

inline void RPSportsMsgPanel::SetLED(u32 anm, u32 pane) {
    s16 frame = mChannel;
    RPSysLytAnmObj* pAnmObj = mpAnmObjs[anm];
    nw4r::lyt::Pane* pPane = mpPanes[pane];
    mpLayout->unbindAnmObj(pAnmObj);
    pAnmObj->init(frame, frame, RPSysFrameCtrl::EPlayMode_OneTime);
    pAnmObj->stop();
    RPSysLayout::forceBindAnmObjToPane(pAnmObj, pPane, true);
}

inline void RPSportsMsgPanel::Reset() {
    mActive = false;
    mReady = false;
    mShownReady = false;
    mChannel = 0;
    mTimer = 0;
    mpPanes[PANE_PLAYER]->SetVisible(false);
}

inline void RPSportsMsgPanel::Start(u8 channel, bool ready, const wchar_t* pName) {
    mActive = true;
    mReady = ready;
    mShownReady = ready;
    mChannel = channel;

    if (ready) {
        SkipIn(ANM_IN_OK, PANE_OK);
        mTimer = 0;
        SkipIn(ANM_IN_OK_2, PANE_LED);
    } else {
        mTimer = PlayIn(ANM_OUT_OK, PANE_OK);
        PlayIn(ANM_OUT_OK_2, PANE_LED);
    }

    SetLED(ANM_LED + 0, PANE_P1LED + 0);
    SetLED(ANM_LED + 1, PANE_P1LED + 1);
    SetLED(ANM_LED + 2, PANE_P1LED + 2);
    SetLED(ANM_LED + 3, PANE_P1LED + 3);

    mpTextBox->setMessage(mpMessage, 0x366, 0, pName);
    mpPanes[PANE_PLAYER]->SetVisible(true);
}

inline void RPSportsMsgPanel::Calculate() {
    if (mActive) {
        if (mTimer > 0) {
            mTimer--;
        } else if (mTimer == 0 && mReady != mShownReady) {
            mShownReady = mReady;
            if (mReady) {
                mTimer = PlayIn(ANM_IN_OK, PANE_OK);
                PlayIn(ANM_IN_OK_2, PANE_LED);
            } else {
                mTimer = PlayIn(ANM_OUT_OK, PANE_OK);
                PlayIn(ANM_OUT_OK_2, PANE_LED);
            }

            SetLED(ANM_LED + 0, PANE_P1LED + 0);
            SetLED(ANM_LED + 1, PANE_P1LED + 1);
            SetLED(ANM_LED + 2, PANE_P1LED + 2);
            SetLED(ANM_LED + 3, PANE_P1LED + 3);
        }
    }
}

/******************************************************************************
 *
 * RPSysNunchukCheckScene
 *
 ******************************************************************************/

RPSysNunchukCheckScene::~RPSysNunchukCheckScene() {}

void RPSysNunchukCheckScene::ResetState() {
    RPSndAudioMgr::instance()->configureNoBgm();
    mState = EState_Start;
    mPlayerIdx = 0;
    mPlayerNum = RPSysPlayerMgr::instance()->getPlayerNum();

    if (RPSysProjectLocal::instance()->getPack() == RPSysSceneCreator::EPackID_SportsPack) {
        mNeedNunchuk = IsNunchukGame();
    } else {
        mNeedNunchuk = false;
    }
}

void RPSysNunchukCheckScene::Configure() {
    RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();
    mpScreen = new RPGrpScreen();
    mpScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    pRenderer->CreateView2D(0, static_cast<RPGrpScreen*>(mpScreen));
    pRenderer->LinkViews();

    bool fitness;
    if (RPSysProjectLocal::instance()->getPack() == RPSysSceneCreator::EPackID_SportsPack) {
        fitness = spSportInfo->mMode == 2;
    } else {
        fitness = false;
    }

    if (fitness) {
        mpFrameDown = new RPFrameDown(1);
    } else {
        mpFrameDown = new RPFrameDown(0);
    }

    for (u32 i = 0; i < PLAYER_NUM; i++) {
        mpPanels[static_cast<u8>(i)] = new RPSportsMsgPanel;
    }

    RPGrpModelResManager::GetCurrent()->CreateResourceList(0x100);
    ResetState();

    RPSysKokeshiGenInfo info;
    info.SetIconWidth(0x80);
    info.SetIconHeight(0x80);
    info.SetIconView(RPSysKokeshiManager::IconView_Front);
    info.SetGenType(RPSysKokeshiManager::GenType_Player);

    for (mPlayerIdx = 0; mPlayerIdx < mPlayerNum; mPlayerIdx++) {
        info.SetPlayerNo(mPlayerIdx);
        mpIcons[mPlayerIdx] = new RPSysKokeshiIcon(&info);
    }
}

void RPSysNunchukCheckScene::LoadResource() {
    u32 i;

    EGG::Archive* pArchive = RPSysResourceManager::LoadGameLocalArchive();
    void* pLytArc = RPSysResourceManager::GetFileFromArchive(pArchive, sArcName, NULL);
    RPSysLytResAccessor* pAccessor = RPSysLytResAccessor::create();
    pAccessor->mountArchive(pLytArc);
    mpLayout = RPSysLayout::create(NULL, pAccessor, sLayoutName);

    for (i = 0; i < 2; i++) {
        mpNunchukPanes[static_cast<u16>(i)] =
            mpLayout->findPane(sNunchukPaneNames[static_cast<u16>(i)]);
    }

    void* pCommonRes = RPSysResourceManager::GetMessageResource("common_message.bmg");
    RPSysMessage* pCommonMsg = new RPSysMessage(pCommonRes, NULL);

    for (i = 0; i < 2; i++) {
        mpLayout->findTextBox(sNunchukTextNames[static_cast<u16>(i)])
            ->setMessage(pCommonMsg, sNunchukMsgIDs[static_cast<u16>(i)], 0, 0);
    }

    void* pMsgRes;
    switch (RPSysProjectLocal::instance()->getPack()) {
    case RPSysSceneCreator::EPackID_SportsPack:
        pMsgRes = RPSysResourceManager::GetMessageResource("sports_message.bmg");
        break;
    }
    mpMessage = new RPSysMessage(pMsgRes, NULL);

    mpFrameDown->Load(pArchive);

    for (i = 0; i < PLAYER_NUM; i++) {
        mpPanels[static_cast<u16>(i)]->Init(static_cast<u16>(i), pAccessor, mpLayout,
                                             pCommonMsg);
    }

    for (mPlayerIdx = 0; mPlayerIdx < mPlayerNum; mPlayerIdx++) {
        mpIcons[mPlayerIdx]->LoadResource();
    }
}

void RPSysNunchukCheckScene::Reset() {
    mSkip = false;
    ResetState();

    mpNunchukPanes[1]->SetVisible(false);
    mpNunchukPanes[0]->SetVisible(false);

    mpFrameDown->Reset();
    mBackHovered = false;

    for (int i = 0; i < PLAYER_NUM; i++) {
        mpPanels[i]->Reset();
        mHoldFrames[i] = 0;
    }

    mpLayout->reset();
}

void RPSysNunchukCheckScene::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() == 0) {
        switch (RPGrpRenderer::GetDrawStep()) {
        case 5:
            if (!mSkip) {
                mpLayout->draw();
                mpFrameDown->Draw();
            }
            break;
        }
    }
}

// Registers match only with copy propagation off (the second loop written out like
// CheckPlayersReady, with an idx copy and the Nunchuk flag copied into a local) plus loop-invariant
// motion off (otherwise the second loop gets an entry test). Under copy propagation off, the
// CoreControllerMgr load is scheduled first only when it is read into a named local; in the
// second loop's remote-only branch the original reads it into a local and then calls through
// instance() again (a local used as the call's object takes idx's register r22).
#pragma push
#pragma opt_propagation off
#pragma opt_loop_invariants off
void RPSysNunchukCheckScene::Start() {
    mState = EState_Check;

    u8 readyNum = 0;
    for (mPlayerIdx = 0; mPlayerIdx < mPlayerNum; mPlayerIdx++) {
        if (mNeedNunchuk) {
            EGG::CoreControllerMgr* pMgr = EGG::CoreControllerMgr::instance();
            EGG::CoreController* pCtrl =
                pMgr->getNthController(RPSysPlayerMgr::instance()->getChannel(mPlayerIdx));
            bool need = mNeedNunchuk;
            readyNum += pCtrl->isFreestyle() == need;
        } else {
            EGG::CoreControllerMgr* pMgr = EGG::CoreControllerMgr::instance();
            EGG::CoreController* pCtrl =
                pMgr->getNthController(RPSysPlayerMgr::instance()->getChannel(mPlayerIdx));
            bool ready = pCtrl->getDevType() == EGG::cDEV_CORE;
            mHoldFrames[mPlayerIdx] = 20;
            readyNum += ready;
        }
    }

    if (mPlayerIdx == readyNum) {
        mSkip = true;
        mState = EState_StartGame;
        return;
    }

    mState = EState_Check;
    RPSysCursorDrawMgr::instance()->startDpdCheck();
    RPSysCursorDrawMgr::instance()->createActiveCursor();
    mpNunchukPanes[0]->SetVisible(!mNeedNunchuk);
    mpNunchukPanes[1]->SetVisible(mNeedNunchuk);

    mpFrameDown->RequestIn();
    while (mpFrameDown->IsIdle()) {
        mpFrameDown->Calculate();
    }

    for (mPlayerIdx = 0; mPlayerIdx < mPlayerNum; mPlayerIdx++) {
        u8 idx = mPlayerIdx;
        bool ready;
        if (mNeedNunchuk) {
            EGG::CoreControllerMgr* pMgr = EGG::CoreControllerMgr::instance();
            EGG::CoreController* pCtrl =
                pMgr->getNthController(RPSysPlayerMgr::instance()->getChannel(idx));
            bool need = mNeedNunchuk;
            ready = pCtrl->isFreestyle() == need;
        } else {
            // pMgr is unused here (the call reads the instance again); the early read is what
            // schedules the CoreControllerMgr load first while it keeps r21
            EGG::CoreControllerMgr* pMgr = EGG::CoreControllerMgr::instance();
            EGG::CoreController* pCtrl = EGG::CoreControllerMgr::instance()->getNthController(
                RPSysPlayerMgr::instance()->getChannel(idx));
            if (pCtrl->getDevType() == EGG::cDEV_CORE) {
                if (++mHoldFrames[idx] >= 20) {
                    mHoldFrames[idx] = 20;
                    ready = true;
                } else {
                    ready = false;
                }
            } else {
                mHoldFrames[idx] = 0;
                ready = false;
            }
        }
        mpPanels[mPlayerIdx]->SetReady(ready);

        const RPSysPlayer& rPlayer = RPSysPlayerMgr::instance()->getPlayer(mPlayerIdx);
        EGG::CoreControllerMgr* pMgr = EGG::CoreControllerMgr::instance();
        mpPanels[mPlayerIdx]->Start(
            pMgr->getNthController(rPlayer.getChannel())->getChannelID(),
            mpPanels[mPlayerIdx]->IsReady(),
            mpIcons[mPlayerIdx]->mAdditionalInfo.name);

        readyNum += mpPanels[mPlayerIdx]->IsReady();
    }
}
#pragma pop
void RPSysNunchukCheckScene::CheckPlayersReady() {
    bool ready;
    u8 readyNum = 0;
    bool first1 = true;
    bool first2 = true;
    bool first3 = true;
    bool first4 = true;
    u32 remoteFlag = 0;
    bool newReady = false;

    for (mPlayerIdx = 0; mPlayerIdx < mPlayerNum; mPlayerIdx++) {
        u8 idx = mPlayerIdx;
        if (mNeedNunchuk) {
            EGG::CoreController* pCtrl = EGG::CoreControllerMgr::instance()->getNthController(
                RPSysPlayerMgr::instance()->getChannel(idx));
            ready = pCtrl->isFreestyle() == mNeedNunchuk;
        } else {
            EGG::CoreController* pCtrl = EGG::CoreControllerMgr::instance()->getNthController(
                RPSysPlayerMgr::instance()->getChannel(idx));
            if (pCtrl->getDevType() == EGG::cDEV_CORE) {
                if (++mHoldFrames[idx] >= 20) {
                    mHoldFrames[idx] = 20;
                    ready = true;
                } else {
                    ready = false;
                }
            } else {
                mHoldFrames[idx] = 0;
                ready = false;
            }
        }

        if (!mpPanels[mPlayerIdx]->IsReady() && ready) {
            newReady = true;
            switch (RPSysPlayerMgr::instance()->getChannel(mPlayerIdx)) {
            case 0:
                if (first1) {
                    first1 = false;
                    remoteFlag += 1;
                }
                break;
            case 1:
                if (first2) {
                    first2 = false;
                    remoteFlag += 2;
                }
                break;
            case 2:
                if (first3) {
                    first3 = false;
                    remoteFlag += 4;
                }
                break;
            case 3:
                if (first4) {
                    first4 = false;
                    remoteFlag += 8;
                }
                break;
            }
        }

        mpPanels[mPlayerIdx]->SetReady(ready);
        readyNum += ready;
    }

    if (mPlayerIdx == readyNum) {
        OnAllReady();
    } else if (newReady) {
        RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_MESS_OK_01_CTSP",
                                                           remoteFlag);
        RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_MESS_OK_01_TVSP",
                                                           remoteFlag);
    }
}

void RPSysNunchukCheckScene::OnAllReady() {
    u32 remoteFlag = 0;
    bool first1 = true;
    bool first2 = true;
    bool first3 = true;
    bool first4 = true;

    for (mPlayerIdx = 0; mPlayerIdx < mPlayerNum; mPlayerIdx++) {
        switch (RPSysPlayerMgr::instance()->getChannel(mPlayerIdx)) {
        case 0:
            if (first1) {
                first1 = false;
                remoteFlag += 1;
            }
            break;
        case 1:
            if (first2) {
                first2 = false;
                remoteFlag += 2;
            }
            break;
        case 2:
            if (first3) {
                first3 = false;
                remoteFlag += 4;
            }
            break;
        case 3:
            if (first4) {
                first4 = false;
                remoteFlag += 8;
            }
            break;
        }
    }

    RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_MESS_OK_02_CTSP",
                                                       remoteFlag);
    RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_MESS_OK_02_TVSP",
                                                       remoteFlag);

    mState = EState_AllReady;
    mTimer = 48;
}

#pragma push
#pragma auto_inline off
void RPSysNunchukCheckScene::StartGame() {
    if (RPSysProjectLocal::instance()->getPack() == RPSysSceneCreator::EPackID_SportsPack) {
        if (spSportInfo->mMode == 2) {
            RPSysSceneCreator::instance()->changeSceneAfterFade(
                RPSysQueuedScene::instance()->getNextSceneID());
        } else if (spSportInfo->mMode == 1) {
            RPSysSceneCreator* pCreator = RPSysSceneCreator::instance();
            pCreator->changeSceneAfterFade(pCreator->getSceneID(
                static_cast<RPSysSceneCreator::ESportID>(spSportInfo->mSportID)));
        } else if (spSportInfo->mMode == 0) {
            s32 sceneID = RPSysSceneCreator::instance()->getSceneID(
                static_cast<RPSysSceneCreator::ESportID>(spSportInfo->mSportID));
            RPSysSceneCreator::instance()->changeSceneAfterFade(sceneID);
        }
    } else {
        RPSysSceneCreator::instance()->changeSceneAfterFade(
            RPSysQueuedScene::instance()->getNextSceneID());
    }
}

void RPSysNunchukCheckScene::GoNext() {
    if (RPSysProjectLocal::instance()->getPack() == RPSysSceneCreator::EPackID_SportsPack) {
        if (spSportInfo->mMode == 2) {
            RPSysSceneCreator::instance()->changeSceneAfterFade(9);
        } else if (spSportInfo->mMode == 1) {
            mState = EState_OpenWindow;
        } else if (spSportInfo->mMode == 0) {
            RPSysSceneCreator::instance()->changeSceneAfterFade(10);
        }
    } else {
        RPSysSceneCreator::instance()->changeSceneAfterFade(
            RPSysQueuedScene::instance()->getNextSceneID());
    }
}

#pragma pop

void RPSysNunchukCheckScene::CalcOkButton() {
    const EGG::Vector2f& rCursorPos = RPSysCursorDrawMgr::instance()->getActiveCursorPos();
    nw4r::math::VEC2 pos;
    pos.x = rCursorPos.x;
    pos.y = rCursorPos.y;
    s32 player = RPSysCursorDrawMgr::instance()->getActiveCursorPlayer();

    if (mpFrameDown->HitTestBackButton(pos)) {
        if (!mBackHovered) {
            mpFrameDown->RequestHover(player);
            mBackHovered = true;
        }

        if (EGG::CoreControllerMgr::instance()->getNthController(player)->downTrigger(0x800)) {
            mpFrameDown->RequestPressed();
            mState = EState_BackPressed;
        }
    } else if (mBackHovered) {
        mpFrameDown->RequestUnhover();
        mBackHovered = false;
    }

    if (EGG::CoreControllerMgr::instance()->getNthController(player)->downTrigger(0x1400) &&
        mState == EState_Check) {
        mpFrameDown->RequestPressed();
        mState = EState_BackPressed;
    }
}

bool RPSysNunchukCheckScene::IsNunchukGame() {
    if (spSportInfo->mMode == 2) {
        return GetSportID(RPSysQueuedScene::instance()->getNextSceneID()) ==
               RPSysSceneCreator::ESportID_Box;
    }

    return spSportInfo->mSportID == RPSysSceneCreator::ESportID_Box;
}

s32 RPSysNunchukCheckScene::GetSportID(s32 sceneID) {
    // Wii Sports scene IDs
    switch (sceneID) {
    case 3: // RPBsbScene
        return RPSysSceneCreator::ESportID_Bsb;
    case 6: // RPTnsScene
        return RPSysSceneCreator::ESportID_Tns;
    case 5:  // RPGolScene
    case 14: // RPGolCourseSelectScene
        return RPSysSceneCreator::ESportID_Gol;
    case 7: // RPBoxScene
        return RPSysSceneCreator::ESportID_Box;
    case 4: // RPBowScene
        return RPSysSceneCreator::ESportID_Bow;
    default:
        return -1;
    }
}

void RPSysNunchukCheckScene::Calculate() {
    switch (mState) {
    case EState_Start:
        Start();
        break;
    case EState_GoBack:
        GoNext();
        break;
    case EState_StartGame:
        StartGame();
        break;
    }

    if (isSceneDisplay()) {
        switch (mState) {
        case EState_Check:
            CheckPlayersReady();
            CalcOkButton();
            break;
        case EState_AllReady:
            if (mTimer == 0) {
                mState = EState_StartGame;
            } else if (mTimer > 0) {
                mTimer--;
            }
            break;
        case EState_BackPressed:
            if (mpFrameDown->IsIdleOrHovered()) {
                mState = EState_GoBack;
            }
            break;
        case EState_OpenWindow:
            RPSysSystemWinMgr::instance()->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT,
                                                           0x1B59, mpMessage, 0);
            RPSysSystemWinMgr::instance()->setCursors();
            mState = EState_WaitWindow;
            break;
        case EState_WaitWindow:
            switch (RPSysSystemWinMgr::instance()->getResult()) {
            case RPSysSystemWinMgr::RESULT_YES:
                mState = EState_OpenWindow2;
                break;
            case RPSysSystemWinMgr::RESULT_NO:
                mState = EState_Check;
                break;
            }
            break;
        case EState_OpenWindow2:
            RPSysSystemWinMgr::instance()->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT,
                                                           0x1B5A, mpMessage, 0);
            RPSysSystemWinMgr::instance()->setCursors();
            mState = EState_WaitWindow2;
            break;
        case EState_WaitWindow2:
            if (RPSysSystemWinMgr::instance()->getResult() != RPSysSystemWinMgr::RESULT_WAIT) {
                RPSysSceneCreator::instance()->changeSceneAfterFade(9);
            }
            break;
        }
    }

    mpFrameDown->Calculate();

    for (u32 i = 0; i < PLAYER_NUM; i++) {
        mpPanels[static_cast<u8>(i)]->Calculate();
    }

    mpLayout->calc();
}

void RPSysNunchukCheckScene::Exit() {
    RPSysCursorDrawMgr::instance()->fadeActiveCursor();
}

/******************************************************************************
 *
 * RPSportsMsgPanel
 *
 ******************************************************************************/

void RPSportsMsgPanel::Init(u32 player, RPSysLytResAccessor* pAccessor,
                           RPSysLayout* pLayout, RPSysMessage* pMessage) {
    mpLayout = pLayout;
    mpMessage = pMessage;
    for (u32 i = 0; i < 8; i++) {
        mpAnmObjs[(u16)i] = pLayout->createAnmObj(
            NULL, pAccessor, sAnimationNames[(u16)i]);
    }

    for (u32 i = 0; i < 7; i++) {
        mpPanes[(u16)i] = pLayout->findPane(
            sPaneNames[player][(u16)i]);
    }

    for (u32 i = 0; i < 2; i++) {
        mpColors[(u16)i] = pLayout->findPicture(
            sColorNames[(u16)player][(u16)i]);
        Lyt_setPaneColorFromTable(mpColors[(u16)i], player);
    }

    RPSysLytTextBox* pTextBox =
        mpLayout->findTextBox(sOkNames[(u16)player]);
    pTextBox->setMessage(pMessage, 0x367, 0, 0);
    mpTextBox = pLayout->findTextBox(sPlayerNames[(u16)player]);
}
