#include <Pack/RPParty/RPPartyCommon/RPPathFollower_3.h>

extern const f32 RPPathFollower_DefaultInterpolation;
extern const f32 RPPathFollower_Percent;
extern const f32 RPPathFollower_Zero;

RPPathFollower::RPPathFollower(f32 speedPercent, f32 scalePercent, const void* pPathRes)
    : RPPathWalker(pPathRes) {
    mSpeed = speedPercent / RPPathFollower_Percent;
    mScale = scalePercent / RPPathFollower_Percent;
    ApplyPointParams();
}

void RPPathFollower::ApplyPointParams() {
    SetSegment(0);

    mReverse = false;
    unk60 = 0;
    unk64 = RPPathFollower_DefaultInterpolation;
    unk50 = unk54 = unk58 = RPPathFollower_Percent * mSpeed;
    unk2C = unk34 = unk38 = RPPathFollower_Percent * mScale;
    unk4C = RPPathFollower_Zero;
    unk48 = RPPathFollower_Zero;
    unk40 = RPPathFollower_Zero;

    const RPPathPoint* pBehind = GetPointBehind(mSegment);
    if (pBehind->param0C > 0) {
        pBehind = GetPointBehind(mSegment);
        const s16 param0C = pBehind->param0C;
        unk60 = param0C;
        unk64 = RPPathFollower_DefaultInterpolation / static_cast<f32>(param0C);
    }

    pBehind = GetPointBehind(mSegment);
    if (pBehind->param0E > 0) {
        pBehind = GetPointBehind(mSegment);
        unk50 = unk54 = mSpeed * static_cast<f32>(pBehind->param0E);
    }

    const RPPathPoint* pAhead = GetPointAhead(mSegment);
    if (pAhead->param0E > 0) {
        pAhead = GetPointAhead(mSegment);
        unk58 = mSpeed * static_cast<f32>(pAhead->param0E);
    }

    pBehind = GetPointBehind(mSegment);
    if (pBehind->param0C == 0) {
        pBehind = GetPointBehind(mSegment);
        if (pBehind->param10 > 0) {
            pBehind = GetPointBehind(mSegment);
            unk2C = unk34 = mScale * static_cast<f32>(pBehind->param10);
        }

        pBehind = GetPointBehind(mSegment);
        if (pBehind->param12 > 0) {
            pBehind = GetPointBehind(mSegment);
            unk40 = unk48 = static_cast<f32>(pBehind->param12);
        }
    }

    pAhead = GetPointAhead(mSegment);
    if (pAhead->param10 > 0) {
        pAhead = GetPointAhead(mSegment);
        unk38 = mScale * static_cast<f32>(pAhead->param10);
    }

    pAhead = GetPointAhead(mSegment);
    if (pAhead->param12 > 0) {
        pAhead = GetPointAhead(mSegment);
        unk4C = static_cast<f32>(pAhead->param12);
    }
}

const f32 RPPathFollower_DefaultInterpolation = 1.0f;

// Keep the signed int-to-float pool entry between 1.0f and 100.0f.
// The declaration flushes deferred code without emitting another data object.
#pragma defer_codegen off
extern const f32 RPPathFollower_DefaultInterpolation;

const f32 RPPathFollower_Percent = 100.0f;
const f32 RPPathFollower_Zero = 0.0f;
