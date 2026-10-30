#include <Pack/RPParty/RPTnkScene/RPTnkExGuide.h>

#include <Pack/RPParty/RPTnkScene/RPTnkTank.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTankManager.h>
#include <Pack/RPParty/RPTnkScene/RPTnkCursor.h>
#include <Pack/RPParty/RPTnkScene/RPTnkCamera.h>
#include <Pack/RPParty/RPTnkScene/RPTnkGameManager.h>
#include <Pack/RPParty/RPTnkScene/RPTnkLyt.h>

extern nw4r::math::VEC3 RP_vecZero;

RPTnkExGuide* RPTnkExGuide::sInstance;

// The original data pool retains this table before the constructor's four
// individual member-function constants.
static RPTnkExGuide::StateFunc sStateFuncs[4] = {
    &RPTnkExGuide::StateCheckMoved,
    &RPTnkExGuide::StateShowHint,
    &RPTnkExGuide::StateShowHintUntilNear,
    &RPTnkExGuide::StateCheckCursorNear
};

inline RPTnkTank* GetPlayerTank(s32 player) {
    return RPTnkTankManager::sInstance->mpPlayers[player];
}

inline nw4r::math::VEC2 ProjectToScreen(const nw4r::math::VEC3& pos) {
    nw4r::math::VEC2 screen;
    RPTnkCamera::sInstance->ProjectToScreen(&screen, &pos);
    return screen;
}

// Shows the hint message of a guide state
inline void ShowHint(s32 state) {
    switch (state) {
    case 0:
        RPTnkLayoutManager::sInstance->GetExMsg()->Show(0, -0.45f);
        break;
    case 1:
        RPTnkLayoutManager::sInstance->GetExMsg()->Show(0, -0.45f);
        break;
    case 2:
        RPTnkLayoutManager::sInstance->GetExMsg()->Show(1, -0.45f);
        break;
    case 3:
        RPTnkLayoutManager::sInstance->GetExMsg()->Show(2, -0.45f);
        break;
    }
}

void RPTnkExGuide::CreateInstance() {
    sInstance = new RPTnkExGuide;
}

void RPTnkExGuide::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkExGuide::~RPTnkExGuide() {}

void RPTnkExGuide::StateCheckMoved() {
    bool moving[2] = {false, false};
    f32 unused[2] = {0.0f, 0.0f};
    for (s32 i = 0; i < 2; i++) {
        RPTnkTank* tank = RPTnkTankManager::sInstance->mpPlayers[i];
        if (tank->mAlive == 0) {
            moving[i] = false;
        } else if (tank->mSpeed > 0.1f) {
            moving[i] = true;
            nw4r::math::VEC3 pos = tank->GetPos();
            nw4r::math::VEC3 delta(pos.x - mStartPos[i].x, 0.0f,
                                  pos.z - mStartPos[i].z);
            mMovedDistance[i] = PSVECMag(delta);
        } else {
            moving[i] = false;
        }
    }
    if (mMovedDistance[0] > 40.0f || mMovedDistance[1] > 40.0f) {
        mState = 1;
    } else if (moving[0] || moving[1]) {
        mIdleTimer = 0;
    } else if (++mIdleTimer > 0) {
        ShowHint(mState);
    }
}

void RPTnkExGuide::StateShowHint() {
    RPTnkTank* tank;
    f32 x[2] = {-1000.0f, -1000.0f};
    s32 i = 0;
    do {
        tank = GetPlayerTank(i);
        if (tank->mAlive != 0) {
            nw4r::math::VEC3 pos = tank->GetPos();
            x[i] = pos.x;
        }
    } while (++i < 2);
    if (x[0] >= 10.0f || x[1] >= 10.0f) {
        mState = 2;
    }
    ShowHint(mState);
}

