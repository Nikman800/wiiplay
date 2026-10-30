// Table Tennis CPU level and ball speed tables (leveltable.bin, rallyleveluptable(2).bin).
// Unit 0x80232C74-0x80232F0C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpLevelTable.h>

#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <cstring>

RPPnpLevelTable* RPPnpLevelTable::sInstance;

void RPPnpLevelTable::CreateInstance() {
    sInstance = new RPPnpLevelTable();
}

void RPPnpLevelTable::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPPnpLevelTable::RPPnpLevelTable() {
    u32 size;
    void* pFile = RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameCommonArchive(), "leveltable.bin", &size);
    LevelEntry* pLevels = reinterpret_cast<LevelEntry*>(new u8[size]);
    std::memcpy(pLevels, pFile, size);
    mpLevels = pLevels;
    mLevelNum = size / sizeof(LevelEntry);

    u32 size1;
    void* pFile1 = RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameCommonArchive(), "rallyleveluptable.bin", &size1);
    RallyEntry* pTable1 = reinterpret_cast<RallyEntry*>(new u8[size1]);
    std::memcpy(pTable1, pFile1, size1);
    mpRallyTables[0] = pTable1;
    mRallyNums[0] = size1 / sizeof(RallyEntry);

    u32 size2;
    void* pFile2 = RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameCommonArchive(), "rallyleveluptable2.bin", &size2);
    RallyEntry* pTable2 = reinterpret_cast<RallyEntry*>(new u8[size2]);
    std::memcpy(pTable2, pFile2, size2);
    mpRallyTables[1] = pTable2;
    mRallyNums[1] = size2 / sizeof(RallyEntry);
}

RPPnpLevelTable::~RPPnpLevelTable() {}

const RPPnpLevelTable::LevelEntry* RPPnpLevelTable::GetLevelEntry(s32 idx) const {
    return &mpLevels[idx];
}

void RPPnpLevelTable::LookupRally(s32 rally, f32* pBallSpeed, s32* pCpuLevel) const {
    s32 mode = RPPnpGame::GetInstance()->IsMultiPlayer();

    s32 i;
    for (i = 0; i < mRallyNums[mode]; i++) {
        if (rally == mpRallyTables[mode][i].mRally) {
            break;
        }

        if (rally < mpRallyTables[mode][i].mRally) {
            i--;
            break;
        }
    }

    if (i >= mRallyNums[mode]) {
        i = mRallyNums[mode] - 1;
    }

    if (pBallSpeed != NULL) {
        *pBallSpeed = mpRallyTables[mode][i].mBallSpeed;
    }

    if (pCpuLevel != NULL) {
        *pCpuLevel = mpRallyTables[mode][i].mCpuLevel;
    }
}
