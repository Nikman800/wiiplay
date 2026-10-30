// Shooting Range can pool (RPDucCanMgr).
// Unit 0x8024B8D0-0x8024BA14 (dtk/game_units.tsv). Functions in reverse address order.
// RPDucCanMgr::SpawnTarget, the file's last function, sits just before this unit's range.
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>

#include <Pack/RPParty/RPDucScene/RPDucCan.h>

RPDucCanMgr* RPDucCanMgr::sInstance;

void RPDucCanMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucTarget* RPDucCanMgr::AllocTarget() {
    return new RPDucCan();
}

RPDucCanMgr::~RPDucCanMgr() {}

void RPDucCanMgr::Load() {
    RPDucTargetMgr::Load();
}

RPDucTargetMgr* RPDucCanMgr::CreateInstance(s32 num) {
    RPDucCanMgr* pMgr = new RPDucCanMgr();
    sInstance = pMgr;
    pMgr->CreatePool(num);
    return pMgr;
}

// Wii Play: the manager file's last-defined function (lowest address of the unit)
RPDucTarget* RPDucCanMgr::SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg) {
    return sInstance->Spawn(rPos, arg);
}
