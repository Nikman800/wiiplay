// Tanks cursor manager (RPTnkCursorManager): one aiming cursor per player.
// Unit 0x8025DD58-0x8025E200 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkCursor.h>

#include <Pack/RPParty/RPTnkScene/RPTnkGameManager.h>

RPTnkCursorManager* RPTnkCursorManager::sInstance;

void RPTnkCursorManager::CreateInstance() {
    sInstance = new RPTnkCursorManager();
}

void RPTnkCursorManager::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkCursorManager::~RPTnkCursorManager() {}

void RPTnkCursorManager::CreateCursors(s32 num) {
    mCursorNum = num;
    s32 i = 0;
    do {
        mpCursors[i] = new RPTnkCursor();
        mpCursors[i]->Create();
        mpCursors[i]->CreateParts(static_cast<u16>(i));
    } while (++i < mCursorNum);
}

void RPTnkCursorManager::Load() {}

void RPTnkCursorManager::Reset() {
    s32 i;

    i = mCursorNum - 1;
    do {
        mpCursors[i]->Reset();
    } while (--i >= 0);

    i = mCursorNum - 1;
    do {
        mpCursors[i]->RestartLineEffect();
    } while (--i >= 0);

    i = mCursorNum - 1;
    do {
        mpCursors[i]->RestartTailEffect();
    } while (--i >= 0);

    i = mCursorNum - 1;
    do {
        mpCursors[i]->RestartMainEffect();
    } while (--i >= 0);

    i = mCursorNum - 1;
    do {
        mpCursors[i]->SetDisplayMode(0);
    } while (--i >= 0);

    if (RPTnkGameManager::sInstance->GetModeIdx() == 0) {
        mActiveNum = 1;
    } else if (RPTnkGameManager::sInstance->GetModeIdx() == 1) {
        mActiveNum = 2;
    }

    i = mActiveNum - 1;
    do {
        mpCursors[i]->SetDisplayMode(1);
    } while (--i >= 0);
}

void RPTnkCursorManager::Calculate() {
    s32 i = 0;
    do {
        mpCursors[i]->Calculate();
    } while (++i < mActiveNum);
}

void RPTnkCursorManager::Exit() {
    s32 i = 0;
    do {
        mpCursors[i]->Exit();
    } while (++i < mCursorNum);
}

void RPTnkCursorManager::Draw() {
    s32 i = 0;
    do {
        mpCursors[i]->Draw();
    } while (++i < mActiveNum);
}

void RPTnkCursorManager::SetDisplayModeAll(s32 mode) {
    s32 i = mActiveNum - 1;
    do {
        mpCursors[i]->SetDisplayMode(mode);
    } while (--i >= 0);
}
