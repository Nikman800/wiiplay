// Wii Remote cursor trail (RPCursorTrail). Unit 0x801BAB3C-0x801BAFC0 (dtk/game_units.tsv).
#include <Pack/RPParty/RPPartyCommon/RPCursorTrail.h>

#include <egg/core.h>

static inline s32 RPCursorTrail_Min(s32 a, s32 b) {
    return b <= a ? b : a;
}

RPCursorTrail::RPCursorTrail(s32 channel, s32 sampleNum) {
    mChannel = channel;
    mSampleNum = sampleNum;
    mpFlags = new s8[mSampleNum];
    mpPos = new nw4r::math::VEC2[mSampleNum];
    mpDir = new nw4r::math::VEC2[mSampleNum];
    mpScale = new f32[mSampleNum];
}

RPCursorTrail::~RPCursorTrail() {}

void RPCursorTrail::Reset() {
    s32 i = 0;
    do {
        mpFlags[i] = -1;
        mpPos[i].x = 0.0f;
        mpPos[i].y = 0.0f;
        mpDir[i].x = 1.0f;
        mpDir[i].y = 0.0f;
        mpScale[i] = 1.0f;
    } while (++i < mSampleNum);
}

void RPCursorTrail::Update() {
    mLatest = -1;
    mLatestDir = -1;
    mLatestScale = -1;

    EGG::CoreStatus status =
        *EGG::CoreControllerMgr::instance()->getNthController(mChannel)->getCoreStatus();

    s32 num = RPCursorTrail_Min(
        EGG::CoreControllerMgr::instance()->getNthController(mChannel)->getReadLength(), mSampleNum);

    if (num != 0) {
        s32 i = 0;
        do {
            status = *EGG::CoreControllerMgr::instance()->getNthController(mChannel)->getCoreStatus(i);
            mpFlags[i] = status.dpd_valid_fg;
            if (mLatest == -1 && mpFlags[i] > 0) {
                mLatest = i;
            }
            if (mLatestDir == -1 && mpFlags[i] >= 2) {
                mLatestDir = i;
            }
            if (mLatestScale == -1 && mpFlags[i] >= 2) {
                mLatestScale = i;
            }
            mpPos[i].x = status.pos.x;
            mpPos[i].y = status.pos.y;
            mpDir[i].x = status.horizon.x;
            mpDir[i].y = status.horizon.y;
            mpScale[i] = status.dist;
        } while (++i < num);

        if (mLatest == -1 || mLatestDir == -1 || mLatestScale == -1) {
            s32 j = 0;
            do {
                if (mpFlags[j] == -1) {
                    if (mLatestDir == -1) {
                        mLatestDir = j;
                    }
                    if (mLatestScale == -1) {
                        mLatestScale = j;
                    }
                    break;
                }
            } while (++j < num);
        }
    }
}

