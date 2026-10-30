// RPTnkMinePool: the mine pool (0x80266508-0x80266C14). The mines themselves are in
// RPTnkMine_2.cpp; the original is two files (see its padding and duplicate 0.0f literal).
#include <Pack/RPParty/RPTnkScene/RPTnkMine.h>

#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPTnkScene/RPTnkAi.h>
#include <Pack/RPParty/RPTnkScene/RPTnkField.h>
#include <Pack/RPParty/RPTnkScene/RPTnkShell.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTank.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTankManager.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

extern nw4r::math::VEC3 RP_vecZero;
extern nw4r::math::VEC3 RP_vecOne;
extern RPAxes RP_mtx33Identity;
extern "C" f32 Tnk_getMineProximityRadius();

namespace {

// Takes the threat by value: the binary copies it once more before the call.
inline void AddMineThreat(RPTnkAiMove* pAiMove, RPTnkAiMove::MineThreat threat) {
    pAiMove->AddMineThreat(threat);
}

} // namespace

RPTnkMinePool* RPTnkMinePool::sInstance;

RPTnkActor* RPTnkMinePool::CreateObject() {
    return new RPTnkMine;
}

void RPTnkMinePool::CreateInstance() {
    sInstance = new RPTnkMinePool;
}

void RPTnkMinePool::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkMinePool::~RPTnkMinePool() {}

void RPTnkMinePool::CountByOwner() {
    s32 i = 0;
    do {
        mMineNum[i] = 0;
    } while (++i < 10);
    if (GetActiveMineNum() != 0) {
        RPTnkActor** it = mppBegin;
        do {
            if ((*it)->IsActive() && !(*it)->IsExploding()) {
                mMineNum[(*it)->GetOwnerSlot()]++;
            }
        } while (++it <= mppLast);
    }
}

BOOL RPTnkMinePool::CheckShellHit(RPTnkActor* pShell) {
    if (GetActiveMineNum() == 0) {
        return FALSE;
    }
    BOOL hit = FALSE;
    RPTnkActor** it = mppBegin;
    do {
        if ((*it)->IsActive() && (*it)->TestHitAlt(pShell, NULL)) {
            s32 age = pShell->GetAge();
            RPTnkTank* owner = pShell->GetOwnerTank();
            if (age >= 16 || (*it)->GetOwnerTank() != owner) {
                hit = TRUE;
            }
        }
    } while (++it <= mppLast);
    return hit;
}

BOOL RPTnkMinePool::IsInExplosion(RPTnkActor* pActor) {
    if (mActiveNum == 0) {
        return FALSE;
    }
    RPTnkActor** it = mppBegin;
    do {
        if ((*it)->IsActive() && static_cast<u32>((*it)->IsInBlastRadius(pActor)) == TRUE) {
            return TRUE;
        }
    } while (++it <= mppLast);
    return FALSE;
}

BOOL RPTnkMinePool::IsInOtherExplosion(RPTnkActor* pActor) {
    if (mActiveNum == 0) {
        return FALSE;
    }
    RPTnkActor** it = mppBegin;
    do {
        if ((*it)->IsActive() && pActor != *it && static_cast<u32>((*it)->IsInBlastRadius(pActor)) == TRUE) {
            return TRUE;
        }
    } while (++it <= mppLast);
    return FALSE;
}

void RPTnkMinePool::AddTankAvoidance(RPTnkAiMove* pAiMove) {
    if (mActiveNum == 0) {
        return;
    }
    nw4r::math::VEC3 pos = pAiMove->GetPos();
    RPTnkAiMove::Params params = pAiMove->GetParams();
    f32 enemyRadius = params.mMineRadiusEnemy;
    f32 playerRadius = params.mMineRadiusPlayer;
    if (enemyRadius == 0.0f && playerRadius == 0.0f) {
        return;
    }
    RPTnkActor** it = mppBegin;
    do {
        if ((*it)->IsActive()) {
            f32 radius;
            if ((*it)->GetOwnerSlot() == 0) {
                radius = playerRadius;
            } else {
                radius = enemyRadius;
            }
            if (radius != 0.0f) {
                nw4r::math::VEC3 center;
                nw4r::math::VEC3 dir;
                nw4r::math::VEC3 minePos = (*it)->GetPos();
                center.x = minePos.x;
                center.y = minePos.y;
                center.z = minePos.z;
                dir.x = pos.x - center.x;
                dir.y = 0.0f;
                dir.z = pos.z - center.z;
                f32 dist = nw4r::math::VEC3Len(&dir);
                if (dist < radius) {
                    if (dist != 0.0f) {
                        nw4r::math::VEC3Normalize(&dir, &dir);
                    }
                    RPTnkAiMove::MineThreat threat;
                    threat.mPos = center;
                    threat.mDist = dist;
                    threat.mDir = dir;
                    AddMineThreat(pAiMove, threat);
                }
            }
        }
    } while (++it <= mppLast);
}

BOOL RPTnkMinePool::IsNearAnyMine(RPTnkActor* pActor) {
    if (mActiveNum == 0) {
        return FALSE;
    }
    RPTnkActor** it = mppBegin;
    do {
        if ((*it)->IsActive() && static_cast<u32>((*it)->IsInTriggerRange(pActor)) == TRUE) {
            return TRUE;
        }
    } while (++it <= mppLast);
    return FALSE;
}

