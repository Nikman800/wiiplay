// Tanks block pool (RPTnkBlockPool): field blocks and the queries on them.
// Unit 0x80261A38-0x80262024 (dtk/game_units.tsv). Functions in reverse address order.
// RPTnkBlockPool::Destroy is defined in RPTnkField.cpp.
#include <Pack/RPParty/RPTnkScene/RPTnkBlock.h>

RPTnkBlockPool* RPTnkBlockPool::sInstance;

static inline bool IsInRange(f32 v, f32 min, f32 max) {
    bool inside = false;
    if (min <= v && max >= v) {
        inside = true;
    }
    return inside;
}

// By value: the caller copies the position word by word into a temporary, as the original does.
static inline nw4r::math::_VEC3 GetBlockPos(const RPTnkActor* pBlock) {
    return pBlock->GetPos();
}

RPTnkActor* RPTnkBlockPool::CreateObject() {
    return new RPTnkBlock();
}

void RPTnkBlockPool::CreateInstance() {
    sInstance = new RPTnkBlockPool();
}

void RPTnkBlockPool::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkBlockPool::~RPTnkBlockPool() {}

BOOL RPTnkBlockPool::CollideObject(RPTnkActor* pActor, s32 mode) {
    if (mActiveNum == 0) {
        return FALSE;
    }

    BOOL hit = FALSE;
    RPTnkActor** ppBlock = mppBegin;
    do {
        if ((*ppBlock)->IsActive() && (u32)(*ppBlock)->GetKind() != 2) {
            hit = (hit | (*ppBlock)->CollideObject(pActor, mode)) != 0;
        }
    } while (++ppBlock <= mppLast);
    return hit;
}

void RPTnkBlockPool::KillAllBlocks() {
    if (mActiveNum != 0) {
        RPTnkActor** ppBlock = mppBegin;
        do {
            if ((*ppBlock)->IsActive() && (u32)(*ppBlock)->GetKind() != 2) {
                (*ppBlock)->Kill();
            }
        } while (++ppBlock <= mppLast);
    }
}

BOOL RPTnkBlockPool::IsPointInsideBlock(const nw4r::math::VEC3& rPos) {
    if (mActiveNum == 0) {
        return FALSE;
    }

    f32 halfWidth = RPTnkField::sCellWidth;
    halfWidth /= 2.0f;
    f32 halfDepth = RPTnkField::sCellDepth;
    halfDepth /= 2.0f;
    RPTnkActor** ppBlock = mppBegin;
    do {
        if ((*ppBlock)->IsActive() && (u32)(*ppBlock)->GetKind() != 2 && (*ppBlock)->GetHeight() != 1) {
            nw4r::math::VEC3 pos = GetBlockPos(*ppBlock);
            f32 minX = pos.x - halfWidth;
            f32 maxX = pos.x + halfWidth;
            if (IsInRange(rPos.x, minX, maxX)) {
                if (IsInRange(rPos.z, pos.z - halfDepth, pos.z + halfDepth)) {
                    return TRUE;
                }
            }
        }
    } while (++ppBlock <= mppLast);
    return FALSE;
}

BOOL RPTnkBlockPool::IsKind0BlockNear(f32 radius, const nw4r::math::VEC3& rPos) {
    if (mActiveNum == 0) {
        return FALSE;
    }

    f32 halfWidth = RPTnkField::sCellWidth;
    halfWidth /= 2.0f;
    RPTnkActor** ppBlock = mppBegin;
    do {
        if ((*ppBlock)->IsActive() && (*ppBlock)->GetKind() == 0) {
            nw4r::math::VEC3 pos = GetBlockPos(*ppBlock);
            nw4r::math::VEC3 diff;
            diff.x = pos.x - rPos.x;
            diff.y = 0.0f;
            diff.z = pos.z - rPos.z;
            if (PSVECMag(diff) < halfWidth + radius) {
                return TRUE;
            }
        }
    } while (++ppBlock <= mppLast);
    return FALSE;
}

s32 RPTnkBlockPool::GetBlockAt(const nw4r::math::VEC3& rPos) const {
    if (mActiveNum == 0) {
        return 0;
    }

    RPTnkActor** ppBlock = mppBegin;
    do {
        if ((*ppBlock)->IsActive()) {
            nw4r::math::VEC3 pos = GetBlockPos(*ppBlock);
            if (pos.x == rPos.x && pos.z == rPos.z) {
                return (*ppBlock)->GetHeight() == 1 ? 1 : 2;
            }
        }
    } while (++ppBlock <= mppLast);
    return 0;
}
