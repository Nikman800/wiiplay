#include <Pack/RPParty/RPWlyScene/RPWlyField_3.h>

RPWlyGround::~RPWlyGround() {}

RPWlyGround::RPWlyGround() {
    mScale.x = 2.0f;
    mScale.y = 1.0f;
    mScale.z = 2.5f;
    mFrame = 0;
    unk48 = 0.5f;
    unk4C = 50.0f;
    unk50 = 600;
}

void RPWlyGround::Load() {
    u16 resource = RPSimpleModel::LoadResource("/G3D/wtd_field02.brres");
    RPSimpleModel::LoadModel("wtd_field02_ground", resource, 1, 0, 0);
}

void RPWlyGround::Reset() {}

void RPWlyGround::Calculate() {
    ++mFrame;
    nw4r::math::VEC3 pos = mPos;
    nw4r::math::VEC3 scale = mScale;
    // NONMATCHING-DEBT: RPSimpleModel::sMtxIdentity is declared VEC3[3] (its static initializer
    // copy-constructs the rows, RPKcl_2.cpp); this copy is one 9-word MTX33 block copy, which only
    // an MTX33 source gives. Fix with the RPSimpleModel declaration (PartyCommon), not here.
    nw4r::math::MTX33 rotation = *reinterpret_cast<const nw4r::math::MTX33*>(sMtxIdentity);
    mRot = rotation;
    mPos.x = pos.x;
    mPos.y = pos.y;
    mPos.z = pos.z;
    mScale.x = scale.x;
    mScale.y = scale.y;
    mScale.z = scale.z;
    CalcModelMtx();
}
