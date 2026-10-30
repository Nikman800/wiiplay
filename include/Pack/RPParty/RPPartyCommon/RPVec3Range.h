#ifndef RP_PARTY_COMMON_VEC3_RANGE_H
#define RP_PARTY_COMMON_VEC3_RANGE_H
#include <Pack/types_pack.h>

#include <egg/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Accumulator of 3D offsets: per-axis most negative and most positive value
 * @details No vtable. Size 0x18 (RPPhysSphere news three of them; RPKclHolder::CalcPushOut
 * keeps one on the stack). GetSum is out of line at 801B94EC (ledger RP_BBox_getSumB), next
 * to RPKclHolder. RPBBox is the same accumulator plus a bounding box.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPVec3Range {
public:
    RPVec3Range() : mMin(EGG::Vector3f::zero), mMax(EGG::Vector3f::zero) {}

    void Reset() {
        mMin = mMax = EGG::Vector3f::zero;
    }

    void Extend(const EGG::Vector3f& rVec) {
        if (rVec.x < mMin.x) {
            mMin.x = rVec.x;
        }
        if (rVec.y < mMin.y) {
            mMin.y = rVec.y;
        }
        if (rVec.z < mMin.z) {
            mMin.z = rVec.z;
        }
        if (rVec.x > mMax.x) {
            mMax.x = rVec.x;
        }
        if (rVec.y > mMax.y) {
            mMax.y = rVec.y;
        }
        if (rVec.z > mMax.z) {
            mMax.z = rVec.z;
        }
    }

    /**
     * @brief mMin + mMax: the net offset when the values pushed both ways
     */
    EGG::Vector3f GetSum() const;

private:
    EGG::Vector3f mMin; // at 0x0 min(0, values)
    EGG::Vector3f mMax; // at 0xC max(0, values)
};

//! @}

#endif
