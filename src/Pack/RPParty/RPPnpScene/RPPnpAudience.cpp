// Table Tennis spectator: one Mii of the crowd, walks in as the single-player rally grows and
// turns towards the ball. Unit 0x8022DE10-0x8022E67C (dtk/game_units.tsv). Functions in reverse
// address order.
#include <Pack/RPParty/RPPnpScene/RPPnpAudience.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPnpScene/RPPnpBall.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPSystem/RPSysPlayer.h>

#include <cmath>
#include <egg/math.h>

RPPnpSpectator::RPPnpSpectator(const RPSysKokeshiGenInfo* pGenInfo, s32 seat, s32 slot)
    : RPSysKokeshi(pGenInfo), IRPSysHostIOSocket() {
    mVisible = TRUE;
    mScale = 10.0f;
    mRotY = 0.0f;
    mSeat = seat;
    mIsSecond = slot == 1;

    // HostIO socket name "Spectator %d %s", "mirror" / "main" (the release build's function is
    // empty). A reference cast: a pointer cast of this would add a NULL check.
    // NONMATCHING-DEBT: r3 is the IRPSysHostIOSocket subobject, so the callee is most likely a
    // variadic IRPSysHostIOSocket member (8018ED00 sits right before that class's constructors);
    // a member `SetNodeName(const char*, ...)` gives the same code with only the symbol renamed.
    // Needs the split/ledger moved (RPSysPlayer.cpp -> IRPSysHostIOSocket.cpp) and RPWlySound.
    RPSysPlayer_DebugSprintf(reinterpret_cast<char*>(&static_cast<IRPSysHostIOSocket&>(*this)),
                             "\x8A\xCF\x8B\x71%d %s", seat,
                             mIsSecond ? "\x83\x7E\x83\x89\x81\x5B" : "\x96\x7B\x91\xCC");

    nw4r::math::VEC3 positions[17] = {
        nw4r::math::VEC3(-215.0f, -5.0f, -290.0f), nw4r::math::VEC3(-215.0f, -5.0f, -148.0f),
        nw4r::math::VEC3(-215.0f, -5.0f, 0.0f),    nw4r::math::VEC3(-215.0f, -5.0f, 150.0f),
        nw4r::math::VEC3(-215.0f, -5.0f, 290.0f),  nw4r::math::VEC3(215.0f, -5.0f, -287.0f),
        nw4r::math::VEC3(215.0f, -5.0f, -152.0f),  nw4r::math::VEC3(215.0f, -5.0f, 0.0f),
        nw4r::math::VEC3(215.0f, -5.0f, 149.0f),   nw4r::math::VEC3(215.0f, -5.0f, 290.0f),
        nw4r::math::VEC3(265.0f, -5.0f, -215.0f),  nw4r::math::VEC3(-265.0f, -5.0f, -220.0f),
        nw4r::math::VEC3(265.0f, -5.0f, -80.0f),   nw4r::math::VEC3(-265.0f, -5.0f, 78.0f),
        nw4r::math::VEC3(265.0f, -5.0f, 221.0f),   nw4r::math::VEC3(-265.0f, -5.0f, 218.0f),
        nw4r::math::VEC3(265.0f, -5.0f, 362.0f),
    };

    SeatParam params[20] = {
        {9, 108, 18, 0},   {0, 108, 18, 0},   {29, 108, 18, 0},  {69, 108, 18, 0},
        {89, 108, 18, 0},  {49, 108, 18, 0},  {19, 108, 18, 0},  {59, 108, 18, 0},
        {39, 108, 18, 0},  {79, 108, 18, 0},  {109, 72, 18, 0},  {119, 72, 18, 0},
        {129, 72, 18, 0},  {139, 72, 18, 0},  {149, 72, 18, 0},  {159, 72, 18, 0},
        {169, 72, 18, 0},  {179, 72, 18, 0},  {189, 72, 18, 0},  {199, 72, 18, 0},
    };

    mPos = positions[mSeat];
    mTargetPos = positions[mSeat];
    mParam = params[mSeat];
}

RPPnpSpectator::~RPPnpSpectator() {}

void RPPnpSpectator::Reset() {
    mVisible = TRUE;
    mScale = 10.0f;
    mParam.mTimer = 0;
}

