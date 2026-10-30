// Charge! far hurdle model batch (RPCowHurdleBatch): LoadResource, the class's key function, so its vtable and
// destructor thunks come out here.
// Unit 0x802ADF14-0x802ADF94 (dtk/game_units.tsv).
#include <Pack/RPParty/RPCowScene/RPCowModelBatch.h>

#include <Pack/RPGraphics/RPGrpModel.h>

#include <egg/math.h>

void RPCowHurdleBatch::LoadResource() {
    mpModel = RPGrpModel::Construct(0x10, 2, 0, 0);
    mCapacity = 32;
    mpMtxs = new EGG::Matrix34f[mCapacity];
}
