// Shooting Range UFO pool (RPDucUfoMgr).
// Unit 0x8023EC0C-0x8023ED50 (dtk/game_units.tsv). Functions in reverse address order.
// RPDucUfoMgr::SpawnTarget, the file's last function, sits just before this unit's range.
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>

#include <Pack/RPParty/RPDucScene/RPDucUfo.h>

RPDucUfoMgr* RPDucUfoMgr::sInstance;

void RPDucUfoMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucTarget* RPDucUfoMgr::AllocTarget() {
    return new RPDucUfo();
}

RPDucUfoMgr::~RPDucUfoMgr() {}

void RPDucUfoMgr::Load() {
    RPDucTargetMgr::Load();
}

RPDucTargetMgr* RPDucUfoMgr::CreateInstance(s32 num) {
    RPDucUfoMgr* pMgr = new RPDucUfoMgr();
    sInstance = pMgr;
    pMgr->CreatePool(num);
    return pMgr;
}

// Wii Play: the manager file's last-defined function (lowest address of the unit)
RPDucTarget* RPDucUfoMgr::SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg) {
    return sInstance->Spawn(rPos, arg);
}
