// Charge! model batch (RPCowModelBatch): draws one model many times, handing the course objects'
// matrices out as joint matrices. Unit 0x802ADAE4-0x802ADE94 (dtk/game_units.tsv). Functions in
// reverse address order; the class's key function is here, so its vtable and thunks are too.
#include <Pack/RPParty/RPCowScene/RPCowModelBatch_4.h>

#include <Pack/RPGraphics/RPGrpModel.h>

extern "C" const EGG::Matrix34f RP_eggMtxIdentity;

// Access to RPGrpModel's protected byte at 0x4 [L]
struct RPCowBatchModel : public RPGrpModel {
    void SetUnk4(u8 value) {
        unk4 = value;
    }
};

bool RPCowModelBatch::AddInstance(const EGG::Matrix34f& rMtx) {
    if (mNum < mCapacity) {
        mpMtxs[mNum++] = rMtx;
        return true;
    }
    return false;
}

void RPCowModelBatch::EditJointLocalMtx(RPGrpModel* pModel, u16 idx, nw4r::math::MTX34* pMtx,
                                        nw4r::math::VEC3* pScale) {
    if (idx - 1 < mNum) {
        *pMtx = mpMtxs[idx - 1];
    } else {
        nw4r::math::VEC3Scale(pScale, pScale, 0.000001f);
    }
    if (++idx < pModel->GetJointNum()) {
        pModel->SetCallbackJointIndex(idx);
    }
}

RPCowModelBatch::RPCowModelBatch() : mNum(0) {}

RPCowModelBatch::~RPCowModelBatch() {}

void RPCowModelBatch::LoadResource() {
    mpModel = RPGrpModel::Construct(0x1B, 2, 0, 0);
    static_cast<RPCowBatchModel*>(mpModel)->SetUnk4(0);
    mCapacity = 128;
    mpMtxs = new EGG::Matrix34f[mCapacity];
}

void RPCowModelBatch::OnStart() {
    mNum = 0;
}

void RPCowModelBatch::OnStop() {}

void RPCowModelBatch::OnActivate() {
    if (mNum > 0) {
        mpModel->SetCallback(this);
        mpModel->SetCallbackJointIndex(1);
        CalcModel(mpModel, 1.0f, &RP_eggMtxIdentity);
        mFlagBits.mCalc = true;
    } else {
        mFlagBits.mCalc = false;
    }
    mNum = 0;
}

void RPCowModelBatch::Calculate() {
    EntryModel(mpModel);
}
