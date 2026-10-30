#include <Pack/RPParty/RPTnkScene/RPTnkTankManager.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTank.h>
#include <Pack/RPParty/RPTnkScene/RPTnkCheckMark.h>
#include <Pack/RPParty/RPTnkScene/RPTnkGameManager.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTask.h>

#include <Pack/RPParty/RPTnkScene/RPTnkAi.h>

struct TankHitResult {
    RPTnkTank* tank;
    bool enemy;
    s32 index;
    BOOL hit;
    f32 distance;
};

extern "C" {
extern u32 RP_randSeedX;
extern u32 RP_randSeedY;
}

RPTnkTankManager* RPTnkTankManager::sInstance;

void RPTnkTankManager::CreateInstance() {
    sInstance = new RPTnkTankManager;
}

void RPTnkTankManager::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkTankManager::~RPTnkTankManager() {}

void RPTnkTankManager::CreateTanks(s32 playerNum, s32 enemyNum) {
    mPlayerTankNum = playerNum;
    s32 i = 0;
    do {
        mpPlayers[i] = new RPTnkTank;
        mpPlayers[i]->Create();
        mpPlayers[i]->SetIsEnemy(false);
        mpPlayers[i]->SetIndex(i);
        mpPlayers[i]->SetSePlayerId();
    } while (++i < mPlayerTankNum);
    mEnemyCapacity = enemyNum;
    mEnemyLeft = enemyNum;
    mEnemyAliveNum = 0;
    mppEnemyBegin = new RPTnkTank*[enemyNum];
    RPTnkTank** p = mppEnemyBegin;
    mppEnemyNext = mppEnemyBegin;
    mppEnemyLast = mppEnemyBegin + mEnemyCapacity - 1;
    i = 0;
    do {
        *p = new RPTnkTank;
        (*p)->Create();
        (*p)->SetIsEnemy(true);
        (*p)->SetIndex(i);
        (*p)->SetSePlayerId();
        ++p;
        ++i;
    } while (p <= mppEnemyLast);
    RPTnkCheckMarkPool::CreateInstance();
    mpCheckMarkPool = RPTnkCheckMarkPool::sInstance;
    mpCheckMarkPool->Init(16);
    RPTnkFootmarkTex::CreateInstance();
    mpFootmarkTex = RPTnkFootmarkTex::sInstance;
    mpFootmarkTex->Create();
}

void RPTnkTankManager::Load() {
    mpFootmarkTex->Load();
    s32 i = 0;
    do {
        mpPlayers[i]->Load();
    } while (++i < mPlayerTankNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        (*p)->Load();
    } while (++p <= mppEnemyLast);
    mpCheckMarkPool->Load();
}

void RPTnkTankManager::Reset() {
    s32 i = 0;
    do {
        mpPlayers[i]->Reset();
    } while (++i < mPlayerTankNum);
    if (RPTnkGameManager::sInstance->GetModeIdx() == 0) {
        mActivePlayerNum = 1;
    } else if (RPTnkGameManager::sInstance->GetModeIdx() == 1) {
        mActivePlayerNum = 2;
    }
    RPTnkTank** p = mppEnemyBegin;
    do {
        (*p)->Reset();
    } while (++p <= mppEnemyLast);
    mEnemyLeft = mEnemyCapacity;
    mppEnemyNext = mppEnemyBegin;
    mEnemyAliveNum = 0;
    mpCheckMarkPool->Reset();
}

void RPTnkTankManager::ApplyVelocities() {
    s32 i = 0;
    do {
        if (mpPlayers[i]->IsActive()) {
            mpPlayers[i]->ApplyVelocity();
        }
    } while (++i < mActivePlayerNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        if ((*p)->IsActive()) {
            (*p)->ApplyVelocity();
        }
    } while (++p <= mppEnemyLast);
    mpCheckMarkPool->ApplyVelocityAll();
}

void RPTnkTankManager::ClearHitInfo() {
    s32 i = 0;
    do {
        if (mpPlayers[i]->IsActive()) {
            mpPlayers[i]->ClearForcesStep();
        }
    } while (++i < mActivePlayerNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        if ((*p)->IsActive()) {
            (*p)->ClearForcesStep();
        }
    } while (++p <= mppEnemyLast);
    
}

void RPTnkTankManager::ResolveCollisions() {
    s32 i = 0;
    do {
        if (mpPlayers[i]->IsActive()) {
            mpPlayers[i]->CollideWithField();
        }
    } while (++i < mActivePlayerNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        if ((*p)->IsActive()) {
            (*p)->CollideWithField();
        }
    } while (++p <= mppEnemyLast);
    
}

