// Billiards ball manager (RPBilBallMgr, singleton RPBilBallMgr::sInstance): the ten balls, their aim
// markers and pocketed-ball icons, the cue ghost, the Mii faces of the cue ball and the
// physics step (table and ball collisions).
// One source file for the unit ranges RPBilBallMgr (0x802C2CA4), RPBilBallMgr_2/_3 (weak
// RPBilContactResult/RPBilContactInput copies) and RPBilBallMgr_4 (to 0x802C52F0).
// The RPBilBallMgr range is defined in reverse address order; `#pragma defer_codegen off`
// then flushes it (with the weak contact copies) and the RPBilBallMgr_4 range follows in
// address order, compiled as parsed (this puts the ctor's GXColor initializer after the
// literal pool in .sdata2, as in the original).
#include <Pack/RPParty/RPBilScene/RPBilBallMgr.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPBilScene/RPBilBall.h>
#include <Pack/RPParty/RPBilScene/RPBilBallIcon.h>
#include <Pack/RPParty/RPBilScene/RPBilGameMgr.h>
#include <Pack/RPParty/RPBilScene/RPBilMarker.h>
#include <Pack/RPParty/RPBilScene/RPBilPocketCamWindow.h>
#include <Pack/RPParty/RPBilScene/RPBilSound.h>
#include <Pack/RPParty/RPBilScene/RPBilTable.h>
#include <Pack/RPParty/RPPartyCommon/RPKcl.h>
#include <Pack/RPParty/RPPartyCommon/RPPhysSphere.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <egg/core.h>
#include <egg/math.h>

// Cushion response (RPBilBallMgr::CalcTableCollisions)
f32 RPBilBallMgr::sCushionParam0 = 2.0f;
f32 RPBilBallMgr::sCushionParam1 = 0.45f;
f32 RPBilBallMgr::sCushionParam2 = 0.6f;
f32 RPBilBallMgr::sCushionParam3 = 0.004f;
f32 RPBilBallMgr::sCushionParam4 = 0.001f;
f32 RPBilBallMgr::sCushionParam5 = 0.04f;
f32 RPBilBallMgr::sCushionParam6 = 0.5f;
f32 RPBilBallMgr::sCushionParam7 = 0.5f;
f32 RPBilBallMgr::sCushionParam8 = 0.6f;
f32 RPBilBallMgr::sCushionParam9 = 0.8f;
f32 RPBilBallMgr::sCushionParam10 = 0.2f;
f32 RPBilBallMgr::sCushionParam11 = 0.1f;
f32 RPBilBallMgr::sCushionParam12 = 0.1f;
f32 RPBilBallMgr::sCushionParam13 = 0.4f;
// Ball-ball response (RPBilBallMgr::CalcBallCollisions)
f32 RPBilBallMgr::sBallCollisionParamA = 0.99f;
f32 RPBilBallMgr::sBallCollisionParamB = 0.007f;
f32 RPBilBallMgr::sCushionParam14 = 0.8f;

RPBilBallMgr* RPBilBallMgr::sInstance;

const s32 RPBilBallMgr::sBallNum[RPBilBallMgr::RACK_MODE_NUM] = {7, 10};

namespace {

// Position, velocity and angular velocity change from a collision
struct SphereImpulse {
    EGG::Vector3f mPos;    // at 0x0
    EGG::Vector3f mVel;    // at 0xC
    EGG::Vector3f mAngVel; // at 0x18
};

struct BallHitInfo {
    EGG::Vector3f unk0; // at 0x0
    f32 unkC;           // at 0xC
    f32 mSpeed;         // at 0x10
};

struct BallCollideParam {
    f32 mParamA;             // at 0x0
    f32 mParamB;             // at 0x4
    RPPhysSphere* mpOther;   // at 0x8
};

inline f32 GetSphereRadius(const RPBilBall* pBall) {
    return pBall->mpPhysSphere->mParam.radius;
}

} // namespace

// Out of line in the framework (ledger names)
extern "C" BOOL RP_PhysSphere_collideB(RPPhysSphere* pSphere, const BallCollideParam* pParam,
                                       SphereImpulse* pImpulse, SphereImpulse* pOtherImpulse,
                                       BallHitInfo* pInfo);
extern "C" s32 RP_PhysSphere_checkContact(RPPhysSphere* pSphere, const nw4r::math::VEC3* pPoint,
                                          EGG::Vector3f* pVel);
