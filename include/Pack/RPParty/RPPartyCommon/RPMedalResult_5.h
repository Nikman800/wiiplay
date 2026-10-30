#ifndef RP_PARTY_COMMON_MEDAL_RESULT_5_H
#define RP_PARTY_COMMON_MEDAL_RESULT_5_H

#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPResultMenu.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>

// Access the result fields without changing the shared scene/state-machine headers.
inline u32& ResultMenuSceneWins(RPSceneInfo* pInfo, s32 player) {
    return reinterpret_cast<u32*>(pInfo)[2 + player];
}

inline s32& ResultMenuSceneStreak(RPSceneInfo* pInfo) {
    return reinterpret_cast<s32*>(pInfo)[4];
}

inline s32& ResultMenuSceneWinner(RPSceneInfo* pInfo) {
    return reinterpret_cast<s32*>(pInfo)[5];
}

inline void ResultMenuClearRetryFlag(RPSceneInfo* pInfo) {
    reinterpret_cast<bool*>(pInfo)[0x2C] = false;
}

inline s32 ResultMenuPreviousState(const RPResultMenuSeq* pSeq) {
    return reinterpret_cast<const s32*>(pSeq)[1];
}

#endif