void RPTnkTankManager::UpdateTreadDistances() {
    s32 i = 0;
    do {
        if (mpPlayers[i]->IsActive()) {
            mpPlayers[i]->IntegrateStep();
        }
    } while (++i < mActivePlayerNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        if ((*p)->IsActive()) {
            (*p)->IntegrateStep();
        }
    } while (++p <= mppEnemyLast);
    
}

void RPTnkTankManager::CalcTanks() {
    s32 i = 0;
    do {
        if (mpPlayers[i]->IsActive()) {
            mpPlayers[i]->Calculate();
        }
    } while (++i < mActivePlayerNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        if ((*p)->IsActive()) {
            (*p)->Calculate();
        }
    } while (++p <= mppEnemyLast);
    mpCheckMarkPool->CalcAll();
}

void RPTnkTankManager::IncrementTimers() {
    s32 i = 0;
    do {
        if (mpPlayers[i]->mAge >= 0) {
            mpPlayers[i]->mAge++;
        }
    } while (++i < mActivePlayerNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        if ((*p)->mAge >= 0) {
            (*p)->mAge++;
        }
    } while (++p <= mppEnemyLast);
    mpCheckMarkPool->TickAge();
}

void RPTnkTankManager::CalcModelMtxAll() {
    s32 i = 0;
    do {
        if (mpPlayers[i]->IsActive()) {
            mpPlayers[i]->CalcModelMtx();
        }
    } while (++i < mActivePlayerNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        if ((*p)->IsActive()) {
            (*p)->CalcModelMtx();
        }
    } while (++p <= mppEnemyLast);
    mpCheckMarkPool->CalcModelMtxAll();
}

void RPTnkTankManager::Exit() {
    s32 i = 0;
    do {
        mpPlayers[i]->Exit();
    } while (++i < mPlayerTankNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        (*p)->Exit();
    } while (++p <= mppEnemyLast);
    RPTnkCheckMarkPool::DestroyInstance();
    RPTnkFootmarkTex::DestroyInstance();
}

void RPTnkTankManager::SpawnPlayer(const nw4r::math::VEC3& pos, s32 player, s32 param, BOOL show) {
    mpPlayers[player]->SpawnPlayer(pos, param, show);
}

void RPTnkTankManager::SpawnNextEnemy(const nw4r::math::VEC3& pos, s32 type, s32 idx, BOOL show) {
    if (mEnemyAliveNum < mEnemyCapacity) {
        RPTnkTank** p = mppEnemyNext;
        do {
            if ((*p)->GetAge() < 0) {
                (*p)->SpawnEnemy(pos, type, idx, show);
                ++mEnemyAliveNum;
                if (p == mppEnemyLast) {
                    mppEnemyNext = mppEnemyBegin;
                } else {
                    mppEnemyNext = p + 1;
                }
                return;
            }
            if (p == mppEnemyLast) {
                p = mppEnemyBegin;
            } else {
                ++p;
            }
        } while (p != mppEnemyNext);
    }
}

void RPTnkTankManager::DeactivateAll() {
    s32 i = 0;
    do {
        mpPlayers[i]->Kill();
    } while (++i < mActivePlayerNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        (*p)->Kill();
    } while (++p <= mppEnemyLast);
    RPTnkTank** begin = mppEnemyBegin;
    mEnemyAliveNum = 0;
    mppEnemyNext = begin;
    mpCheckMarkPool->KillAll();
}

void RPTnkTankManager::ActivatePlayers() {
    s32 i = 0;
    do {
        if (mpPlayers[i]->IsActive()) {
            mpPlayers[i]->Activate();
        }
    } while (++i < mActivePlayerNum);
}

void RPTnkTankManager::ActivateEnemies() {
    RPTnkTank** p = mppEnemyBegin;
    do {
        if ((*p)->IsActive()) {
            (*p)->Activate();
        }
    } while (++p <= mppEnemyLast);
}

// The trailing blr after the bctr tail call is the split's Tnk_8025D1F4 (status shows 96.55%,
// the linked bytes are identical).
BOOL RPTnkTankManager::TestMineHitTank(RPTnkActor* mine, s32 idx, BOOL enemy) {
    if (enemy == FALSE) {
        RPTnkTank* tank = mpPlayers[idx];
        if (tank->GetAge() < 0) {
            return FALSE;
        }
        return tank->TestHit(mine, NULL);
    } else {
        RPTnkTank* tank = mppEnemyBegin[idx];
        if (tank->GetAge() < 0) {
            return FALSE;
        }
        return tank->TestHit(mine, NULL);
    }
}

