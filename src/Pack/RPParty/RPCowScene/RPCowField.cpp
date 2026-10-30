// Charge! field actor (RPCowField). Unit 0x802A64BC-0x802A65F0 (dtk/game_units.tsv): its dtor
// thunk ends the file; .data 8035D080-8035D0E0, .sdata2 80440C40-80440C48. Functions are defined
// in reverse address order.
#include <Pack/RPParty/RPCowScene/RPCowField.h>

#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPParty/RPPartyCommon/RPCollisionShape.h>

extern "C" const EGG::Matrix34f RP_eggMtxIdentity;

RPCowField::RPCowField() {}

RPCowField::~RPCowField() {}

void RPCowField::LoadResource() {
    mpCollision = new RPCollisionShape();
    mpModel = RPGrpModel::Construct(2, 2, 0, 0);
}

void RPCowField::OnStart() {
    mpCollision->Reset();
}

void RPCowField::OnStop() {}

void RPCowField::OnActivate() {
    CalcModel(mpModel, sModelScale, &RP_eggMtxIdentity);
}

void RPCowField::Calculate() {
    EntryModel(mpModel);
}

const f32 RPCowField::sModelScale = 1.0f;
