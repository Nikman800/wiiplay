// Charge! far scarecrow model batch (RPCowKakasiBatch): LoadResource, the class's key function, so its vtable and
// destructor thunks come out here.
// Unit 0x802ADF94-0x802AE014 (dtk/game_units.tsv).
#include <Pack/RPParty/RPCowScene/RPCowModelBatch.h>

#include <Pack/RPGraphics/RPGrpModel.h>

#include <egg/math.h>

void RPCowKakasiBatch::LoadResource() {
    mpModel = RPGrpModel::Construct(8, 2, 0, 0);
    mCapacity = 32;
    mpMtxs = new EGG::Matrix34f[mCapacity];
}
