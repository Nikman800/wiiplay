// Shooting Range ammunition (RPDucAmmoMgr).
// Unit 0x802504E4-0x8025070C (dtk/game_units.tsv). Functions in reverse address order.
// The unit's first function (highest address) is RPDucCircleTargetMgr::SpawnTarget, the last
// function of the round target manager file that follows.
#include <Pack/RPParty/RPDucScene/RPDucGame.h>

#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>


RPDucAmmoMgr* RPDucAmmoMgr::sInstance;

void RPDucAmmoMgr::CreateInstance() {
    sInstance = new RPDucAmmoMgr();
}

void RPDucAmmoMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucAmmoMgr::~RPDucAmmoMgr() {}

void RPDucAmmoMgr::Setup(s16 playerNum) {
    mPlayerNum = playerNum;
    s32 i = 0;
    do {
        mAmmo[i] = 0;
        mShots[i] = 0;
        mTotalShots[i] = 0;
    } while (++i < mPlayerNum);
    mMaxAmmo = 99;
    mLimitedAmmo = false;
}

void RPDucAmmoMgr::ClearShotCounts() {
    s32 i = 0;
    do {
        mShots[i] = 0;
    } while (++i < mPlayerNum);
}

void RPDucAmmoMgr::Refill() {
    s32 i = 0;
    do {
        mAmmo[i] = mMaxAmmo;
    } while (++i < mPlayerNum);
}

s32 RPDucAmmoMgr::TryFire(s32 player) {
    switch (RPDucGame::sInstance->GetState()) {
    case 1:
    case 2:
        return 2;
    case 3:
    case 4:
        if (mAmmo[player] > 0) {
            if (mLimitedAmmo) {
                mAmmo[player]--;
            }
            mShots[player]++;
            mTotalShots[player]++;
            return 2;
        }
        return 1;
    default:
        return 0;
    }
}
