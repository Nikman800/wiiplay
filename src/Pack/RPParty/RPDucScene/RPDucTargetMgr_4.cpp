// Shooting Range target pool base: a ring of targets of one type, driven per sub-step.
// Unit 0x8023D1D0-0x8023D908 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>

#include <Pack/RPParty/RPDucScene/RPDucTarget.h>

RPDucTargetMgr::RPDucTargetMgr() {}

RPDucTargetMgr::~RPDucTargetMgr() {}

void RPDucTargetMgr::CreatePool(s32 num) {
    mPoolNum = num;
    mActiveNum = 0;
    mppPool = new RPDucTarget*[num];
    mppNext = mppPool;
    mppPoolEnd = mppPool + mPoolNum - 1;

    s32 i = 0;
    RPDucTarget** pp = mppPool;
    do {
        *pp = AllocTarget();
        (*pp)->Create();
        (*pp)->mIndex = i++;
        (*pp)->mpMgr = this;
        pp++;
    } while (pp <= mppPoolEnd);
}

void RPDucTargetMgr::Load() {
    RPDucTarget** pp = mppPool;
    do {
        (*pp)->Load();
        pp++;
    } while (pp <= mppPoolEnd);
}

void RPDucTargetMgr::Reset() {
    RPDucTarget** pp = mppPool;
    mActiveNum = 0;
    mppNext = pp;
    do {
        (*pp)->Reset();
        pp++;
    } while (pp <= mppPoolEnd);
}

void RPDucTargetMgr::ClearHitRecords() {
    if (mActiveNum == 0) {
        return;
    }

    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            (*pp)->ClearHitOrders();
            (*pp)->ClearHitCounts();
            (*pp)->ClearHitPositions();
        }
        pp++;
    } while (pp <= mppPoolEnd);
}

void RPDucTargetMgr::ApplyVelocityAll() {
    if (mActiveNum == 0) {
        return;
    }
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            (*pp)->ApplyVelocity();
        }
        pp++;
    } while (pp <= mppPoolEnd);
}

void RPDucTargetMgr::ClearForcesHookAll() {
    if (mActiveNum == 0) {
        return;
    }
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            (*pp)->ClearForcesHook();
        }
        pp++;
    } while (pp <= mppPoolEnd);
}

void RPDucTargetMgr::VF_0x80All() {
    if (mActiveNum == 0) {
        return;
    }
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            (*pp)->VF_0x80();
        }
        pp++;
    } while (pp <= mppPoolEnd);
}

void RPDucTargetMgr::ApplyForcesHookAll() {
    if (mActiveNum == 0) {
        return;
    }
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            (*pp)->ApplyForcesHook();
        }
        pp++;
    } while (pp <= mppPoolEnd);
}

void RPDucTargetMgr::CalculateAll() {
    if (mActiveNum == 0) {
        return;
    }
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            (*pp)->Calculate();
        }
        pp++;
    } while (pp <= mppPoolEnd);
}

void RPDucTargetMgr::IncAgeAll() {
    if (mActiveNum == 0) {
        return;
    }
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            (*pp)->mAge++;
        }
        pp++;
    } while (pp <= mppPoolEnd);
}

void RPDucTargetMgr::CalcModelMtxAll() {
    if (mActiveNum == 0) {
        return;
    }
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            (*pp)->CalcModelMtx();
        }
        pp++;
    } while (pp <= mppPoolEnd);
}

void RPDucTargetMgr::DebugDrawAll() {
    if (mActiveNum == 0) {
        return;
    }
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            (*pp)->DebugDrawBox();
        }
        pp++;
    } while (pp <= mppPoolEnd);
}

RPDucTarget* RPDucTargetMgr::Spawn(const nw4r::math::VEC3& rPos, u32 arg) {
    if (mActiveNum >= mPoolNum) {
        return NULL;
    }

    RPDucTarget** pStart = mppNext;
    RPDucTarget** pp = pStart;
    do {
        if ((*pp)->mAge < 0) {
            (*pp)->Activate(rPos, arg);
            mActiveNum++;
            if (pp == mppPoolEnd) {
                mppNext = mppPool;
            } else {
                mppNext = pp + 1;
            }
            return *pp;
        }

        if (pp == mppPoolEnd) {
            pp = mppPool;
        } else {
            pp++;
        }
    } while (pp != pStart);

    return NULL;
}

void RPDucTargetMgr::RemoveAll() {
    RPDucTarget** pp = mppPool;
    do {
        (*pp)->Remove();
        pp++;
    } while (pp <= mppPoolEnd);
    mActiveNum = 0;
}
