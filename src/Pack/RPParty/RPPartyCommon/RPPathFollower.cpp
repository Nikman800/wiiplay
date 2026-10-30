// RPPathWalker (RPPathFollower's base) and the path segment classes (one file:
// 0x801C5204-0x801C5DD0). Definitions in reverse address order: segment arrays and segments
// first, then RPPathWalker.

// ---- from RPPathSegment_5.cpp ----
#include <Pack/RPParty/RPPartyCommon/RPPathSegment_5.h>

// The merged definitions keep RPPathSegment's weak vtable after the two array vtables,
// as in the original .data layout.

RPBezierSegmentArray::~RPBezierSegmentArray() {
    delete[] mpSegments;
}

RPPathSegment* RPBezierSegmentArray::GetSegment(s32 idx) {
    return idx < mCount ? &mpSegments[idx] : NULL;
}

s32 RPBezierSegmentArray::GetCount() const {
    return mCount;
}

// ---- from RPPathSegment_4.cpp ----
#include <Pack/RPParty/RPPartyCommon/RPPathSegment_4.h>

RPLinearSegmentArray::~RPLinearSegmentArray() {
    delete[] mpSegments;
}

RPPathSegment* RPLinearSegmentArray::GetSegment(s32 idx) {
    return idx < mCount ? &mpSegments[idx] : NULL;
}

s32 RPLinearSegmentArray::GetCount() const {
    return mCount;
}

// ---- from RPPathSegment_3.cpp ----
#include <Pack/RPParty/RPPartyCommon/RPPathSegment_3.h>

// Out-of-line copies of RPBezierSegment's constructor and destructor (RPPathFollower passes
// them to __construct_new_array).
RPBezierSegment::RPBezierSegment() {}

RPBezierSegment::~RPBezierSegment() {}

// ---- from RPPathSegment_2.cpp ----
#include <Pack/RPParty/RPPartyCommon/RPPathSegment_2.h>

// Out-of-line copies of RPLinearSegment's constructor and destructor (RPPathFollower passes
// them to __construct_new_array).
RPLinearSegment::RPLinearSegment() {}

RPLinearSegment::~RPLinearSegment() {}

// ---- RPPathWalker ----
#include <Pack/RPParty/RPPartyCommon/RPPathFollower.h>
#include <Pack/RPParty/RPPartyCommon/RPPathSegment.h>

static inline const EGG::Vector3f& GetPointPos(const RPPathRes* pRes, u16 idx) {
    return RP_getPathPoint(pRes, idx)->pos;
}

// The point lookups go through a small inline returning the point's position: with the four
// positions as inline results, CodeWarrior numbers them above i/next/prev, so they are coloured
// first (closed loop p0..p3 r28/r26/r25/r27, then i r24, next r25, prev r23; the open loop's
// prev opens r22 -> _savegpr_22), as in the original. Named `const RPPathPoint*` locals took the
// low vregs of named locals and were coloured after i (22 words; the 120+ declaration orders
// tried before could not fix that). `cur` is the u16 index computed once at the top of each loop
// (the original's early clrlwi). The open path has numPoints - 1 segments and clamps to the
// last point: `i + 1 < n + 1` is the original's compare against n + 1 (addi r27, r31, 1;
// `<= n` costs 31 words).
// The earlier `#pragma opt_lifetimes off` (it kept n one web, r31 in both branches) is no longer
// needed with this form: it matches with and without it.
RPPathWalker::RPPathWalker(const void* pPathRes) {
    mpRes = NULL;
    mSegment = 0;
    mDistance = 0.0f;
    mStopped = false;
    mReverse = false;
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mTangent.x = 0.0f;
    mTangent.y = 0.0f;
    mTangent.z = 1.0f;
    mpRes = static_cast<const RPPathRes*>(pPathRes);
    if (mSegment == 0) {
        int n;
        const RPPathRes* pRes = mpRes;
        if (pRes->flags & 1) {
            n = pRes->numPoints;
            if (pRes->type == 1) {
                mpSegments = new RPBezierSegmentArray(n);
            } else {
                mpSegments = new RPLinearSegmentArray(n);
            }
            for (s32 i = 0; i < n; i++) {
                u16 cur = i;
                u16 next = i + 1 < n ? i + 1 : i + 1 - n;
                u16 prev = i - 1 >= 0 ? i - 1 : i - 1 + n;
                u16 next2 = i + 2 < n ? i + 2 : i + 2 - n;
                const EGG::Vector3f& p0 = GetPointPos(pRes, cur);
                const EGG::Vector3f& p1 = GetPointPos(pRes, next);
                const EGG::Vector3f& p2 = GetPointPos(pRes, prev);
                const EGG::Vector3f& p3 = GetPointPos(pRes, next2);
                mpSegments->GetSegment(i)->Init(p0, p1, p2, p3);
            }
        } else {
            n = pRes->numPoints - 1;
            if (pRes->type == 1) {
                mpSegments = new RPBezierSegmentArray(n);
            } else {
                mpSegments = new RPLinearSegmentArray(n);
            }
            for (s32 i = 0; i < n; i++) {
                u16 cur = i;
                u16 next = i + 1 < n + 1 ? i + 1 : cur;
                u16 prev = i - 1 >= 0 ? i - 1 : cur;
                u16 next2 = i + 2 < n + 1 ? i + 2 : next;
                const EGG::Vector3f& p0 = GetPointPos(pRes, cur);
                const EGG::Vector3f& p1 = GetPointPos(pRes, next);
                const EGG::Vector3f& p2 = GetPointPos(pRes, prev);
                const EGG::Vector3f& p3 = GetPointPos(pRes, next2);
                mpSegments->GetSegment(i)->Init(p0, p1, p2, p3);
            }
        }
    }
}