extern "C" void RP_PhysSphere_collideA(RPPhysSphere* pSphere, s32 num,
                                       const RPBilContactInput* pInputs,
                                       SphereImpulse* pImpulse, RPBilContactResult* pResults,
                                       s32 arg);

inline bool RPBilBallMgr::IsPocketedThisShot(s32 ball) const {
    bool pocketed;
    if (mpBalls[ball]->mpStateMachine->mState == RPBilBall::STATE_POCKETED &&
        mShotNo == mpBalls[ball]->mPocketedShot) {
        pocketed = true;
    } else {
        pocketed = false;
    }
    return pocketed;
}

inline bool RPBilBallMgr::IsOutThisShot(s32 ball) const {
    bool out;
    if (mpBalls[ball]->mpStateMachine->mState == RPBilBall::STATE_OUT &&
        mShotNo == mpBalls[ball]->mOutShot) {
        out = true;
    } else {
        out = false;
    }
    return out;
}

// NONMATCHING-DEBT: CalcBallCollisions' reset loop matches only when each range pointer passes
// through a cast inside an inline (the conversion node is coloured before the sphere pointer under
// optimization_level 3). Tried without a match: direct pSphere->mpContactX->Reset(), reference
// locals, a plain inline taking RPVec3Range*, an RPPhysSphere::ResetContacts member, a const
// sphere pointer, a direct static_cast. Same-type cast kept here instead of the old overlay.
inline RPVec3Range& GetRange(RPVec3Range* pRange) {
    return *reinterpret_cast<RPVec3Range*>(pRange);
}

inline void AddImpulse(RPPhysSphere* pSphere, const SphereImpulse& rImpulse) {
    pSphere->GetPosRange().Extend(rImpulse.mPos);
    pSphere->GetVelRange().Extend(rImpulse.mVel);
    pSphere->GetAngVelRange().Extend(rImpulse.mAngVel);
}