BOOL RPTnkTankManager::FindMineHit(RPTnkActor* actor, void* result) {
    f32 nearest = 10000.0f;
    BOOL hit = FALSE;
    RPTnkTank* tank = NULL;
    bool enemy = false;
    s32 index;
    f32 distance;
    s32 i = 0;
    do {
        if (mpPlayers[i]->IsActive() &&
            static_cast<u32>(mpPlayers[i]->TestHit(actor, &distance)) == TRUE &&
            distance < nearest) {
            nearest = distance;
            tank = mpPlayers[i];
            index = i;
            hit = TRUE;
            enemy = false;
        }
    } while (++i < mActivePlayerNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        if ((*p)->IsActive() &&
            static_cast<u32>((*p)->TestHit(actor, &distance)) == TRUE &&
            distance < nearest) {
            tank = *p;
            nearest = distance;
            hit = TRUE;
            enemy = true;
            index = tank->GetIndex();
        }
    } while (++p <= mppEnemyLast);
    if (!hit) {
        return FALSE;
    }
    TankHitResult* out = static_cast<TankHitResult*>(result);
    out->tank = tank;
    out->enemy = enemy;
    out->index = index;
    out->hit = hit;
    out->distance = nearest;
    return hit;
}

BOOL RPTnkTankManager::FindShellHit(RPTnkActor* actor, void* result) {
    f32 nearest = 10000.0f;
    BOOL hit = FALSE;
    RPTnkTank* tank = NULL;
    bool enemy = false;
    s32 index;
    f32 distance;
    s32 i = 0;
    do {
        if (mpPlayers[i]->IsActive() &&
            static_cast<u32>(mpPlayers[i]->TestHitAlt(actor, &distance)) == TRUE &&
            distance < nearest) {
            nearest = distance;
            tank = mpPlayers[i];
            index = i;
            hit = TRUE;
            enemy = false;
        }
    } while (++i < mActivePlayerNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        if ((*p)->IsActive() &&
            static_cast<u32>((*p)->TestHitAlt(actor, &distance)) == TRUE &&
            distance < nearest) {
            tank = *p;
            nearest = distance;
            hit = TRUE;
            enemy = true;
            index = tank->GetIndex();
        }
    } while (++p <= mppEnemyLast);
    if (!hit) {
        return FALSE;
    }
    TankHitResult* out = static_cast<TankHitResult*>(result);
    out->tank = tank;
    out->enemy = enemy;
    out->index = index;
    out->hit = hit;
    out->distance = nearest;
    return hit;
}

// Matching idiom (docs/NONMATCHING.md, "Matching idioms"): element accessor for the enemy table. Reading the tank through it (not mppEnemyBegin[idx]
// directly) makes it an inline-result temporary that is coloured before the player walker, as in
// the original (r30 tank, r29 walker; the direct read swaps them).
static inline RPTnkTank* TnkEnemyAt(RPTnkTank** ppEnemies, s32 i) { return ppEnemies[i]; }

void RPTnkTankManager::SeparateTank(s32 idx, bool enemy) {
    if (!enemy) {
        if (mActivePlayerNum >= 2) {
            s32 i = 0;
            do {
                if (i != idx && mpPlayers[i]->IsActive()) {
                    mpPlayers[i]->PushApartFrom(mpPlayers[idx]);
                }
            } while (++i < mActivePlayerNum);
        }
        RPTnkTank** p = mppEnemyBegin;
        do {
            if ((*p)->IsActive()) {
                (*p)->PushApartFrom(mpPlayers[idx]);
            }
        } while (++p <= mppEnemyLast);
    } else {
        s32 i = 0;
        RPTnkTank* tank = TnkEnemyAt(mppEnemyBegin, idx);
        do {
            if (mpPlayers[i]->IsActive()) {
                mpPlayers[i]->PushApartFrom(tank);
            }
        } while (++i < mActivePlayerNum);
        RPTnkTank** p = mppEnemyBegin;
        do {
            if ((*p)->IsActive() && tank != *p) {
                (*p)->PushApartFrom(tank);
            }
        } while (++p <= mppEnemyLast);
    }
}

u32 RPTnkTankManager::GetPlayersDeadState(s32 playerNum) const {
    switch (playerNum) {
    case 1:
        if (mpPlayers[0]->GetAlive() == 0) {
            return 3;
        }
        return 1;
    case 2:
        if (mpPlayers[0]->GetAlive() == 0) {
            return (mpPlayers[1]->GetAlive() == 0) + 2;
        }
        return mpPlayers[1]->GetAlive() == 0;
    default:
        return 0;
    }
}

