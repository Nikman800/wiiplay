#include <Pack/RPParty/RPPartyCommon/RPActorScene.h>

#include <Pack/RPParty/RPPartyCommon/RPActorList.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPSystem/RPSysSceneMgr.h>

extern "C" void RP_allocDebugBuffers();
extern "C" void RP_freeDebugBuffers();
extern "C" void RP_clearDebugBuffers();
extern "C" void RP_updateRumblePatterns();
extern "C" void RP_clearRumblePatterns();

extern "C" u32 RP_actorSceneUnk684;
extern "C" u32 RP_actorSceneUnk688;

RPActorList* RPActorScene::spActorList = NULL;
u32 RPActorScene::sUnk684 = 0;
u32 RPActorScene::sUnk688 = 0;
extern "C" u32 RP_actorSceneFlags = 0;
u32 RPActorScene::sFrame = 0;
u32 RPActorScene::sUnk694 = 0;

extern "C" u32 RP_actorSceneUnk698[2] = {0, 0};

RPActorScene::RPActorScene() {
    spActorList = NULL;
    RP_actorSceneFlags = 0;
    sFrame = 0;
    sUnk694 = 0;
    RP_actorSceneUnk698[0] = 0;
    RP_allocDebugBuffers();
}

RPActorScene::~RPActorScene() {
    RP_freeDebugBuffers();
}

void RPActorScene::Configure() {
    sUnk684 = RPSysSceneMgr::instance()->getCurrentSceneID();
    sUnk688 = 0;
    CreateActorList();
}

void RPActorScene::Exit() {
    if (spActorList != NULL) {
        delete spActorList;
        spActorList = NULL;
    }
}

void RPActorScene::LoadResource() {
    spActorList->LoadResource();
    spActorList->LoadResourceAll();
}

void RPActorScene::Reset() {
    RP_actorSceneFlags = 0;
    sFrame = 0;
    sUnk694 = 0;
    RP_actorSceneUnk698[0] = 0;
    spActorList->StopFlaggedActors();
    spActorList->Reset();
    RP_clearDebugBuffers();
    RP_clearRumblePatterns();
}

void RPActorScene::Calculate() {
    if ((RP_actorSceneFlags & 8) == 0) {
        sFrame++;
        spActorList->Calculate();
        spActorList->SortAndActivate();
        RP_updateRumblePatterns();
        spActorList->CalculatePost();
        spActorList->CalcActive();
    }
}

void RPActorScene::UserDraw() {
    if ((RP_actorSceneFlags & 8) == 0) {
        spActorList->Draw();
    }
}

extern "C" s32 Lyt_getDrawStep() {
    return RPGrpRenderer::GetDrawStep();
}

extern "C" u8 Lyt_getCurrentViewNo() {
    return RPGrpRenderer::GetCurrentViewNo();
}

#pragma dont_inline on
void RPActorScene::DebugDraw() {
    if ((RP_actorSceneFlags & 1) != 0 && Lyt_getDrawStep() == 3) {
        spActorList->DrawPassAll(Lyt_getCurrentViewNo());
        spActorList->DrawPass(Lyt_getCurrentViewNo());
    }
}
#pragma dont_inline off
