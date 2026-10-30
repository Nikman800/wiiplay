// Shooting Range clay pigeon pool (RPDucClayMgr).
// Unit 0x80238D68-0x80238EAC (dtk/game_units.tsv). Functions in reverse address order.
// RPDucClayMgr::SpawnTarget, the file's last function, sits just before this unit's range.
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>

#include <Pack/RPParty/RPDucScene/RPDucClay.h>

RPDucClayMgr* RPDucClayMgr::sInstance;

void RPDucClayMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucTarget* RPDucClayMgr::AllocTarget() {
    return new RPDucClay();
}

RPDucClayMgr::~RPDucClayMgr() {}

void RPDucClayMgr::Load() {
    RPDucTargetMgr::Load();
}

RPDucTargetMgr* RPDucClayMgr::CreateInstance(s32 num) {
    RPDucClayMgr* pMgr = new RPDucClayMgr();
    sInstance = pMgr;
    pMgr->CreatePool(num);
    return pMgr;
}

// Wii Play: the manager file's last-defined function (lowest address of the unit)
RPDucTarget* RPDucClayMgr::SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg) {
    return sInstance->Spawn(rPos, arg);
}
