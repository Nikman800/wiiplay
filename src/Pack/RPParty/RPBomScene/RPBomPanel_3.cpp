#include <Pack/RPParty/RPBomScene/RPBomPanel_3.h>

#include <Pack/RPParty/RPBomScene/RPBomPanel_2.h>
#include <Pack/RPParty/RPBomScene/RPBomQuestionParam.h>

#include <revolution/MTX/vec.h>

namespace {

inline bool IsInHitZone(const RPBomQuestionParamFile* pFile, f32 z) {
    f32 zNear = pFile->mHitZoneNear;
    f32 zFar = pFile->mHitZoneFar;
    return zNear <= z && zFar >= z;
}

} // namespace

RPBomPoolObj* RPBomPanelPool::CreateItem() {
    return new RPBomPanel;
}

void RPBomPanelPool::CreateInstance() {
    sInstance = new RPBomPanelPool;
}

void RPBomPanelPool::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBomPanelPool::~RPBomPanelPool() {}

void RPBomPanelPool::PostCalcItems() {
    RPBomObjPool::PostCalcItems();
}

RPBomPoolObj* RPBomPanelPool::SpawnPanel(const nw4r::math::VEC3& rPos, u32 flags,
                                         RPBomSpawnInfo info) {
    if (mActiveNum >= mCapacity) {
        return NULL;
    }

    RPBomPoolObj** start = mppNext;
    RPBomPoolObj** end = start;
    do {
        RPBomPoolObj* obj = *end;
        if (obj->mAge < 0) {
            (*end)->Spawn(rPos, flags, info);
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

    return NULL;
}

void RPBomPanelPool::MissAll() {
    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0) {
            (*item)->Miss();
        }
        item++;
    } while (item <= mppLast);
}

void RPBomPanelPool::MissFrontPanel() {
    if (mActiveNum == 0) {
        return;
    }

    RPBomPoolObj* front = NULL;
    f32 maxZ = -10000.0f;
    nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0 && (*item)->IsFalling()) {
            pos = GetPos(*item);
            if (pos.z > maxZ) {
                front = *item;
                maxZ = pos.z;
            }
        }
        item++;
    } while (item <= mppLast);

    if (front != NULL) {
        front->Miss();
    }
}

bool RPBomPanelPool::IsPositionOccupied(const nw4r::math::VEC3& rPos, f32 radius) {
    f32 minDist = radius * 2.0f;
    nw4r::math::VEC3 diff(0.0f, 0.0f, 0.0f);
    if (mActiveNum == 0) {
        return false;
    }

    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0) {
            nw4r::math::VEC3 pos = GetPos(*item);
            diff.x = pos.x - rPos.x;
            diff.y = 0.0f;
            diff.z = pos.z - rPos.z;
            if (PSVECMag(diff) < minDist) {
                return true;
            }
        }
        item++;
    } while (item <= mppLast);

    return false;
}

u8 RPBomPanelPool::GetPanelTransform(nw4r::math::VEC3& rPos, RPAxes& rRot, s32& rColorType,
                                     s32 index) {
    RPBomPoolObj* obj = mppBegin[index];
    rPos = GetPos(obj);
    rRot = GetRot(obj);
    rColorType = obj->GetColorType();
    return GetUnk40(obj);
}

s32 RPBomPanelPool::CountActiveInRange() {
    s32 count = 0;
    RPBomQuestionParam* param = RPBomQuestionParam::sInstance;
    if (mActiveNum == 0) {
        return 0;
    }

    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0 && (*item)->IsFalling()) {
            if (IsInHitZone(param->mpFile, GetPos(*item).z)) {
                count++;
            }
        }
        item++;
    } while (item <= mppLast);

    return count;
}

s32 RPBomPanelPool::CountSpecialInRange() {
    s32 count = 0;
    RPBomQuestionParam* param = RPBomQuestionParam::sInstance;
    if (mActiveNum == 0) {
        return 0;
    }

    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0 && (*item)->IsFalling()) {
            if (IsInHitZone(param->mpFile, GetPos(*item).z) &&
                static_cast<u32>((*item)->GetColorType()) == 2) {
                count++;
            }
        }
        item++;
    } while (item <= mppLast);

    return count;
}

f32 RPBomPanelPool::GetMaxFallingZ() {
    f32 maxZ = -10000.0f;
    if (mActiveNum == 0) {
        return maxZ;
    }

    nw4r::math::VEC3 pos;
    RPBomPoolObj** item = mppBegin;
    do {
        if ((*item)->mAge >= 0 && (*item)->IsFalling()) {
            pos = GetPos(*item);
            if (pos.z > maxZ) {
                maxZ = pos.z;
            }
        }
        item++;
    } while (item <= mppLast);

    return maxZ;
}
