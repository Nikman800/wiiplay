#include <Pack/RPParty/RPBomScene/RPBomCursor.h>

#include <Pack/RPParty/RPBomScene/RPBomGameManager.h>

void RPBomCursorManager::CreateInstance() {
    sInstance = new RPBomCursorManager();
}

void RPBomCursorManager::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBomCursorManager::~RPBomCursorManager() {
}

void RPBomCursorManager::CreateCursors(s32 num) {
    mCursorNum = num;

    s32 controller = 0;
    do {
        RPBomCursor* newCursor = new RPBomCursor();
        mpCursors[controller] = newCursor;
        newCursor->Create();
        mpCursors[controller]->SetControllerIndex(static_cast<u16>(controller));
        controller++;
    } while (controller < mCursorNum);
}

void RPBomCursorManager::Load() {}

void RPBomCursorManager::Reset() {
    s32 index = mCursorNum - 1;
    do {
        mpCursors[index]->Reset();
        index--;
    } while (index >= 0);

    if (RPBomGameManager::sInstance->mMode == 0) {
        mActiveNum = 1;
    } else if (RPBomGameManager::sInstance->mMode == 1) {
        mActiveNum = 2;
    }
}

void RPBomCursorManager::Calculate() {
    s32 index = 0;
    do {
        mpCursors[index]->Calculate();
        index++;
    } while (index < mActiveNum);
}

void RPBomCursorManager::Exit() {
    s32 index = 0;
    do {
        mpCursors[index]->Exit();
        index++;
    } while (index < mCursorNum);
}

void RPBomCursorManager::Draw() {
    s32 index = 0;
    do {
        mpCursors[index]->Draw();
        index++;
    } while (index < mActiveNum);
}
