// Shooting Range target registry: holds the seven target pools and forwards each step to them.
// Unit 0x8023E440-0x8023EC0C (dtk/game_units.tsv). Functions in reverse address order.
// The unit's first function (highest address) is RPDucUfoMgr::SpawnTarget, the last function of
// the UFO manager file that follows (every RPDucXMgr::SpawnTarget sits just before its manager).
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>

#include <Pack/RPParty/RPDucScene/RPDucTarget.h>

typedef RPDucTarget* (*SpawnFunc)(const nw4r::math::VEC3& rPos, u32 arg);
typedef RPDucTargetMgr* (*CreateFunc)(s32 num);

SpawnFunc sSpawnFuncs[] = {
    RPDucClayMgr::SpawnTarget,         RPDucUfoMgr::SpawnTarget,  RPDucBalloonMgr::SpawnTarget,
    RPDucCircleTargetMgr::SpawnTarget, RPDucDuckMgr::SpawnTarget, RPDucCanMgr::SpawnTarget,
    RPDucAvatarMgr::SpawnTarget,
};

CreateFunc sCreateFuncs[] = {
    RPDucClayMgr::CreateInstance,         RPDucUfoMgr::CreateInstance,
    RPDucBalloonMgr::CreateInstance,      RPDucCircleTargetMgr::CreateInstance,
    RPDucDuckMgr::CreateInstance,         RPDucCanMgr::CreateInstance,
    RPDucAvatarMgr::CreateInstance,
};

RPDucTargetRegistry* RPDucTargetRegistry::sInstance;

void RPDucTargetRegistry::CreateInstance() {
    sInstance = new RPDucTargetRegistry();
}

void RPDucTargetRegistry::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucTargetRegistry::~RPDucTargetRegistry() {}

void RPDucTargetRegistry::AddManager(s32 type, s32 num) {
    mpMgrs[mMgrNum] = sCreateFuncs[type](num);
    mpMgrs[mMgrNum]->SetType(type);
    mMgrNum++;
}

void RPDucTargetRegistry::DestroyManagers() {
    if (mMgrNum != 0) {
        u32 i = 0;
        do {
            mpMgrs[i]->DestroyInstance();
        } while (++i < mMgrNum);
    }
}

void RPDucTargetRegistry::Load() {
    if (mMgrNum != 0) {
        u32 i = 0;
        do {
            mpMgrs[i]->Load();
        } while (++i < mMgrNum);
    }
}

void RPDucTargetRegistry::Reset() {
    if (mMgrNum != 0) {
        u32 i = 0;
        do {
            mpMgrs[i]->Reset();
        } while (++i < mMgrNum);
    }
}

void RPDucTargetRegistry::ClearHitRecords() {
    if (mMgrNum != 0) {
        u32 i = 0;
        do {
            mpMgrs[i]->ClearHitRecords();
        } while (++i < mMgrNum);
    }
}

void RPDucTargetRegistry::ApplyVelocityAll() {
    if (mMgrNum != 0) {
        u32 i = 0;
        do {
            mpMgrs[i]->ApplyVelocityAll();
        } while (++i < mMgrNum);
    }
}

void RPDucTargetRegistry::ClearForcesHookAll() {
    if (mMgrNum != 0) {
        u32 i = 0;
        do {
            mpMgrs[i]->ClearForcesHookAll();
        } while (++i < mMgrNum);
    }
}

void RPDucTargetRegistry::VF_0x80All() {
    if (mMgrNum != 0) {
        u32 i = 0;
        do {
            mpMgrs[i]->VF_0x80All();
        } while (++i < mMgrNum);
    }
}

void RPDucTargetRegistry::ApplyForcesHookAll() {
    if (mMgrNum != 0) {
        u32 i = 0;
        do {
            mpMgrs[i]->ApplyForcesHookAll();
        } while (++i < mMgrNum);
    }
}

void RPDucTargetRegistry::CalculateAll() {
    if (mMgrNum != 0) {
        u32 i = 0;
        do {
            mpMgrs[i]->CalculateAll();
        } while (++i < mMgrNum);
    }
}

void RPDucTargetRegistry::IncAgeAll() {
    if (mMgrNum != 0) {
        u32 i = 0;
        do {
            mpMgrs[i]->IncAgeAll();
        } while (++i < mMgrNum);
    }
}

void RPDucTargetRegistry::CalcModelMtxAll() {
    if (mMgrNum != 0) {
        u32 i = 0;
        do {
            mpMgrs[i]->CalcModelMtxAll();
        } while (++i < mMgrNum);
    }
}

RPDucTarget* RPDucTargetRegistry::Spawn(s32 type, const nw4r::math::VEC3& rPos, u32 arg) {
    return sSpawnFuncs[type](rPos, arg);
}

void RPDucTargetRegistry::RemoveAll() {
    if (mMgrNum != 0) {
        u32 i = 0;
        do {
            mpMgrs[i]->RemoveAll();
        } while (++i < mMgrNum);
    }
}

s32 RPDucTargetRegistry::CountActive() const {
    if (mMgrNum == 0) {
        return 0;
    }

    s32 num = 0;
    u32 i = 0;
    do {
        num += mpMgrs[i]->GetActiveNum();
    } while (++i < mMgrNum);
    return num;
}

s32 RPDucTargetRegistry::CountActiveExclDucksAvatars() const {
    if (mMgrNum == 0) {
        return 0;
    }

    s32 num = 0;
    u32 i = 0;
    do {
        if (mpMgrs[i]->GetType() != 6 && mpMgrs[i]->GetType() != 4) {
            num += mpMgrs[i]->GetActiveNum();
        }
    } while (++i < mMgrNum);
    return num;
}
