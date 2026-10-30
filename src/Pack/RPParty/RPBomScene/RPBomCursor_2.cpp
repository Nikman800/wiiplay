#include <Pack/RPParty/RPBomScene/RPBomCursor_2.h>

#include <Pack/RPParty/RPBomScene/RPBomCamera.h>
#include <Pack/RPParty/RPPartyCommon/RPCursorTrail.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <egg/core/eggController.h>
#include <egg/gfxe/eggScreen.h>

extern nw4r::math::VEC2 RP_vec2Zero;

// The original symbol names this VEC2 normalization routine as a matrix helper.
static inline nw4r::math::VEC2 GetTrailPos(RPCursorTrail* trail) {
    return trail->GetPos(trail->GetLatest());
}

static inline nw4r::math::VEC2 GetTrailDir(RPCursorTrail* trail) {
    return trail->GetDir(trail->GetLatestDir());
}

static inline f32 CursorMax(f32 value, f32 bound) {
    return bound > value ? bound : value;
}

RPBomCursor::RPBomCursor() {
    mPos.x = RP_vecZero.x;
    mPos.y = RP_vecZero.y;
    mPos.z = RP_vecZero.z;
    mScale.x = RP_vecOne.x;
    mScale.y = RP_vecOne.y;
    mScale.z = RP_vecOne.z;
    mRot = RP_mtx33Identity;
    mPointer.x = RP_vec2Zero.x;
    mPointer.y = RP_vec2Zero.y;
    mController = 0;
}

RPBomCursor::~RPBomCursor() {}

static inline void ClampPointer(nw4r::math::VEC2& rPointer, const nw4r::math::VEC2& pos) {
    if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
        f32 x = CursorMax(pos.x, -0.965f);
        rPointer.x = 0.965f < x ? 0.965f : x;
        f32 y = CursorMax(pos.y, -0.53f);
        rPointer.y = 0.53f < y ? 0.53f : y;
    } else if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_STD) {
        f32 x = CursorMax(pos.x, -0.965f);
        rPointer.x = 0.965f < x ? 0.965f : x;
        f32 y = CursorMax(pos.y, -0.73f);
        rPointer.y = 0.73f < y ? 0.73f : y;
    }
}

static inline void UpdatePointer(nw4r::math::VEC2& rPointer, RPCursorTrail* trail) {
    nw4r::math::VEC2 pos = GetTrailPos(trail);
    ClampPointer(rPointer, pos);
}

static inline void ApproachDir(nw4r::math::VEC2& rDir, const nw4r::math::VEC2& t) {
    rDir.x += t.x - rDir.x;
    rDir.y += t.y - rDir.y;
}

static inline void UpdateDir(nw4r::math::VEC2& rDir, RPCursorTrail* trail) {
    nw4r::math::VEC2 t = GetTrailDir(trail);
    ApproachDir(rDir, t);
}

void RPBomCursor::UpdateFromPointer() {
    nw4r::math::VEC3 dir = RPBomCamera::sInstance->GetDir();
    nw4r::math::VEC3 side = RPBomCamera::sInstance->GetSide();
    side.x = -side.x;
    side.y = -side.y;
    side.z = -side.z;
    nw4r::math::VEC3 up;
    nw4r::math::VEC2 screen;
    PSVECCrossProduct(side, dir, up);

    mpTrail->Update();
    if (mpTrail->GetLatest() != -1) {
        UpdatePointer(mPointer, mpTrail);
    }

    screen.x = mPointer.x * mRange;
    screen.y = mPointer.y * mRange;
    mPos.x = screen.x;
    mPos.y = 1000.0f;
    mPos.z = screen.y;
    mScale.x = 2.8f;
    mScale.y = 2.8f;
    mScale.z = 2.8f;
    mRot.z.x = -dir.x;
    mRot.z.y = -dir.y;
    mRot.z.z = -dir.z;

    if (mpTrail->GetLatestDir() != -1) {
        UpdateDir(mDir, mpTrail);
        nw4r::math::VEC2Normalize(&mDir, &mDir);
        mAngle = nw4r::math::Atan2Deg(mDir.y, mDir.x);
    }

    mRot.x.x = mDir.x;
    mRot.x.y = 0.0f;
    mRot.x.z = mDir.y;
    PSVECCrossProduct(mRot.z, mRot.x, mRot.y);
}

void RPBomCursor::Create() {
    mpTrail = new RPCursorTrail(0, 10);
}

void RPBomCursor::Reset() {
    mRange = static_cast<f32>(static_cast<u16>(EGG::Screen::GetSizeXMax() / 2));
    mPointer.x = mController == 0 ? -0.4f : 0.4f;
    mPointer.y = 0.15f;
    mDir.x = 1.0f;
    mDir.y = 0.0f;
    mAngle = 0.0f;
    mpTrail->SetChannel(mController);
    mpTrail->Reset();
}

void RPBomCursor::Calculate() {
    EGG::CoreControllerMgr::instance()->getNthController(static_cast<u16>(mController));
    UpdateFromPointer();
}

void RPBomCursor::Draw() {}

void RPBomCursor::SetControllerIndex(s32 idx) {
    mController = idx;
}
