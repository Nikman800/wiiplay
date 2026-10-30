
#include <Pack/RPParty/RPBomScene/RPBomCamera.h>

#include <Pack/RPParty/RPPartyCommon/RPPolyline.h>

#include <egg/core.h>

// Zoom-in: start position and target, then the path through sZoomPath1-3 to sTopPos
static nw4r::math::VEC3 sZoomPos(0.0f, 5.0f, 120.0f);
static nw4r::math::VEC3 sZoomTarget(0.0f, 15.0f, 0.0f);
static nw4r::math::VEC3 sZoomPath1(0.0f, 5.0f, 155.0f);
static nw4r::math::VEC3 sZoomPath2(0.0f, 8.0f, 175.0f);
static nw4r::math::VEC3 sZoomPath3(0.0f, 10.0f, 190.0f);
static nw4r::math::VEC3 sTopPos(0.0f, 2000.0f, 0.0f);
static nw4r::math::VEC3 sTopTarget(0.0f, 0.0f, 0.0f);
static nw4r::math::VEC3 sTopUp(0.0f, 0.0f, -1.0f);

RPBomCamera* RPBomCamera::sInstance;

void RPBomCamera::CreateInstance() {
    sInstance = new RPBomCamera();
}

void RPBomCamera::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBomCamera::~RPBomCamera() {}

void RPBomCamera::StateZoomIn() {
    if (mPathT >= 0.9f) {
        mPathSpeed *= 0.94f;
        if (mPathSpeed < 0.002f) {
            mPathSpeed = 0.002f;
        }
    }

    mPathT += mPathSpeed;
    if (mPathT >= 1.0f) {
        mPathT = 1.0f;
    }

    mpPath->Calc(mPathT, &mLookPos);
    f32 t = mPathT;
    f32 start;
    f32 end;
    end = sTopTarget.y;
    start = sZoomTarget.y;
    f32 delta = end - start;
    mLookTarget.y = delta * t + start;
    if (1.0f == t) {
        mMode = 1;
    }
}

void RPBomCamera::StateIdle() {}

void RPBomCamera::Reset() {
    RPLookAtCam::Reset();
    mpPath = new RPPolyline(5);
    mPathT = 0.0f;
    mPathSpeed = 1.0f / 150.0f;
    unk1E4 = 0;
    mMode = 1;
}

void RPBomCamera::InitView() {
    mLookVec.x = mLookTarget.x - mLookPos.x;
    mLookVec.y = mLookTarget.y - mLookPos.y;
    mLookVec.z = mLookTarget.z - mLookPos.z;
    PSVECNormalize(mLookVec, mDir);
    PSVECNormalize(mLookUp, mUpOrtho);
    PSVECCrossProduct(mUpOrtho, mDir, mSide);

    SetPosition(mLookPos.x, mLookPos.y, mLookPos.z);
    SetTarget(mLookTarget.x, mLookTarget.y, mLookTarget.z);
    SetUp(mUpOrtho.x, mUpOrtho.y, mUpOrtho.z);
}

void RPBomCamera::Calculate() {
    EGG::CoreControllerMgr::instance()->func_800A6F50(0);
    (this->*mStateFuncs[mMode])();
    UpdateView();
}

void RPBomCamera::SetMode(s32 mode) {
    SetCameraType(ECameraType_LookAt);

    switch (mode) {
    case 0: {
        SetLookPos(sZoomPos);
        SetLookTarget(sZoomTarget);
        nw4r::math::VEC3 points[5] = {
            static_cast<const nw4r::math::_VEC3&>(sZoomPos),
            static_cast<const nw4r::math::_VEC3&>(sZoomPath1),
            static_cast<const nw4r::math::_VEC3&>(sZoomPath2),
            static_cast<const nw4r::math::_VEC3&>(sZoomPath3),
            static_cast<const nw4r::math::_VEC3&>(sTopPos)};
        mpPath->SetPoints(points);
        mPathT = 0.0f;
        mPathSpeed = 1.0f / 150.0f;
        unk1E4 = 0;
        InitView();
        break;
    }
    case 1: {
        nw4r::math::VEC3 pos;
        pos.x = 0.0f;
        pos.y = 5000.0f * nw4r::math::SinDeg(90.0f);
        pos.z = 5000.0f * nw4r::math::CosDeg(90.0f);
        nw4r::math::VEC3 up;
        up.x = 0.0f;
        up.y = nw4r::math::CosDeg(90.0f);
        up.z = -nw4r::math::SinDeg(90.0f);
        SetLookPos(pos);
        SetLookTarget(sTopTarget);
        SetLookUp(up);
        InitView();
        break;
    }
    }

    mMode = mode;
    CalcMatrix();
}

void RPBomCamera::UpdateView() {
    mLookVec.x = mLookTarget.x - mLookPos.x;
    mLookVec.y = mLookTarget.y - mLookPos.y;
    mLookVec.z = mLookTarget.z - mLookPos.z;
    PSVECNormalize(mLookVec, mDir);
    PSVECNormalize(mLookUp, mUpOrtho);
    PSVECCrossProduct(mUpOrtho, mDir, mSide);

    SetPosition(mLookPos.x, mLookPos.y, mLookPos.z);
    SetTarget(mLookTarget.x, mLookTarget.y, mLookTarget.z);
    SetUp(mUpOrtho.x, mUpOrtho.y, mUpOrtho.z);
}
