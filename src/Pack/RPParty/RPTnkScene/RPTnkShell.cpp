#include <Pack/RPParty/RPTnkScene/RPTnkShell.h>

#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPTnkScene/RPTnkAi.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTank.h>

extern "C" f32 RP_calcPerpendicularDistance(const nw4r::math::VEC3*,
                                            const nw4r::math::VEC3*,
                                            const nw4r::math::VEC3*);
void RPCalcPerpendicularDir(nw4r::math::VEC3*, const nw4r::math::VEC3*,
                            const nw4r::math::VEC3*, const nw4r::math::VEC3*);

namespace {

// Two separate 12-byte copies (a 0x18 struct would be placed above the other locals).
struct ShellVec3Copy {
    nw4r::math::_VEC3 v;
};

struct ShellCollisionEffect {
    static void Spawn(const nw4r::math::_VEC3& pos, const nw4r::math::_VEC3& scale) {
        RPSysEffect effect("Tnk_shell_bomb", 0, 0);
        effect.create();
        effect.setPos(nw4r::math::VEC3(pos.x, pos.y, pos.z));
        effect.setScale(nw4r::math::VEC3(scale.x, scale.y, scale.z));
        effect.update();
    }

    static void Create(const nw4r::math::_VEC3& scale, const nw4r::math::_VEC3& pos) {
        ShellVec3Copy s = {scale};
        ShellVec3Copy p = {pos};
        Spawn(p.v, s.v);
    }
};

// Takes the threat by value: the binary copies it once more before the call.
inline void AddShellThreat(RPTnkAiMove* pAiMove, RPTnkAiMove::ShellThreat threat) {
    pAiMove->AddShellThreat(threat);
}

} // namespace

RPTnkShellPool* RPTnkShellPool::sInstance;

void RPTnkShellPool::Destroy() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkActor* RPTnkShellPool::CreateObject() {
    return new RPTnkShell;
}

void RPTnkShellPool::CreateInstance() {
    sInstance = new RPTnkShellPool;
}

RPTnkShellPool::~RPTnkShellPool() {}

#pragma opt_unroll_loops off
void RPTnkShellPool::CountShellsPerOwner() {
    s32 i = 0;
    do {
        mShellNum[i] = 0;
        ++i;
    } while (i < 10);
    if (mActiveNum != 0) {
        RPTnkActor** p = mppBegin;
        do {
            if ((*p)->IsActive()) {
                ++mShellNum[(*p)->GetOwnerSlot()];
            }
            ++p;
        } while (p <= mppLast);
    }
}

void RPTnkShellPool::StopAllEffects() {
    if (mActiveNum != 0) {
        RPTnkActor** p = mppBegin;
        do {
            if ((*p)->IsActive()) {
                (*p)->StopEffects();
            }
            ++p;
        } while (p <= mppLast);
    }
}

BOOL RPTnkShellPool::IsObjectHit(RPTnkActor* pActor, BOOL ignoreOwnShells) {
    if (mActiveNum == 0) {
        return FALSE;
    }
    BOOL hit = FALSE;
    RPTnkActor** p = mppBegin;
    do {
        if ((*p)->IsActive() && (*p)->TestHit(pActor, NULL)) {
            if (static_cast<u32>(ignoreOwnShells) == TRUE) {
                s32 age = (*p)->GetAge();
                RPTnkTank* pOwner = (*p)->GetOwnerTank();
                if (age < 16 && pActor->GetOwnerTank() == pOwner) {
                    goto next;
                }
            }
            hit = TRUE;
        }
next:
        ++p;
    } while (p <= mppLast);
    return hit;
}

BOOL RPTnkShellPool::CollideShellWithShells(RPTnkShell* pShell) {
    if (mActiveNum == 0) {
        return FALSE;
    }
    RPTnkActor** p = mppBegin;
    BOOL hit = FALSE;
    do {
        if ((*p)->IsActive() && pShell != *p && (*p)->TestHit(pShell, NULL)) {
            nw4r::math::_VEC3 otherPos = (*p)->GetPos();
            nw4r::math::_VEC3 shellPos = pShell->GetPos();
            nw4r::math::_VEC3 pos;
            pos.x = 0.5f * (otherPos.x + shellPos.x);
            pos.y = 0.5f * (otherPos.y + shellPos.y);
            pos.z = 0.5f * (otherPos.z + shellPos.z);
            nw4r::math::_VEC3 scale = (*p)->mScale;
            ShellCollisionEffect::Create(scale, pos);
            if (pShell->GetIndex() < (*p)->GetIndex()) {
                pShell->PlayHitSound();
            }
            hit = TRUE;
        }
        ++p;
    } while (p <= mppLast);
    return hit;
}

void RPTnkShellPool::DetectIncomingShells(RPTnkAiMove* pAiMove) {
    if (mActiveNum == 0) {
        return;
    }
    nw4r::math::VEC3 tankPos = pAiMove->GetTank()->GetHitCenterAlt();
    RPTnkAiMove::Params params = pAiMove->GetParams();
    f32 enemyRange = params.mShellRangeEnemy;
    f32 playerRange = params.mShellRangePlayer;
    if (enemyRange == 0.0f && playerRange == 0.0f) {
        return;
    }
    RPTnkActor** p = mppBegin;
    do {
        if ((*p)->IsActive()) {
            f32 range = (*p)->GetOwnerSlot() == 0 ? playerRange : enemyRange;
            if (range != 0.0f) {
                RPTnkAiMove::ShellThreat threat;
                nw4r::math::VEC3 shellPos = (*p)->GetPosCopy();
                RPAxes axes = (*p)->GetRot();
                nw4r::math::VEC3 dir;
                dir.x = axes.z.x;
                dir.y = axes.z.y;
                dir.z = axes.z.z;
                nw4r::math::VEC3 delta;
                delta.x = tankPos.x - shellPos.x;
                delta.y = 0.0f;
                delta.z = tankPos.z - shellPos.z;
                f32 dist = nw4r::math::VEC3Len(&delta);
                if (dist < range && nw4r::math::VEC3Dot(&delta, &dir) > 0.0f) {
                    threat.mPos = shellPos;
                    threat.mDir = dir;
                    threat.mDist = dist;
                    threat.mPerpendicularDist =
                        RP_calcPerpendicularDistance(&tankPos, &shellPos, &dir);
                    RPCalcPerpendicularDir(&threat.mPerpendicularDir, &tankPos,
                                            &shellPos, &dir);
                    threat.mPerpendicularDir.x = -threat.mPerpendicularDir.x;
                    threat.mPerpendicularDir.z = -threat.mPerpendicularDir.z;
                    AddShellThreat(pAiMove, threat);
                }
            }
        }
        ++p;
    } while (p <= mppLast);
}