void RPBilBallMgr::CalcTableCollisions() {
    RPKcl* pKcl = RPBilTable::sInstance->GetKcl();
    RPPhysSphere* pSphere;
    s32 hitNum;

    for (int i = 0; i < mBallNum; i++) {
        if (!mpBalls[i]->mpStateMachine->IsStateSettled(RPBilBall::STATE_ON_TABLE)) {
            continue;
        }

        mpBalls[i]->unk71 = false;
        bool pocketed = false;
        bool out = false;
        s32 foul = -1;
        pSphere = mpBalls[i]->mpPhysSphere;
        f32 radius = pSphere->mParam.radius;

        RPKclHit hits[0x40];
        hitNum = pKcl->SearchSphereOctree(hits, 0x40, pSphere->GetPos(), radius);
        if (hitNum > 0x40) {
            hitNum = 0x40;
        }

        RPBilContactInput inputs[0x40];
        for (int j = 0; j < hitNum; j++) {
            RPBilContactInput& rInput = inputs[j];
            RPKclHit& rHit = hits[j];

            EGG::Vector3f contactVel;
            EGG::Vector3f normal(rHit.normal.x, rHit.normal.y, rHit.normal.z);
            rInput.mPoint = normal;
            rInput.unk18 = rHit.distance;
            rInput.unk1C = 0.0001f;

            f32 speed = -static_cast<const EGG::Vector3f&>(pSphere->GetVec1C())
                             .dot(static_cast<EGG::Vector3f&>(rInput.mPoint));
            if (speed > sCushionParam0) {
                speed = sCushionParam0;
            }
            if (speed < 0.0f) {
                speed = 0.0f;
            }
            f32 rate = (sCushionParam0 - speed) / sCushionParam0;

            switch (rHit.attribute) {
            case 1: {
                rInput.unk0.x = rate * (sCushionParam2 - sCushionParam1) + sCushionParam1;
                if (RP_PhysSphere_checkContact(pSphere, &rInput.mPoint, &contactVel) == 1) {
                    f32 speed2 = -static_cast<const EGG::Vector3f&>(pSphere->GetVec1C())
                                      .dot(static_cast<EGG::Vector3f&>(rInput.mPoint));
                    if (speed2 > sCushionParam6) {
                        speed2 = sCushionParam6;
                    }
                    if (speed2 < 0.0f) {
                        speed2 = 0.0f;
                    }
                    rInput.unk0.z = sCushionParam4 + (sCushionParam6 - speed2) *
                                                            (sCushionParam5 - sCushionParam4) /
                                                            sCushionParam6;
                    f32 slide = contactVel.length();
                    if (slide > sCushionParam7) {
                        slide = sCushionParam7;
                    }
                    if (slide < 0.0f) {
                        slide = 0.0f;
                    }
                    rInput.unk0.z = (rInput.unk0.z +
                                     (sCushionParam4 + slide * (sCushionParam5 - sCushionParam4) /
                                                              sCushionParam7)) /
                                    2.0f;
                    rInput.unk0.y = rInput.unk0.z / sCushionParam14;
                } else {
                    rInput.unk0.y = sCushionParam3;
                    rInput.unk0.z = sCushionParam3 * sCushionParam14;
                }
                mpBalls[i]->unk71 = true;
                break;
            }
            case 2:
                rInput.unk0.x = rate * (sCushionParam9 - sCushionParam8) + sCushionParam8;
                rInput.unk0.y = sCushionParam10;
                rInput.unk0.z = sCushionParam10 * sCushionParam14;
                break;
            case 3:
                rInput.unk0.x = rate * (sCushionParam12 - sCushionParam11) + sCushionParam11;
                rInput.unk0.y = sCushionParam13;
                rInput.unk0.z = sCushionParam13 * sCushionParam14;
                break;
            case 4:
                rInput.unk0.x = 0.0f;
                pocketed = true;
                rInput.unk0.y = 0.0f;
                rInput.unk0.z = 0.0f;
                break;
            default:
                rInput.unk0.x = 0.0f;
                rInput.unk0.y = 0.0f;
                rInput.unk0.z = 0.0f;
                break;
            case 6:
                rInput.unk0.x = 0.4f;
                out = true;
                rInput.unk0.y = 0.0f;
                rInput.unk0.z = 0.0f;
                break;
            }
        }

        RPBilContactResult results[0x40];
        if (hitNum > 0) {
            SphereImpulse impulse;
            RP_PhysSphere_collideA(pSphere, hitNum, inputs, &impulse, results, 1);
            pSphere->SetPos(static_cast<const EGG::Vector3f&>(pSphere->GetPos()) + impulse.mPos);
            pSphere->SetVec1C(static_cast<const EGG::Vector3f&>(pSphere->GetVec1C()) +
                              impulse.mVel);
            static_cast<EGG::Vector3f&>(pSphere->mAngVel) =
                static_cast<EGG::Vector3f&>(pSphere->mAngVel) + impulse.mAngVel;
        }

        if (pocketed) {
            mpBalls[i]->SetPocketed(mShotNo);
            if (i == 0) {
                foul = 1;
            } else if (unk70 == -1) {
                s32 num = CountPocketed() + 1;
                if (num <= 1) {
                    mpSoundActor->StartPocketSound(0x26);
                } else if (num <= 2) {
                    mpSoundActor->StartPocketSound(0x27);
                } else {
                    mpSoundActor->StartPocketSound(0x28);
                }
                RPBilMusic::sInstance->GetState()->SetState(4);
            }
        }

        if (out) {
            mpBalls[i]->mHitFlags++;
        }

        if (mpBalls[i]->GetPos().y < -85.0f || mpBalls[i]->mHitFlags >= 3) {
            mpBalls[i]->SetOut(mShotNo);
            foul = 0;
        }

        if (mpBalls[i]->mpStateMachine->IsStateSettled(RPBilBall::STATE_ON_TABLE) &&
            mpBalls[i]->mpPhysSphere->IsAtRest(true)) {
            RPBilBall* pBall = mpBalls[i];
            f32 ballRadius = pBall->mpPhysSphere->mParam.radius;
            if (pBall->GetPos().y - ballRadius > 0.5f * RPBilTable::GetCushionParam()) {
                mpBalls[i]->SetOut(mShotNo);
                foul = 0;
            }
        }

        if (foul != -1 && unk70 == -1) {
            unk70 = foul;
        }

        if (foul != -1 && !unk74) {
            RPSndAudioMgr::instance()->startSystemSe(0x29);
            RPBilMusic::sInstance->GetState()->SetState(3);
            unk74 = true;
        }

        for (int j = 0; j < hitNum; j++) {
            if (results[j].mHit) {
                mpBalls[i]->PlayKclHitSound(hits[j].attribute, results[j].mSpeed);
                if (hits[j].attribute == 1) {
                    mpBalls[i]->UpdateRollingSound();
                }
            }
        }
    }
}

