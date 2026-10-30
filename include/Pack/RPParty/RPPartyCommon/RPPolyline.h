#ifndef RP_PARTY_COMMON_POLYLINE_H
#define RP_PARTY_COMMON_POLYLINE_H
#include <Pack/types_pack.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Bezier curve through n control points, evaluated by repeated linear interpolation
 * @details vtable 80357310. Size 0x14, verified (Duc/Tnk/Bom cameras). Calc(t) runs de
 * Casteljau's algorithm with the two n-1 point work buffers. Used for camera intro paths.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPPolyline {
public:
    explicit RPPolyline(s32 pointNum);

    /**
     * @brief Copies mPointNum points into mpPoints [H]
     */
    void SetPoints(const nw4r::math::VEC3* pPoints);
    void Calc(f32 t, nw4r::math::VEC3* pOut) const;

private:
    s32 mPointNum;                 // at 0x0
    nw4r::math::VEC3* mpPoints;    // at 0x4
    nw4r::math::VEC3* mpWork[2];   // at 0x8 mPointNum - 1 each

public:
    /**
     * @brief Does not free the point arrays [H]
     */
    virtual ~RPPolyline(); // at 0x8
};

//! @}

#endif
