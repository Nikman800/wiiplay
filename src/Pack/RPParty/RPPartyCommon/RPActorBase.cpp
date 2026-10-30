#define RP_ACTOR_BASE_DEFINE_VF
#include <Pack/RPParty/RPPartyCommon/RPActorBase.h>

#include <Pack/RPGraphics/RPGrpModel.h>

#include <nw4r/ut.h>

RPActorBase::RPActorBase() {
    ResetAxes();
    mPos = EGG::Vector3f::zero;
    mVec1C = EGG::Vector3f::zero;
    mVec28 = EGG::Vector3f::zero;
    mScale = 1.0f;
    mpModel = NULL;
}

void RPActorBase::CalcModelMtx() {
    EGG::Matrix34f mtx;
    mtx(0, 0) = mAxisX.x;
    mtx(1, 0) = mAxisX.y;
    mtx(2, 0) = mAxisX.z;
    mtx(0, 1) = mAxisY.x;
    mtx(1, 1) = mAxisY.y;
    mtx(2, 1) = mAxisY.z;
    mtx(0, 2) = mAxisZ.x;
    mtx(1, 2) = mAxisZ.y;
    mtx(2, 2) = mAxisZ.z;
    mtx(0, 3) = mPos.x;
    mtx(1, 3) = mPos.y;
    mtx(2, 3) = mPos.z;
    if (mpModel->GetModelEx()->getScnObj() != NULL) {
        mpModel->GetModelEx()->getScnObj()->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }
    mpModel->SetBaseScale(EGG::Math<f32>::max(0.001f, mScale));
    mpModel->UpdateFrame();
    mpModel->Calc();
    mpModel->Entry();
}

void RPActorBase::ResetAxes() {
    mAxisX = EGG::Vector3f(1.0f, 0.0f, 0.0f);
    mAxisY = EGG::Vector3f(0.0f, 1.0f, 0.0f);
    mAxisZ = EGG::Vector3f(0.0f, 0.0f, 1.0f);
}
