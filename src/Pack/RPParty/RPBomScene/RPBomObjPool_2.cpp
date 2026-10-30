#include <Pack/RPParty/RPBomScene/RPBomObjPool_2.h>

RPBomObjPool::RPBomObjPool() {
}

RPBomObjPool::~RPBomObjPool() {
}

void RPBomObjPool::CreateItems(s32 num) {
    mCapacity = num;
    mActiveNum = 0;
    RPBomPoolObj** items = new RPBomPoolObj*[num];
    mppBegin = items;
    mppNext = items;
    mppLast = &items[mCapacity];
    --mppLast;

    s32 index = 0;
    RPBomPoolObj** item = items;
    do {
        *item = CreateItem();
        (*item)->Create();
        (*item)->mIndex = index;
        index++;
        (*item)->mpPool = this;
        item++;
    } while (item <= mppLast);
}

void RPBomObjPool::Load() {
    RPBomPoolObj** item = mppBegin;
    do {
        (*item)->Load();
        item++;
    } while (item <= mppLast);
}

void RPBomObjPool::Reset() {
    mActiveNum = 0;
    mppNext = mppBegin;
    RPBomPoolObj** item = mppBegin;
    do {
        (*item)->Reset();
        item++;
    } while (item <= mppLast);
}

void RPBomObjPool::MoveItems() {
    if (mActiveNum == 0) {
        return;
    }

    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0) {
            (*item)->Move();
        }
        item++;
    } while (item <= mppLast);
}

void RPBomObjPool::PreCalcItems() {
    if (mActiveNum == 0) {
        return;
    }

    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0) {
            (*item)->PreCalc();
        }
        item++;
    } while (item <= mppLast);
}

void RPBomObjPool::CalcHitItems() {
    if (mActiveNum == 0) {
        return;
    }

    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0) {
            (*item)->CalcHit();
        }
        item++;
    } while (item <= mppLast);
}

void RPBomObjPool::PostCalcItems() {
    if (mActiveNum == 0) {
        return;
    }

    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0) {
            (*item)->PostCalc();
        }
        item++;
    } while (item <= mppLast);
}

void RPBomObjPool::CalcItems() {
    if (mActiveNum == 0) {
        return;
    }

    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0) {
            (*item)->Calculate();
        }
        item++;
    } while (item <= mppLast);
}

void RPBomObjPool::IncrementAges() {
    if (mActiveNum == 0) {
        return;
    }

    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0) {
            (*item)->mAge++;
        }
        item++;
    } while (item <= mppLast);
}

void RPBomObjPool::CalcModelMtxItems() {
    if (mActiveNum == 0) {
        return;
    }

    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0) {
            (*item)->CalcModelMtx();
        }
        item++;
    } while (item <= mppLast);
}

RPBomPoolObj* RPBomObjPool::Spawn(const nw4r::math::VEC3& rPos, u32 flags) {
    if (mActiveNum >= mCapacity) {
        return 0;
    }

    RPBomPoolObj* obj;
    RPBomPoolObj** start = mppNext;
    RPBomPoolObj** end = start;
    do {
        obj = *end;
        if (obj->mAge < 0) {
            obj->Activate(rPos, flags);
            mActiveNum++;
            if (end == mppLast) {
                mppNext = mppBegin;
            } else {
                mppNext = end + 1;
            }
            return *end;
        }

        if (end == mppLast) {
            end = mppBegin;
        } else {
            end++;
        }
    } while (end != start);

    return 0;
}

void RPBomObjPool::DeactivateAll() {
    RPBomPoolObj** item = mppBegin;
    do {
        (*item)->Deactivate();
        item++;
    } while (item <= mppLast);
    mActiveNum = 0;
}
