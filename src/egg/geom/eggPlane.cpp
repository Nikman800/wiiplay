// TODO(kiwi) REMOVE AFTER REFACTOR
#pragma ipa file

#include <egg/geom.h>

namespace EGG {

// STAND-IN (invented, dead-stripped at link; user-approved 2026-10-28): in the original the weak
// operator*(f32, const Vector3f&) instance comes before project, so an earlier function in this file
// used it and was stripped. Replace this if the real function is found.
Vector3f PlaneStandIn(f32 f, const Vector3f& rVec) {
    return f * rVec;
}

Vector3f Plane3f::project(const Vector3f& rVec) {
    f32 s = normal.dot(rVec) - depth;
    Vector3f work(rVec);
    work -= s * normal;
    return work;
}

} // namespace EGG