void RPPnpSpectator::UpdateWalkIn() {
    mPos = mTargetPos;

    s32 timer = mParam.mTimer;
    s32 walkFrames = mParam.mWalkFrames;
    if (mSeat < 10) {
        if (timer < walkFrames - 36) {
            f32 side;
            if (mTargetPos.x < 0.0f) {
                side = -55.0f;
            } else {
                side = 55.0f;
            }

            f32 dz = RPInterpolate(600.0f, 55.0f, timer, walkFrames - 36, 1);
            mPos.x += side;
            mPos.z += dz;
            mTargetRotY = NW4R_MATH_PI;
            mRotY = NW4R_MATH_PI;
        } else {
            f32 angle = NW4R_MATH_PI * ((timer - (walkFrames - 36)) / 36.0f) / 2.0f;
            f32 dx;
            if (mTargetPos.x < 0.0f) {
                dx = -55.0f * cosf(angle);
            } else {
                dx = 55.0f * cosf(angle);
            }
            mPos.x += dx;
            mPos.z += 55.0f * (1.0f - sinf(angle));
        }
    } else {
        f32 dz = RPInterpolate(600.0f, 0.0f, timer, walkFrames, 1);
        mPos.z += dz;
        mTargetRotY = NW4R_MATH_PI;
        mRotY = NW4R_MATH_PI;
    }

    GetBodyModel()->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0,
                                         mParam.mTimer % mParam.mAnimCycle);
}

// Turns rCur one step (2 degrees) towards target unless it is already within ~2.5 degrees
static inline f64 PnpTurnTowards(f32& rCur, f32 target) {
    f32 cur = rCur;
    f32 rot = target;
    f32 plus = 0.034906585f + cur;
    f32 minus = cur - 0.034906585f;

    nw4r::math::VEC3 plusDir;
    nw4r::math::VEC3 minusDir;
    nw4r::math::VEC3 targetDir;
    f32 s = sinf(plus);
    plusDir.x = cosf(plus);
    plusDir.y = 0.0f;
    plusDir.z = s;
    s = sinf(minus);
    minusDir.x = cosf(minus);
    minusDir.y = 0.0f;
    minusDir.z = s;
    s = sinf(rot);
    targetDir.x = cosf(rot);
    targetDir.y = 0.0f;
    targetDir.z = s;

    if (nw4r::math::VEC3Dot(&plusDir, &targetDir) > nw4r::math::VEC3Dot(&minusDir, &targetDir)) {
        if (nw4r::math::VEC3Dot(&plusDir, &targetDir) > 0.999f) {
            rot = rot;
        } else {
            rot = plus;
        }
    } else {
        if (nw4r::math::VEC3Dot(&minusDir, &targetDir) > 0.999f) {
            rot = rot;
        } else {
            rot = minus;
        }
    }
    rCur = rot;
    return rot;
}

void RPPnpSpectator::Calculate() {
    if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
        if (RPPnpGame::GetInstance()->IsUnk18() &&
            RPPnpGame::GetInstance()->GetBall()->GetHitCount() / 2 >= mParam.mMinRally) {
            if (mParam.mTimer < mParam.mWalkFrames) {
                UpdateWalkIn();
                mParam.mTimer++;
            } else {
                mPos = mTargetPos;
            }
        } else {
            return;
        }
    }

    GetNigaoeModel()->SetBaseScale(mScale);
    GetBodyModel()->SetBaseScale(mScale);

    EGG::Matrix34f mtx;
    EGG::Matrix34f headMtx;

    f64 rot = PnpTurnTowards(mRotY, mTargetRotY);
    f32 rotY = rot;
    mtx.makeRT(EGG::Vector3f(0.0f, rotY, 0.0f), EGG::Vector3f(mPos.x, mPos.y, mPos.z));

    if (mIsSecond) {
        mtx(0, 0) *= -1.0f;
        mtx(1, 0) *= -1.0f;
        mtx(2, 0) *= -1.0f;
        mtx(0, 1) *= -1.0f;
        mtx(1, 1) *= -1.0f;
        mtx(2, 1) *= -1.0f;
    }

    GetBodyModel()->GetModelEx()->setLocalMtx(&mtx);
    GetBodyModel()->Calc();

    GetHeadJointMtx(&headMtx);
    GetNigaoeModel()->GetModelEx()->setLocalMtx(&headMtx);
    GetNigaoeModel()->Calc();

    if (mVisible) {
        GetNigaoeModel()->Entry();
        GetBodyModel()->Entry();
    }
}

void RPPnpSpectator::Draw() {}