void RPTnkTankManager::DrawTrackMarks() {
    s32 i = 0;
    do {
        if (mpPlayers[i]->IsActive()) {
            mpPlayers[i]->DrawTreadMark();
        }
    } while (++i < mActivePlayerNum);
    RPTnkTank** p = mppEnemyBegin;
    do {
        if ((*p)->IsActive()) {
            (*p)->DrawTreadMark();
        }
    } while (++p <= mppEnemyLast);
}

void RPTnkTankManager::RecordShellHit(BOOL enemy, s32 idx, const void* hit) {
    if (!enemy) {
        RPTnkTank::HitInfo info = *static_cast<const RPTnkTank::HitInfo*>(hit);
        mpPlayers[idx]->KeepNearestShellHit(info);
    } else {
        RPTnkTank* tank = mppEnemyBegin[idx];
        tank->KeepNearestShellHit(RPTnkTank::HitInfo(*static_cast<const RPTnkTank::HitInfo*>(hit)));
    }
}

void RPTnkTankManager::RecordMineHit(BOOL enemy, s32 idx, const void* hit) {
    if (!enemy) {
        RPTnkTank::HitInfo info = *static_cast<const RPTnkTank::HitInfo*>(hit);
        mpPlayers[idx]->KeepNearestMineHit(info);
    } else {
        RPTnkTank* tank = mppEnemyBegin[idx];
        tank->KeepNearestMineHit(RPTnkTank::HitInfo(*static_cast<const RPTnkTank::HitInfo*>(hit)));
    }
}

f32 RPTnkTankManager::GetNearestEnemyDistance(RPTnkActor* actor) {
    f32 zero;
    f32 nearest = 10000.0f;
    nw4r::math::VEC3 origin = actor->GetPos();
    zero = 0.0f;
    RPTnkTank** p = mppEnemyBegin;
    do {
        if (actor != *p && (*p)->IsActive()) {
            nw4r::math::VEC3 pos = (*p)->GetPos();
            nw4r::math::VEC3 delta;
            delta.x = pos.x - origin.x;
            delta.y = zero;
            delta.z = pos.z - origin.z;
            f32 distance = PSVECMag(delta);
            if (nearest > distance) {
                nearest = distance;
            }
        }
    } while (++p <= mppEnemyLast);
    return nearest;
}

void RPTnkTankManager::LimitAiFlag110() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 0x3039;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    RPTnkAiAttack* attack;
    u32 random = (RP_randSeedX ^ RP_randSeedY) & 0xFFFF;
    RPTnkTank** start = mppEnemyBegin;
    start += random % static_cast<u32>(mEnemyCapacity);
    s32 count = 0;
    RPTnkTank** p = start;
    do {
        if ((*p)->IsActive()) {
            RPTnkAiController* controller = (*p)->GetAiController();
            attack = controller->GetAttack();
            if (attack->mFireInterval == 1) {
                ++count;
                if (count > 2) {
                    ++attack->mFireInterval;
                }
            }
        }
        if (p == mppEnemyLast) {
            p = mppEnemyBegin;
        } else {
            ++p;
        }
    } while (p != start);
}

void RPTnkTankManager::LimitAiFlag118() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 0x3039;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    RPTnkAiAttack* attack;
    u32 random = (RP_randSeedX ^ RP_randSeedY) & 0xFFFF;
    RPTnkTank** start = mppEnemyBegin;
    start += random % static_cast<u32>(mEnemyCapacity);
    s32 count = 0;
    RPTnkTank** p = start;
    do {
        if ((*p)->IsActive()) {
            RPTnkAiController* controller = (*p)->GetAiController();
            attack = controller->GetAttack();
            if (attack->mMineInterval == 1) {
                ++count;
                if (count > 1) {
                    ++attack->mMineInterval;
                }
            }
        }
        if (p == mppEnemyLast) {
            p = mppEnemyBegin;
        } else {
            ++p;
        }
    } while (p != start);
}

// The 750 (Gekko) scheduler moves the mppEnemyLast load above `type = 1`, so the maximum is
// coalesced into r3. The original keeps the maximum in r5 (mr r3, r5 at the end): the 604
// scheduling model reproduces it exactly; no source-level form found (~500 variants).
#pragma scheduling 604
s32 RPTnkTankManager::GetMaxActiveEnemyType() const {
    s32 type = 1;
    RPTnkTank** p = mppEnemyBegin;
    do {
        if ((*p)->IsActive() && (*p)->GetEnemyType() > type) {
            type = (*p)->GetEnemyType();
        }
    } while (++p <= mppEnemyLast);
    return type;
}
#pragma scheduling reset
