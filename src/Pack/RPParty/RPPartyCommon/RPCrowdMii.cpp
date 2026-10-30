// Background Miis on the game-select screen.
#include <Pack/RPParty/RPPartyCommon/RPCrowdMii.h>

#include <Pack/RPParty/RPPartyCommon/RPFaceCtrl.h>
#include <Pack/RPKernel/RPSysKokeshi.h>
#include <Pack/RPKernel/RPSysKokeshiGenInfo.h>
#include <Pack/RPUtility/RPUtlRandom.h>
#include <egg/gfxe/eggScreen.h>

inline f32 CrowdLookAngle() {
    return 1.5707964f;
}

f32 RPCrowdMii::sScale = 18.0f;
f32 RPCrowdMii::sBaseY = -275.0f;
f32 RPCrowdMii::sBaseZ = -1500.0f;
f32 RPCrowdMii::sEdgeX = 1500.0f;
f32 RPCrowdMii::sSpacingZ = 50.0f;
f32 RPCrowdMii::sMove1Speed = 3.0f;
f32 RPCrowdMii::sMove2Speed = 5.0f;
f32 RPCrowdMii::sRunawaySpeed = 8.0f;
f32 RPCrowdMii::sSwimSpeed = 4.0f;

f32 RPCrowdMii::sLookDistance = -RPCrowdMii::sBaseZ;
f32 RPCrowdMii::sLookAngle = CrowdLookAngle();
RPCrowdMgr* RPCrowdMgr::sInstance;

inline f32 CrowdMin(f32 a, f32 b) {
    if (a < b) return a;
    return b;
}

inline f32 CrowdAbs(f32 a) {
    if (a > 0.0f) return a;
    return -a;
}

inline void CrowdSetDirection(EGG::Vector3f& rDir, f32 x, f32 y, f32 z) {
    rDir = EGG::Vector3f(x, y, z);
}

RPCrowdMii::~RPCrowdMii() {}

RPCrowdMii::RPCrowdMii(s32 index) : RPDollBase(index) {
    mpStateMachine = new RPCrowdMiiSeq(this, 8);
    RPCrowdMiiSeq::StateFunc funcs[8][2] = {
        {NULL, NULL},
        {&RPCrowdMii::EnterIdle, &RPCrowdMii::CalcIdle},
        {&RPCrowdMii::EnterMove1, &RPCrowdMii::CalcMove1},
        {&RPCrowdMii::EnterMove2, &RPCrowdMii::CalcMove2},
        {&RPCrowdMii::EnterRunaway, &RPCrowdMii::CalcRunaway},
        {&RPCrowdMii::EnterSwim, &RPCrowdMii::CalcSwim},
        {&RPCrowdMii::EnterFlail, &RPCrowdMii::CalcFlail},
        {&RPCrowdMii::EnterLook, &RPCrowdMii::CalcLook}
    };
    for (s32 i = 0; i < 8; i++) {
        mpStateMachine->SetStateFuncs(funcs[i][0], funcs[i][1], i);
    }

    RPSysKokeshiGenInfo info;
    info.SetGenType(static_cast<RPSysKokeshiManager::GenType>(1));
    info.SetResolution(static_cast<RFLResolution>(64));
    info.SetBodyType(4);
    info.SetExpressionFlag(0x7f);
    mpKokeshi = new RPSysKokeshi(&info);
    mpFaceCtrl = new RPFaceCtrl;
}

void RPCrowdMii::Load() {
    RPDollBase::Load();
    RPGrpModelAnm* pAnm = mpKokeshi->GetBodyModel()->GetModelAnm();
    const char* names[7] = {
        "cmn_waitR", "cmn_walkR", "cmn_jogR", "cmn_runawayR",
        "cmn_swimR", "cmn_batabataR", "cmn_winR"
    };
    pAnm->CreateBuffer(RPGrpModelAnm::Anm_Chr, 7);
    for (s32 i = 0; i < 7; i++) {
        pAnm->RegistFromResFile(RPGrpModelAnm::Anm_Chr, i, 1, names[i], 0);
    }
}

