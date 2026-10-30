#ifndef EGG_MATH_QUAT_H
#define EGG_MATH_QUAT_H
#include <egg/types_egg.h>

#include <egg/math/eggVector.h>

namespace EGG {

class Quatf {
public:
    Quatf() {}
    Quatf(f32 fw, Vector3f vxyz);
    ~Quatf() {}

    void multScalar(f32 scale) {
        v *= scale;
        w *= scale;
    }

    void set(f32 fw, f32 fx, f32 fy, f32 fz);
    void setAxisRotation(const Vector3f& rAxis, f32 angle);
    // Wii Play: RPSlerpBasis
    void slerpTo(const Quatf& rTo, f32 t, Quatf& rOut) const;
    void normalise();
    Quatf conjugate();
    Vector3f rotateVector(const Vector3f& rVec);
    Quatf operator*(const Vector3f& rVec) const;
    Quatf operator*(const Quatf& rRhs) const;

public:
    Vector3f v; // at 0x0
    f32 w;      // at 0xC
};

} // namespace EGG

#endif
