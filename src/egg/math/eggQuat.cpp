// TODO(kiwi) REMOVE AFTER REFACTOR
#pragma ipa file

#include <egg/math.h>

namespace EGG {

void Quatf::set(f32 fw, f32 fx, f32 fy, f32 fz) {
    w = fw;
    v.x = fx;
    v.y = fy;
    v.z = fz;
}

void Quatf::setAxisRotation(const Vector3f& rAxis, f32 angle) {
    f32 half = angle * 0.5f;
    f32 cosa = Math<f32>::cos(half);
    f32 sina = Math<f32>::sin(half);
    set(cosa, sina * rAxis.x, sina * rAxis.y, sina * rAxis.z);
}

void Quatf::normalise() {
    f32 mag = Math<f32>::sqrt(w * w + v.dot(v));
    if (mag > 0.0f) {
        multScalar(1.0f / mag);
    }
}

Quatf Quatf::conjugate() {
    Quatf q;
    q.w = w;
    q.v = -1.0f * v;
    return q;
}

Quatf::Quatf(f32 fw, Vector3f vxyz) : v(vxyz), w(fw) {}

Vector3f Quatf::rotateVector(const Vector3f& rVec) {
    Quatf conj;
    Quatf res;
    conj = conjugate();
    res = *this * rVec;
    res = res * conj;
    return res.v;
}

Quatf Quatf::operator*(const Vector3f& rVec) const {
    return Quatf(-v.dot(rVec), v.cross(rVec) + w * rVec);
}

Quatf Quatf::operator*(const Quatf& rRhs) const {
    return Quatf(w * rRhs.w - v.dot(rRhs.v), v.cross(rRhs.v) + w * rRhs.v + rRhs.w * v);
}

void Quatf::slerpTo(const Quatf& rTo, f32 t, Quatf& rOut) const {
    f32 dot = v.x * rTo.v.x + v.y * rTo.v.y + v.z * rTo.v.z + w * rTo.w;

    if (dot > 1.0f) {
        dot = 1.0f;
    } else if (dot < -1.0f) {
        dot = -1.0f;
    }

    bool neg;
    if (dot < 0.0) {
        dot = -dot;
        neg = true;
    } else {
        neg = false;
    }

    f32 theta = Math<f32>::acos(dot);
    f32 sinTheta = Math<f32>::sin(theta);

    f32 s;
    if (Math<f32>::abs(sinTheta) < 0.00001f) {
        s = 1.0f - t;
    } else {
        f32 invSin = 1.0f / sinTheta;
        f32 tTheta = t * theta;
        s = invSin * Math<f32>::sin(theta - tTheta);
        t = invSin * Math<f32>::sin(tTheta);
    }

    if (neg) {
        t = -t;
    }

    rOut.v.x = s * v.x + t * rTo.v.x;
    rOut.v.y = s * v.y + t * rTo.v.y;
    rOut.v.z = s * v.z + t * rTo.v.z;
    rOut.w = s * w + t * rTo.w;
}

} // namespace EGG
