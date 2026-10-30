// Position history (RPPosHistory). Unit 0x801BA474-0x801BAB3C.
#include <Pack/RPParty/RPPartyCommon/RPPosHistory.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>

#include <egg/core.h>
#include <egg/gfxe/eggDrawGX.h>

static inline s32 RPPosHistory_Min(s32 a, s32 b) {
    return b <= a ? b : a;
}

static inline void RPPosHistory_DrawQuad(const nw4r::math::MTX34& mtx,
                                         const GXColor& color) {
    EGG::DrawGX::DrawDL(EGG::DrawGX::DL_QUAD, mtx, color);
}

RPPosHistory::RPPosHistory(s32 channel, s32 capacity) {
    mChannel = channel;
    mCapacity = capacity;
    mpSamples = new nw4r::math::VEC3[capacity];
    Clear();
}

RPPosHistory::~RPPosHistory() {}

void RPPosHistory::Push(nw4r::math::VEC3 pos) {
    s32 i = mCapacity - 1;
    do {
        mpSamples[i] = mpSamples[i - 1];
    } while (--i > 0);
    mpSamples[0] = pos;
    mCount = RPPosHistory_Min(mCount + 1, mCapacity);
}

void RPPosHistory::Clear() {
    s32 i = 0;
    do {
        mpSamples[i].x = 0.0f;
        mpSamples[i].y = 0.0f;
        mpSamples[i].z = 0.0f;
    } while (++i < mCapacity);
    mCount = 0;
}

void RPPosHistory::PullFromController() {
    s32 num = RPPosHistory_Min(
        EGG::CoreControllerMgr::instance()->getNthController(mChannel)->getReadLength(), mCapacity);

    if (num != 0) {
        s32 i = num - 1;
        do {
            EGG::CoreStatus status;
            status = *EGG::CoreControllerMgr::instance()->getNthController(mChannel)->getCoreStatus(i);
            nw4r::math::VEC3 acc;
            acc.x = status.acc.x;
            acc.y = status.acc.y;
            acc.z = status.acc.z;
            Push(acc);
        } while (--i >= 0);
    }
}

void RPPosHistory::DrawDebug() {
    GXColor background = {0, 0, 0, 128};
    GXColor axisColor = {0, 0, 255, 255};
    GXColor sampleColor = {255, 255, 255, 255};
    nw4r::math::VEC3 scale(400.0f, 160.0f, 1.0f);

    EGG::DrawGX::BeginDrawQuad(EGG::DrawGX::COLORCHAN_COLOR, EGG::DrawGX::ZMODE_ALWAYS,
                             EGG::DrawGX::BLEND_NORMAL, false, false);
    nw4r::math::MTX34 mtx;
    RPDucMakeMtx(&mtx, RP_mtx33Identity, scale, RP_vecZero);
    RPPosHistory_DrawQuad(mtx, background);

    nw4r::math::VEC3 axis[2];
    axis[0].x = -200.0f;
    axis[0].y = 0.0f;
    axis[0].z = 0.0f;
    axis[1].x = 200.0f;
    axis[1].y = 0.0f;
    axis[1].z = 0.0f;
    EGG::DrawGX::BeginDrawLine(EGG::DrawGX::COLORCHAN_COLOR, EGG::DrawGX::ZMODE_ALWAYS);
    EGG::DrawGX::DrawLine(axis, 2, axisColor, 8);

    f32 step = scale.x / (f32)mCapacity;
    f32 height = scale.y / 4.0f;
    s32 i = 0;
    do {
        nw4r::math::VEC3 points[2];
        points[0].x = axis[1].x - step * (f32)i;
        points[0].y = height * mpSamples[i].z;
        points[0].z = 0.0f;
        points[1].x = axis[1].x - step * (f32)(i + 1);
        points[1].y = height * mpSamples[i + 1].z;
        points[1].z = 0.0f;
        EGG::DrawGX::DrawLine(points, 2, sampleColor, 8);
    } while (++i < mCapacity - 1);
}

