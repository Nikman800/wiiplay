#include "Pack/RPParty/RPPartyCommon/RPAnimButton.h"

void RPFrameOverrideTable::Clear() {
    mEntries[0].mEnabled = false;
    mEntries[0].mFrame = 0;
    mEntries[1].mEnabled = false;
    mEntries[1].mFrame = 0;
    mEntries[2].mEnabled = false;
    mEntries[2].mFrame = 0;
    mEntries[3].mEnabled = false;
    mEntries[3].mFrame = 0;
    mEntries[4].mEnabled = false;
    mEntries[4].mFrame = 0;
    mEntries[5].mEnabled = false;
    mEntries[5].mFrame = 0;
    mEntries[6].mEnabled = false;
    mEntries[6].mFrame = 0;
    mEntries[7].mEnabled = false;
    mEntries[7].mFrame = 0;
}

bool RPFrameOverrideTable::Has(s32 anim) const {
    return mEntries[anim].mEnabled;
}

s32 RPFrameOverrideTable::Get(s32 anim) const {
    if (mEntries[anim].mEnabled) {
        return mEntries[anim].mFrame;
    }
    return -1;
}