void RPCrowdMii::Reset() {
    RPDollBase::Reset();
    mpStateMachine->Reset(0);
    mScreenAspect = EGG::Screen::GetSizeXMax() /
                    EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD);
    mPos = EGG::Vector3f(sEdgeX * mScreenAspect, sBaseY,
                         -(mIndex * sSpacingZ - sBaseZ));
    mAxisX = EGG::Vector3f(1.0f, 0.0f, 0.0f);
    mAxisY = EGG::Vector3f(0.0f, 1.0f, 0.0f);
    mAxisZ = EGG::Vector3f(0.0f, 0.0f, 1.0f);
    mScale = sScale;
    mSpeed = 0.0f;
    RPSysKokeshiManager::ChangeExpression(mpKokeshi->GetNigaoeModel(),
                                         static_cast<RFLExpression>(0));
    mpFaceCtrl->Reset(mpKokeshi);
}

void RPCrowdMii::Calculate() {
    mScale = sScale;
    mPos.y = sBaseY;
    mPos.z = -(mIndex * sSpacingZ - sBaseZ);
    mpStateMachine->Calculate();
    if (mIsLooking) {
        LookAt(EGG::Vector2f::zero, sLookDistance, sLookAngle);
    }
    RPDollBase::Calculate();
    if (mpStateMachine->GetState() != 0) {
        mpFaceCtrl->Calculate(mpKokeshi);
        UpdateModelMatrices();
    }
}

void RPCrowdMii::EnterIdle() {
    if (static_cast<s32>(RPUtlRandom::getU32(2)) == 0) {
        LookAt(EGG::Vector2f::zero, sLookDistance, sLookAngle);
    }
    PlayAnm(0, 30.0f);
    mSpeed = 0.0f;
}

void RPCrowdMii::CalcIdle() {
    if (mpStateMachine->GetTimer() > 120) {
        s32 states[6] = {2, 3, 4, 5, 6, 7};
        s32 weights[6] = {10, 4, 6, 4, 2, 6};
        u32 total = 0;
        for (s32 i = 0; i < 6; i++) total += weights[i];
        s32 pick = RPUtlRandom::getU32(total);
        s32 sum = 0;
        for (s32 i = 0; i < 6; i++) {
            sum += weights[i];
            if (pick < sum) {
                mpStateMachine->ChangeState(states[i]);
                break;
            }
        }
    }
}

void RPCrowdMii::EnterMove1() {
    if (static_cast<s32>(RPUtlRandom::getU32(2)) == 0) ClearLook();
    PlayAnm(1, 40.0f);
    mTargetSpeed = sMove1Speed;
    mStateEndTime = RPUtlRandom::getU32(200) + 150;
}

void RPCrowdMii::CalcMove1() {
    mSpeed += 0.1f * mTargetSpeed;
    mSpeed = CrowdMin(mSpeed, mTargetSpeed);
    static_cast<EGG::Vector3f&>(mPos) += mAxisZ * mSpeed;
    if (CrowdAbs(mPos.x) > sEdgeX * mScreenAspect) {
        mpStateMachine->ChangeState(0);
    }
    if (mpStateMachine->GetTimer() > mStateEndTime) PickMoveState();
}

void RPCrowdMii::EnterRunaway() {
    if (static_cast<s32>(RPUtlRandom::getU32(2)) == 0) ClearLook();
    PlayAnm(3, 30.0f);
    mTargetSpeed = sRunawaySpeed;
    mStateEndTime = RPUtlRandom::getU32(100) + 100;
}

void RPCrowdMii::CalcRunaway() {
    mSpeed += 0.1f * mTargetSpeed;
    mSpeed = CrowdMin(mSpeed, mTargetSpeed);
    static_cast<EGG::Vector3f&>(mPos) += mAxisZ * mSpeed;
    if (CrowdAbs(mPos.x) > sEdgeX * mScreenAspect) {
        mpStateMachine->ChangeState(0);
    }
    if (mpStateMachine->GetTimer() > mStateEndTime) {
        if (static_cast<s32>(RPUtlRandom::getU32(4)) == 0) {
            mpStateMachine->ChangeState(3);
        } else {
            mpStateMachine->ChangeState(2);
        }
    }
}

void RPCrowdMii::EnterMove2() {
    if (static_cast<s32>(RPUtlRandom::getU32(2)) == 0) ClearLook();
    PlayAnm(2, 30.0f);
    mTargetSpeed = sMove2Speed;
    mStateEndTime = RPUtlRandom::getU32(100) + 100;
}

