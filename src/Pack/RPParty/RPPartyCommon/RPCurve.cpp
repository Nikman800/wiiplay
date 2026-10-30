// Point chain with per-segment lengths (RPCurve). Unit 0x801B9660-0x801B9D74.
#include <Pack/RPParty/RPPartyCommon/RPCurve.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>

static inline const EGG::Vector3f& RPCurve_AsVec(const nw4r::math::VEC3& rVec) {
    return static_cast<const EGG::Vector3f&>(rVec);
}

RPCurve::RPCurve(const RPCurveDesc& rDesc) {
    mPointNum = rDesc.mPointNum;
    unk4 = rDesc.unk4;
    unk8 = rDesc.unk8;
    unkC = rDesc.unkC;

    mpSegLength = new f32[mPointNum - 1];
    mpPoints = new EGG::Vector3f[mPointNum];
    mpTargets = new EGG::Vector3f[mPointNum];
    mpParams = new f32[mPointNum];

    for (s32 i = 0; i < mPointNum - 1; i++) {
        mpSegLength[i] = rDesc.mpSegLength[i];
    }

    if (rDesc.mpParams != NULL) {
        for (s32 i = 0; i < mPointNum; i++) {
            mpParams[i] = rDesc.mpParams[i];
        }
    }
}

void RPCurve::SamplePoints(const RPCurveLine& rLine, const RPCurveSegLengths* pSegLengths) {
    f32 dist = 0.0f;

    for (s32 i = 0; i < mPointNum; i++) {
        mpPoints[i] = RPCurve_AsVec(rLine.mPos) + RPCurve_AsVec(rLine.mDir) * dist;
        mpTargets[i] = EGG::Vector3f(0.0f, 0.0f, 0.0f);

        if (i < mPointNum - 1) {
            if (pSegLengths != NULL) {
                mpSegLength[i] = pSegLengths->mpSegLength[i];
            }
            dist += mpSegLength[i];
        }
    }
}

void RPCurve::Build(const RPCurveAnchor& rAnchor) {
    for (s32 i = 0; i < mPointNum; i++) {
        if (i == 0) {
            mpTargets[0] = RPCurve_AsVec(rAnchor.mPos) - mpPoints[0];
            mpPoints[0] = RPCurve_AsVec(rAnchor.mPos);
            continue;
        }

        mpTargets[i] *= unk8;
        mpTargets[i].y += unk4;

        EGG::Vector3f pos = mpPoints[i] + mpTargets[i];
        f32 segLen = mpSegLength[i - 1];

        if (rAnchor.mpKcl != NULL) {
            pos += rAnchor.mpKcl->CalcPushOut(pos, mpParams[i]);
        }

        EGG::Vector3f prev = mpPoints[i - 1];
        EGG::Vector3f dir = pos - prev;
        f32 len = dir.length();
        if (len > 1.1920929e-7f) {
            dir *= segLen / len;
        } else {
            dir = EGG::Vector3f(segLen, 0.0f, 0.0f);
        }

        EGG::Vector3f tan;
        if (i == 1) {
            tan = RPCurve_AsVec(rAnchor.unkC);
        } else {
            EGG::Vector3f prev2 = mpPoints[i - 2];
            tan = prev - prev2;
            if (tan.length() > 1.1920929e-7f) {
                tan.normalise();
            } else {
                tan = EGG::Vector3f(1.0f, 0.0f, 0.0f);
            }
        }

        EGG::Vector3f d = dir;
        d.normalise();

        f32 t = unkC;
        if (t < 0.0f) {
            t = 0.0f;
        }
        if (t > 1.0f) {
            t = 1.0f;
        }

        EGG::Vector3f blend = d * t + tan * (1.0f - t);
        if (blend.length() >= 1.1920929e-7f) {
            blend.normalise();
            dir = blend * segLen;
        }

        EGG::Vector3f old = mpPoints[i];
        mpPoints[i] = prev + dir;
        mpTargets[i] = mpPoints[i] - old;
    }
}

const EGG::Vector3f& RPCurve::GetPoint(s32 idx) const {
    return mpPoints[idx];
}

void RPCurve::SetPoint(s32 idx, const EGG::Vector3f& rPos) {
    mpPoints[idx] = rPos;
}

const EGG::Vector3f& RPCurve::GetTargetPoint(s32 idx) const {
    return mpTargets[idx];
}

void RPCurve::SetTargetPoint(s32 idx, const EGG::Vector3f& rPos) {
    mpTargets[idx] = rPos;
}

// Unreferenced stand-in (dropped by the linker): compiled before SetTargetPoint, it gives
// SetTargetPoint the original's float registers.
extern "C" void RPCurve_standInSetPoint(RPCurve* pCurve, s32 idx, const EGG::Vector3f& rPos) {
    pCurve->SetPoint(idx, rPos);
}
