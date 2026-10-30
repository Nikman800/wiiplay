// Title scene (RPCommonMessage). Unit 0x801E3C68-0x801E4360.
#include <Pack/RPParty/RPPartyCommon/RPCommonMessage.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPKernel/RPSysController.h>
#include <Pack/RPKernel/RPSysCursorDrawMgr.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPKernel/RPSysSystemWinMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPButtonSeq.h>
#include <Pack/RPParty/RPPartyCommon/RPTitleLyt.h>
#include <Pack/RPSystem/RPSysQueuedScene.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPSystem/RPSysSceneCreator.h>

#include <egg/core.h>

RPCommonMessage::~RPCommonMessage() {}

inline void RPCommonMessage::StartTitle() {
    if (getCreatorSceneID() == 0) {
        setFadeColor(nw4r::ut::Color(0xFFFFFFFF));
    } else {
        setFadeColor(nw4r::ut::Color(0x000000FF));
    }

    mSaveState = 17;
    mpTitleLyt->Open();
}

void RPCommonMessage::Configure() {
    BindSystems();

    RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();
    RPGrpScreen* pScreen = new RPGrpScreen();
    pScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    pRenderer->CreateView2D(0, pScreen);
    pRenderer->LinkViews();

    mpTitleLyt = new RPTitleLyt();
    mpButtonSeq = new RPButtonSeqHolder();

    StartTitle();
}

void RPCommonMessage::LoadResource() {
    void* pResource = RPSysResourceManager::GetMessageResource("common_message.bmg");
    mpMessage = new RPSysMessage(pResource, NULL);
    unk48 = mpMessage->GetMsg(0x15, 0);

    RPUtlLoadSaveDataScene::LoadResource();
    mpTitleLyt->Load();
}

void RPCommonMessage::Reset() {
    RPUtlLoadSaveDataScene::Reset();
    mpTitleLyt->Reset();

    StartTitle();

    RPSysCursorDrawMgr::instance()->startDpdCheck();
    RPSysCursorDrawMgr::instance()->createActiveCursor();
}

void RPCommonMessage::Calculate() {
    CalcCurrentState();

    switch (mSaveState) {
    case 17: {
        if (isSceneDisplay()) {
            setFadeColor(nw4r::ut::Color(0x000000FF));
            mTimer = 0;
            unk58[0] = 0;
            unk58[1] = 0;
            unk58[2] = 0;
            unk58[3] = 0;
            mSaveState = 19;
        }
        break;
    }

    case 18: {
        if (mpSystemWinMgr->getResult() == RPSysSystemWinMgr::RESULT_CONTINUE) {
            setFadeColor(nw4r::ut::Color(0x000000FF));
            mTimer = 18000;
            unk58[0] = 0;
            unk58[1] = 0;
            unk58[2] = 0;
            unk58[3] = 0;
            mSaveState = 19;
        }
        break;
    }

    case 19: {
        CalcWaitInput();
        break;
    }
    }

    mpTitleLyt->Calculate();
    mpButtonSeq->Update();
}

void RPCommonMessage::UserDraw() {
    RPUtlLoadSaveDataScene::UserDraw();

    if (RPGrpRenderer::GetCurrentViewNo() == 0 && RPGrpRenderer::GetDrawStep() == 5) {
        mpTitleLyt->Draw();

        if (mpButtonSeq->IsComplete()) {
            drawTimeStamp(nw4r::ut::Color(0x000000FF));
        }
    }
}

void RPCommonMessage::Exit() {
    RPUtlLoadSaveDataScene::Exit();
    RPSysCursorDrawMgr::instance()->fadeActiveCursor();
    RPSysCursorDrawMgr::instance()->finishDpdCheck();
}

void RPCommonMessage::CalcWaitInput() {
    if (--mTimer < 0) {
        mTimer = 0;

        for (s32 i = 0; i < 4; i++) {
            if (unk58[i]) {
                if (RP_GET_CORE_CTRL(i)->getCoreInfo().isInfoDone() &&
                    RP_GET_CORE_CTRL(i)->getCoreInfo().isLowBattery() &&
                    mpTitleLyt->GetState() >= 3) {
                    mpSystemWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_ERROR, 150,
                                                    NULL, 0);
                    mSaveState = 18;
                    return;
                }
            } else {
                unk58[i] = RP_GET_CORE_CTRL(i)->requestCoreInfoAsync();
            }
        }
    }

    mpTitleLyt->OnButtonPress(0, RPSysCoreController::getActiveController()->down(0x800));
    mpTitleLyt->OnButtonPress(1, RPSysCoreController::getActiveController()->down(0x400));

    if (RPSysCoreController::getActiveController()->downTrigger(0x800) &&
        mpTitleLyt->IsStateSettled(3)) {
        RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_KEY_AB_pri_A_01");
    } else if (RPSysCoreController::getActiveController()->downTrigger(0x400) &&
               mpTitleLyt->IsStateSettled(3)) {
        RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_KEY_AB_pri_B_01");
    }

    if (mpTitleLyt->IsStateSettled(6)) {
        StartSaveCheck();
        RPSndAudioMgr::instance()->stopStream(35);
    }
}

void RPCommonMessage::CalcState4() {
    RPSysQueuedScene::instance()->setNextSceneID(0x1A);
    RPSysSceneCreator::instance()->changeSceneAfterFade(1);
}