void RPTnkExGuide::StateShowHintUntilNear() {
    RPTnkTank* tank;
    f32 x[2] = {-1000.0f, -1000.0f};
    f32 z[2] = {0.0f, 0.0f};
    s32 player = 0;
    do {
        tank = GetPlayerTank(player);
        if (tank->mAlive != 0) {
            nw4r::math::VEC3 pos = tank->GetPos();
            x[player] = pos.x;
            z[player] = pos.z;
        }
    } while (++player < 2);
    if (x[0] < 10.0f && x[1] < 10.0f) {
        mState = 1;
        return;
    }
    ShowHint(mState);
    nw4r::math::VEC2 cursor(0.0f, 0.0f);
    s32 i;
    bool near = false;
    for (i = 0; i < 2; i++) {
        if (RPTnkTankManager::sInstance->mpPlayers[i]->mAlive != 0 && x[i] >= 10.0f) {
            nw4r::math::VEC2 delta;
            nw4r::math::VEC2 pos = RPTnkCursorManager::sInstance->mpCursors[i]->mScreenPos;
            cursor = pos;
            delta.x = cursor.x - mScreenPos.x;
            delta.y = cursor.y - mScreenPos.y;
            if (30.0f > nw4r::math::VEC2Len(&delta)) {
                near = true;
            }
        }
    }
    if (near) {
        mState = 3;
    }
}

void RPTnkExGuide::StateCheckCursorNear() {
    RPTnkTank* tank;
    f32 x[2] = {-1000.0f, -1000.0f};
    f32 z[2] = {0.0f, 0.0f};
    s32 player = 0;
    do {
        tank = GetPlayerTank(player);
        if (tank->mAlive != 0) {
            nw4r::math::VEC3 pos = tank->GetPos();
            x[player] = pos.x;
            z[player] = pos.z;
        }
    } while (++player < 2);
    if (x[0] < 10.0f && x[1] < 10.0f) {
        mState = 1;
        return;
    }
    nw4r::math::VEC2 cursor(0.0f, 0.0f);
    s32 i;
    bool near = false;
    for (i = 0; i < 2; i++) {
        if (RPTnkTankManager::sInstance->mpPlayers[i]->mAlive != 0 && x[i] >= 10.0f) {
            nw4r::math::VEC2 delta;
            nw4r::math::VEC2 pos = RPTnkCursorManager::sInstance->mpCursors[i]->mScreenPos;
            cursor = pos;
            delta.x = cursor.x - mScreenPos.x;
            delta.y = cursor.y - mScreenPos.y;
            if (30.0f > nw4r::math::VEC2Len(&delta)) {
                near = true;
            }
        }
    }
    if (!near) {
        mState = 2;
    } else {
        ShowHint(mState);
    }
}

void RPTnkExGuide::Create() {}

void RPTnkExGuide::Reset() {
    mStartPos[0] = RP_vecZero;
    mStartPos[1] = RP_vecZero;
    mScreenPos.x = 0.0f;
    mScreenPos.y = 0.0f;
    mDone = false;
    mState = 0;
    mMovedDistance[0] = 0.0f;
    mMovedDistance[1] = 0.0f;
    mIdleTimer = 0;
}

void RPTnkExGuide::Calculate() {
    if (!mDone) {
        (this->*mStateFuncs[mState])();
    }
    if (mpTarget != NULL) {
        RPTnkTank* target = mpTarget;
        nw4r::math::VEC3 pos = target->GetPos();
        pos.y += 11.0f * target->mScale.y;
        mScreenPos = ProjectToScreen(pos);
    }
}

void RPTnkExGuide::Exit() {}

void RPTnkExGuide::Start() {
    nw4r::math::VEC3 pos = RPTnkTankManager::sInstance->mpPlayers[0]->GetPos();
    mStartPos[0] = pos;
    if (RPTnkGameManager::sInstance->GetModeIdx() == 1) {
        nw4r::math::VEC3 pos = RPTnkTankManager::sInstance->mpPlayers[1]->GetPos();
        mStartPos[1] = pos;
    }
    mDone = false;
    mState = 0;
    mMovedDistance[0] = 0.0f;
    mMovedDistance[1] = 0.0f;
    mIdleTimer = 0;
}

