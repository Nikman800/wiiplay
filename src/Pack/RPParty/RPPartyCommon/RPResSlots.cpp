#include <Pack/RPParty/RPPartyCommon/RPResSlots.h>

#include <Pack/RPGraphics/RPGrpModelResManager.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <cstring>

extern "C" {
RPResSlots* RP_resSlotsInstance;
}

static inline u16 RPResSlots_GetSlotNum(RPResSlots* pSlots) {
    return pSlots->mSlotNum;
}

static inline RPResSlots::Slot* RPResSlots_GetSlots(RPResSlots* pSlots) {
    return pSlots->mpSlots;
}

extern "C" void RP_ResSlots_createInstance() {
    RP_resSlotsInstance = new RPResSlots;
}

void RPResSlots::DestroyInstance() {
    delete static_cast<RPSceneObj*>(RP_resSlotsInstance);
    RP_resSlotsInstance = NULL;
}

RPResSlots::~RPResSlots() {}

extern "C" void RP_ResSlots_init(RPResSlots* pSlots, u32 slotNum) {
    pSlots->mSlotNum = slotNum;
    pSlots->mpResManager = RPGrpModelResManager::GetCurrent();
    pSlots->mpResManager->CreateResourceList(slotNum);
    pSlots->mpSlots = new RPResSlots::Slot[pSlots->mSlotNum];

    u16 index = 0;
    do {
        pSlots->mpSlots[index].pFile = NULL;
        pSlots->mpSlots[index].pResName = NULL;
        pSlots->mpSlots[index].kind = 0;
        pSlots->mpSlots[index].handle = 0;
        ++index;
    } while (index < pSlots->mSlotNum);
}

u16 RP_ResSlots_findOrLoad(RPResSlots* pSlots, const char* pResName,
                           u16 start, u32 kind, u32 arg) {
#pragma push
#pragma scheduling 604
    RPResSlots::Slot* pSlotList = pSlots->mpSlots;
    u16 index = 0;
    do {
        if (pSlotList[index].handle != 0) {
            if (std::strcmp(pSlotList[index].pResName, pResName) == 0) {
                return index;
            }
        }
        ++index;
    } while (index < RPResSlots_GetSlotNum(pSlots));

    index = start;
    for (;;) {
        if (pSlots->mpSlots[index].handle == 0) {
            if (static_cast<s32>(pSlots->mpSlots[index].handle) != 1) {
                EGG::Archive* pArchive =
                    RPSysResourceManager::LoadGameCommonArchive();
                pSlots->mpSlots[index].pFile =
                    RPSysResourceManager::GetFileFromArchive(pArchive, pResName);
                pSlots->mpSlots[index].pResName = pResName;
                pSlots->mpSlots[index].kind = kind;
                pSlots->mpSlots[index].handle = 1;
            }

            if (pSlots->mpSlots[index].handle != 0) {
                u32 createArg = 0;
                if (pSlots->mpSlots[index].kind == 1) {
                    createArg = arg;
                }
                pSlots->mpResManager->CreateDataAt(
                    index,
                    static_cast<RPGrpModelResManager::Type>(
                        pSlots->mpSlots[index].kind),
                    pSlots->mpSlots[index].pFile,
                    createArg);
            }
            return index;
        }

        ++index;
        if (index == static_cast<u16>(pSlots->mSlotNum)) {
            index = 0;
        }
        if (index == start) {
            return 999;
        }
    }
#pragma pop
}
