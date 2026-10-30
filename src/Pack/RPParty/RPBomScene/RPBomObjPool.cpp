#include "Pack/RPParty/RPBomScene/RPBomObjPool.h"

#include "Pack/RPAudio/RPSndAudioMgr.h"
#include "Pack/RPAudio/RPSndObject.h"

#include "Pack/RPParty/RPDucScene/RPDucUtil.h"
#include "Pack/RPParty/RPBomScene/RPBomSound.h"

void RPBomPoolObj::Miss() {}

bool RPBomPoolObj::IsFalling() {
    return false;
}

s32 RPBomPoolObj::GetPoseIndex() {
    return 0;
}

s32 RPBomPoolObj::GetColorType() {
    return 0;
}

RPBomPoolObj::RPBomPoolObj() {
    mPosDelta.x = 0.0f;
    mPosDelta.y = 0.0f;
    mPosDelta.z = 0.0f;
    mPosDelta2.x = 0.0f;
    mPosDelta2.y = 0.0f;
    mPosDelta2.z = 0.0f;
    mVelDelta.x = 0.0f;
    mVelDelta.y = 0.0f;
    mVelDelta.z = 0.0f;
    mVelDelta2.x = 0.0f;
    mVelDelta2.y = 0.0f;
    mVelDelta2.z = 0.0f;
    mVel = RP_vecZero;
    mPosDelta.x = 0.0f;
    mPosDelta.y = 0.0f;
    mPosDelta.z = 0.0f;
    mPosDelta2.x = 0.0f;
    mPosDelta2.y = 0.0f;
    mPosDelta2.z = 0.0f;
    mVelDelta.x = 0.0f;
    mVelDelta.y = 0.0f;
    mVelDelta.z = 0.0f;
    mVelDelta2.x = 0.0f;
    mVelDelta2.y = 0.0f;
    mVelDelta2.z = 0.0f;
    unk84 = RP_vecZero;
    unk90 = 0.0f;
    mAge = 0;
    mIndex = 0;
    mpPool = NULL;
}

RPBomPoolObj::~RPBomPoolObj() {}

void RPBomPoolObj::ResetState() {
    mPos = RP_vecZero;
    mScale = RP_vecOne;
    mRot = RP_mtx33Identity;
    mVel = RP_vecZero;
    mPosDelta.x = 0.0f;
    mPosDelta.y = 0.0f;
    mPosDelta.z = 0.0f;
    mPosDelta2.x = 0.0f;
    mPosDelta2.y = 0.0f;
    mPosDelta2.z = 0.0f;
    mVelDelta.x = 0.0f;
    mVelDelta.y = 0.0f;
    mVelDelta.z = 0.0f;
    mVelDelta2.x = 0.0f;
    mVelDelta2.y = 0.0f;
    mVelDelta2.z = 0.0f;
    unk84 = RP_vecZero;
    unk90 = 1.0f;
    mAge = -1;
    SetUnk40(0);
}

void RPBomPoolObj::ClearForces() {
    mPosDelta.x = 0.0f;
    mPosDelta.y = 0.0f;
    mPosDelta.z = 0.0f;
    mPosDelta2.x = 0.0f;
    mPosDelta2.y = 0.0f;
    mPosDelta2.z = 0.0f;
    mVelDelta.x = 0.0f;
    mVelDelta.y = 0.0f;
    mVelDelta.z = 0.0f;
    mVelDelta2.x = 0.0f;
    mVelDelta2.y = 0.0f;
    mVelDelta2.z = 0.0f;
}

void RPBomPoolObj::Integrate() {
    mPos.x += mPosDelta.x + mPosDelta2.x;
    mPos.y += mPosDelta.y + mPosDelta2.y;
    mPos.z += mPosDelta.z + mPosDelta2.z;
    mVel.x += mVelDelta.x + mVelDelta2.x;
    mVel.y += mVelDelta.y + mVelDelta2.y;
    mVel.z += mVelDelta.z + mVelDelta2.z;
}

void RPBomPoolObj::InitFields() {}

void RPBomPoolObj::Setup(u32) {}

void RPBomPoolObj::Create() {
    mpSound = new RPBomSoundActorBase(RPSndAudioMgr::instance()->getPlayer());
}

void RPBomPoolObj::Load() {}

void RPBomPoolObj::Reset() {
    ResetState();
}

void RPBomPoolObj::Move() {
    mPos.x += mVel.x;
    mPos.y += mVel.y;
    mPos.z += mVel.z;
}

void RPBomPoolObj::PreCalc() {
    ClearForces();
}

void RPBomPoolObj::CalcHit() {}

void RPBomPoolObj::PostCalc() {
    Integrate();
}

void RPBomPoolObj::Calculate() {}

void RPBomPoolObj::Activate(const nw4r::math::VEC3& rPos, u32 flags) {
    ResetState();
    mPos = rPos;
    InitFields();
    Setup(flags);
    mAge = 0;
    SetUnk40(1);
}

void RPBomPoolObj::Deactivate() {
    mAge = -999;
    if (mpPool != NULL) {
        --mpPool->mActiveNum;
    }
    SetUnk40(0);
}

void RPBomPoolObj::VF_0x84() {}

void RPBomPoolObj::Spawn(const nw4r::math::VEC3&, u32, RPBomSpawnInfo) {}

void RPBomPoolObj::VF_0x94() {}
