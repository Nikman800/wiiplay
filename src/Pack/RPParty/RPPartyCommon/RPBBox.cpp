// RPBBox only: the debug log/graph code that shares this ledger range is a separate file
// (src/Pack/RPParty/RPPartyCommon/RPDebugLog.cpp); see the split note there.
#include <Pack/RPParty/RPPartyCommon/RPBBox.h>

extern "C" void RP_BBox_reset2(RPBBox* box) {
    box->Reset();
}

void RPBBox::Reset() {
    mMinNeg.x = mMinNeg.y = mMinNeg.z = 0.0f;
    mMaxPos.x = mMaxPos.y = mMaxPos.z = 0.0f;
    mMax.x = -3.402823466e38f;
    mMax.y = -3.402823466e38f;
    mMax.z = -3.402823466e38f;
    mMin.x = 3.402823466e38f;
    mMin.y = 3.402823466e38f;
    mMin.z = 3.402823466e38f;
}

void RPBBox::Extend(const EGG::Vector3f& v) {
    mMaxPos.x = v.x > mMaxPos.x ? v.x : mMaxPos.x;
    mMaxPos.y = v.y > mMaxPos.y ? v.y : mMaxPos.y;
    mMaxPos.z = v.z > mMaxPos.z ? v.z : mMaxPos.z;
    mMinNeg.x = v.x < mMinNeg.x ? v.x : mMinNeg.x;
    mMinNeg.y = v.y < mMinNeg.y ? v.y : mMinNeg.y;
    mMinNeg.z = v.z < mMinNeg.z ? v.z : mMinNeg.z;
    mMin.x = v.x < mMin.x ? v.x : mMin.x;
    mMin.y = v.y < mMin.y ? v.y : mMin.y;
    mMin.z = v.z < mMin.z ? v.z : mMin.z;
    mMax.x = v.x > mMax.x ? v.x : mMax.x;
    mMax.y = v.y > mMax.y ? v.y : mMax.y;
    mMax.z = v.z > mMax.z ? v.z : mMax.z;
}

EGG::Vector3f RPBBox::GetSum() const {
    return mMinNeg + mMaxPos;
}

