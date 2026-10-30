#pragma auto_inline off
#include <egg/math.h>
#pragma auto_inline on

#include <Pack/RPParty/RPPartyCommon/RPInterpCamera_2.h>

#include <Pack/RPParty/RPPartyCommon/RPActorBase.h>
#include <egg/core.h>
#include <egg/gfxe.h>

#include <float.h>

RPInterpCamera::RPInterpCamera() : RPGrpCamera() {
    SetCameraType(ECameraType_LookAt);
    mpEase = new RPRange();
}

void RPInterpCamera::Reset() {
    mActive = false;
}

void RPInterpCamera::Calculate() {
    if (mActive) {
        if (!mpEase->IsEnd()) {
            mpEase->Step();
            f32 t = mpEase->GetValue();

            EGG::Vector3f pos = mFrom[1] * (1.0f - t) + mTo[1] * t;
            EGG::Vector3f x;
            EGG::Vector3f y;
            EGG::Vector3f z;
            f32 dist = mFromDist * (1.0f - t) + mToDist * t;
            RPSlerpBasis(t, &mFrom[4], &mFrom[2], &mFrom[3], mTo[4], mTo[2],
                         mTo[3], x, y, z);

            EGG::Vector3f cameraPos = pos - z * dist;
            SetPosition(cameraPos);
            SetTarget(pos);
            SetUp(y);
        } else {
            SetPosition(mTo[0]);
            SetTarget(mTo[1]);
            SetUp(mTo[2]);
            mActive = false;
        }
    }

    CalcMatrix();
}

void RPInterpCamera::Start(s32 duration, const EGG::Vector3f& rPos,
                           const EGG::Vector3f& rTarget,
                           const EGG::Vector3f& rUp, s32 easeType) {
    mFrom[0].set(GetPosition().x, GetPosition().y, GetPosition().z);
    mFrom[1].set(GetTargetRef().x, GetTargetRef().y, GetTargetRef().z);
    mFrom[2].set(GetUp().x, GetUp().y, GetUp().z);
    mFrom[3] = mFrom[1] - mFrom[0];
    mFromDist = mFrom[3].normalise();
    mFrom[4] = mFrom[2].cross(mFrom[3]);

    mTo[0].x = rPos.x;
    mTo[0].y = rPos.y;
    mTo[0].z = rPos.z;
    mTo[1].x = rTarget.x;
    mTo[1].y = rTarget.y;
    mTo[1].z = rTarget.z;
    mTo[2].x = rUp.x;
    mTo[2].y = rUp.y;
    mTo[2].z = rUp.z;
    mTo[3] = mTo[1] - mTo[0];
    mToDist = mTo[3].normalise();
    mTo[4] = mTo[2].cross(mTo[3]);

    mpEase->Start(easeType, duration);
    mActive = true;
}

EGG::Vector3f RPInterpCamera::CalcUpVector(const EGG::Vector3f& rPos,
                                           const EGG::Vector3f& rTarget) {
    EGG::Vector3f dir = rTarget - rPos;
    dir.normalise();
    EGG::Vector3f up(0.0f, 1.0f, 0.0f);
    EGG::Vector3f side = up.cross(dir);
    side.normalise();
    bool parallel = side.squaredLength() <= FLT_EPSILON;
    if (parallel) {
        EGG::Vector3f axis(-1.0f, 0.0f, 0.0f);
        EGG::Vector3f up2 = dir.cross(axis);
        up2.normalise();
        return up2;
    }
    up = dir.cross(side);
    return up;
}
