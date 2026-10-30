#include <Pack/RPParty/RPWlyScene/RPWlyAvatar.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLevelTable.h>

RPWlyAvatarShadow::~RPWlyAvatarShadow() {}

RPWlyAvatarShadow::RPWlyAvatarShadow() {}

void RPWlyAvatarShadow::Load() {
    RPGrpHandle resource = RPSimpleModel::LoadResource("/G3D/wtd_ava_shadow.brres");
    RPSimpleModel::LoadModel("wtd_ava_shadow", resource, 1, 0, 0);
}

void RPWlyAvatarShadow::Reset() {
    mPos.x = sVecZero.x;
    mPos.y = sVecZero.y;
    mPos.z = sVecZero.z;
    mScale.x = sVecOne.x;
    mScale.y = sVecOne.y;
    mScale.z = sVecOne.z;
}

void RPWlyAvatarShadow::Calculate() {
    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType != 1 &&
        RPWlyLevelTable::sInstance->GetCurrentLevel()->mFieldType != 2) {
        nw4r::math::VEC3 pos = mPos;
        nw4r::math::VEC3 scale = mScale;
        // NONMATCHING-DEBT: RPSimpleModel::sMtxIdentity is declared VEC3[3] (its static initializer
        // copy-constructs the rows, RPKcl_2.cpp); this copy is one 9-word MTX33 block copy, which only
        // an MTX33 source gives. Fix with the RPSimpleModel declaration (PartyCommon), not here.
        nw4r::math::MTX33 rot = *reinterpret_cast<const nw4r::math::MTX33*>(sMtxIdentity);

        mRot = rot;
        mPos.x = pos.x;
        mPos.y = pos.y;
        mPos.z = pos.z;
        mScale.x = scale.x;
        mScale.y = scale.y;
        mScale.z = scale.z;
        CalcModelMtx();
    }
}