void RPCrowdMii::CalcMove2() {
    mSpeed += 0.1f * mTargetSpeed;
    mSpeed = CrowdMin(mSpeed, mTargetSpeed);
    static_cast<EGG::Vector3f&>(mPos) += mAxisZ * mSpeed;
    if (CrowdAbs(mPos.x) > sEdgeX * mScreenAspect) {
        mpStateMachine->ChangeState(0);
    }
    if (mpStateMachine->GetTimer() > mStateEndTime) {
        if (static_cast<s32>(RPUtlRandom::getU32(3)) == 0) {
            mpStateMachine->ChangeState(4);
        } else {
            mpStateMachine->ChangeState(2);
        }
    }
}

void RPCrowdMii::EnterSwim() {
    if (static_cast<s32>(RPUtlRandom::getU32(2)) == 0) ClearLook();
    PlayAnm(4, 40.0f);
    mTargetSpeed = sSwimSpeed;
    mStateEndTime = RPUtlRandom::getU32(200) + 200;
}

void RPCrowdMii::CalcSwim() {
    mSpeed += 0.1f * mTargetSpeed;
    mSpeed = CrowdMin(mSpeed, mTargetSpeed);
    static_cast<EGG::Vector3f&>(mPos) += mAxisZ * mSpeed;
    if (CrowdAbs(mPos.x) > sEdgeX * mScreenAspect) {
        mpStateMachine->ChangeState(0);
    }
    if (mpStateMachine->GetTimer() > mStateEndTime) {
        mpStateMachine->ChangeState(1);
    }
}

void RPCrowdMii::EnterFlail() {
    if (static_cast<s32>(RPUtlRandom::getU32(2)) == 0) {
        LookAt(EGG::Vector2f::zero, sLookDistance, sLookAngle);
    }
    PlayAnm(5, 20.0f);
}

void RPCrowdMii::CalcFlail() {
    if (mpStateMachine->GetTimer() > 100) PickMoveState();
}

void RPCrowdMii::EnterLook() {
    LookAt(EGG::Vector2f::zero, sLookDistance, sLookAngle);
    PlayAnm(6, 20.0f);
}

void RPCrowdMii::CalcLook() {
    if (mpStateMachine->GetTimer() > 120) PickMoveState();
}

// STAND-IN: dead-stripped; compiled before EnterLook under deferred codegen, it puts 0.1f
// (lbl_8043F250) ahead of 20.0f (8043F254), as in the original .sdata2. Without it the two are
// swapped.
extern "C" f32 Party_crowdPoolOrder(f32 x) {
    return x * 0.1f;
}

void RPCrowdMii::PickMoveState() {
    s32 states[4] = {1, 2, 4, 5};
    s32 weights[4] = {25, 50, 15, 10};
    u32 total = 0;
    for (s32 i = 0; i < 4; i++) total += weights[i];
    s32 pick = RPUtlRandom::getU32(total);
    s32 sum = 0;
    for (s32 i = 0; i < 4; i++) {
        sum += weights[i];
        if (pick < sum) {
            mpStateMachine->ChangeState(states[i]);
            break;
        }
    }
}

bool RPCrowdMii::Start(s32 kind) {
    switch (kind) {
    case 0:
        if (mpStateMachine->GetState() == 0) {
            s32 side = RPUtlRandom::getU32(2) * 2 - 1;
            mPos.x = side * (sEdgeX * mScreenAspect);
            CrowdSetDirection(mAxisZ, -side, 0.0f, 0.0f);
            mAxisX = mAxisY.cross(mAxisZ);
        }
        mpStateMachine->ChangeState(1);
        break;
    }
    return true;
}

#pragma defer_codegen off
#pragma auto_inline off
template void RPStateMachine<RPCrowdMii>::Calculate();
template void RPStateMachine<RPCrowdMii>::ChangeState(s32);
template void RPStateMachine<RPCrowdMii>::Reset(s32);
template void RPStateMachine<RPCrowdMii>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template RPStateMachine<RPCrowdMii>::RPStateMachine(RPCrowdMii*, s32);
template void RPStateMachine<RPCrowdMii>::CallEnter();
template void RPStateMachine<RPCrowdMii>::CallCalc();
template void RPStateMachine<RPCrowdMii>::CallExit();
