// Fishing camera (RPFshCamera): default, catch and result views, the intro move.
// Unit 0x8020B0F0-0x8020BDFC (dtk/game_units.tsv; its last function, Fsh_isInPond, is
// RPFshStage::IsInPond and belongs to RPFshStage.cpp). Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshCamera.h>

#include <Pack/RPGraphics.h>

#include <egg/core.h>

EGG::Vector3f RPFshCamera::sDefaultEye(0.0f, 210.0f, 430.0f);
EGG::Vector3f RPFshCamera::sDefaultTarget(0.0f, 10.0f, 0.0f);
EGG::Vector3f RPFshCamera::sCatchEye(0.0f, 280.0f, 430.0f);
EGG::Vector3f RPFshCamera::sCatchTarget(0.0f, 280.0f, 0.0f);
EGG::Vector3f RPFshCamera::sResultEye(0.0f, 280.0f, 430.0f);
EGG::Vector3f RPFshCamera::sResultTarget(0.0f, 280.0f, 0.0f);
EGG::Vector3f RPFshCamera::sIntroEye(sDefaultEye.x, 150.0f + sDefaultEye.y,
                                     130.0f + sDefaultEye.z);
EGG::Vector3f RPFshCamera::sIntroTarget = sDefaultTarget;
EGG::Vector3f RPFshCamera::sIntroEndEye = sDefaultEye;
EGG::Vector3f RPFshCamera::sIntroEndTarget = sDefaultTarget;

f32 RPFshCamera::sFovy = 37.5f;
s32 RPFshCamera::sCatchFrames = 20;
s32 RPFshCamera::sDefaultFrames = 40;
s32 RPFshCamera::sResultFrames = 120;
s32 RPFshCamera::sIntroFrames = 180;

RPFshCamera::~RPFshCamera() {}

RPFshCamera::RPFshCamera() {}

void RPFshCamera::Reset() {
    RPInterpCamera::Reset();
    mIsIntroMoving = false;
    SetDefault();

    RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();
    pRenderer->GetView(0)->GetScreen()->SetFovy(sFovy);
    pRenderer->GetView(1)->GetScreen()->SetFovy(sFovy);
    pRenderer->GetView(3)->GetScreen()->SetFovy(sFovy);

    CalcMatrix();
}

void RPFshCamera::Calculate() {
    if (mIsIntroMoving) {
        CalcIntroMove();
        CalcMatrix();
    } else {
        EGG::CoreControllerMgr::instance()->func_800A6F50(0);

        RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();
        pRenderer->GetView(0)->GetScreen()->SetFovy(sFovy);
        pRenderer->GetView(1)->GetScreen()->SetFovy(sFovy);
        pRenderer->GetView(3)->GetScreen()->SetFovy(sFovy);

        RPInterpCamera::Calculate();
    }
}

void RPFshCamera::CalcIntroMove() {
    f32 t = EGG::Math<f32>::sqrt(mpEase->GetValue());

    EGG::Vector3f pos = sIntroEye * (1.0f - t) + sIntroEndEye * t;
    EGG::Vector3f startDir = sIntroTarget - sIntroEye;
    f32 startDist = startDir.length();
    EGG::Vector3f endDir = sIntroEndTarget - sIntroEndEye;
    f32 endDist = endDir.length();
    f32 dist = startDist * (1.0f - t) + endDist * t;
    EGG::Vector3f target = sIntroTarget * (1.0f - t) + sIntroEndTarget * t;

    EGG::Vector3f dir = pos - target;
    dir.multScalar(dist / dir.length());

    SetTarget(target);
    SetPosition(target + dir);
    SetUp(CalcUpVector(EGG::Vector3f(GetPosition().x, GetPosition().y, GetPosition().z),
                       EGG::Vector3f(GetTargetRef().x, GetTargetRef().y, GetTargetRef().z)));

    if (mpEase->IsEnd()) {
        mIsIntroMoving = false;
    } else {
        mpEase->Step();
    }
}

void RPFshCamera::StartIntroMove() {
    if (!mIsIntroMoving) {
        mIsIntroMoving = true;
        mIntroFrame = 0;
        mpEase->Start(1, sIntroFrames);
    }
}

void RPFshCamera::MoveToCatchView() {
    Start(sCatchFrames, sCatchEye, sCatchTarget, CalcUpVector(sCatchEye, sCatchTarget), 1);
}

void RPFshCamera::MoveToDefault() {
    Start(sDefaultFrames, sDefaultEye, sDefaultTarget, CalcUpVector(sDefaultEye, sDefaultTarget),
          1);
}

void RPFshCamera::MoveToResultView() {
    Start(sResultFrames, sResultEye, sResultTarget, CalcUpVector(sResultEye, sResultTarget), 1);
}

void RPFshCamera::SetDefault() {
    SetPosition(sDefaultEye);
    SetTarget(sDefaultTarget);
    SetUp(CalcUpVector(EGG::Vector3f(GetPosition().x, GetPosition().y, GetPosition().z),
                       EGG::Vector3f(GetTargetRef().x, GetTargetRef().y, GetTargetRef().z)));

    mActive = false;
}

bool RPFshCamera::IsOffPreset() const {
    if ((GetPosition() == sCatchEye && GetTargetRef() == sCatchTarget) ||
        (GetPosition() == sResultEye && GetTargetRef() == sResultTarget)) {
        return false;
    }

    return true;
}
