#ifndef RP_PARTY_COMMON_ANIM_BUTTON_3_H
#define RP_PARTY_COMMON_ANIM_BUTTON_3_H

#include <Pack/RPParty/RPPartyCommon/RPAnimButton.h>

inline void ClearAnimButtonEntries(RPAnimButtonTable::Entry* pEntries) {
    for (int i = 0; i < RPAnimButtonTable::ENTRY_MAX; i++) {
        pEntries[i].mpButton = NULL;
    }
}

inline bool AnimButtonTableHitTest(RPAnimButton* const volatile& rpButton,
                                  const nw4r::math::VEC2& rPos, RPSysLayout* pLayout) {
    // Preserve the original word copy and reload the button after both stores.
    // The enum keeps the first word in the original register.
    enum PointWord { POINT_WORD_ZERO = 0 };
    nw4r::math::VEC2 pos;
    PointWord x = reinterpret_cast<const PointWord*>(&rPos)[0];
    u32 y = reinterpret_cast<const u32*>(&rPos)[1];
    reinterpret_cast<volatile u32*>(&pos)[0] = x;
    reinterpret_cast<volatile u32*>(&pos)[1] = y;
    return rpButton->HitTest(pos, pLayout);
}

#endif
