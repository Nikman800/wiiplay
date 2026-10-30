// Doll manager (RPDollMgr): one RPDoll per player on the game-select screen.
// Unit 0x801F4C2C-0x801F527C. Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPDoll.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpDrawPathXluSnap.h>

inline RPDollMgr::RPDollMgr(s32 playerNum) {
    mPlayerNum = playerNum;
    for (s32 i = 0; i < mPlayerNum; i++) {
        mpDolls[i] = new RPDoll(mPlayerNum, i);
    }
}

void RPDollMgr::CreateInstance(s32 playerNum) {
    sInstance = new RPDollMgr(playerNum);
}

RPDollMgr::~RPDollMgr() {}

void RPDollMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

void RPDollMgr::Load() {
    for (s32 i = 0; i < mPlayerNum; i++) {
        mpDolls[i]->Load();
    }
}

void RPDollMgr::Reset() {
    for (s32 i = 0; i < mPlayerNum; i++) {
        mAlpha = 255;
        mAlphaStep = 0;
        mpDolls[i]->Reset();
    }
}

void RPDollMgr::Calculate() {
    mAlpha += mAlphaStep;
    s32 alpha = mAlpha;
    if (alpha < 0) {
        alpha = 0;
    } else if (alpha > 255) {
        alpha = 255;
    }
    mAlpha = alpha;

    EGG::DrawPathXluSnap* pXluSnap =
        RPGrpModelManager::GetCurrent()->GetDrawPathManager(1)->GetDrawPathXluSnap()->GetXluSnap();
    pXluSnap->setAlpha(mAlpha);

    for (s32 i = 0; i < mPlayerNum; i++) {
        mpDolls[i]->Calculate();
    }
}

RPSysKokeshi* RPDollMgr::GetAvatar(s32 player) const {
    return mpDolls[player]->GetKokeshi();
}

bool RPDollMgr::RequestReaction(s32 reaction, s32 player) {
    s32 num = 1;
    if (player == -1) {
        num = mPlayerNum;
        player = 0;
    }

    bool result = true;
    for (s32 i = player; i < player + num; i++) {
        if (!mpDolls[i]->RequestReaction(reaction)) {
            result = false;
        }
    }
    return result;
}

bool RPDollMgr::AreAllAsleep() const {
    for (s32 i = 0; i < mPlayerNum; i++) {
        if (!mpDolls[i]->mpStateMachine->IsStateSettled(7)) {
            return false;
        }
    }
    return true;
}

void RPDollMgr::SetIdleExitAllowed(bool allowed) {
    for (s32 i = 0; i < mPlayerNum; i++) {
        mpDolls[i]->mIdleExitAllowed = allowed;
    }
}

void RPDollMgr::SetDozeAllowed(bool allowed) {
    for (s32 i = 0; i < mPlayerNum; i++) {
        mpDolls[i]->mDozeAllowed = allowed;
    }
}

void RPDollMgr::SetDollScreenPos(const nw4r::math::VEC2& rPos, s32 player) {
    mpDolls[player]->SetScreenPos(rPos);
}

const nw4r::math::VEC3& RPDollMgr::GetDollPos(s32 player) const {
    return mpDolls[player]->mHeadPos;
}

void RPDollMgr::ClearLook(s32 player) {
    if (player == -1) {
        for (s32 i = 0; i < mPlayerNum; i++) {
            if (mpDolls[i]->mpStateMachine->IsStateSettled(0)) {
                mpDolls[i]->ClearLook();
            }
        }
    } else {
        if (mpDolls[player]->mpStateMachine->IsStateSettled(0)) {
            mpDolls[player]->ClearLook();
        }
    }
}

void RPDollMgr::LookAt(const nw4r::math::VEC2& rPos, s32 player) {
    s32 num = 1;
    if (player == -1) {
        num = mPlayerNum;
        player = 0;
    }

    for (s32 i = player; i < player + num; i++) {
        if (mpDolls[i]->mpStateMachine->IsStateSettled(0)) {
            mpDolls[i]->LookAtPointer(rPos);
        }
    }
}

void RPDollMgr::SetPointerActive(bool active) {
    for (s32 i = 0; i < mPlayerNum; i++) {
        mpDolls[i]->SetPointerActive(active);
    }
}

s32 RPDollMgr::HitTestDoll(const nw4r::math::VEC2& rPos, s32 player) const {
    return mpDolls[player]->HitTestPart(rPos, player);
}
