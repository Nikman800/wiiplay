#ifndef RP_PARTY_COMMON_BBOX_H
#define RP_PARTY_COMMON_BBOX_H
#include <Pack/types_pack.h>

#include <egg/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Accumulator of 3D offsets: per-axis most negative and most positive value plus the
 * bounding box of all values
 * @details No vtable. Size 0x30 [H] (embedded twice in RPCollisionShape at 0x78 and 0xA8;
 * stack copies in RPColliderList::CheckHit). GetSum() = mMinNeg + mMaxPos is the combined
 * push-out offset.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPBBox {
public:
    /**
     * @brief Zeroes mMinNeg/mMaxPos, mMax = -FLT_MAX, mMin = FLT_MAX (RP_BBox_reset and the
     * identical RP_BBox_reset2) [H]
     */
    void Reset();
    void Extend(const EGG::Vector3f& rVec);
    EGG::Vector3f GetSum() const; // RP_BBox_sumMinMax

private:
    EGG::Vector3f mMinNeg; // at 0x0 min(0, values)
    EGG::Vector3f mMaxPos; // at 0xC max(0, values)
    EGG::Vector3f mMax;    // at 0x18
    EGG::Vector3f mMin;    // at 0x24
};

//! @}

#endif
