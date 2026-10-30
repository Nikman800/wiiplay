// Tanks actor pool (RPTnkActorPool): a ring of actors spawned round robin.
// Unit 0x80258ADC-0x80259114 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkActorPool.h>

#include <Pack/RPParty/RPTnkScene/RPTnkActor.h>

// Wii Play: six of RPTnkActor's empty virtuals are defined here
void RPTnkActor::VF_0x90() {}

BOOL RPTnkActor::CollideObject(RPTnkActor* pOther, s32 mode) {
    return FALSE;
}

void RPTnkActor::SetTargetPos(const nw4r::math::VEC3& rPos) {}

BOOL RPTnkActor::IsInBlastRadius(const RPTnkActor* pOther) const {
    return FALSE;
}

BOOL RPTnkActor::IsInTriggerRange(const RPTnkActor* pOther) const {
    return FALSE;
}

void RPTnkActor::SetOwnerTank(RPTnkTank* pTank) {}

RPTnkActorPool::RPTnkActorPool() {}

RPTnkActorPool::~RPTnkActorPool() {}

void RPTnkActorPool::Init(s32 num) {
    mCapacity = num;
    mActiveNum = 0;
    mppBegin = new RPTnkActor*[num];
    s32 i = 0;
    RPTnkActor** ppActor = mppBegin;
    mppNext = ppActor;
    mppLast = ppActor + mCapacity - 1;

    do {
        *ppActor = CreateObject();
        (*ppActor)->Create();
        (*ppActor)->mIndex = i++;
        (*ppActor)->mpPool = this;
    } while (++ppActor <= mppLast);
}

void RPTnkActorPool::Load() {
    RPTnkActor** ppActor = mppBegin;
    do {
        (*ppActor)->Load();
    } while (++ppActor <= mppLast);
}

void RPTnkActorPool::Reset() {
    RPTnkActor** ppActor = mppBegin;
    mActiveNum = 0;
    mppNext = ppActor;
    do {
        (*ppActor)->Reset();
    } while (++ppActor <= mppLast);
}

void RPTnkActorPool::ApplyVelocityAll() {
    if (mActiveNum != 0) {
        RPTnkActor** ppActor = mppBegin;
        do {
            if ((*ppActor)->IsActive()) {
                (*ppActor)->ApplyVelocity();
            }
        } while (++ppActor <= mppLast);
    }
}

void RPTnkActorPool::ClearForcesAll() {
    if (mActiveNum != 0) {
        RPTnkActor** ppActor = mppBegin;
        do {
            if ((*ppActor)->IsActive()) {
                (*ppActor)->ClearForcesStep();
            }
        } while (++ppActor <= mppLast);
    }
}

void RPTnkActorPool::CollideAllWithField() {
    if (mActiveNum != 0) {
        RPTnkActor** ppActor = mppBegin;
        do {
            if ((*ppActor)->IsActive()) {
                (*ppActor)->CollideWithField();
            }
        } while (++ppActor <= mppLast);
    }
}

void RPTnkActorPool::IntegrateAll() {
    if (mActiveNum != 0) {
        RPTnkActor** ppActor = mppBegin;
        do {
            if ((*ppActor)->IsActive()) {
                (*ppActor)->IntegrateStep();
            }
        } while (++ppActor <= mppLast);
    }
}

void RPTnkActorPool::CalcAll() {
    if (mActiveNum != 0) {
        RPTnkActor** ppActor = mppBegin;
        do {
            if ((*ppActor)->IsActive()) {
                (*ppActor)->Calculate();
            }
        } while (++ppActor <= mppLast);
    }
}

void RPTnkActorPool::TickAge() {
    if (mActiveNum != 0) {
        RPTnkActor** ppActor = mppBegin;
        do {
            if ((*ppActor)->mAge >= 0) {
                (*ppActor)->mAge += 1;
            }
        } while (++ppActor <= mppLast);
    }
}

void RPTnkActorPool::CalcModelMtxAll() {
    if (mActiveNum != 0) {
        RPTnkActor** ppActor = mppBegin;
        do {
            if ((*ppActor)->IsActive()) {
                (*ppActor)->CalcModelMtx();
            }
        } while (++ppActor <= mppLast);
    }
}

RPTnkActor* RPTnkActorPool::Spawn(const nw4r::math::VEC3& rPos, u32 param) {
    if (mActiveNum >= mCapacity) {
        return NULL;
    }

    {
        RPTnkActor** ppActor = mppNext;
        do {
            if (!(*ppActor)->IsActive()) {
                (*ppActor)->Spawn(rPos, param);
                mActiveNum++;
                if (ppActor == mppLast) {
                    mppNext = mppBegin;
                } else {
                    mppNext = ppActor + 1;
                }
                return *ppActor;
            }

            if (ppActor == mppLast) {
                ppActor = mppBegin;
            } else {
                ppActor++;
            }
        } while (ppActor != mppNext);
    }

    return NULL;
}

void RPTnkActorPool::KillAll() {
    RPTnkActor** ppActor = mppBegin;
    do {
        (*ppActor)->Kill();
    } while (++ppActor <= mppLast);
    mActiveNum = 0;
}
