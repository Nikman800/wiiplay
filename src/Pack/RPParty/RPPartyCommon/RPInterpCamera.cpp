#include <Pack/RPParty/RPPartyCommon/RPInterpCamera.h>

RPRange::RPRange() {
    mEaseType = 0;
    mDuration = 0;
    mFrame = 0;
}

void RPRange::Start(s32 easeType, s32 duration) {
    mEaseType = easeType;
    mDuration = duration;
    mFrame = 0;
}

void RPRange::Step() {
    mFrame++;
    if (mFrame > mDuration) {
        mFrame = mDuration;
    }
}

f32 RPRange::GetValue() {
    return CalcEase((f32)mFrame / (f32)mDuration, mEaseType);
}

bool RPRange::IsEnd() const {
    return mDuration - mFrame == 0;
}

f32 RPRange::CalcEase(f32 t, s32 easeType) {
    if (t < 0.0f) {
        t = 0.0f;
    }
    if (t > 1.0f) {
        t = 1.0f;
    }

    switch (easeType) {
    case 0:
        return t;
    case 1:
        return EGG::Math<f32>::sin(t * EGG::Math<f32>::pi() * 0.5f);
    case 2:
        return t * t;
    case 3:
        return EGG::Math<f32>::sqrt(t);
    case 4:
        return (1.0f - EGG::Math<f32>::cos(t * EGG::Math<f32>::pi())) * 0.5f;
    default:
        return 0.0f;
    }
}