bool RPBilBallMgr::AreAllBallsStopped() const {
    if (unk78 == 0) {
        return true;
    }

    for (int i = 0; i < mBallNum; i++) {
        if (mpBalls[i]->mpStateMachine->IsChanging()) {
            return false;
        }
        if (mpBalls[i]->mpStateMachine->GetState() == RPBilBall::STATE_ON_TABLE &&
            (!mpBalls[i]->unk71 || !mpBalls[i]->mpPhysSphere->IsAtRest(true))) {
            return false;
        }
    }

    return true;
}

bool RPBilBallMgr::HasObjectBallsOnTable() const {
    for (int i = 0; i < mBallNum; i++) {
        if (i != 0 && mpBalls[i]->mpStateMachine->IsStateSettled(RPBilBall::STATE_ON_TABLE)) {
            return true;
        }
    }
    return false;
}

bool RPBilBallMgr::RaycastBalls(const RPBilSegment& rSeg, s32* pBall, EGG::Vector3f* pHitPos,
                                f32 radius) {
    bool hit = false;
    f32 minDist;

    for (int i = 0; i < mBallNum; i++) {
        s32 state = mpBalls[i]->mpStateMachine->GetState();
        if (state == RPBilBall::STATE_NONE || state == RPBilBall::STATE_POCKETED ||
            state == RPBilBall::STATE_OUT) {
            continue;
        }

        EGG::Vector3f hitPos;
        f32 dist;
        if (mpBalls[i]->IntersectRay(rSeg, &hitPos, &dist, radius) && (!hit || dist < minDist)) {
            hit = true;
            *pHitPos = hitPos;
            *pBall = i;
            minDist = dist;
        }
    }

    return hit;
}

bool RPBilBallMgr::RaycastObjectBalls(const EGG::Vector3f& rTarget, s32* pBall,
                                      EGG::Vector3f* pHitPos) const {
    RPBilSegment seg;
    seg.mStart = rTarget;
    seg.mEnd = static_cast<const EGG::Vector3f&>(mpBalls[0]->GetPos());

    bool hit = false;
    f32 minDist;

    for (int i = 0; i < mBallNum; i++) {
        s32 state = mpBalls[i]->mpStateMachine->GetState();
        if (state == RPBilBall::STATE_NONE || state == RPBilBall::STATE_POCKETED || i == 0) {
            continue;
        }

        EGG::Vector3f hitPos;
        f32 dist;
        if (mpBalls[i]->IntersectRay(seg, &hitPos, &dist, mpBalls[0]->mpPhysSphere->mParam.radius) &&
            (!hit || dist < minDist)) {
            hit = true;
            *pHitPos = hitPos;
            *pBall = i;
            minDist = dist;
        }
    }

    return hit;
}

bool RPBilBallMgr::IsBallPocketed(s32 ball) const {
    if (mpBalls[ball]->mpStateMachine->GetState() == RPBilBall::STATE_POCKETED &&
        mShotNo == mpBalls[ball]->mPocketedShot) {
        return true;
    }
    return false;
}

s32 RPBilBallMgr::CountPocketed() const {
    s32 num = 0;
    for (int i = 0; i < mBallNum; i++) {
        if (i != 0 && IsBallPocketed(i)) {
            num++;
        }
    }
    return num;
}

s32 RPBilBallMgr::SumPocketedNumbers() const {
    s32 sum = 0;
    for (int i = 0; i < mBallNum; i++) {
        if (i != 0 && IsBallPocketed(i)) {
            sum += i;
        }
    }
    return sum;
}

void RPBilBallMgr::EndShot() {
    bool moved = false;
    for (int i = 0; i < mBallNum; i++) {
        if (IsPocketedThisShot(i) || IsOutThisShot(i)) {
            moved = true;
            break;
        }
    }
    if (moved) {
        mShotNo++;
    }

    mFirstHitBall = -1;
    unk70 = -1;
    unk74 = false;

    for (int i = 0; i < mBallNum; i++) {
        mpBalls[i]->mHitFlags = 0;
    }

    RPBilPocketCamWindow::sInstance->ClampTimers();
}

s32 RPBilBallMgr::GetLowestBall() const {
    for (int i = 0; i < mBallNum; i++) {
        if (i != 0 && (mpBalls[i]->mpStateMachine->IsStateOrNext(RPBilBall::STATE_ON_TABLE) ||
                       IsPocketedThisShot(i) || IsOutThisShot(i))) {
            return i;
        }
    }
    return -1;
}

