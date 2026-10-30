// Billiards break lottery (RPBilBreakLottery, singleton RPBilBreakLottery::sInstance): spins the cue ball
// carrying the players' Mii faces to choose who breaks.
// Unit 0x802CC57C-0x802CCC18. Functions in reverse address order.
#include <Pack/RPParty/RPBilScene/RPBilBreakLottery.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPBilScene/RPBilBall.h>
#include <Pack/RPParty/RPBilScene/RPBilBallMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPPhysSphere.h>
#include <Pack/RPUtility/RPUtlRandom.h>

#include <egg/math.h>

s32 RPBilBreakLottery::sSpinFrames = 240;
f32 RPBilBreakLottery::sSpinDecay = EGG::Math<f32>::pi() * 0.001f;

RPBilBreakLottery* RPBilBreakLottery::sInstance;

// RPMtx33MultVec (the ledger name is used by linked RPFshBonusFish)
extern "C" EGG::Vector3f RP_mtx33MultVec(const EGG::Matrix34f&, const EGG::Vector3f&);

namespace {

} // namespace

inline RPBilBreakLottery::RPBilBreakLottery() {
    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));
}

void RPBilBreakLottery::CreateInstance() {
    sInstance = new RPBilBreakLottery();
}

void RPBilBreakLottery::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilBreakLottery::~RPBilBreakLottery() {}

void RPBilBreakLottery::Load() {}

void RPBilBreakLottery::Reset() {
    mState = STATE_NONE;
    mNextState = STATE_NONE;
    mIsChanging = false;
    mTimer = 0;
    mWinner = 0;
}

void RPBilBreakLottery::Exit() {}

void RPBilBreakLottery::Calculate() {
    RPBilStateFuncs<RPBilBreakLottery> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPBilBreakLottery::WaitEnter, &RPBilBreakLottery::WaitCalc},
        {&RPBilBreakLottery::SpinEnter, &RPBilBreakLottery::SpinCalc},
        {&RPBilBreakLottery::DoneEnter, &RPBilBreakLottery::DoneCalc},
        {NULL, NULL},
    };

    if (mIsChanging) {
        mState = mNextState;
        mIsChanging = false;
        mTimer = 0;

        RPBilStateFuncs<RPBilBreakLottery>::Func enter = funcs[mState].mEnter;
        if (enter) {
            (this->*enter)();
        }
    }

    RPBilStateFuncs<RPBilBreakLottery>::Func calc = funcs[mState].mCalc;
    if (calc) {
        (this->*calc)();
    }
}

void RPBilBreakLottery::WaitEnter() {}

void RPBilBreakLottery::WaitCalc() {
    if (mPlayerNum == 1) {
        mWinner = 0;
        mNextState = STATE_END;
        mIsChanging = true;
        return;
    }

    if (RPBilBallMgr::sInstance->GetBall(0)->GetState() == RPBilBall::STATE_ON_TABLE) {
        mNextState = STATE_SPIN;
        mIsChanging = true;
    }
}

void RPBilBreakLottery::SpinEnter() {
    mWinner = RPUtlRandom::getU32(mPlayerNum);

    RPBilBall* pBall = RPBilBallMgr::sInstance->GetBall(0);
    if (mWinner == 1) {
        mAngle = 3.1415927f;
    } else {
        mAngle = 0.0f;
    }

    for (s32 i = 0; i < sSpinFrames; i++) {
        mAngle += sSpinDecay * (i + 1);
    }

    const nw4r::math::VEC3& rAxisY = pBall->GetAxisY();
    mBaseAxisY.x = rAxisY.x;
    mBaseAxisY.y = rAxisY.y;
    mBaseAxisY.z = rAxisY.z;
}

void RPBilBreakLottery::SpinCalc() {
    mAngle -= sSpinDecay * (sSpinFrames - mTimer);

    RPBilBall* pBall = RPBilBallMgr::sInstance->GetBall(0);

    EGG::Matrix34f mtx;
    mtx.setAxisRotation(static_cast<const EGG::Vector3f&>(pBall->GetAxisZ()), mAngle);
    EGG::Vector3f axisY = RP_mtx33MultVec(mtx, mBaseAxisY);
    EGG::Vector3f axisX = axisY.cross(static_cast<const EGG::Vector3f&>(pBall->GetAxisZ()));
    pBall->SetAxisY(axisY);
    pBall->SetAxisX(axisX);
    pBall->GetPhysSphere()->ResetMatrices();

    if (mTimer >= sSpinFrames) {
        RPBilBallMgr::sInstance->SetCueBallFace(mWinner, -1);
        mNextState = STATE_END;
        mIsChanging = true;
        RPSndAudioMgr::instance()->startSystemSe(5);
    }

    pBall->PlayRollSound(sSpinDecay * (sSpinFrames - mTimer) *
                         pBall->GetPhysSphere()->GetRadius());
    mTimer++;
}

void RPBilBreakLottery::DoneEnter() {}

void RPBilBreakLottery::DoneCalc() {}

void RPBilBreakLottery::Start(s32 playerNum) {
    mPlayerNum = playerNum;
    mNextState = STATE_WAIT;
    mIsChanging = true;

    RPBilBallMgr* pBallMgr = RPBilBallMgr::sInstance;
    if (playerNum == 1) {
        pBallMgr->SetCueBallFace(0, -1);
    } else {
        pBallMgr->SetCueBallFace(0, 0);
        pBallMgr->SetCueBallFace(1, 1);
    }
}

void RPBilBreakLottery::UserDraw() {}
