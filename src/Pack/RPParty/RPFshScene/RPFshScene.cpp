// Fishing scene (RPFshScene): model scenes, views, resources and sound; the game itself is the
// RPFshGame singleton.
// Unit 0x801FE978-0x801FF03C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshScene.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpDrawPathSV.h>
#include <Pack/RPGraphics/RPGrpDrawPathXluSnap.h>
#include <Pack/RPGraphics/RPGrpLightManager.h>
#include <Pack/RPKernel/RPSysEffectCreator.h>
#include <Pack/RPParty/RPFshScene/RPFshCamera.h>
#include <Pack/RPParty/RPFshScene/RPFshGame.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <egg/gfxe.h>

#include <cstdio>

// Wii Play: these shared entry points retain their original C symbol names.
extern "C" void RP_Cls803572E0_createInstance();

RPFshSceneSound::~RPFshSceneSound() {}

RPFshScene::~RPFshScene() {}

void RPFshSceneSound::initSound3D(s32 arg) {
    switch (arg) {
    case 0:
        // Wii Play: a never-read local, as in RPHkySceneSound::initSound3D
        f32 param[16];
        *(s32*)&param[12] = 0x20;
        param[1] = 1.0f;
        param[6] = 1.0f;
        param[11] = 1.0f;
        break;
    }
}

void RPFshScene::Configure() {
    RPSimpleModel::SetRandomSeed(OSGetTick());
    RPGrpModelResManager::GetCurrent()->CreateResourceList(RPCommon::GetResourceNum() + 0x1C);

    RPFshGame::CreateInstance();
    RP_Cls803572E0_createInstance();

    RPGrpModelManager* pModelMgr = RPGrpModelManager::GetCurrent();
    pModelMgr->CreateModelScene(0, 0xE, 1, NULL, NULL);
    pModelMgr->CreateModelScene(1, 1, 1, pModelMgr->GetModelScene(0)->GetLightManager(),
                                pModelMgr->GetModelScene(0)->GetFogManager());
    pModelMgr->CreateModelScene(2, 4, 1, pModelMgr->GetModelScene(0)->GetLightManager(),
                                pModelMgr->GetModelScene(0)->GetFogManager());

    GXColor shadowColor = {0, 0, 0, 0x60};
    pModelMgr->GetDrawPathManager(0)->GetDrawPathSV()->GetShadowVolume()->SetShadowColor(
        shadowColor);

    RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();
    RPGrpScreen* pScreen = new RPGrpScreen();
    pScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);

    pRenderer->CreateView3D(0, RPFshCameraMgr::GetInstance()->GetCamera(), pScreen);
    pRenderer->CreateView3D(1, RPFshCameraMgr::GetInstance()->GetCamera(), pScreen);
    pRenderer->CreateView2D(2, pScreen);
    pRenderer->CreateView3D(3, RPFshCameraMgr::GetInstance()->GetCamera(), pScreen);
    pRenderer->CreateView2D(4, pScreen);
    pRenderer->LinkViews();

    pRenderer->GetView(0)->SetDrawScene(0);
    pRenderer->GetView(1)->SetDrawScene(1);
    pRenderer->GetView(3)->SetDrawScene(2);
    pRenderer->GetView(1)->SetFlag(1 << 3);
    pRenderer->GetView(3)->SetProjType(RPGrpView::EProjType_Ortho);
    pRenderer->GetView(4)->CreateScreenEffect();

    RPGrpDrawPathXluSnap* pXluSnap =
        RPGrpModelManager::GetCurrent()->GetDrawPathManager(0)->GetDrawPathXluSnap();
    pXluSnap->GetXluSnap()->onFlag(2);
    pXluSnap->GetXluSnap()->onFlag(1);
    pXluSnap =
        RPGrpModelManager::GetCurrent()->GetDrawPathManager(2)->GetDrawPathXluSnap();
    pXluSnap->GetXluSnap()->onFlag(2);
    pXluSnap->GetXluSnap()->onFlag(1);

    RPSndAudioMgr::instance()->configureMiniGame(1);
    RPSndAudioMgr::instance()->setPlayerNum(RPSceneInfo::spInstance->GetPlayerNum());

    mpSound = new RPFshSceneSound();
    mpSound->initSound3D(0);
}

void RPFshScene::LoadResource() {
    RPCls803572E0::GetInstance()->LoadMessage("party_message.bmg");

    const char* brresNames[16] = {"sceneFile",      "fsh_rod_B",      "fsh_rod_R",
                                  "fsh_rope",       "fsh_lake",       "fsh_tree",
                                  "fsh_cloud",      "fsh_waterface",  "fsh_fish_A",
                                  "fsh_fish_B",     "fsh_fish_C",     "fsh_fish_sukaA",
                                  "fsh_fish_rareA", "fsh_fish_rareB", "fsh_fish_anim",
                                  "fsh_hand_R_anim"};

    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    for (int i = 0; i < 16; i++) {
        char path[64];
        snprintf(path, sizeof(path), "G3D/%s.brres", brresNames[i]);
        pResMgr->CreateDataAt(i, RPGrpModelResManager::Type_ResFile,
                              RPSysResourceManager::GetFileFromArchive(pArchive, path));
    }

    static_cast<RPGrpLightManager*>(
        RPGrpModelManager::GetCurrent()->GetModelScene(1)->GetLightManager())
        ->LoadScnLight(0, "RPScene", 0.0f, -1);

    void* pEffect = RPSysResourceManager::GetFileFromArchive(pArchive, "basic_fishing.breff");
    void* pTexture = RPSysResourceManager::GetFileFromArchive(pArchive, "basic_fishing.breft");
    getEffectCreator()->setEffectResource(pEffect, pTexture);

    RPSndAudioMgr::instance()->attachArchive("rp_Fsh_sound.brsar", false, NULL);
    RPSndAudioMgr::instance()->loadGroup(0U);

    GXColor ambient = {0x4E, 0x4E, 0x36, 0xFF};
    RPGrpModelManager::GetCurrent()->GetLightManager(1)->GetAmbientColor(0) = ambient;

    RPFshGame::GetInstance()->CreateObjects();
}

void RPFshScene::Reset() {
    RPFshGame::GetInstance()->ResetObjects();
}

void RPFshScene::Calculate() {
    RPFshGame::GetInstance()->Calculate();
}

void RPFshScene::Exit() {
    RPFshGame::GetInstance()->DestroyObjects();
    RPFshGame::DestroyInstance();
    RPCls803572E0::DestroyInstance();
    RPSndAudioMgr::instance()->changeScene();
}
