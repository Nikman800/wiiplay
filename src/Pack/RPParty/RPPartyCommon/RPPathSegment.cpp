// Path segments (RPLinearSegment, RPBezierSegment). Unit 0x801C49B4-0x801C50B8.
#include <Pack/RPParty/RPPartyCommon/RPPathSegment.h>

void RPLinearSegment::Init(const EGG::Vector3f& rStart, const EGG::Vector3f& rEnd,
                           const EGG::Vector3f& rPrev, const EGG::Vector3f& rNext) {
    mLength = mInvLength = 0.0f;
    mPoints[0] = mPoints[1] = rStart;
    mPoints[2] = mPoints[3] = rEnd;

    f32 len = (rEnd - rStart).length();
    if (len > 0.0f) {
        mLength = len;
        mInvLength = 1.0f / mLength;
    }
}

EGG::Vector3f* RPLinearSegment::EvalPos(f32 t, EGG::Vector3f* pPos) {
    f32 s = t * mInvLength;
    *pPos = (1.0f - s) * mPoints[0] + s * mPoints[3];
    return pPos;
}

EGG::Vector3f* RPLinearSegment::EvalPosAndTangent(f32 t, EGG::Vector3f* pPos,
                                        EGG::Vector3f* pTangent) {
    f32 s = t * mInvLength;
    *pPos = (1.0f - s) * mPoints[0] + s * mPoints[3];
    *pTangent = (-mPoints[0] + mPoints[3]) * mInvLength;
    return pPos;
}

void RPBezierSegment::Init(const EGG::Vector3f& rStart, const EGG::Vector3f& rEnd,
                           const EGG::Vector3f& rPrev, const EGG::Vector3f& rNext) {
    mLength = mInvLength = 0.0f;
    mPoints[0] = mPoints[1] = rStart;
    mPoints[2] = mPoints[3] = rEnd;

    EGG::Vector3f chord = rEnd - rStart;
    f32 len = chord.normalize();
    if (len > 0.0f) {
        if (&rPrev != &rStart) {
            EGG::Vector3f dir = rEnd - rPrev;
            if (dir.normalize() > 0.0f) {
                mPoints[1] += dir * len / 3.0f;
            }
        }
        if (&rNext != &rEnd) {
            EGG::Vector3f dir = rStart - rNext;
            if (dir.normalize() > 0.0f) {
                mPoints[2] += dir * len / 3.0f;
            }
        }
        mLength = len;
        mInvLength = 1.0f / mLength;
    }
}

EGG::Vector3f* RPBezierSegment::EvalPos(f32 t, EGG::Vector3f* pPos) {
    f32 s = t * mInvLength;
    f32 u = 1.0f - s;
    f32 b[4];
    b[0] = u * u * u;
    b[1] = 3.0f * u * u * s;
    b[2] = 3.0f * u * s * s;
    b[3] = s * s * s;
    *pPos = b[0] * mPoints[0] + b[1] * mPoints[1] + b[2] * mPoints[2] + b[3] * mPoints[3];
    return pPos;
}

EGG::Vector3f* RPBezierSegment::EvalPosAndTangent(f32 t, EGG::Vector3f* pPos,
                                        EGG::Vector3f* pTangent) {
    f32 s = t * mInvLength;
    f32 u = 1.0f - s;
    f32 uu = u * u;
    f32 ss = s * s;
    f32 e = 3.0f * uu;
    f32 a = 3.0f * ss;
    f32 c = 6.0f * (u * s);
    f32 uuu = uu * u;
    f32 sss = ss * s;

    f32 b[4];
    f32 d[4];
    b[0] = uuu;
    b[1] = e * s;
    b[2] = u * a;
    b[3] = sss;
    d[0] = -e;
    d[1] = -c + e;
    d[2] = -a + c;
    d[3] = a;

    *pPos = b[0] * mPoints[0] + b[1] * mPoints[1] + b[2] * mPoints[2] + b[3] * mPoints[3];
    *pTangent =
        (d[0] * mPoints[0] + d[1] * mPoints[1] + d[2] * mPoints[2] + d[3] * mPoints[3]) *
        mInvLength;
    return pPos;
}

