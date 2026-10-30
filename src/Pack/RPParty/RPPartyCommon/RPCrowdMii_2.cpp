// Crowd Mii manager (RPCrowdMgr): ten background Miis on the game-select screen.
// Unit 0x801F82A8-0x801F8650. Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPCrowdMii.h>

#include <Pack/RPUtility/RPUtlRandom.h>

inline bool RPCrowdMii::IsHidden() const {
    return mpStateMachine->IsStateSettled(0);
}

inline RPCrowdMgr::RPCrowdMgr() {
    for (s32 i = 0; i < MII_NUM; i++) {
        mpMiis[i] = new RPCrowdMii(i);
    }
}

inline s32 RPCrowdMgr::GetWalkingNum() const {
    s32 walking = 0;
    for (s32 i = 0; i < MII_NUM; i++) {
        if (!mpMiis[i]->IsHidden()) {
            walking++;
        }
    }
    return walking;
}

void RPCrowdMgr::CreateInstance() {
    sInstance = new RPCrowdMgr();
}

void RPCrowdMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPCrowdMgr::~RPCrowdMgr() {}

void RPCrowdMgr::Load() {
    for (s32 i = 0; i < MII_NUM; i++) {
        mpMiis[i]->Load();
    }
}

void RPCrowdMgr::Reset() {
    for (s32 i = 0; i < MII_NUM; i++) {
        mpMiis[i]->Reset();
    }
}

void RPCrowdMgr::Calculate() {
    s32 walking = GetWalkingNum();

    if (walking <= 4) {
        if (RPUtlRandom::getU32(1000) < 8 || walking == 0) {
            for (s32 i = 0; i < MII_NUM; i++) {
                u32 idx = RPUtlRandom::getU32(MII_NUM);
                if (mpMiis[idx]->IsHidden()) {
                    mpMiis[idx]->Start(0);
                    break;
                }
            }
        }
    }

    for (s32 i = 0; i < MII_NUM; i++) {
        mpMiis[i]->Calculate();
    }
}
