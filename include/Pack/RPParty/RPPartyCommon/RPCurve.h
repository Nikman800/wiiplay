#ifndef RP_PARTY_COMMON_CURVE_H
#define RP_PARTY_COMMON_CURVE_H
#include <Pack/types_pack.h>

#include <egg/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Construction parameters of an RPCurve (built on the stack by the users; RPDucBalloon
 * fills it from an aggregate initializer) [M]
 */
class RPKclHolder;

struct RPCurveDesc {
    s32 mPointNum;           // at 0x0
    f32 unk4;                // at 0x4 [L]
    f32 unk8;                // at 0x8 [L]
    f32 unkC;                // at 0xC [L]
    const f32* mpSegLength;  // at 0x10 mPointNum - 1 lengths, copied
    const f32* mpParams;     // at 0x14 [L] optional, copied
};

/**
 * @brief Anchor of RPCurve::Build: the first point's position [M] and two more members [L]
 */
struct RPCurveAnchor {
    nw4r::math::VEC3 mPos; // at 0x0
    nw4r::math::VEC3 unkC; // at 0xC [L]
    RPKclHolder* mpKcl;    // at 0x18 [M] collision for the points, or NULL
};

/**
 * @brief Line RPCurve::SamplePoints lays the points along: start and direction [M]
 */
struct RPCurveLine {
    nw4r::math::VEC3 mPos; // at 0x0
    nw4r::math::VEC3 mDir; // at 0xC
};

/**
 * @brief Optional new segment lengths for RPCurve::SamplePoints [M]
 */
struct RPCurveSegLengths {
    const f32* mpSegLength; // at 0x0
};

/**
 * @brief Point chain with per-segment lengths, sampled into points and tangents (fishing line)
 * @details No vtable. Size 0x20 [M] (Copy writes every member; used by Fsh_Rod and Duc).
 * @note Wii Play-only framework class; the name is a project convention. The ledger's
 * RP_PointArray (setPoint) is this class.
 */
class RPCurve {
public:
    /**
     * @brief Allocates the four arrays and copies the segment lengths (and parameters) from the
     * description [H] (ledger RP_Curve_copy)
     */
    RPCurve(const RPCurveDesc& rDesc);

    /**
     * @brief Moves the chain one step from the anchor (ledger RP_Curve_build) [M]
     */
    void Build(const RPCurveAnchor& rAnchor);
    /**
     * @brief Lays the points along a line, segment by segment (ledger RP_Curve_samplePoints) [M]
     */
    void SamplePoints(const RPCurveLine& rLine, const RPCurveSegLengths* pSegLengths);
    /**
     * @brief Point idx (ledger Fsh_Rope_getPoint) [H]
     */
    const EGG::Vector3f& GetPoint(s32 idx) const;

    // Wii Play: out of line (ledger RP_Curve_setPoint, 801B9694)
    void SetPoint(s32 idx, const EGG::Vector3f& rPos);

    /**
     * @brief Target of point idx, which Build pulls the point toward (ledger
     * Fsh_Rope_setTargetPoint / Fsh_Rope_getTargetPoint, 801B9660 / 801B9684) [M]
     */
    void SetTargetPoint(s32 idx, const EGG::Vector3f& rPos);
    const EGG::Vector3f& GetTargetPoint(s32 idx) const;

    void SetUnk4(f32 value) {
        unk4 = value;
    }
    void SetUnk8(f32 value) {
        unk8 = value;
    }

private:
    s32 mPointNum;             // at 0x0
    f32 unk4;                  // at 0x4 [L]
    f32 unk8;                  // at 0x8 [L]
    f32 unkC;                  // at 0xC [L]
    f32* mpSegLength;          // at 0x10 mPointNum - 1
    EGG::Vector3f* mpPoints;   // at 0x14
    EGG::Vector3f* mpTargets;  // at 0x18 [M] see SetTargetPoint
    f32* mpParams;             // at 0x1C [L] mPointNum
};

//! @}

#endif
