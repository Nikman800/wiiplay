// Tanks game statistics (RPTnkGameStats): kills per player and enemy type, lives.
// Unit 0x8026DF6C-0x8026E350 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkGameManager.h>

#include <Pack/RPParty/RPTnkScene/RPTnkLyt.h>

#include <nw4r/ut.h>

RPTnkGameStats* RPTnkGameStats::sInstance;

void RPTnkGameStats::CreateInstance() {
    sInstance = new RPTnkGameStats();
}

void RPTnkGameStats::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkGameStats::~RPTnkGameStats() {}

void RPTnkGameStats::Setup(s16 playerNum) {
    mPlayerNum = playerNum;
    ClearPlayerCounts();
    ClearKills();
    ClearTypes();
    SetLives(1);
    ClearRecords();
}

void RPTnkGameStats::SetAllLives(s32 lives) {
    s32 i = 0;
    do {
        mLives[i] = lives;
    } while (++i < mPlayerNum);
}

void RPTnkGameStats::AddKill(s32 player, s32 type, s32 num) {
    mKills[player] += num;
    mKillsByType[player][type] += num;
    RPTnkLayoutManager::sInstance->GetHud()->PlayScoreAnim(player);
}

BOOL RPTnkGameStats::AddLives(s32 player, s32 num) {
    mLives[player] += num;
    mLives[player] = nw4r::ut::Min<s32>(nw4r::ut::Max<s32>(mLives[player], 0), 99);
    if (mLives[player] <= 0) {
        return TRUE;
    }
    return FALSE;
}

void RPTnkGameStats::AddKilledType(s32 type) {
    if (mTypeSeen[type]) {
        return;
    }
    mTypeSeen[type] = true;
    mTypeOrder[mTypeNum] = type;
    mTypeNum++;
}

void RPTnkGameStats::ClearCounts() {
    ClearPlayerCounts();
    ClearKills();
    ClearTypes();
    ClearRecords();
}

s32 RPTnkGameStats::GetWinner() const {
    if (mKills[0] > mKills[1]) {
        return 0;
    }
    if (mKills[0] < mKills[1]) {
        return 1;
    }
    return -1;
}
