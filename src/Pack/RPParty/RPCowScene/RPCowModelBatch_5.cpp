// Charge! far tree model batch (RPCowTreeBatch): LoadResource, the class's key function, so its vtable and
// destructor thunks come out here.
// Unit 0x802ADE94-0x802ADF14 (dtk/game_units.tsv).
#include <Pack/RPParty/RPCowScene/RPCowModelBatch.h>

#include <Pack/RPGraphics/RPGrpModel.h>

#include <egg/math.h>

void RPCowTreeBatch::LoadResource() {
    mpModel = RPGrpModel::Construct(0x16, 2, 0, 0);
    mCapacity = 64;
    mpMtxs = new EGG::Matrix34f[mCapacity];
}
