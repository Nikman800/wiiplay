// Table Tennis scene: forwards the scene hooks to the RPPnpGame singleton, sound setup.
// Unit 0x802244D0-0x80224868 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpScene.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel/RPSysEffectCreator.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPnpScene/RPPnpBgmCtrl.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPParty/RPPnpScene/RPPnpSound.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

RPPnpSceneSound::~RPPnpSceneSound() {}

RPPnpScene::~RPPnpScene() {}

// The original fills a never-read local (type unknown): one slot per player count, each a
// 28-word block and a 16-byte block holding the priority reduction (32), laid out as below.
// NONMATCHING-DEBT: word 12 of each block is an int (32) stored through an f32 slot; the real
// type (likely an nw4r::snd 3D listener/manager parameter block) is unknown, so it stays a view.
void RPPnpSceneSound::initSound3D(s32 arg) {
    struct {
        f32 value[2][28];
        s32 priority[2][4];
    } param;
    if (arg != 1) {
        param.priority[1][0] = 32;
        param.value[1][1] = 1.0f;
        param.value[1][6] = 1.0f;
        param.value[1][11] = 1.0f;
        reinterpret_cast<s32&>(param.value[1][12]) = 32;
        param.value[1][17] = 20000.0f;
        param.value[1][22] = 10000.0f;
        param.value[1][27] = 10000.0f;
    } else {
        param.priority[0][0] = 32;
        param.value[0][1] = 1.0f;
        param.value[0][6] = 1.0f;
        param.value[0][11] = 1.0f;
        reinterpret_cast<s32&>(param.value[0][12]) = 32;
        param.value[0][17] = 20000.0f;
        param.value[0][22] = 10000.0f;
        param.value[0][27] = 10000.0f;
    }
}

void RPPnpScene::Configure() {
    RPPnpGame::CreateInstance();
    mpGame = RPPnpGame::GetInstance();
    mpGame->Create();

    RPSndAudioMgr::instance()->configureMiniGame(1);
    RPSndAudioMgr::instance()->setPlayerNum(RPSceneInfo::spInstance->GetPlayerNum());

    mpSound = new RPPnpSceneSound();
    mpSound->initSound3D(RPSceneInfo::spInstance->GetPlayerNum() - 1);
}

void RPPnpScene::LoadResource() {
    RPSndAudioMgr::instance()->attachArchive("rp_Pnp_sound.brsar", false, NULL);
    RPSndAudioMgr::instance()->loadGroup(0U);

    void* pEffect = RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameCommonArchive(), "basic_pingpong.breff");
    void* pTexture = RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameCommonArchive(), "basic_pingpong.breft");
    getEffectCreator()->setEffectResource(pEffect, pTexture);

    mpGame->Load();
}

void RPPnpScene::Reset() {
    mpGame->GetBgmCtrl()->Stop();
    mpGame->Reset();
}

void RPPnpScene::Calculate() {
    mpGame->Calculate();
}

void RPPnpScene::Exit() {
    mpGame->GetBgmCtrl()->Stop();
    RPSndAudioMgr::instance()->changeScene();
    mpGame->Exit();
}

void RPPnpScene::UserDraw() {
    mpGame->Draw();
}
