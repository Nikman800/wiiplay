// Laser Hockey scene: forwards the scene hooks to the RPHkyGame singleton, sound setup.
// Unit 0x80216B44-0x80216E8C (dtk/game_units.tsv). With -inline deferred CodeWarrior emits a
// file's functions in reverse order, so the source runs from the last address to the first.
#include <Pack/RPParty/RPHkyScene/RPHkyScene.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel/RPSysEffectCreator.h>
#include <Pack/RPParty/RPHkyScene/RPHkyBgmCtrl.h>
#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

RPHkySceneSound::~RPHkySceneSound() {}

RPHkyScene::~RPHkyScene() {}

void RPHkySceneSound::initSound3D(s32 arg) {
    switch (arg) {
    case 0:
        // Wii Play: the original fills part of a 16-word local here and never reads it
        // (Billiards and Fishing have the same function); the type is unknown. A struct local
        // does not match: CodeWarrior gives every member store its own copy of it.
        f32 param[16];
        *(s32*)&param[12] = 0x20;
        param[1] = 1.0f;
        param[6] = 1.0f;
        param[11] = 1.0f;
        break;
    }
}

void RPHkyScene::Configure() {
    RPHkyGame::CreateInstance();
    mpGame = RPHkyGame::GetInstance();
    mpGame->Create();

    RPSndAudioMgr::instance()->configureMiniGame(1);
    RPSndAudioMgr::instance()->setPlayerNum(RPSceneInfo::spInstance->GetPlayerNum());

    mpSound = new RPHkySceneSound();
    mpSound->initSound3D(0);
}

void RPHkyScene::LoadResource() {
    RPSndAudioMgr::instance()->attachArchive("rp_Hky_sound.brsar", false, NULL);
    RPSndAudioMgr::instance()->loadGroup(0U);

    void* pEffect = RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameCommonArchive(), "basic_hokey.breff");
    void* pTexture = RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameCommonArchive(), "basic_hokey.breft");
    getEffectCreator()->setEffectResource(pEffect, pTexture);

    mpGame->Load();
}

void RPHkyScene::Reset() {
    mpGame->GetBgmCtrl()->Stop();
    mpGame->Reset();
}

void RPHkyScene::Calculate() {
    mpGame->Calculate();
}

void RPHkyScene::Exit() {
    mpGame->GetBgmCtrl()->Stop();
    RPSndAudioMgr::instance()->changeScene();
    mpGame->Exit();
}

void RPHkyScene::UserDraw() {
    mpGame->Draw();
}
