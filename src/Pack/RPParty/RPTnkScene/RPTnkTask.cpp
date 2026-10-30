// Tanks fixed-rate step counter (RPTnkStepTimer).
// Unit 0x802688E8-0x80268A68 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkTask.h>

RPTnkStepTimer* RPTnkStepTimer::sInstance;

void RPTnkStepTimer::CreateInstance() {
    sInstance = new RPTnkStepTimer();
}

void RPTnkStepTimer::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkStepTimer::~RPTnkStepTimer() {}

void RPTnkStepTimer::Create() {}

void RPTnkStepTimer::Load() {}

void RPTnkStepTimer::Reset() {
    mSteps = 0;
    mRate = 0.0f;
    mAccum = 0.0f;
    mClear = true;
}

void RPTnkStepTimer::Start(f32 rate) {
    mRate = rate;
    mAccum = 0.0f;
    mSteps = 0;
    mClear = true;
}

void RPTnkStepTimer::Calculate() {
    mAccum += mRate;
    mSteps = mAccum;
    mAccum -= mSteps;
}
