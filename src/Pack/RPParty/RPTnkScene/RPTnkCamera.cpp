// Tanks! field camera (RPTnkCamera): intro fly-in along a polyline, then a fixed top view.
// Unit 0x80256DE4-0x802575BC (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkCamera.h>

#include <Pack/RPParty/RPPartyCommon/RPPolyline.h>

#include <egg/core.h>

// Intro: start position and target, then the path through sIntroPath1-3 to sPlayPos
static nw4r::math::VEC3 sIntroPos(0.0f, 5.0f, 120.0f);
static nw4r::math::VEC3 sIntroTarget(0.0f, 15.0f, 0.0f);
static nw4r::math::VEC3 sIntroPath1(0.0f, 5.0f, 155.0f);
static nw4r::math::VEC3 sIntroPath2(0.0f, 8.0f, 175.0f);
static nw4r::math::VEC3 sIntroPath3(0.0f, 10.0f, 190.0f);
static nw4r::math::VEC3 sPlayPos(0.0f, 2000.0f, 0.0f);
static nw4r::math::VEC3 sPlayUp(0.0f, 0.0f, -14.0f);
static nw4r::math::VEC3 sFixedTargetUp(0.0f, 0.0f, -1.0f);

RPTnkCamera* RPTnkCamera::sInstance;

void RPTnkCamera::CreateInstance() {
    sInstance = new RPTnkCamera();
}

void RPTnkCamera::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkCamera::~RPTnkCamera() {}

void RPTnkCamera::StateIntroMove() {
    if (mIntroT >= 0.9f) {
        mIntroSpeed *= 0.94f;
        if (mIntroSpeed < 0.002f) {
            mIntroSpeed = 0.002f;
        }
    }

    mIntroT += mIntroSpeed;
    if (mIntroT >= 1.0f) {
        mIntroT = 1.0f;
    }

    mpIntroPath->Calc(mIntroT, &mLookPos);
    f32 t = mIntroT;
    f32 start;
    f32 end;
    end = sPlayUp.y;
    start = sIntroTarget.y;
    f32 delta = end - start;
    mLookTarget.y = delta * t + start;
    if (1.0f == t) {
        mMode = 1;
    }
}

void RPTnkCamera::StateFixed() {}

void RPTnkCamera::Reset() {
    RPLookAtCam::Reset();
    mpIntroPath = new RPPolyline(5);
    mIntroT = 0.0f;
    mIntroSpeed = 1.0f / 150.0f;
    unk1E4 = 0;
    mMode = 1;
}

void RPTnkCamera::InitView() {
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

void RPTnkCamera::Calculate() {
    EGG::CoreControllerMgr::instance()->func_800A6F50(0);
    (this->*mStateFuncs[mMode])();
    UpdateView();
}

void RPTnkCamera::SetMode(s32 mode) {
    SetCameraType(ECameraType_LookAt);

    switch (mode) {
    case 0: {
        SetLookPos(sIntroPos);
        SetLookTarget(sIntroTarget);
        nw4r::math::VEC3 points[5] = {
            static_cast<const nw4r::math::_VEC3&>(sIntroPos),
            static_cast<const nw4r::math::_VEC3&>(sIntroPath1),
            static_cast<const nw4r::math::_VEC3&>(sIntroPath2),
            static_cast<const nw4r::math::_VEC3&>(sIntroPath3),
            static_cast<const nw4r::math::_VEC3&>(sPlayPos)};
        mpIntroPath->SetPoints(points);
        mIntroT = 0.0f;
        mIntroSpeed = 1.0f / 150.0f;
        unk1E4 = 0;
        InitView();
        break;
    }
    case 1: {
        nw4r::math::VEC3 pos;
        pos.x = 0.0f;
        pos.y = 2000.0f * nw4r::math::SinDeg(42.0f);
        pos.z = 2000.0f * nw4r::math::CosDeg(42.0f) + sPlayUp.z;
        nw4r::math::VEC3 up;
        up.x = 0.0f;
        up.y = nw4r::math::CosDeg(42.0f);
        up.z = -nw4r::math::SinDeg(42.0f);
        SetLookPos(pos);
        SetLookTarget(sPlayUp);
        SetLookUp(up);
        InitView();
        break;
    }
    }

    mMode = mode;
    CalcMatrix();
}

void RPTnkCamera::UpdateView() {
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
