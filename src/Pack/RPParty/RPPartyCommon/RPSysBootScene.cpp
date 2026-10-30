#include <Pack/RPParty/RPPartyCommon/RPSysBootScene.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPKernel/RPSysControllerMgr.h>
#include <Pack/RPKernel/RPSysEffectMgr.h>
#include <Pack/RPKernel/RPSysHomeMenuMgr.h>
#include <Pack/RPKernel/RPSysKokeshiManager.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPKernel/RPSysSystem.h>
#include <Pack/RPKernel/RPSysSystemWinMgr.h>
#include <Pack/RPKernel/RPSysTextWriter.h>
#include <Pack/RPParty/RPPartyCommon/RPMessageWindow.h>
#include <Pack/RPSystem/RPSysPauseMgr.h>
#include <Pack/RPSystem/RPSysProjectLocal.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPSystem/RPSysSaveDataMgr.h>

#include <egg/core/eggArchive.h>
#include <egg/core/eggDecomp.h>
#include <revolution/OS.h>

extern "C" {
extern u8 Health_strapArchive[];
}

bool Health_isFirstBoot = true;

RPSysBootScene::~RPSysBootScene() {}

void RPSysBootScene::Configure() {
    RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();
    RPGrpScreen* pScreen = new RPGrpScreen();
    pScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    pRenderer->CreateView2D(1, pScreen);
    pRenderer->LinkViews();

    mState = EState_WaitDisplay;
    mTimer = 0;
    mpProjectLocal = RPSysProjectLocal::instance();
    mpLayout = NULL;
    mpAnmObj = NULL;

    u32 size = EGG::Decomp::getExpandSize(Health_strapArchive);
    u8* pBinary = new (32) u8[size];
    EGG::Decomp::decode(Health_strapArchive, pBinary);
    DCFlushRange(pBinary, size);
    EGG::Archive* pArchive = EGG::Archive::mount(pBinary, NULL, 4);
    void* pLayoutArchive = pArchive->getFile("strap.arc", NULL);
    RPSysLytResAccessor* pAccessor = RPSysLytResAccessor::create(NULL);
    pAccessor->mountArchive(pLayoutArchive, NULL);

    const char* pLayoutName = "strap_00_U.brlyt";
    const char* pAnmName = "strap_00_U.brlan";
    switch (mpProjectLocal->getRegion()) {
    case RPSysProjectLocal::ERegion_NTSC_J:
        pLayoutName = "strap_00_J.brlyt";
        pAnmName = "strap_00_J.brlan";
        break;
    case RPSysProjectLocal::ERegion_NTSC_U:
        switch (mpProjectLocal->getLanguage()) {
        case RPSysProjectLocal::EArea_France:
            pLayoutName = "strap_00_F.brlyt";
            pAnmName = "strap_00_F.brlan";
            break;
        case RPSysProjectLocal::EArea_Spain:
            pLayoutName = "strap_00_S_US.brlyt";
            pAnmName = "strap_00_S_US.brlan";
            break;
        default:
            pLayoutName = "strap_00_U.brlyt";
            pAnmName = "strap_00_U.brlan";
            break;
        }
        break;
    case RPSysProjectLocal::ERegion_PAL:
        switch (mpProjectLocal->getLanguage()) {
        case RPSysProjectLocal::EArea_France:
            pLayoutName = "strap_00_F.brlyt";
            pAnmName = "strap_00_F.brlan";
            break;
        case RPSysProjectLocal::EArea_Germany:
            pLayoutName = "strap_00_G.brlyt";
            pAnmName = "strap_00_G.brlan";
            break;
        case RPSysProjectLocal::EArea_Italy:
            pLayoutName = "strap_00_I.brlyt";
            pAnmName = "strap_00_I.brlan";
            break;
        case RPSysProjectLocal::EArea_Spain:
            pLayoutName = "strap_00_S.brlyt";
            pAnmName = "strap_00_S.brlan";
            break;
        case RPSysProjectLocal::EArea_Netherlands:
            pLayoutName = "strap_00_D.brlyt";
            pAnmName = "strap_00_D.brlan";
            break;
        default:
            pLayoutName = "strap_00_E.brlyt";
            pAnmName = "strap_00_E.brlan";
            break;
        }
        break;
    }

    mpLayout = RPSysLayout::create(NULL, pAccessor, pLayoutName);
    mpAnmObj = mpLayout->createAnmObj(NULL, pAccessor, pAnmName);
    mpLayout->bindAnmObj(mpAnmObj);
    mpAnmObj->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    mpAnmObj->stop();
    if (Health_isFirstBoot) {
        setTaskAsync();
    }
}

