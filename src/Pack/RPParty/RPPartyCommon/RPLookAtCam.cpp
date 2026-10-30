#include <Pack/RPParty/RPPartyCommon/RPLookAtCam.h>

#include <Pack/RPParty/RPDucScene/RPDucUtil.h>

#include <egg/core.h>

RPLookAtCam::RPLookAtCam() {}

RPLookAtCam::~RPLookAtCam() {}

void RPLookAtCam::Reset() {
    mLookPos = RP_vecZero;
    mLookUp = RP_vecEy;
    mLookTarget = RP_vecZero;
    mLookVec = RP_vecEz;
    mLookDist = 0.0f;
    mDir = RP_vecEz;
    mSide = RP_vecEx;
    mUpOrtho = RP_vecEy;
    unk1D0 = 1.0f;
    unk1D4 = 0.0f;
}

void RPLookAtCam::InitView() {
    mLookVec.x = mLookTarget.x - mLookPos.x;
    mLookVec.y = mLookTarget.y - mLookPos.y;
    mLookVec.z = mLookTarget.z - mLookPos.z;
    mLookDist = nw4r::math::VEC3Len(&mLookVec);

    if (mLookDist != 0.0f) {
        nw4r::math::VEC3Normalize(&mDir, &mLookVec);
        nw4r::math::VEC3Cross(&mSide, &mLookUp, &mDir);
        nw4r::math::VEC3Normalize(&mSide, &mSide);
        nw4r::math::VEC3Cross(&mUpOrtho, &mDir, &mSide);
    }

    SetPosition(mLookPos.x, mLookPos.y, mLookPos.z);
    SetTarget(mLookTarget.x, mLookTarget.y, mLookTarget.z);
    SetUp(mUpOrtho.x, mUpOrtho.y, mUpOrtho.z);
}

void RPLookAtCam::Calculate() {
    EGG::CoreControllerMgr::instance()->func_800A6F50(0);
    UpdateView();
}

void RPLookAtCam::UpdateView() {
    mLookVec.x = mLookTarget.x - mLookPos.x;
    mLookVec.y = mLookTarget.y - mLookPos.y;
    mLookVec.z = mLookTarget.z - mLookPos.z;
    mLookDist = nw4r::math::VEC3Len(&mLookVec);

    if (mLookDist != 0.0f) {
        nw4r::math::VEC3Normalize(&mDir, &mLookVec);
        nw4r::math::VEC3Cross(&mSide, &mLookUp, &mDir);
        nw4r::math::VEC3Normalize(&mSide, &mSide);
        nw4r::math::VEC3Cross(&mUpOrtho, &mDir, &mSide);
    }

    SetPosition(mLookPos.x, mLookPos.y, mLookPos.z);
    SetTarget(mLookTarget.x, mLookTarget.y, mLookTarget.z);
    SetUp(mUpOrtho.x, mUpOrtho.y, mUpOrtho.z);
}
