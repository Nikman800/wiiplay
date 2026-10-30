#include <types.h>
#include <Pack/RPSystem/RPSysNunchukCheckScene.h>

#pragma auto_inline off

// The sport the Nunchuk check scene (RPSportsMessageScene) was opened for; only that scene reads
// it. Its .sbss slot (8043C6B8) lies between RPPlayerSettingPanel::spInstance and
// RPPlayerConfirm::sInstance, so it belongs to this file.
RPSysNunchukCheckSportInfo* RPSysNunchukCheckScene::spSportInfo = NULL;

// Game id <-> menu slot helpers (Menu_gameIdToSlot / Menu_slotToGameId): a file of their own after
// RPPlayerSettingPanel; their table sits after that file's string pool, 8-aligned.

inline s32 PlayerSettingSlotToGameId(s32 slot) {
    if (slot == 9) {
        return -1;
    }
    const s32 games[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    return games[slot];
}

// Linked units call these two by their ledger names.
extern "C" s32 Menu_gameIdToSlot(s32 game) {
    if (game == -1) {
        return 9;
    }
    for (s32 i = 0; i < 9; i++) {
        if (game == PlayerSettingSlotToGameId(i)) {
            return i;
        }
    }
    return 9;
}

extern "C" s32 Menu_slotToGameId(s32 slot) {
    return PlayerSettingSlotToGameId(slot);
}
