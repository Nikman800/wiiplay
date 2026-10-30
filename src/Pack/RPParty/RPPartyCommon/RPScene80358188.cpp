// Party root scene (vtable 80358188): creates the RPSceneInfo singleton on Configure, resets it
// and moves on to scene 0x1B, and destroys it on Exit. Exit (0x801E4994) is emitted first, so the
// unit starts there, not at Reset (0x801E4998).
#include <Pack/RPParty/RPPartyCommon/RPScene80358188.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPSystem/RPSysScene.h>
#include <Pack/RPSystem/RPSysSceneCreator.h>

void RP_createSceneInfo();


RPPartyRootScene::~RPPartyRootScene() {}

void RPPartyRootScene::Configure() {
    RP_createSceneInfo();
}

void RPPartyRootScene::Reset() {
    RPSceneInfo::spInstance->Reset();
    RPSysSceneCreator::instance()->changeSceneAfterFade(0x1B, false);
}

void RPPartyRootScene::Exit() {
    RPSceneInfo::DestroyInstance();
}