void RPBilBallMgr::ShowAimMarker(const nw4r::math::VEC3& rPos) {
    mpAimMarkers[0]->SetVisible(true);
    mpAimMarkers[0]->SetPos(rPos);
}

void RPBilBallMgr::HideAimMarkers() {
    for (int i = 0; i < BALL_MAX; i++) {
        mpAimMarkers[i]->SetVisible(false);
    }
}

void RPBilBallMgr::ShowCueGhost(const EGG::Vector3f& rPos, bool valid) {
    mpCueGhost->mVisible = true;
    mpCueGhost->mIsValid = valid;
    EGG::Vector3f pos = rPos;
    pos.y = mpBalls[0]->mpPhysSphere->mParam.radius;
    mpCueGhost->SetPos(pos);
}

void RPBilBallMgr::HideCueGhost() {
    mpCueGhost->mVisible = false;
}

bool RPBilBallMgr::CanPlaceBall(s32 ball, const EGG::Vector3f& rPos) const {
    EGG::Vector3f pos = rPos;
    RPBilBall* pBall = mpBalls[ball];
    pos.y = GetSphereRadius(pBall);
    if (!RPBilTable::sInstance->IsInsideBallRange(GetSphereRadius(pBall), pos)) {
        return false;
    }

    for (int i = 0; i < mBallNum; i++) {
        if (i != 0 && mpBalls[i]->mpStateMachine->IsStateSettled(RPBilBall::STATE_ON_TABLE)) {
            f32 dist = (pos - static_cast<const EGG::Vector3f&>(mpBalls[i]->GetPos())).length();
            if (dist < 1.01f * (GetSphereRadius(pBall) + GetSphereRadius(mpBalls[i]))) {
                return false;
            }
        }
    }

    return true;
}

void RPBilBallMgr::PlaceCueBall(const EGG::Vector3f& rPos) {
    EGG::Vector3f pos = rPos;
    pos.y = mpBalls[0]->mpPhysSphere->mParam.radius;
    mpBalls[0]->SetPos(pos);
    mpBalls[0]->SetVec1C(EGG::Vector3f::zero);
    static_cast<EGG::Vector3f&>(mpBalls[0]->mpPhysSphere->mAngVel) = EGG::Vector3f::zero;
    EGG::Vector3f axisX(1.0f, 0.0f, 0.0f);
    mpBalls[0]->SetAxisX(axisX);
    EGG::Vector3f axisY(0.0f, 1.0f, 0.0f);
    mpBalls[0]->SetAxisY(axisY);
    EGG::Vector3f axisZ(0.0f, 0.0f, 1.0f);
    mpBalls[0]->SetAxisZ(axisZ);
    mpBalls[0]->mpPhysSphere->ResetMatrices();
    mpBalls[0]->unk70 = true;
    mpBalls[0]->mpStateMachine->ChangeState(RPBilBall::STATE_ON_TABLE);
    RPBilBall* pBall = mpBalls[0];
    pBall->mPocketedShot = -1;
    pBall->mOutShot = -1;
}

// STAND-IN: dead-stripped; puts 0.0f (lbl_80441320) ahead of PlaceCueBall's 1.0f (80441324),
// as in the original .sdata2. Without it the two are swapped.
extern "C" f32 Bil_ballMgrPoolOrderZero() {
    return 0.0f;
}

void RPBilBallMgr::ShowPocketedIcons(f32 y, u32 player) {
    s32 num = 0;
    for (int i = 0; i < mBallNum; i++) {
        if (IsBallPocketed(i)) {
            num++;
        }
    }

    if (num == 0) {
        return;
    }

    s32 slot = 0;
    for (int i = 0; i < mBallNum; i++) {
        if (IsBallPocketed(i)) {
            mpBallIcons[i]->Show(y, player, num, slot);
            slot++;
        }
    }
}

bool RPBilBallMgr::AreIconsDone() const {
    for (int i = 0; i < mBallNum; i++) {
        if (!mpBallIcons[i]->IsSettled()) {
            return false;
        }
    }
    return true;
}

void RPBilBallMgr::ResetIcons() {
    for (int i = 0; i < mBallNum; i++) {
        mpBallIcons[i]->Hide();
    }
}

