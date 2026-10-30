#include "eggLightObject.h"

#include "eggDrawGX.h"
#include "eggLightTexture.h"

#include <nw4r/g3d.h>
#include <nw4r/math.h>

#include <revolution/GX.h>
#include <revolution/MTX.h>

namespace EGG {

template <> const char* IBinary<LightObject>::spExtension = "blobj";

namespace {

static const f32 ZERO_VEC[3] = {0.0f, 0.0f, 0.0f};

} // namespace

LightObject::LightObject()
    : mAmbIndex(0),
      mAt(0.0f, 0.0f, 0.0f),
      mPos(-10000.0f, 10000.0f, 10000.0f),
      mDir(0.0f, -1.0f, 0.0f),
      mBrightnessColor(DrawGX::WHITE),
      mIntensity(1.0f),
      mViewMode(0),
      mDistance(1.0f),
      mDarknessColor(DrawGX::BLACK),
      mLightType(cLightType_Dir),
      mCutoff(90.0f),
      mRefDistance(0.5f),
      mRefBrightness(0.5f),
      mSpotFn(GX_SP_OFF),
      mDistAttnFn(GX_DA_OFF),
      mShininess(16.0f),
      mFlags(0x661) {}

void LightObject::Reset() {
    SetPosAt(nw4r::math::VEC3(-10000.0f, 10000.0f, 10000.0f),
             nw4r::math::VEC3(0.0f, 0.0f, 0.0f));

    mFlags = 0x661;
    mBrightnessColor = DrawGX::WHITE;
    mDarknessColor = DrawGX::BLACK;
    mIntensity = 1.0f;
    mViewMode = 0;
    mLightType = cLightType_Dir;
    mCutoff = 90.0f;
    mRefDistance = 0.5f;
    mRefBrightness = 0.5f;
    mSpotFn = GX_SP_OFF;
    mDistAttnFn = GX_DA_OFF;
    mShininess = 16.0f;

    mAttnAA = mAttnKA = 0.0f;
    mAttnAB = mAttnKB = 0.0f;
    mAttnAC = mAttnKC = 0.0f;
}

void LightObject::Calc() {
    mLightVector = mDir;

    mLightColor.r = mBrightnessColor.r * mIntensity <= 255.0f
                        ? mBrightnessColor.r * mIntensity
                        : 255.0f;
    mLightColor.g = mBrightnessColor.g * mIntensity <= 255.0f
                        ? mBrightnessColor.g * mIntensity
                        : 255.0f;
    mLightColor.b = mBrightnessColor.b * mIntensity <= 255.0f
                        ? mBrightnessColor.b * mIntensity
                        : 255.0f;
    mLightColor.a = mBrightnessColor.a;
}

void LightObject::CalcView(const nw4r::math::MTX34& rCameraMtx) {
    switch (mViewMode) {
    case 1: {
        nw4r::math::VEC3 eye(-rCameraMtx._20, -rCameraMtx._21,
                             -rCameraMtx._22);

        if (PSVECMag(eye) > 0.0f) {
            nw4r::math::VEC3 half;
            C_VECHalfAngle(mDir, eye, half);
            PSVECScale(half, mLightVector, -1.0f);
        } else {
            mLightVector.x = mLightVector.y = 0.0f;
            mLightVector.z = 1.0f;
        }
        break;
    }
    case 0:
    case 2:
    case 3:
    case 5:
        break;
    }

    switch (mViewMode) {
    case 3: {
        mViewDir.x = mViewAt.x = 0.0f;
        mViewDir.y = mViewAt.y = 0.0f;
        mViewDir.z = mViewAt.z = -1.0f;
        mViewAt.z *= mDistance;
        mViewPos.x = mViewPos.y = mViewPos.z = 0.0f;
        break;
    }
    case 4: {
        mViewDir.x = mViewPos.x = 0.0f;
        mViewDir.y = mViewPos.y = 0.0f;
        mViewDir.z = mViewPos.z = 1.0f;
        mViewPos.z *= mDistance;
        mViewAt.x = mViewAt.y = mViewAt.z = 0.0f;
        break;
    }
    case 2: {
        mViewDir = mDir;
        mViewAt = mAt;
        mViewPos = mPos;
        break;
    }
    case 0:
    case 1: {
        PSMTXMultVec(rCameraMtx, mPos, mViewPos);
        PSMTXMultVec(rCameraMtx, mAt, mViewAt);
        nw4r::math::VEC3TransformNormal(&mViewDir, &rCameraMtx,
                                        &mLightVector);

        f32 mag = PSVECMag(mViewDir);
        if (mag > 0.0f) {
            f32 inv = 1.0f / mag;
            mViewDir.x *= inv;
            mViewDir.y *= inv;
            mViewDir.z *= inv;
        } else {
            mViewDir.x = mViewDir.y = mViewDir.z = 0.0f;
        }
        break;
    }
    case 5:
        break;
    }
}

void LightObject::CalcFinalDirColor(const LightTexture& rTexture,
                                    nw4r::math::VEC3* pDir,
                                    GXColor* pColor) const {
#line 194
    EGG_ASSERT(pDir);
    EGG_ASSERT(pColor);

    f32 scale = rTexture.getEffectScale(mAmbIndex);
    if (scale < 1.0f) {
        pColor->r = mLightColor.r * scale;
        pColor->g = mLightColor.g * scale;
        pColor->b = mLightColor.b * scale;
        pColor->a = mLightColor.a * scale;
    } else {
        *pColor = mLightColor;
    }

    *pDir = mViewDir;
}

void LightObject::InitGX(GXLightObj* pObj) const {
#line 281
    EGG_ASSERT(pObj);

    if (pObj != NULL && (mFlags & cFlag_0) && (mFlags & cFlag_6)) {
        GXInitLightColor(pObj, mLightColor);

        switch (mLightType) {
        case cLightType_Dir: {
            nw4r::math::VEC3 pos = mViewDir * -1e10f;
            GXInitLightPos(pObj, pos.x, pos.y, pos.z);
            GXInitLightDir(pObj, ZERO_VEC[0], ZERO_VEC[1], ZERO_VEC[2]);
            break;
        }
        case cLightType_Pos: {
            GXInitLightPos(pObj, mViewPos.x, mViewPos.y, mViewPos.z);
            GXInitLightDir(pObj, ZERO_VEC[0], ZERO_VEC[1], ZERO_VEC[2]);
            break;
        }
        case cLightType_Spot: {
            GXInitLightPos(pObj, mViewPos.x, mViewPos.y, mViewPos.z);
            GXInitLightDir(pObj, mViewDir.x, mViewDir.y, mViewDir.z);
            break;
        }
        }

        if (mFlags & 0x80) {
            GXInitLightAttnA(pObj, mAttnAA, mAttnAB, mAttnAC);
        } else {
            GXInitLightSpot(pObj, mCutoff > 0.0f ? mCutoff : 0.0001f, mSpotFn);
        }

        if (mFlags & 0x100) {
            GXInitLightAttnK(pObj, mAttnKA, mAttnKB, mAttnKC);
        } else {
#line 324
            EGG_ASSERT_MSG(0.0f < mRefBrightness && mRefBrightness < 1.0f, "ref brightness B must be (0 < B && B < 1)");
            GXInitLightDistAttn(pObj, mRefDistance * GetDistance(), mRefBrightness,
                                mDistAttnFn);
        }

        if (mFlags & 0x800) {
            GXInitLightAttn(pObj, 0.0f, 0.0f, 1.0f, mShininess / 2.0f, 0.0f,
                            1.0f - mShininess / 2.0f);
        }
    } else {
        GXColor black = {0, 0, 0, 255};
        GXInitLightColor(pObj, black);
    }
}

void LightObject::CopyToG3D_World(nw4r::g3d::LightObj& rLightObj) const {
    if ((mFlags & cFlag_0) && (mFlags & cFlag_6)) {
        rLightObj.Clear();

        if (!(mFlags & cFlag_9)) {
            rLightObj.DisableColor();
        }

        if (!(mFlags & cFlag_10)) {
            rLightObj.DisableAlpha();
        }

        rLightObj.Enable();
    } else {
        rLightObj.Disable();
    }
}

void LightObject::CopyToG3D_View(nw4r::g3d::LightObj& rLightObj,
                                 const nw4r::math::MTX34& rViewMtx) const {
    if (!rLightObj.IsEnable()) {
        return;
    }

    rLightObj.InitLightColor(mLightColor);

    nw4r::math::VEC3 dir;
    nw4r::math::VEC3 pos;

    switch (mLightType) {
    case cLightType_Dir: {
        nw4r::math::VEC3TransformNormal(&dir, &rViewMtx, &mViewDir);
        nw4r::math::VEC3Scale(&pos, &dir, -1e10f);
        dir.x = dir.y = dir.z = 0.0f;
        break;
    }
    case cLightType_Pos: {
        PSMTXMultVec(rViewMtx, mViewPos, pos);
        dir.x = dir.y = dir.z = 0.0f;
        break;
    }
    case cLightType_Spot: {
        PSMTXMultVec(rViewMtx, mViewPos, pos);
        nw4r::math::VEC3TransformNormal(&dir, &rViewMtx, &mViewDir);
        break;
    }
    }

    rLightObj.InitLightPos(pos.x, pos.y, pos.z);
    rLightObj.InitLightDir(dir.x, dir.y, dir.z);

    if (mFlags & 0x80) {
        rLightObj.InitLightAttnA(mAttnAA, mAttnAB, mAttnAC);
    } else {
        rLightObj.InitLightSpot(mCutoff > 0.0f ? mCutoff : 0.0001f, mSpotFn);
    }

    if (mFlags & 0x100) {
        rLightObj.InitLightAttnK(mAttnKA, mAttnKB, mAttnKC);
    } else {
#line 426
        EGG_ASSERT_MSG(0.0f < mRefBrightness && mRefBrightness < 1.0f, "ref brightness B must be (0 < B && B < 1)");
        rLightObj.InitLightDistAttn(mRefDistance * GetDistance(), mRefBrightness,
                                    mDistAttnFn);
    }

    if (mFlags & 0x800) {
        rLightObj.InitLightShininess(mShininess);
    }
}

void LightObject::CopyFromG3D(const nw4r::g3d::ResAnmLight& rAnmLight,
                              f32 frame) {
    nw4r::g3d::LightAnmResult result;
    rAnmLight.GetAnmResult(&result, frame);

    if (result.flags & nw4r::g3d::LightAnmResult::FLAG_LIGHT_ENABLE) {
        nw4r::g3d::LightAnmResult::LightType type =
            static_cast<nw4r::g3d::LightAnmResult::LightType>(
                result.flags &
                nw4r::g3d::LightAnmResult::FLAG_LIGHT_TYPE_MASK);
        Reset();

        switch (type) {
        case nw4r::g3d::LightAnmResult::LIGHTTYPE_POINT: {
            mLightType = cLightType_Pos;
            mBrightnessColor = result.color;
            SetPos(result.pos);
            CalcDirDist();
            SetDir(nw4r::math::VEC3(0.0f, 0.0f, 0.0f));
            mSpotFn = GX_SP_OFF;
            mCutoff = 0.0f;
            mRefDistance = result.refDistance;
            mRefBrightness = result.refBrightness;
            mDistAttnFn = result.distFunc;
            break;
        }
        case nw4r::g3d::LightAnmResult::LIGHTTYPE_DIRECTIONAL: {
            mLightType = cLightType_Dir;
            mBrightnessColor = result.color;
            SetPos(result.pos);
            CalcDirDist();
            SetAt(result.aim);
            CalcDirDist();
            mAttnAA = 1.0f;
            mAttnAB = 0.0f;
            mAttnAC = 0.0f;
            mFlags |= 0x80 | 0x100;
            mAttnKA = 1.0f;
            mAttnKB = 0.0f;
            mAttnKC = 0.0f;
            break;
        }
        case nw4r::g3d::LightAnmResult::LIGHTTYPE_SPOT: {
            mLightType = cLightType_Spot;
            mBrightnessColor = result.color;
            SetPos(result.pos);
            CalcDirDist();
            SetAt(result.aim);
            CalcDirDist();
            mSpotFn = result.spotFunc;
            mCutoff = result.cutoff;
            mDistAttnFn = result.distFunc;
            mRefDistance = result.refDistance;
            mRefBrightness = result.refBrightness;
            break;
        }
        default: {
#line 493
            EGG_ASSERT(0);
            break;
        }
        }

        if (result.flags & nw4r::g3d::LightAnmResult::FLAG_COLOR_ENABLE) {
            mFlags |= cFlag_9;
        } else {
            mFlags &= ~cFlag_9;
        }

        if (result.flags & nw4r::g3d::LightAnmResult::FLAG_ALPHA_ENABLE) {
            mFlags |= cFlag_10;
        } else {
            mFlags &= ~cFlag_10;
        }

        mFlags |= cFlag_0;
    } else {
        mFlags &= ~cFlag_0;
    }
}

void LightObject::CalcDirDist() {
    mDir.x = mAt.x - mPos.x;
    mDir.y = mAt.y - mPos.y;
    mDir.z = mAt.z - mPos.z;

    mDistance = nw4r::math::VEC3Len(&mDir);
    if (mDistance > 0.0f) {
        nw4r::math::VEC3Scale(&mDir, &mDir, 1.0f / mDistance);
    } else {
        mDir.x = mDir.y = mDir.z = 0.0f;
    }
}

void LightObject::CalcAt() {
    mAt.x = mPos.x + mDir.x * mDistance;
    mAt.y = mPos.y + mDir.y * mDistance;
    mAt.z = mPos.z + mDir.z * mDistance;
}

const char* LightObject::GetBinaryType() const {
    return "LOBJ";
}

void LightObject::SetBinaryInner(const Bin& rBin) {
    switch (rBin.header.version) {
    case 0: {
        mLightType = cLightType_Dir;
        mSpotFn = static_cast<GXSpotFn>(rBin.data.spotFn);
        mDistAttnFn = static_cast<GXDistAttnFn>(rBin.data.distAttnFn);
        mAmbIndex = rBin.data.ambLightIdx;
        mFlags = rBin.data.flags | 0x660;
        mViewMode = rBin.data.BYTE_0x12;
        mIntensity = rBin.data.intensity;
        SetPos(rBin.data.pos);
        CalcDirDist();
        SetAt(rBin.data.target);
        CalcDirDist();
        mBrightnessColor = rBin.data.color;
        mDarknessColor = rBin.data.COLOR_0x38;
        f32 cutoff = rBin.data.spotArg;
        f32 refDistance = rBin.data.distAttnArg0;
        f32 refBrightness = rBin.data.distAttnArg1;
        mCutoff = cutoff;
        mRefDistance = refDistance;
        mRefBrightness = refBrightness;
        break;
    }
    case 1: {
        mLightType = static_cast<LightType>(rBin.data.BYTE_0x13);
        mSpotFn = static_cast<GXSpotFn>(rBin.data.spotFn);
        mDistAttnFn = static_cast<GXDistAttnFn>(rBin.data.distAttnFn);
        mAmbIndex = rBin.data.ambLightIdx;
        mFlags = rBin.data.flags;
        mViewMode = rBin.data.BYTE_0x12;
        mIntensity = rBin.data.intensity;
        SetPos(rBin.data.pos);
        CalcDirDist();
        SetAt(rBin.data.target);
        CalcDirDist();
        mBrightnessColor = rBin.data.color;
        mDarknessColor = rBin.data.COLOR_0x38;
        f32 cutoff = rBin.data.spotArg;
        f32 refDistance = rBin.data.distAttnArg0;
        f32 refBrightness = rBin.data.distAttnArg1;
        mCutoff = cutoff;
        mRefDistance = refDistance;
        mRefBrightness = refBrightness;
        break;
    }
    }

    Calc();
}

void LightObject::GetBinaryInner(Bin* pData) const {
#line 624
    EGG_ASSERT(pData);

    pData->data.BYTE_0x13 = mLightType;
    pData->data.spotFn = mSpotFn;
    pData->data.distAttnFn = mDistAttnFn;
    pData->data.ambLightIdx = mAmbIndex;
    pData->data.flags = mFlags;
    pData->data.BYTE_0x12 = mViewMode;
    pData->data.intensity = mIntensity;
    pData->data.pos = mPos;
    pData->data.target = mAt;
    pData->data.color = mBrightnessColor;
    pData->data.COLOR_0x38 = mDarknessColor;
    f32 cutoff = mCutoff;
    f32 refDistance = mRefDistance;
    f32 refBrightness = mRefBrightness;
    pData->data.spotArg = cutoff;
    pData->data.distAttnArg0 = refDistance;
    pData->data.distAttnArg1 = refBrightness;
}

} // namespace EGG
