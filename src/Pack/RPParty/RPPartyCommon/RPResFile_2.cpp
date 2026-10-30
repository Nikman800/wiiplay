#include <Pack/RPParty/RPPartyCommon/RPResFile_2.h>

static inline RPResEntry* RPResFile_GetEntry(RPResData* pData, u16 i) {
    return &pData->pEntries[i];
}

static inline bool RPResFile_IsMatch(u8 type, s32 filter) {
    bool match = false;
    if (type == 0 || type == filter) {
        match = true;
    }
    return match;
}

int RPResFile::CountFilteredEntries() const {
    RPResData* pData = static_cast<RPResData*>(mpData);
    u16 n = pData->numEntries;
    u16 num = 0;

    for (u16 i = 0; i < n; i++) {
        if (RPResFile_IsMatch(RPResFile_GetEntry(pData, i)->type, mFilter)) {
            num++;
        }
    }

    return num;
}

void* RPResFile::GetFilteredEntry(u16 idx) const {
    RPResData* pData = static_cast<RPResData*>(mpData);
    RPResEntry* pFound = NULL;
    u16 num = 0;

    for (u16 i = 0; i < pData->numEntries; i++) {
        RPResEntry* pEntry = RPResFile_GetEntry(pData, i);
        if (RPResFile_IsMatch(pEntry->type, mFilter)) {
            if (num == idx) {
                pFound = pEntry;
                break;
            }
            num++;
        }
    }

    return pFound;
}