void RPSysBootScene::LoadResource() {
    if (mpProjectLocal->getPack() == RPSysSceneCreator::EPackID_HealthPack) {
        RPSndAudioMgr::instance()->attachArchive("rp_Health_sound.brsar", true, NULL);
    }
}

void RPSysBootScene::Reset() {
    RPGrpRenderer::SetClearColor(255, 255, 255);
    setFadeFrame(30);
    if (mpProjectLocal->getPack() == RPSysSceneCreator::EPackID_HealthPack) {
        setFadeColor(nw4r::ut::Color(0x000000ff));
        RPSysHomeMenuMgr::instance()->disable(true);
    } else {
        setFadeColor(nw4r::ut::Color(0xffffffff));
    }
    mState = EState_WaitDisplay;
    mTimer = 0;
    if (mpLayout != NULL) {
        mpLayout->reset();
    }
}

void RPSysBootScene::CalcWaitInput() {
    bool ready = Health_isFirstBoot ? isTaskAsyncFinish() : true;
    if (mTimer < 1200) {
        ++mTimer;
    }
    if (ready) {
        bool advance = false;
        int i = 0;
        do {
            if (EGG::CoreControllerMgr::instance()->getNthController(i)->downTrigger(0x1f1f)) {
                advance = true;
                break;
            }
            ++i;
        } while (i < 4);
        if (mTimer >= 1200) {
            advance = true;
        }
        if (advance) {
            s32 nextScene = RPSysSystem::instance()->getBootScene();
            if (RPSysSceneCreator::instance()->changeSceneAfterFade(nextScene, false)) {
                mState = EState_Exit;
                mTimer = 0;
            }
        }
    }
}

void RPSysBootScene::Calculate() {
    if (mpLayout != NULL) {
        mpLayout->calc();
    }
    switch (mState) {
    case EState_WaitDisplay:
        if (isSceneDisplay()) {
            mState = EState_FadeIn;
            mTimer = 0;
            setFadeColor(nw4r::ut::Color(0xffffffff));
            mpAnmObj->restart();
        }
        break;
    case EState_FadeIn:
        if (++mTimer >= 60) {
            mState = EState_WaitInput;
            mTimer = 0;
        }
        break;
    case EState_WaitInput:
        CalcWaitInput();
        break;
    case EState_Exit:
        Health_isFirstBoot = false;
        break;
    }
}

void RPSysBootScene::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() == 1 && RPGrpRenderer::GetDrawStep() == 5) {
        switch (RPSysProjectLocal::instance()->getPack()) {
        case RPSysSceneCreator::EPackID_SportsPack:
        case RPSysSceneCreator::EPackID_PartyPack:
            if (mpLayout != NULL) {
                mpLayout->draw();
            }
            break;
        default:
            RPSysTextWriter::instance()->Begin();
            RPSysTextWriter::instance()->SetDrawFlag(0);
            RPSysTextWriter::instance()->SetScale(1.5f, 1.5f);
            RPSysTextWriter::instance()->SetTextColor(nw4r::ut::Color(0x000000ff));
            RPSysTextWriter::instance()->PrintfZeroCenter(-0.4f, 0.0f,
                "\x83\x58\x83\x67\x83\x89\x83\x62\x83\x76\x8c\x78\x8d\x90\x89\xe6\x96\xca");
            RPSysTextWriter::instance()->End();
            break;
        }
    }
}

void RPSysBootScene::Exit() {
    RPGrpRenderer::SetClearColor(0, 0, 0);
    if (mpProjectLocal->getPack() != RPSysSceneCreator::EPackID_HealthPack) {
        RPSysCoreControllerMgr::instance()->setSearchByAddress();
    }
}

void RPSysBootScene::taskAsync() {
    RPSysResourceManager* pResources = RPSysResourceManager::instance();
    pResources->LoadStaticArchives();
    RPSysSystem::instance()->createTimeStamp();
    RPMessageWindow::GetInstance()->Load();
    RPSysSystemWinMgr::instance()->load();
    RPSysKokeshiManager::instance()->LoadStaticResource();
    RPSysEffectMgr::instance()->LoadResource();
    RPSysPauseMgr::instance()->loadMenu();
    RPSysHomeMenuMgr::instance()->initHomeButtonInfo();
    RPSysSaveDataMgr::instance()->initBanner();
    pResources->LoadCacheArchives();
}
