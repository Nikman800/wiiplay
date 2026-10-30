// Shooting Range round target pool (RPDucCircleTargetMgr) with its 32-slot placement grid.
// Unit 0x8025070C-0x802508A8 (dtk/game_units.tsv). Functions in reverse address order.
// RPDucCircleTargetMgr::SpawnTarget, the file's last function, sits just before this unit's range.
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>

#include <Pack/RPParty/RPDucScene/RPDucCircleTarget.h>

RPDucCircleTargetMgr* RPDucCircleTargetMgr::sInstance;

void RPDucCircleTargetMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucTarget* RPDucCircleTargetMgr::AllocTarget() {
    return new RPDucCircleTarget();
}

RPDucCircleTargetMgr::~RPDucCircleTargetMgr() {}

void RPDucCircleTargetMgr::Load() {
    RPDucTargetMgr::Load();
}

void RPDucCircleTargetMgr::Reset() {
    u32 i = 0;
    do {
        mGridSlotUsed[i] = false;
    } while (++i < 32);
    RPDucTargetMgr::Reset();
}

void RPDucCircleTargetMgr::ClearGridSlots() {
    u32 i = 0;
    do {
        mGridSlotUsed[i] = false;
    } while (++i < 32);
}

void RPDucCircleTargetMgr::SetGridSlotUsed(s32 slot, bool used) {
    mGridSlotUsed[slot] = used;
}

bool RPDucCircleTargetMgr::IsGridSlotUsed(s32 slot) const {
    return mGridSlotUsed[slot];
}

RPDucTargetMgr* RPDucCircleTargetMgr::CreateInstance(s32 num) {
    RPDucCircleTargetMgr* pMgr = new RPDucCircleTargetMgr();
    sInstance = pMgr;
    pMgr->CreatePool(num);
    return pMgr;
}

// Wii Play: the manager file's last-defined function (lowest address of the unit)
RPDucTarget* RPDucCircleTargetMgr::SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg) {
    return sInstance->Spawn(rPos, arg);
}
