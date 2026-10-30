// Shooting Range camera (RPDucCamera): intro fly-in along a polyline, then a fixed view whose
// frustum edges give the visible range at a depth.
// Unit 0x802420CC-0x80242B40 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucCamera.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPPolyline.h>

#include <egg/core.h>
#include <egg/gfxe.h>

// Intro: start position and target, then the path through sIntroPath1-3 to sPlayPos
static nw4r::math::VEC3 sIntroPos(0.0f, 5.0f, 120.0f);
static nw4r::math::VEC3 sIntroTarget(0.0f, 15.0f, 0.0f);
static nw4r::math::VEC3 sIntroPath1(0.0f, 5.0f, 155.0f);
static nw4r::math::VEC3 sIntroPath2(0.0f, 8.0f, 175.0f);
static nw4r::math::VEC3 sIntroPath3(0.0f, 10.0f, 190.0f);
static nw4r::math::VEC3 sPlayPos(0.0f, 15.0f, 200.0f);
static nw4r::math::VEC3 sPlayTarget(0.0f, 35.0f, 0.0f);
static nw4r::math::VEC3 sPlayUp(0.0f, 1.0f, 0.0f);

RPDucCamera* RPDucCamera::sInstance;

void RPDucCamera::CreateInstance() {
    sInstance = new RPDucCamera();
}

void RPDucCamera::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucCamera::~RPDucCamera() {}

void RPDucCamera::StateIntro() {
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
    end = sPlayTarget.y;
    start = sIntroTarget.y;
    f32 delta = end - start;
    mLookTarget.y = delta * t + start;
    if (1.0f == t) {
        mMode = 1;
    }
}

void RPDucCamera::StateIdle() {}

void RPDucCamera::Reset() {
    RPLookAtCam::Reset();
    mpIntroPath = new RPPolyline(5);
    mIntroT = 0.0f;
    mIntroSpeed = 1.0f / 150.0f;
    unk1E4 = 0;
    mMode = 1;
}

void RPDucCamera::Calculate() {
    EGG::CoreControllerMgr::instance()->func_800A6F50(0);
    (this->*mStateFuncs[mMode])();
    UpdateView();

    RPSndAudioMgr* pAudioMgr = RPSndAudioMgr::instance();
    nw4r::math::MTX34 viewMtx = GetViewMatrix();
    if (pAudioMgr->getCameraCount() > 0) {
        pAudioMgr->setListenerMatrix(0, viewMtx);
    }
}

void RPDucCamera::SetMode(s32 mode) {
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
    case 1:
        SetLookPos(sPlayPos);
        SetLookTarget(sPlayTarget);
        SetLookUp(sPlayUp);
        InitView();
        CalcFrustumEdges();
        break;
    }

    mMode = mode;
    CalcMatrix();
}

void RPDucCamera::CalcFrustumEdges() {
    RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
    f32 nearZ = pScreen->GetNearZ();
    f32 nearHalfHeight = nearZ * pScreen->GetTanFovy();
    f32 halfHeight = (u16)(EGG::Screen::GetSizeYMax() / 2);
    f32 halfWidth = (u16)(EGG::Screen::GetSizeXMax() / 2);

    nw4r::math::VEC3 center;
    center.x = mDir.x * nearZ + mLookPos.x;
    center.y = mDir.y * nearZ + mLookPos.y;
    center.z = mDir.z * nearZ + mLookPos.z;

    nw4r::math::VEC3 left = mSide;
    left.x = -left.x;
    left.y = -left.y;
    left.z = -left.z;
    f32 unit = nearHalfHeight / halfHeight;
    f32 nearHalfWidth = unit * halfWidth;

    nw4r::math::VEC3 up;
    PSVECCrossProduct(left, mDir, up);

    mEdgeLeft.x = center.x - left.x * nearHalfWidth;
    mEdgeLeft.y = center.y - left.y * nearHalfWidth;
    mEdgeLeft.z = center.z - left.z * nearHalfWidth;
    mEdgeRight.x = left.x * nearHalfWidth + center.x;
    mEdgeRight.y = left.y * nearHalfWidth + center.y;
    mEdgeRight.z = left.z * nearHalfWidth + center.z;
    mEdgeTop.x = up.x * nearHalfHeight + center.x;
    mEdgeTop.y = up.y * nearHalfHeight + center.y;
    mEdgeTop.z = up.z * nearHalfHeight + center.z;
    mEdgeBottom.x = center.x - up.x * nearHalfHeight;
    mEdgeBottom.y = center.y - up.y * nearHalfHeight;
    mEdgeBottom.z = center.z - up.z * nearHalfHeight;
}

f32 RPDucCamera::GetScreenLeftAtDepth(f32 z) const {
    nw4r::math::VEC3 normal(0.0f, 0.0f, 1.0f);
    nw4r::math::VEC3 point(0.0f, 0.0f, z);
    nw4r::math::VEC3 out;
    RPIntersectLinePlane(&out, &mLookPos, &mEdgeLeft, &point, &normal);
    return out.x;
}

f32 RPDucCamera::GetScreenRightAtDepth(f32 z) const {
    nw4r::math::VEC3 normal(0.0f, 0.0f, 1.0f);
    nw4r::math::VEC3 point(0.0f, 0.0f, z);
    nw4r::math::VEC3 out;
    RPIntersectLinePlane(&out, &mLookPos, &mEdgeRight, &point, &normal);
    return out.x;
}

f32 RPDucCamera::GetScreenTopAtDepth(f32 z) const {
    nw4r::math::VEC3 normal(0.0f, 0.0f, 1.0f);
    nw4r::math::VEC3 point(0.0f, 0.0f, z);
    nw4r::math::VEC3 out;
    RPIntersectLinePlane(&out, &mLookPos, &mEdgeTop, &point, &normal);
    return out.y;
}

void RPDucCamera::ProjectToScreen(nw4r::math::VEC2* pOut, const nw4r::math::VEC3& rPos) const {
    nw4r::math::VEC3 screenPos;
    GetWorldToScreen(&screenPos, rPos, RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen());
    pOut->x = screenPos.x / ((u16)(EGG::Screen::GetSizeXMax() / 2));
    pOut->y = screenPos.y / ((u16)(EGG::Screen::GetSizeYMax() / 2));
}
