// Shooting Range balloon pool (RPDucBalloonMgr).
// Unit 0x80237AAC-0x80237BF0 (dtk/game_units.tsv). Functions in reverse address order.
// RPDucBalloonMgr::SpawnTarget, the file's last function, sits just before this unit's range.
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>

#include <Pack/RPParty/RPDucScene/RPDucBalloon.h>

RPDucBalloonMgr* RPDucBalloonMgr::sInstance;

void RPDucBalloonMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucTarget* RPDucBalloonMgr::AllocTarget() {
    return new RPDucBalloon();
}

RPDucBalloonMgr::~RPDucBalloonMgr() {}

void RPDucBalloonMgr::Load() {
    RPDucTargetMgr::Load();
}

RPDucTargetMgr* RPDucBalloonMgr::CreateInstance(s32 num) {
    RPDucBalloonMgr* pMgr = new RPDucBalloonMgr();
    sInstance = pMgr;
    pMgr->CreatePool(num);
    return pMgr;
}

// Wii Play: the manager file's last-defined function (lowest address of the unit)
RPDucTarget* RPDucBalloonMgr::SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg) {
    return sInstance->Spawn(rPos, arg);
}