const RPPathPoint* RPPathWalker::Advance(f32 delta) {
    const RPPathPoint* pPoint = NULL;
    RPPathSegment* pSeg = mpSegments->GetSegment(mSegment);

    if (!mReverse) {
        mDistance += delta;
    } else {
        mDistance -= delta;
    }

    while (mDistance > pSeg->GetLength()) {
        if (++mSegment > mpSegments->GetCount() - 1) {
            if (mStopped) {
                mSegment = mpSegments->GetCount() - 1;
                mDistance = pSeg->GetLength();
                pPoint = RP_getPathPoint(mpRes, mSegment + 1);
            } else if (mpRes->flags & 1) {
                mSegment = 0;
                mDistance -= pSeg->GetLength();
                pSeg = mpSegments->GetSegment(mSegment);
                pPoint = RP_getPathPoint(mpRes, mSegment);
            } else {
                mReverse ^= true;
                mSegment = mpSegments->GetCount() - 1;
                mDistance = 2.0f * pSeg->GetLength() - mDistance;
                pSeg = mpSegments->GetSegment(mSegment);
                pPoint = RP_getPathPoint(mpRes, mSegment);
            }
        } else {
            mDistance -= pSeg->GetLength();
            pSeg = mpSegments->GetSegment(mSegment);
            pPoint = RP_getPathPoint(mpRes, mSegment);
        }
    }

    while (mDistance < 0.0f) {
        pPoint = RP_getPathPoint(mpRes, mSegment);
        if (--mSegment < 0) {
            if (mStopped) {
                mSegment = 0;
                mDistance = 0.0f;
            } else if (mpRes->flags & 1) {
                mSegment = mpSegments->GetCount() - 1;
                pSeg = mpSegments->GetSegment(mSegment);
                mDistance += pSeg->GetLength();
            } else {
                mReverse ^= true;
                mSegment = 0;
                pSeg = mpSegments->GetSegment(0);
                mDistance = -mDistance;
            }
        } else {
            pSeg = mpSegments->GetSegment(mSegment);
            mDistance += pSeg->GetLength();
        }
    }

    if (!mReverse) {
        pSeg->EvalPosAndTangent(mDistance, &mPos, &mTangent);
    } else {
        pSeg->EvalPosAndTangent(mDistance, &mPos, &mTangent);
        mTangent = -mTangent;
    }

    return pPoint;
}

void RPPathWalker::SetSegment(s32 idx) {
    mSegment = idx;
    if (!mReverse) {
        mDistance = 0.0f;
    } else {
        mDistance = mpSegments->GetSegment(mSegment)->GetLength();
    }
    Advance(0.0f);
}

const RPPathPoint* RPPathWalker::GetPointBehind(s32 idx) {
    if (!mReverse) {
        return RP_getPathPoint(mpRes, idx);
    }
    if (mSegment >= mpSegments->GetCount() - 1 && (mpRes->flags & 1)) {
        return RP_getPathPoint(mpRes, 0);
    }
    return RP_getPathPoint(mpRes, idx + 1);
}

const RPPathPoint* RPPathWalker::GetPointAhead(s32 idx) {
    if (!mReverse) {
        if (mSegment >= mpSegments->GetCount() - 1 && (mpRes->flags & 1)) {
            return RP_getPathPoint(mpRes, 0);
        }
        return RP_getPathPoint(mpRes, idx + 1);
    }
    return RP_getPathPoint(mpRes, idx);
}

// Unreferenced stand-in (dropped by the linker). Defined last, so compiled first: it creates
// .sdata2's 1.0f before the functions above create 0.0f and 2.0f.
extern "C" f32 RPPathFollower_poolOrderOne() {
    return 1.0f;
}
