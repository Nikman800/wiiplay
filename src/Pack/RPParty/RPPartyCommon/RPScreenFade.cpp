#include <Pack/RPParty/RPPartyCommon/RPScreenFade.h>

s32 RPScreenFade::sFadeFrames = 20;
GXColor RPScreenFade::sTargetColor = {200, 200, 200, 200};
bool RPScreenFade::sReloadColor;

RPScreenFade::~RPScreenFade() {
}

RPScreenFade::RPScreenFade() : IRPSysHostIOSocket() {
}

void RPScreenFade::Reset() {
    const GXColor& targetColor = sTargetColor;
    mFadeIn = false;
    mTimer = 0;

    GXColor color = {0xFF, 0xFF, 0xFF, 0x00};
    mColor = color;

    mTargetColor.r = targetColor.r;
    mTargetColor.g = targetColor.g;
    mTargetColor.b = targetColor.b;
    mTargetColor.a = targetColor.a;
}

void RPScreenFade::Calculate() {
    if (sReloadColor) {
        mTargetColor.r = sTargetColor.r;
        mTargetColor.g = sTargetColor.g;
        mTargetColor.b = sTargetColor.b;
        mTargetColor.a = sTargetColor.a;
    }

    if (mFadeIn) {
        mTimer++;
    } else {
        mTimer--;
    }

    s32 timer = mTimer;
    s32 frames = sFadeFrames;
    if (timer < 0) {
        timer = 0;
    } else if (timer > frames) {
        timer = frames;
    }

    mTimer = timer;
    mColor.r = 0xFF - timer * (0xFF - mTargetColor.r) / sFadeFrames;
    mColor.g = 0xFF - timer * (0xFF - mTargetColor.g) / sFadeFrames;
    mColor.b = 0xFF - timer * (0xFF - mTargetColor.b) / sFadeFrames;
    mColor.a = timer * mTargetColor.a / sFadeFrames;
}
