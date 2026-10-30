// Billiards pocketed-ball icon (RPBilBallIcon): a bad_ball model that slides in on the 2D
// screen and out again (ten made by RPBilBallMgr).
// Unit 0x802D0434-0x802D0EC8. Functions in reverse address order; the state machine
// instantiation follows at the end of the file.
#include <Pack/RPParty/RPBilScene/RPBilBallIcon.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPPartyCommon/RPInterpCamera.h>

#include <egg/gfxe.h>
#include <egg/math.h>

f32 RPBilBallIcon::sBaseScale = 20.0f;
s32 RPBilBallIcon::sEnterFrames = 20;
s32 RPBilBallIcon::sLeaveFrames = 15;

namespace {

// Element-wise copy (all loads before the stores)
inline void SetVec(EGG::Vector3f& rDst, const nw4r::math::VEC3& rSrc) {
    rDst.set(rSrc.x, rSrc.y, rSrc.z);
}

} // namespace

RPBilBallIcon::~RPBilBallIcon() {}

RPBilBallIcon::RPBilBallIcon(s32 index) {
    mIndex = index;
    mpStateMachine = new RPStateMachine<RPBilBallIcon>(this, STATE_NUM);

    RPBilStateFuncs<RPBilBallIcon> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPBilBallIcon::EnterEnter, &RPBilBallIcon::EnterCalc},
        {&RPBilBallIcon::WaitEnter, &RPBilBallIcon::WaitCalc},
        {&RPBilBallIcon::LeaveEnter, &RPBilBallIcon::LeaveCalc},
    };
    for (int i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }
}

void RPBilBallIcon::Load() {
    mpModel = RPGrpModel::Construct(2, "bad_ball", 1, 0, 3);
    mpModel->SetDrawScene(4);
    mpModel->CreateBoundingInfo(1);

    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_TexPat, 1);
    mpModel->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_TexPat, 0, 2, "bad_ball");
    mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexPat, 0, mIndex);
    mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_TexPat, 0, 0.0f);
}

void RPBilBallIcon::Reset() {
    mpStateMachine->Reset(STATE_NONE);
}

void RPBilBallIcon::Exit() {}

void RPBilBallIcon::Calculate() {
    mpStateMachine->Calculate();

    if (mpStateMachine->GetState() != STATE_NONE) {
        RPGrpView* pView = RPGrpRenderer::GetCurrent()->GetView(4);
        pView->GetScreenToWorld(&mPos, mScreenPos);

        RPGrpCamera* pCamera = pView->GetCamera();
        EGG::Vector3f dir;
        SetVec(dir, pCamera->GetTargetRef() - pCamera->GetPosition());
        dir.normalise();
        mAxisY = -dir;

        const nw4r::math::VEC3& rUp = pCamera->GetUp();
        mAxisZ = EGG::Vector3f(-rUp.x, -rUp.y, -rUp.z);
        mAxisX = mAxisY.cross(mAxisZ);

        CalcModelMtx();
    }
}

void RPBilBallIcon::EnterEnter() {}

void RPBilBallIcon::EnterCalc() {
    f32 t = RPRange::CalcEase(
        (f32)mpStateMachine->GetTimer() / (f32)sEnterFrames, 1);
    mScreenPos = mBasePos;
    mScreenPos.x -= (1.0f - t) * EGG::Screen::GetSizeXMax();

    EGG::Vector2f offset = GetRowOffset();
    mScreenPos.x += offset.x;
    mScreenPos.y += offset.y;

    if (mpStateMachine->GetTimer() >= sEnterFrames) {
        mpStateMachine->ChangeState(STATE_WAIT);
    }
}

void RPBilBallIcon::WaitEnter() {}

void RPBilBallIcon::WaitCalc() {}

void RPBilBallIcon::LeaveEnter() {}

void RPBilBallIcon::LeaveCalc() {
    f32 t = RPRange::CalcEase(
        (f32)mpStateMachine->GetTimer() / (f32)sLeaveFrames, 1);
    mScreenPos = mBasePos;
    mScreenPos.x += t * EGG::Screen::GetSizeXMax();

    EGG::Vector2f offset = GetRowOffset();
    mScreenPos.x += offset.x;
    mScreenPos.y += offset.y;

    if (mpStateMachine->GetTimer() >= sLeaveFrames) {
        mpStateMachine->ChangeState(STATE_NONE);
    }
}

void RPBilBallIcon::Show(f32 y, u32 player, s32 row, s32 slot) {
    mPlayer = player;
    mRow = row;
    mSlot = slot;

    nw4r::math::VEC2 pos(0.0f, y);
    mBasePos = pos;

    const f32 rowScales[9] = {1.0f, 1.0f, 1.0f, 1.0f, 0.8f, 0.7f, 0.6f, 0.52f, 0.44f};
    s32 idx = row - 1;
    mScale = sBaseScale * rowScales[idx];
    mSize = mScale * 2.85f * 2.0f * 1.2f;

    mpStateMachine->ChangeState(STATE_ENTER);
}

void RPBilBallIcon::Hide() {
    s32 state = mpStateMachine->GetState();
    if (state == STATE_LEAVE) {
        return;
    }
    if (state == STATE_NONE) {
        return;
    }
    mpStateMachine->ChangeState(STATE_LEAVE);
}

EGG::Vector2f RPBilBallIcon::GetRowOffset() const {
    EGG::Vector2f offset = EGG::Vector2f::zero;
    offset.x = mBasePos.x;
    offset.y = mBasePos.y;
    offset.x -= mSize * (0.5f * (mRow - 1) - mSlot);
    return offset;
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template void RPStateMachine<RPBilBallIcon>::Calculate();
template void RPStateMachine<RPBilBallIcon>::ChangeState(s32);
template void RPStateMachine<RPBilBallIcon>::Reset(s32);
template void RPStateMachine<RPBilBallIcon>::SetStateFuncs(const StateFunc&, const StateFunc&,
                                                           s32);
template RPStateMachine<RPBilBallIcon>::RPStateMachine(RPBilBallIcon*, s32);
template void RPStateMachine<RPBilBallIcon>::CallEnter();
template void RPStateMachine<RPBilBallIcon>::CallCalc();
template void RPStateMachine<RPBilBallIcon>::CallExit();