bool RPBilBallMgr::AreIconsIdle() const {
    for (int i = 0; i < mBallNum; i++) {
        if (!mpBallIcons[i]->mpStateMachine->IsStateSettled(RPBilBallIcon::STATE_NONE)) {
            return false;
        }
    }
    return true;
}

void RPBilBallMgr::SetCueBallFace(s32 side, s32 player) {
    RPGrpTexture texture(mpKokeshiIcons[side]->GetResTIMG());
    if (player == -1) {
        mpBalls[0]->mpModel->ReplaceTexture("ball_face1", texture, false);
        mpBalls[0]->mpModel->ReplaceTexture("ball_face2", texture, false);
    } else if (player == 0) {
        mpBalls[0]->mpModel->ReplaceTexture("ball_face1", texture, false);
    } else {
        mpBalls[0]->mpModel->ReplaceTexture("ball_face2", texture, false);
    }
}

void RPBilBallMgr::NextRackMode() {
    if (++mRackMode == RACK_MODE_NUM) {
        mRackMode -= RACK_MODE_NUM;
    }
    mBallNum = sBallNum[mRackMode];
    Reset();
}

// The rest is compiled as parsed, in address order (the RPBilBallMgr_4 range); the flush
// above emits the weak RPBilContactInput/RPBilContactResult copies after CalcTableCollisions.
#pragma defer_codegen off

