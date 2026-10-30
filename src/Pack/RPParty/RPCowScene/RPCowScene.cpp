// Charge! scene (RPCowScene): an RPActorScene whose actor list is the RPCowGame.
// Unit 0x80295D3C-0x80295DD0 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPCowScene/RPCowScene.h>

#include <Pack/RPParty/RPCowScene/RPCowGame.h>

RPCowScene::~RPCowScene() {}

void RPCowScene::CreateActorList() {
    spActorList = new RPCowGame();
}
