// Billiards number tag over the targeted ball (RPBilBallNumberTag, billiard_07.brlyt,
// singleton RPBilBallNumberTag::sInstance).
// Unit 0x802D2D28-0x802D3940. Functions in reverse address order; the state machine
// instantiation follows at the end of the file.
#include <Pack/RPParty/RPBilScene/RPBilLyt.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPBilScene/RPBilBall.h>
#include <Pack/RPParty/RPBilScene/RPBilBallMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPPhysSphere.h>
#include <Pack/RPSystem.h>

#include <egg/gfxe.h>
#include <egg/math.h>

// EGG::Vector2f::operator-(const Vector2f&) const (weak copy in egg; linked RPWlyCursor calls it
// by this name)
extern "C" EGG::Vector2f EGG_Vector2f_operator_minus(const EGG::Vector2f*,
                                                  const EGG::Vector2f*);

RPBilBallNumberTag* RPBilBallNumberTag::sInstance;

namespace {

// EGG::Vector3f made element by element from a VEC3
struct Vec3Copy : public EGG::Vector3f {
    Vec3Copy(const nw4r::math::VEC3& rSrc) {
        set(rSrc.x, rSrc.y, rSrc.z);
    }
};

} // namespace

inline RPBilBallNumberTag::RPBilBallNumberTag() {
    mpStateMachine = new RPStateMachine<RPBilBallNumberTag>(this, STATE_NUM);

    RPBilStateFuncs<RPBilBallNumberTag> funcs[STATE_NUM] = {
        {&RPBilBallNumberTag::IdleEnter, &RPBilBallNumberTag::IdleCalc},
        {&RPBilBallNumberTag::InEnter, &RPBilBallNumberTag::InCalc},
        {&RPBilBallNumberTag::ShownEnter, &RPBilBallNumberTag::ShownCalc},
        {&RPBilBallNumberTag::OutEnter, &RPBilBallNumberTag::OutCalc},
    };
    for (int i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));
}

void RPBilBallNumberTag::CreateInstance() {
    sInstance = new RPBilBallNumberTag();
}

void RPBilBallNumberTag::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilBallNumberTag::~RPBilBallNumberTag() {}

void RPBilBallNumberTag::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "billiard.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "billiard_07.brlyt");

    const char* anmNames[ANM_NUM] = {
        "billiard_07_in_00.brlan",        "billiard_07_out_00.brlan",
        "billiard_07_number_01.brlan",    "billiard_07_number_02.brlan",
        "billiard_07_number_03.brlan",    "billiard_07_number_04.brlan",
        "billiard_07_number_05.brlan",    "billiard_07_number_06.brlan",
        "billiard_07_number_07.brlan",    "billiard_07_number_08.brlan",
        "billiard_07_number_09.brlan",
    };
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    mpNumberPane = mpLayout->findPane("N_number_00");
}

void RPBilBallNumberTag::Reset() {
    mpStateMachine->Reset(STATE_IDLE);

    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }

    mpLayout->unbindAllAnmObj();

    mShowRequest = false;
    mHideRequest = false;
    mSwitchRequest = false;
    mBall = -1;
}

void RPBilBallNumberTag::Calculate() {
    mpStateMachine->Calculate();

    if (mpStateMachine->GetState() != STATE_IDLE) {
        if (mBall != -1) {
            RPBilBall* pBall = RPBilBallMgr::sInstance->GetBall(mBall);
            RPGrpView* pView = RPGrpRenderer::GetCurrent()->GetView(4);
            RPGrpCamera* pCamera = RPGrpRenderer::GetCurrent()->GetView(4)->GetCamera();
            nw4r::math::VEC3 ballScreen;
            nw4r::math::VEC3 topScreen;
            pView->GetWorldToScreen(&ballScreen, pBall->GetPos());
            pView->GetWorldToScreen(
                &topScreen,
                static_cast<const EGG::Vector3f&>(pBall->GetPos()) +
                    Vec3Copy(pCamera->GetUp() * pBall->GetPhysSphere()->GetRadius()));

            EGG::Vector2f pos(ballScreen.x, ballScreen.y);
            EGG::Vector2f topPos(topScreen.x, topScreen.y);
            f32 dist = EGG_Vector2f_operator_minus(&pos, &topPos).length();

            pos.x += 0.7092199f * dist;
            pos.y += 0.7092199f * dist;
            pos.x *= (f32)EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD) /
                     (f32)EGG::Screen::GetSizeXMax();
            mpNumberPane->SetTranslate(pos);
        }

        mpLayout->calc();
    }
}

void RPBilBallNumberTag::IdleEnter() {}

void RPBilBallNumberTag::IdleCalc() {
    if (mShowRequest) {
        mpStateMachine->ChangeState(STATE_IN);
    }

    if (mSwitchRequest) {
        mBall = mNextBall;
        if (mpStateMachine->GetState() == STATE_IDLE) {
            mBall = mNextBall;
            mShowRequest = true;
            mHideRequest = false;
        }
        mSwitchRequest = false;
    }
}

void RPBilBallNumberTag::InEnter() {
    mShowRequest = false;
    mpLayout->unbindAllAnmObj();
    int idx = mBall + 1;
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[idx]);
    mpAnmObjs[0]->restart();
    mpAnmObjs[idx]->restart();
}

void RPBilBallNumberTag::InCalc() {
    if (mpAnmObjs[0]->isFinished()) {
        mpStateMachine->ChangeState(STATE_SHOWN);
    }
}

void RPBilBallNumberTag::ShownEnter() {}

void RPBilBallNumberTag::ShownCalc() {
    if (mHideRequest && mpStateMachine->GetTimer() > 40) {
        mpStateMachine->ChangeState(STATE_OUT);
    }
}

void RPBilBallNumberTag::OutEnter() {
    mHideRequest = false;
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
}

void RPBilBallNumberTag::OutCalc() {
    if (mpAnmObjs[1]->isFinished()) {
        mpStateMachine->ChangeState(STATE_IDLE);
    }
}

void RPBilBallNumberTag::Show(s32 ball) {
    if (mpStateMachine->GetState() == STATE_IDLE) {
        mBall = ball;
        mShowRequest = true;
        mHideRequest = false;
    } else if (mBall != ball) {
        mNextBall = ball;
        mHideRequest = true;
        mShowRequest = false;
        mSwitchRequest = true;
    }
}

void RPBilBallNumberTag::Hide() {
    mHideRequest = true;
    mShowRequest = false;
    mSwitchRequest = false;
}

void RPBilBallNumberTag::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() != 5) {
        return;
    }
    if (RPGrpRenderer::GetDrawStep() != 5) {
        return;
    }
    if (mpStateMachine->GetState() == STATE_IDLE) {
        return;
    }
    mpLayout->draw();
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPBilBallNumberTag>::RPStateMachine(RPBilBallNumberTag*, s32);
template void RPStateMachine<RPBilBallNumberTag>::SetStateFuncs(const StateFunc&,
                                                                const StateFunc&, s32);
template void RPStateMachine<RPBilBallNumberTag>::Reset(s32);
template void RPStateMachine<RPBilBallNumberTag>::ChangeState(s32);
template void RPStateMachine<RPBilBallNumberTag>::Calculate();
template void RPStateMachine<RPBilBallNumberTag>::CallEnter();
template void RPStateMachine<RPBilBallNumberTag>::CallCalc();
template void RPStateMachine<RPBilBallNumberTag>::CallExit();
