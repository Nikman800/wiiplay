// Shooting Range duck pool (RPDucDuckMgr).
// Unit 0x80239ECC-0x8023A010 (dtk/game_units.tsv). Functions in reverse address order.
// RPDucDuckMgr::SpawnTarget, the file's last function, sits just before this unit's range.
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>

#include <Pack/RPParty/RPDucScene/RPDucDuck.h>

RPDucDuckMgr* RPDucDuckMgr::sInstance;

void RPDucDuckMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucTarget* RPDucDuckMgr::AllocTarget() {
    return new RPDucDuck();
}

RPDucDuckMgr::~RPDucDuckMgr() {}

void RPDucDuckMgr::Load() {
    RPDucTargetMgr::Load();
}

RPDucTargetMgr* RPDucDuckMgr::CreateInstance(s32 num) {
    RPDucDuckMgr* pMgr = new RPDucDuckMgr();
    sInstance = pMgr;
    pMgr->CreatePool(num);
    return pMgr;
}

// Wii Play: the manager file's last-defined function (lowest address of the unit)
RPDucTarget* RPDucDuckMgr::SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg) {
    return sInstance->Spawn(rPos, arg);
}
