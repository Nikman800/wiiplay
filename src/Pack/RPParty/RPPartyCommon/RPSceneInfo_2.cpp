// Needs `RPSceneInfo();` declared in RPSceneInfo.h (public, before the destructor); the
// constructor is inline here, so `new RPSceneInfo()` expands it under the new's null check.
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo_2.h>

#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPHkyScene/RPHkyPuck.h>
#include <Pack/RPKernel/RPSysMiddleDB.h>
#include <Pack/RPKernel/RPSysMiddleDBGenInfo.h>
#include <Pack/RPSystem/RPSysPlayerMgr.h>

void RP_getPairFromField1C(EGG::Vector2f* pOut, const RPHkyGame* pGame) {
    const RPHkyPuck* pPuck = pGame->GetPuck();
    f32 y = *(const f32*)((const u8*)pPuck + 0xA0);
    pOut->x = *(const f32*)((const u8*)pPuck + 0x90);
    pOut->y = y;
}

void RP_getPairFromField20(EGG::Vector2f* pOut, const RPHkyGame* pGame) {
    const RPHkyField* pField = pGame->GetField();
    f32 y = *(const f32*)((const u8*)pField + 0xA0);
    pOut->x = *(const f32*)((const u8*)pField + 0x90);
    pOut->y = y;
}

inline RPSceneInfo::RPSceneInfo() {
    RPSysMiddleDBGenInfo genInfo;
    genInfo.SetType(RFLMiddleDBType_Random);
    genInfo.SetSize(100);
    mpMiddleDB = new RPSysMiddleDB(&genInfo);
}

extern "C" void RP_createSceneInfo() {
    RPSceneInfo::spInstance = new RPSceneInfo();
}

void RPSceneInfo::DestroyInstance() {
    delete spInstance;
    spInstance = NULL;
}

void RPSceneInfo::Reset() {
    mPlayerNum = RPSysPlayerMgr::instance()->getPlayerNum();
    unk8[0] = 0;
    unk8[1] = 0;
    unk10 = 0;
    unk14 = -1;
    unk18 = 0;
    unk28 = 0;
    unk1C = 0;
    unk20 = 0;
    unk24 = 0;
    unk2C = true;
}

RPSceneInfo::~RPSceneInfo() {}

