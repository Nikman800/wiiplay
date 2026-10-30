// Shooting Range score keeping (RPDucScoreMgr): scores, hits, streaks and streak bonuses.
// Unit 0x8024FF80-0x802504E4 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucGame.h>

#include <Pack/RPParty/RPDucScene/RPDucHud.h>

#include <nw4r/ut.h>

RPDucScoreMgr* RPDucScoreMgr::sInstance;

void RPDucScoreMgr::CreateInstance() {
    sInstance = new RPDucScoreMgr();
}

void RPDucScoreMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucScoreMgr::~RPDucScoreMgr() {}

void RPDucScoreMgr::Setup(s16 playerNum) {
    mPlayerNum = playerNum;
    s32 i = 0;
    do {
        mScores[i] = 0;
        mHits[i] = 0;
        mStreaks[i] = 0;
        mMaxStreaks[i] = 0;
        mBonus[i] = 0;
        mBonusTimer[i] = 0;
        mLastColors[i] = -1;
        mCircleHits[i] = 0;
    } while (++i < mPlayerNum);
}

void RPDucScoreMgr::ClearScores() {
    s32 i = 0;
    do {
        mScores[i] = 0;
    } while (++i < mPlayerNum);
}

void RPDucScoreMgr::ClearHits() {
    s32 i = 0;
    do {
        mHits[i] = 0;
    } while (++i < mPlayerNum);
}

void RPDucScoreMgr::ResetStreak(s32 player) {
    if (RPDucGame::sInstance->GetMode() == 2) {
        mStreaks[0] = 0;
    } else {
        mStreaks[player] = 0;
    }
}

inline void RPDucScoreMgr::ClearBonuses() {
    s32 i = 0;
    do {
        mBonus[i] = 0;
        mBonusTimer[i] = 0;
    } while (++i < mPlayerNum);
}

void RPDucScoreMgr::ClearStreaks() {
    s32 i = 0;
    do {
        mMaxStreaks[i] = 0;
        mStreaks[i] = 0;
    } while (++i < mPlayerNum);

    ClearBonuses();
}

void RPDucScoreMgr::ClearLastColors() {
    s32 i = 0;
    do {
        mLastColors[i] = -1;
    } while (++i < mPlayerNum);
}

void RPDucScoreMgr::ClearCircleHits() {
    s32 i = 0;
    do {
        mCircleHits[i] = 0;
    } while (++i < mPlayerNum);
}

void RPDucScoreMgr::AddScore(s32 player, s32 points) {
    if (RPDucGame::sInstance->GetMode() == 2) {
        mScores[0] += points;
        if (mScores[0] < 0) {
            mScores[0] = 0;
        }
        if (mScores[0] > 999) {
            mScores[0] = 999;
        }
    } else {
        mScores[player] += points;
        if (mScores[player] < 0) {
            mScores[player] = 0;
        }
        if (mScores[player] > 999) {
            mScores[player] = 999;
        }
    }
}

void RPDucScoreMgr::AddHits(s32 player, s32 num) {
    if (RPDucGame::sInstance->GetMode() == 2) {
        mHits[0] += num;
    } else {
        mHits[player] += num;
    }
}

void RPDucScoreMgr::AddStreak(s32 player, s32 num) {
    if (RPDucGame::sInstance->GetMode() == 2) {
        mStreaks[0] += num;
        mStreaks[0] = nw4r::ut::Min<s32>(nw4r::ut::Max<s32>(mStreaks[0], 0), 999);
        mMaxStreaks[0] = (mMaxStreaks[0] <= mStreaks[0]) ? mStreaks[0] : mMaxStreaks[0];
        if (mStreaks[0] == 5) {
            RPDucGame::sInstance->PlayStreakSound();
        }
    } else {
        mStreaks[player] += num;
        mStreaks[player] = nw4r::ut::Min<s32>(nw4r::ut::Max<s32>(mStreaks[player], 0), 999);
        mMaxStreaks[player] = (mMaxStreaks[player] <= mStreaks[player]) ? mStreaks[player] : mMaxStreaks[player];
        if (mStreaks[player] == 5) {
            RPDucGame::sInstance->PlayStreakSound();
        }
    }
}

void RPDucScoreMgr::AddCircleHits(s32 player, s32 num) {
    if (RPDucGame::sInstance->GetMode() == 2) {
        mCircleHits[0] += num;
    } else {
        mCircleHits[player] += num;
    }
}

void RPDucScoreMgr::SetLastColor(s32 player, s32 color) {
    if (RPDucGame::sInstance->GetMode() == 2) {
        mLastColors[0] = color;
    } else {
        mLastColors[player] = color;
    }
}

bool RPDucScoreMgr::HasStreakBonus(s32 player) const {
    if (RPDucGame::sInstance->GetMode() == 2) {
        player = 0;
    }
    if (mStreaks[player] >= 5) {
        return true;
    }
    return false;
}

bool RPDucScoreMgr::AwardStreakBonus(s32 player) {
    bool awarded = false;
    if (RPDucGame::sInstance->GetMode() == 2) {
        player = 0;
    }
    if (mStreaks[player] >= 5) {
        mBonus[player] = mStreaks[player] - 4;
        mBonusTimer[player] = 180;
        AddScore(player, mBonus[player]);
        RPDucHud::sInstance->ShowBonus(player, mBonus[player]);
        RPDucGame::sInstance->PlayStreakBonusSound();
        awarded = true;
    }
    return awarded;
}

s32 RPDucScoreMgr::GetLeader() const {
    if (mScores[0] > mScores[1]) {
        return 0;
    }
    if (mScores[0] < mScores[1]) {
        return 1;
    }
    return -1;
}
