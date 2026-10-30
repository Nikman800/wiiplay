#include <Pack/RPParty/RPPartyCommon/RPAnimButton_3.h>

RPAnimButtonTable::RPAnimButtonTable() {
    ClearAnimButtonEntries(mEntries);
}

void RPAnimButtonTable::VF_0x8() {}

void RPAnimButtonTable::Add(s32 id, RPAnimButton* pButton) {
    int index = -1;
    for (int i = 0; i < ENTRY_MAX; i++) {
        if (mEntries[i].mpButton == NULL) {
            index = i;
            break;
        }
    }
    mEntries[index].mpButton = pButton;
    mEntries[index].mId = id;
}

// The search shared by PlayAnimById, GetPaneById and GetAnimLengthById.
static inline int FindAnimButtonEntry(const RPAnimButtonTable::Entry* pEntries, s32 id) {
    int index = -1;
    int i;
    for (i = 0; i < RPAnimButtonTable::ENTRY_MAX; i++) {
        if (id == pEntries[i].mId && pEntries[i].mpButton != NULL) {
            index = i;
            break;
        }
    }
    return index;
}

void RPAnimButtonTable::PlayAnimById(s32 id, s32 anim, RPSysLayout* pLayout) {
    mEntries[FindAnimButtonEntry(mEntries, id)].mpButton->PlayAnim(anim, pLayout);
}

s32 RPAnimButtonTable::HitTest(const nw4r::math::VEC2& rPos,
                             RPSysLayout* pLayout) const {
    for (int i = 0; i < ENTRY_MAX; i++) {
        if (mEntries[i].mpButton != NULL) {
            if (AnimButtonTableHitTest(mEntries[i].mpButton, rPos, pLayout) == true) {
                return mEntries[i].mId;
            }
        }
    }
    return -1;
}

nw4r::lyt::Pane* RPAnimButtonTable::GetPaneById(s32 id) const {
    int index = FindAnimButtonEntry(mEntries, id);
    if (index == -1) {
        return NULL;
    }
    return mEntries[index].mpButton->GetPane();
}

s32 RPAnimButtonTable::GetAnimLengthById(s32 id) const {
    int index = FindAnimButtonEntry(mEntries, id);
    if (index == -1) {
        return 0;
    }
    return mEntries[index].mpButton->GetCurrentAnimLength();
}