#pragma push
#pragma optimization_level 3
void RPBilBallMgr::CalcBallCollisions() {
    for (int i = 0; i < mBallNum; i++) {
        RPPhysSphere* pSphere = mpBalls[i]->GetPhysSphere();
        pSphere->GetPosRange().Reset();
        pSphere->GetVelRange().Reset();
        pSphere->GetAngVelRange().Reset();
    }

    for (int i = 0; i < mBallNum - 1; i++) {
        for (int j = i + 1; j < mBallNum; j++) {
            if (mpBalls[i]->mpStateMachine->IsStateSettled(RPBilBall::STATE_ON_TABLE) &&
                mpBalls[j]->mpStateMachine->IsStateSettled(RPBilBall::STATE_ON_TABLE)) {
                SphereImpulse impulse;
                SphereImpulse otherImpulse;
                BallHitInfo info;
                BallCollideParam param;

                RPPhysSphere* pSphere = mpBalls[i]->GetPhysSphere();
                RPPhysSphere* pOther = mpBalls[j]->GetPhysSphere();
                param.mParamA = sBallCollisionParamA;
                param.mParamB = sBallCollisionParamB;
                param.mpOther = pOther;
                if (RP_PhysSphere_collideB(pSphere, &param, &impulse, &otherImpulse, &info)) {
                    AddImpulse(pSphere, impulse);
                    AddImpulse(pOther, otherImpulse);

                    mpBalls[i]->PlayBallHitSound(info.mSpeed);

                    if (mFirstHitBall == -1) {
                        if (i == 0) {
                            mFirstHitBall = j;
                        } else if (j == 0) {
                            mFirstHitBall = i;
                        }

                        if (mFirstHitBall != -1 && mFirstHitBall != GetLowestBall() &&
                            unk70 == -1 && !unk74) {
                            unk70 = 3;
                            RPSndAudioMgr::instance()->startSystemSe(0x29);
                            RPBilMusic::sInstance->GetState()->SetState(3);
                            unk74 = true;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < mBallNum; i++) {
        RPPhysSphere* pSphere = mpBalls[i]->GetPhysSphere();
        pSphere->SetPos(static_cast<const EGG::Vector3f&>(pSphere->GetPos()) +
                        pSphere->mpContactA->GetSum());
        pSphere->SetVec1C(static_cast<const EGG::Vector3f&>(pSphere->GetVec1C()) +
                          pSphere->mpContactB->GetSum());
        static_cast<EGG::Vector3f&>(pSphere->mAngVel) =
            static_cast<EGG::Vector3f&>(pSphere->mAngVel) +
            pSphere->mpContactC->GetSum();
    }
}
#pragma pop

void RPBilBallMgr::Calculate() {
    if (unk78 > 0) {
        unk78--;
    }

    for (int i = 0; i < mBallNum; i++) {
        mpBalls[i]->PreCalc();
    }

    for (int step = 0; step < 5; step++) {
        for (int i = 0; i < mBallNum; i++) {
            mpBalls[i]->Calculate();
        }
        CalcBallCollisions();
        CalcTableCollisions();
    }

    for (int i = 0; i < mBallNum; i++) {
        mpBalls[i]->UpdateModel();
    }
    for (int i = 0; i < BALL_MAX; i++) {
        mpAimMarkers[i]->Calculate();
    }
    // RPBilCueGhost's names for slots 0x80/0x84 are swapped: this is slot 0x80
    mpCueGhost->Exit();
    for (int i = 0; i < mBallNum; i++) {
        mpBallIcons[i]->Calculate();
    }

    RPBilPocketCamWindow* pWindow = RPBilPocketCamWindow::sInstance;
    for (int i = 0; i < mBallNum; i++) {
        s32 pocket = mpBalls[i]->CheckPocket();
        if (pocket != -1) {
            pWindow->ShowPocket(pocket);
        }
    }

    EGG::CoreControllerMgr::instance()->getNthController(0);
}

void RPBilBallMgr::Exit() {
    for (int i = 0; i < BALL_MAX; i++) {
        mpBalls[i]->Exit();
    }
    for (int i = 0; i < BALL_MAX; i++) {
        mpAimMarkers[i]->Exit();
    }
    // RPBilCueGhost's names for slots 0x80/0x84 are swapped: this is slot 0x84
    mpCueGhost->Calculate();
    for (int i = 0; i < BALL_MAX; i++) {
        mpBallIcons[i]->Exit();
    }
}

void RPBilBallMgr::Reset() {
    mShotNo = 0;
    mFirstHitBall = -1;
    unk70 = -1;
    unk74 = false;

    for (int i = 0; i < BALL_MAX; i++) {
        mpBalls[i]->Reset();
    }
    for (int i = 0; i < mBallNum; i++) {
        mpBalls[i]->ResetToRack();
    }
    for (int i = 0; i < BALL_MAX; i++) {
        mpAimMarkers[i]->Reset();
    }
    mpCueGhost->Reset();
    for (int i = 0; i < BALL_MAX; i++) {
        mpBallIcons[i]->Reset();
    }

    unk78 = -1;
}

void RPBilBallMgr::Load() {
    for (int i = 0; i < BALL_MAX; i++) {
        mpBalls[i]->Load();
    }
    for (int i = 0; i < BALL_MAX; i++) {
        mpAimMarkers[i]->Load();
    }
    mpCueGhost->Load();
    for (int i = 0; i < BALL_MAX; i++) {
        mpBallIcons[i]->Load();
    }
    for (int i = 0; i < KOKESHI_ICON_NUM; i++) {
        mpKokeshiIcons[i]->LoadResource(NULL);
    }
}

void RPBilBallMgr::UserDraw() {}

RPBilBallMgr::RPBilBallMgr() {
    mRackMode = 1;
    mBallNum = sBallNum[mRackMode];

    for (int i = 0; i < BALL_MAX; i++) {
        mpBalls[i] = new RPBilBall(i);
    }

    for (int i = 0; i < BALL_MAX; i++) {
        mpAimMarkers[i] = new RPBilAimMarker(i);
    }

    mpCueGhost = new RPBilCueGhost();

    for (int i = 0; i < BALL_MAX; i++) {
        mpBallIcons[i] = new RPBilBallIcon(i);
    }

    for (int i = 0; i < KOKESHI_ICON_NUM; i++) {
        RPSysKokeshiGenInfo info;

        info.SetGenType(RPSysKokeshiManager::GenType_Player);
        info.SetPlayerNo(i);
        info.SetIconWidth(0xC0);
        info.SetIconHeight(0xC0);
        info.SetIconBGType(RFLIconBG_Direct);
        GXColor bgColor = {0xFF, 0xE5, 0xB3, 0xFF};
        info.SetIconBGColor(bgColor);
        info.SetIconMipMap(true);

        mpKokeshiIcons[i] = new RPSysKokeshiIcon(&info);
    }

    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));
    mpSoundActor = new RPBilSoundActor();
}

RPBilBallMgr::~RPBilBallMgr() {}

void RPBilBallMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

void RPBilBallMgr::CreateInstance() {
    sInstance = new RPBilBallMgr();
}

// The state machine members in original address order.
#pragma auto_inline off
template bool RPStateMachine<RPBilBall>::IsStateSettled(s32) const;
template bool RPStateMachine<RPBilBall>::IsStateOrNext(s32) const;
template bool RPStateMachine<RPBilBallIcon>::IsStateSettled(s32) const;
