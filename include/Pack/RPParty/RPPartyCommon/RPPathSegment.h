#ifndef RP_PARTY_COMMON_PATH_SEGMENT_H
#define RP_PARTY_COMMON_PATH_SEGMENT_H
#include <Pack/types_pack.h>

#include <egg/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief One segment of a path: four points, length and inverse length
 * @details Abstract: vtable 80357568 has three pure virtuals; the constructors install it
 * before their own. Size 0x3C [H] (array stride in the segment arrays). No virtual destructor:
 * the arrays destroy segments with the concrete non-virtual destructors.
 * @note Name is a project convention.
 */
class RPPathSegment {
public:
    // The constructors of the concrete segments install this vtable and construct mPoints
    // only; mLength/mInvLength are left to Init.
    RPPathSegment() {}

protected:
    /**
     * @brief Linear: start, start, end, end. Bezier: start, control 1, control 2, end
     */
    EGG::Vector3f mPoints[4]; // at 0x0
    f32 mLength;              // at 0x30
    f32 mInvLength;           // at 0x34

public:
    virtual void Init(const EGG::Vector3f& rStart, const EGG::Vector3f& rEnd,
                      const EGG::Vector3f& rPrev, const EGG::Vector3f& rNext) = 0; // at 0x8
    // Wii Play: EvalPos and EvalPosAndTangent return pPos (mr r3 at the end of every override)
    virtual EGG::Vector3f* EvalPos(f32 t, EGG::Vector3f* pPos) = 0;                    // at 0xC
    virtual EGG::Vector3f* EvalPosAndTangent(f32 t, EGG::Vector3f* pPos,
                                   EGG::Vector3f* pTangent) = 0; // at 0x10

    f32 GetLength() const {
        return mLength;
    }
};

/**
 * @brief Straight path segment
 * @details vtable 8035752C. Size 0x3C [H].
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPLinearSegment : public RPPathSegment {
public:
    // Out of line (RPPathSegment_2): RPPathFollower passes both to __construct_new_array
    RPLinearSegment();
    ~RPLinearSegment();

    /**
     * @brief Points from rStart/rEnd; length = |rEnd - rStart| (rPrev/rNext unused) [H]
     */
    virtual void Init(const EGG::Vector3f& rStart, const EGG::Vector3f& rEnd,
                      const EGG::Vector3f& rPrev, const EGG::Vector3f& rNext); // at 0x8
    virtual EGG::Vector3f* EvalPos(f32 t, EGG::Vector3f* pPos);                    // at 0xC
    virtual EGG::Vector3f* EvalPosAndTangent(f32 t, EGG::Vector3f* pPos,
                                   EGG::Vector3f* pTangent); // at 0x10
};

/**
 * @brief Cubic Bezier path segment
 * @details vtable 80357518. Size 0x3C [H]. Init offsets the inner control points by a third of
 * the chord length along the directions from the neighbouring points.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPBezierSegment : public RPPathSegment {
public:
    // Out of line (RPPathSegment_3): RPPathFollower passes both to __construct_new_array
    RPBezierSegment();
    ~RPBezierSegment();

    virtual void Init(const EGG::Vector3f& rStart, const EGG::Vector3f& rEnd,
                      const EGG::Vector3f& rPrev, const EGG::Vector3f& rNext); // at 0x8
    virtual EGG::Vector3f* EvalPos(f32 t, EGG::Vector3f* pPos);                    // at 0xC
    virtual EGG::Vector3f* EvalPosAndTangent(f32 t, EGG::Vector3f* pPos,
                                   EGG::Vector3f* pTangent); // at 0x10
};

/**
 * @brief Abstract array of path segments as RPPathFollower sees it
 * @details Abstract: no vtable of its own exists; the two concrete arrays share this layout.
 * @note Name is a project convention.
 */
class IRPPathSegmentArray {
public:
    virtual s32 GetCount() const = 0;                // at 0x8
    virtual RPPathSegment* GetSegment(s32 idx) = 0;  // at 0xC
    virtual ~IRPPathSegmentArray() {}                // at 0x10
};

/**
 * @brief new[]-allocated array of RPBezierSegment
 * @details vtable 80357554. Size 0xC [H].
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPBezierSegmentArray : public IRPPathSegmentArray {
public:
    // Inlined into RPPathWalker's constructor
    RPBezierSegmentArray(s32 count) {
        mCount = count;
        mpSegments = new RPBezierSegment[count];
    }

    virtual s32 GetCount() const; // at 0x8
    /**
     * @brief NULL when idx >= mCount [H]
     */
    virtual RPPathSegment* GetSegment(s32 idx); // at 0xC
    virtual ~RPBezierSegmentArray(); // at 0x10

private:
    s32 mCount;                    // at 0x4
    RPBezierSegment* mpSegments;   // at 0x8
};

/**
 * @brief new[]-allocated array of RPLinearSegment
 * @details vtable 80357540. Size 0xC [H].
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPLinearSegmentArray : public IRPPathSegmentArray {
public:
    // Inlined into RPPathWalker's constructor
    RPLinearSegmentArray(s32 count) {
        mCount = count;
        mpSegments = new RPLinearSegment[count];
    }

    virtual s32 GetCount() const; // at 0x8
    /**
     * @brief NULL when idx >= mCount [H]
     */
    virtual RPPathSegment* GetSegment(s32 idx); // at 0xC
    virtual ~RPLinearSegmentArray(); // at 0x10

private:
    s32 mCount;                    // at 0x4
    RPLinearSegment* mpSegments;   // at 0x8
};

//! @}

#endif
