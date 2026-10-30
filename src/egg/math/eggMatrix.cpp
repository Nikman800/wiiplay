// TODO(kiwi) REMOVE AFTER REFACTOR
#pragma ipa file

#include <egg/math.h>

#include <revolution/GX.h>

namespace EGG {

// clang-format off
Matrix34f Matrix34f::ident(1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f);
// clang-format on

void Matrix34f::slerpTo(const Matrix34f& rTo, Matrix34f& rOut, f32 t) const {
    Quatf qFrom;
    Quatf qTo;
    toQuat(qFrom);
    rTo.toQuat(qTo);

    Quatf q;
    qFrom.slerpTo(qTo, t, q);
    rOut.fromQuat(q);
}

void Matrix34f::makeIdentity() {
    makeZero();
    _00 = _11 = _22 = 1.0f;
}

Vector3f Matrix34f::calcRPY() const {
    Vector3f rpy;

    f32 m00 = _00;
    f32 m10 = _10;
    f32 m20 = _20;
    f32 m21 = _21;
    f32 m22 = _22;

    f32 absM20 = Math<f32>::abs(m20);
    if (absM20 > 0.999999f) {
        rpy.x = 0.0f;
        rpy.y = (m20 / absM20) * -Math<f32>::pi_half();
        f32 m01 = _01;
        f32 m02 = _02;
        rpy.z = Math<f32>::atan2(-m01, -m20 * m02);
    } else {
        rpy.x = Math<f32>::atan2(m21, m22);
        rpy.y = Math<f32>::asin(-m20);
        rpy.z = Math<f32>::atan2(m10, m00);
    }

    return rpy;
}

Matrix34f::Matrix34f(f32 m00, f32 m01, f32 m02, f32 m03, f32 m10, f32 m11,
                     f32 m12, f32 m13, f32 m20, f32 m21, f32 m22, f32 m23) {

    _00 = m00;
    _01 = m01;
    _02 = m02;
    _03 = m03;
    _10 = m10;
    _11 = m11;
    _12 = m12;
    _13 = m13;
    _20 = m20;
    _21 = m21;
    _22 = m22;
    _23 = m23;
}

void Matrix34f::makeSRT(const Vector3f& rScale, const Vector3f& rRot,
                        const Vector3f& rTrans) {
    f32 s[3];
    f32 c[3];

    s[0] = Math<f32>::sin(rRot.x);
    s[1] = Math<f32>::sin(rRot.y);
    s[2] = Math<f32>::sin(rRot.z);

    c[0] = Math<f32>::cos(rRot.x);
    c[1] = Math<f32>::cos(rRot.y);
    c[2] = Math<f32>::cos(rRot.z);

    f32 cxcz = c[0] * c[2];
    f32 sxsy = s[0] * s[1];
    f32 cxsz = c[0] * s[2];

    _00 = rScale.x * (c[1] * c[2]);
    _10 = rScale.x * (c[1] * s[2]);
    _20 = rScale.x * -s[1];

    _01 = rScale.y * (sxsy * c[2] - cxsz);
    _11 = rScale.y * (sxsy * s[2] + cxcz);
    _21 = rScale.y * (s[0] * c[1]);

    _02 = rScale.z * (cxcz * s[1] + s[0] * s[2]);
    _12 = rScale.z * (cxsz * s[1] - s[0] * c[2]);
    _22 = rScale.z * (c[0] * c[1]);

    _03 = rTrans.x;
    _13 = rTrans.y;
    _23 = rTrans.z;
}

void Matrix34f::makeRT(const Vector3f& rRot, const Vector3f& rTrans) {
    f32 s[3];
    f32 c[3];

    s[0] = Math<f32>::sin(rRot.x);
    s[1] = Math<f32>::sin(rRot.y);
    s[2] = Math<f32>::sin(rRot.z);

    c[0] = Math<f32>::cos(rRot.x);
    c[1] = Math<f32>::cos(rRot.y);
    c[2] = Math<f32>::cos(rRot.z);

    f32 cxcz = c[0] * c[2];
    f32 sxsy = s[0] * s[1];
    f32 cxsz = c[0] * s[2];

    _00 = c[1] * c[2];
    _10 = c[1] * s[2];
    _20 = -s[1];

    _01 = sxsy * c[2] - cxsz;
    _11 = sxsy * s[2] + cxcz;
    _21 = s[0] * c[1];

    _02 = cxcz * s[1] + s[0] * s[2];
    _12 = cxsz * s[1] - s[0] * c[2];
    _22 = c[0] * c[1];

    _03 = rTrans.x;
    _13 = rTrans.y;
    _23 = rTrans.z;
}

void Matrix34f::makeR(const Vector3f& rRot) {
    f32 s[3];
    f32 c[3];

    s[0] = Math<f32>::sin(rRot.x);
    s[1] = Math<f32>::sin(rRot.y);
    s[2] = Math<f32>::sin(rRot.z);

    c[0] = Math<f32>::cos(rRot.x);
    c[1] = Math<f32>::cos(rRot.y);
    c[2] = Math<f32>::cos(rRot.z);

    f32 cxcz = c[0] * c[2];
    f32 sxsy = s[0] * s[1];
    f32 cxsz = c[0] * s[2];

    _00 = c[1] * c[2];
    _10 = c[1] * s[2];
    _20 = -s[1];

    _01 = sxsy * c[2] - cxsz;
    _11 = sxsy * s[2] + cxcz;
    _21 = s[0] * c[1];

    _02 = cxcz * s[1] + s[0] * s[2];
    _12 = cxsz * s[1] - s[0] * c[2];
    _22 = c[0] * c[1];

    _03 = 0.0f;
    _13 = 0.0f;
    _23 = 0.0f;
}

void Matrix34f::makeST(const Vector3f& rScale, const Vector3f& rTrans) {
    _00 = rScale.x;
    _10 = 0.0f;
    _20 = 0.0f;

    _01 = 0.0f;
    _11 = rScale.y;
    _21 = 0.0f;

    _02 = 0.0f;
    _12 = 0.0f;
    _22 = rScale.z;

    _03 = rTrans.x;
    _13 = rTrans.y;
    _23 = rTrans.z;
}

void Matrix34f::makeQ(const Quatf& rQuat) {
    f32 yy = 2.0f * rQuat.v.y * rQuat.v.y;
    f32 zz = 2.0f * rQuat.v.z * rQuat.v.z;
    f32 xx = 2.0f * rQuat.v.x * rQuat.v.x;
    f32 xy = 2.0f * rQuat.v.x * rQuat.v.y;
    f32 xz = 2.0f * rQuat.v.x * rQuat.v.z;
    f32 yz = 2.0f * rQuat.v.y * rQuat.v.z;
    f32 wz = 2.0f * rQuat.w * rQuat.v.z;
    f32 wx = 2.0f * rQuat.w * rQuat.v.x;
    f32 wy = 2.0f * rQuat.w * rQuat.v.y;

    _00 = 1.0f - yy - zz;
    _01 = xy - wz;
    _02 = xz + wy;

    _10 = xy + wz;
    _11 = 1.0f - xx - zz;
    _12 = yz - wx;

    _20 = xz - wy;
    _21 = yz + wx;
    _22 = 1.0f - xx - yy;

    _03 = 0.0f;
    _13 = 0.0f;
    _23 = 0.0f;
}

void Matrix34f::makeS(const Vector3f& rScale) {
    _00 = rScale.x;
    _01 = 0.0f;
    _02 = 0.0f;

    _10 = 0.0f;
    _11 = rScale.y;
    _12 = 0.0f;

    _20 = 0.0f;
    _21 = 0.0f;
    _22 = rScale.z;

    _03 = 0.0f;
    _13 = 0.0f;
    _23 = 0.0f;
}

void Matrix34f::fromQuat(const Quatf& rQuat) {
    f32 w = rQuat.w;

    _00 = 1.0f - 2.0f * rQuat.v.y * rQuat.v.y - 2.0f * rQuat.v.z * rQuat.v.z;
    _01 = 2.0f * rQuat.v.x * rQuat.v.y - 2.0f * w * rQuat.v.z;
    _02 = 2.0f * rQuat.v.x * rQuat.v.z + 2.0f * w * rQuat.v.y;

    _10 = 2.0f * rQuat.v.x * rQuat.v.y + 2.0f * w * rQuat.v.z;
    _11 = 1.0f - 2.0f * rQuat.v.x * rQuat.v.x - 2.0f * rQuat.v.z * rQuat.v.z;
    _12 = 2.0f * rQuat.v.y * rQuat.v.z - 2.0f * w * rQuat.v.x;

    _20 = 2.0f * rQuat.v.x * rQuat.v.z - 2.0f * w * rQuat.v.y;
    _21 = 2.0f * rQuat.v.y * rQuat.v.z + 2.0f * w * rQuat.v.x;
    _22 = 1.0f - 2.0f * rQuat.v.x * rQuat.v.x - 2.0f * rQuat.v.y * rQuat.v.y;

    _03 = _13 = _23 = 0.0f;
}

void Matrix34f::toQuat(Quatf& rQuat) const {
    f32 tr = _00 + _11 + _22;
    f32 ww = 0.25f * (1.0f + tr);
    f32 xx = ww - 0.5f * (_11 + _22);
    f32 yy = ww - 0.5f * (_22 + _00);
    f32 zz = ww - 0.5f * (_00 + _11);

    int i;
    if (ww > xx) {
        if (ww > yy) {
            if (ww > zz) {
                i = 0;
            } else {
                i = 3;
            }
        } else if (yy > zz) {
            i = 2;
        } else {
            i = 3;
        }
    } else if (xx > yy) {
        if (xx > zz) {
            i = 1;
        } else {
            i = 3;
        }
    } else if (yy > zz) {
        i = 2;
    } else {
        i = 3;
    }

    f32 f;
    switch (i) {
    case 0:
        rQuat.w = Math<f32>::sqrt(ww);
        f = 0.25f / rQuat.w;
        rQuat.v.x = f * (_21 - _12);
        rQuat.v.y = f * (_02 - _20);
        rQuat.v.z = f * (_10 - _01);
        break;
    case 1:
        rQuat.v.x = Math<f32>::sqrt(xx);
        f = 0.25f / rQuat.v.x;
        rQuat.w = f * (_21 - _12);
        rQuat.v.y = f * (_01 + _10);
        rQuat.v.z = f * (_02 + _20);
        break;
    case 2:
        rQuat.v.y = Math<f32>::sqrt(yy);
        f = 0.25f / rQuat.v.y;
        rQuat.w = f * (_02 - _20);
        rQuat.v.z = f * (_12 + _21);
        rQuat.v.x = f * (_10 + _01);
        break;
    case 3:
        rQuat.v.z = Math<f32>::sqrt(zz);
        f = 0.25f / rQuat.v.z;
        rQuat.w = f * (_10 - _01);
        rQuat.v.x = f * (_20 + _02);
        rQuat.v.y = f * (_21 + _12);
        break;
    }

    if (rQuat.w < 0.0f) {
        rQuat.w = -rQuat.w;
        rQuat.v.x = -rQuat.v.x;
        rQuat.v.y = -rQuat.v.y;
        rQuat.v.z = -rQuat.v.z;
    }

    f32 mag = Math<f32>::sqrt(rQuat.w * rQuat.w + rQuat.v.x * rQuat.v.x +
                              rQuat.v.y * rQuat.v.y + rQuat.v.z * rQuat.v.z);
    f32 inv = 1.0f / mag;
    rQuat.w *= inv;
    rQuat.v.x *= inv;
    rQuat.v.y *= inv;
    rQuat.v.z *= inv;
}

void Matrix34f::setAxisRotation(const Vector3f& rAxis, f32 angle) {
    Quatf q;
    q.setAxisRotation(rAxis, angle);
    makeQ(q);
}

void Matrix34f::loadPosMtx(u32 id) {
    GXLoadPosMtxImm(m, id);
}

void Matrix34f::multiplyTo(const Matrix34f& rRhs, Matrix34f& rOut) const {
    PSMTXConcat(m, rRhs.m, rOut.m);
}

} // namespace EGG
