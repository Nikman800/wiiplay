// Find Mii scene: sound setup, the progress kept between stages, and the hooks forwarded to the
// RPWlyGameMgr singleton.
// Unit 0x802753A4-0x80275E74 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPWlyScene/RPWlyScene.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel/RPSysEffectCreator.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPWlyScene/RPWlyAvatarMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyCursor.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameConfig.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlySound.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

RPWlySoundHolder::~RPWlySoundHolder() {}

RPWlyScene::~RPWlyScene() {}

// The original constructs this local sound parameter object without applying it.
void RPWlySoundHolder::initSound3D(s32 arg) {
    switch (arg) {
    case 0: {
        f32 unit;
        f32 maximum;
        f32 interior;
        f32 one;
        one = 1.0f;
        interior = 2400.0f;
        maximum = 80.0f;
        unit = 1200.0f;
        struct {
            f32 value[28];
            s32 priority;
            u8 padding[12];
        } param;
        param.priority = 32;
        param.value[1] = one;
        param.value[6] = one;
        param.value[11] = one;
        // NONMATCHING-DEBT: the never-read parameter block's word 12 is an s32. The real type is
        // unknown; giving it an s32 member (or dropping the cast) leaves the struct
        // non-address-taken and the frame grows from 0x90 to 0x410.
        reinterpret_cast<s32&>(param.value[12]) = 32;
        param.value[17] = interior;
        param.value[22] = maximum;
        param.value[27] = unit;
        break;
    }
    }
}

void RPWlyScene::Configure() {
    if (RPSceneInfo::spInstance->GetUnk1C() == 0) {
        RPSndAudioMgr::instance()->configureMiniGame(1);
        RPSndAudioMgr::instance()->setPlayerNum(RPSceneInfo::spInstance->GetPlayerNum());

        mpSound = new RPWlySoundHolder();
        mpSound->initSound3D(0);

        RPWlyGameConfig config = RPSceneInfo::spInstance->mMiiEntry.GetConfig();
        config.mpSceneSound = mpSound;
        RPSceneInfo::spInstance->mMiiEntry.SetConfig(config);
    } else {
        RPWlyGameConfig config = RPSceneInfo::spInstance->mMiiEntry.GetConfig();
        mpSound = config.mpSceneSound;
    }

    RPWlyGameMgr::CreateInstance();
    mpGameMgr = RPWlyGameMgr::sInstance;
    mpGameMgr->Init();
}

void RPWlyScene::LoadResource() {
    if (RPSceneInfo::spInstance->GetUnk1C() == 0) {
        RPSndAudioMgr::instance()->attachArchive("rp_Wly_sound.brsar", false, NULL);
        RPSndAudioMgr::instance()->loadGroup(0U);
    }

    void* pEffect = RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameCommonArchive(), "basic_wanted.breff");
    void* pTexture = RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameCommonArchive(), "basic_wanted.breft");
    getEffectCreator()->setEffectResource(pEffect, pTexture);

    mpGameMgr->Create();
}

void RPWlyScene::Reset() {
    mpGameMgr->mpTimerSe->Reset();
    mpGameMgr->Reset();
    setFadeFrame(25);
}

void RPWlyScene::Calculate() {
    mpGameMgr->Calculate();
}

void RPWlyScene::Exit() {
    if (RPSceneInfo::spInstance->GetUnk1C() == 0) {
        RPSndAudioMgr::instance()->changeScene();
    }
    mpGameMgr->Destroy();
}

void RPWlyScene::UserDraw() {
    mpGameMgr->Draw();
}

void RPWlyScene::pauseCallBack(bool enter) {
    RPWlyGameMgr::sInstance->mpCursors[0]->SetVisible(enter);
    RPWlyGameMgr::sInstance->mpCursors[1]->SetVisible(enter);
    RPWlyAvatarMgr::sInstance->SetPaused(enter);
}
