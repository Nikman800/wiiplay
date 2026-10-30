#include <Pack/RPParty/RPPartyCommon/RPLeadCursor_2.h>

#include <Pack/RPKernel/RPSysController.h>

#include <egg/core/eggController.h>

// Debug override of both guide timers (RPLeadCursor::sDebugTimer / sUseDebugTimer)
s32 RPLeadCursor::sDebugTimer = 120;
bool RPLeadCursor::sUseDebugTimer = false;

RPLeadCursor::~RPLeadCursor() {}

RPLeadCursor::RPLeadCursor(s32 playerNum) : mPlayerNum(playerNum) {
    mpLayout = new RPLeadCursorLyt(playerNum);
}

void RPLeadCursor::Load() {
    mpLayout->Load();
}

void RPLeadCursor::Reset() {
    mCounters[0][0] = 0;
    mCounters[0][1] = 0;
    mCounters[1][0] = 0;
    mCounters[1][1] = 0;
    mTimers[0] = 120;
    mTimers[1] = 120;
    mEnabled[0] = false;
    mEnabled[1] = false;

    mpLayout->Reset();
}

void RPLeadCursor::Calculate() {
    if (sUseDebugTimer) {
        s32 timer = sDebugTimer;
        mTimers[0] = timer;
        mTimers[1] = timer;
    }

    for (s32 player = 0; player < mPlayerNum; player++) {
        if (mEnabled[player] != false) {
            if (RP_GET_CORE_CTRL(player)->mAccVYType == 0) {
                mCounters[player][0] = 0;
                mCounters[player][1] = 0;
            } else if (RP_GET_CORE_CTRL(player)->mAccVYType == 1) {
                mCounters[player][0] = 0;
                mCounters[player][1]++;
            } else if (RP_GET_CORE_CTRL(player)->mAccVYType == 2) {
                mCounters[player][0]++;
                mCounters[player][1] = 0;
            }

            mCounters[player][0] = EGG::Math<s32>::min(mCounters[player][0], mTimers[0] + 1);
            mCounters[player][1] = EGG::Math<s32>::min(mCounters[player][1], mTimers[1] + 1);

            for (s32 side = 0; side < RPLeadCursorLyt::SIDE_NUM; side++) {
                if (mCounters[player][side] >= mTimers[side] + 1) {
                    mpLayout->Show(player, side);
                } else if (mCounters[player][side] == 0) {
                    mpLayout->Hide(player, side);
                }
            }
        } else {
            for (s32 side = 0; side < RPLeadCursorLyt::SIDE_NUM; side++) {
                mCounters[player][side] = 0;
                mpLayout->Hide(player, side);
            }
        }
    }

    mpLayout->Calculate();
}

void RPLeadCursor::Draw() {
    mpLayout->Draw();
}

void RPLeadCursor::SetTimers(s32 frames) {
    mTimers[0] = frames;
    mTimers[1] = frames;
}

void RPLeadCursor::SetTimer(s32 frames, s32 side) {
    mTimers[side] = frames;
}

void RPLeadCursor::SetAllEnabled(bool enabled) {
    for (s32 player = 0; player < mPlayerNum; player++) {
        mEnabled[player] = enabled;
    }
}

void RPLeadCursor::SetPlayerEnabled(s32 player, bool enabled) {
    mEnabled[player] = enabled;
}
