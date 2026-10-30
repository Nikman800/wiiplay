// Tanks weapon manager (RPTnkWeaponManager): owns the shell and mine pools.
// Unit 0x802638A4-0x802641C8 (dtk/game_units.tsv). Functions in reverse address order.
// RPTnkMinePool::Destroy is defined here.
#include <Pack/RPParty/RPTnkScene/RPTnkWeaponManager.h>

#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPParty/RPTnkScene/RPTnkField.h>
#include <Pack/RPParty/RPTnkScene/RPTnkMine.h>
#include <Pack/RPParty/RPTnkScene/RPTnkShell.h>

RPTnkWeaponManager* RPTnkWeaponManager::sInstance;

inline void RPTnkWeaponManager::SetVelocity(RPTnkActor* pActor, nw4r::math::VEC3 velocity) {
    pActor->mVelocity = velocity;
}

inline void RPTnkWeaponManager::SetDirection(RPTnkActor* pActor, nw4r::math::VEC3 direction) {
    pActor->mRot.z = direction;
    PSVECNormalize(pActor->mRot.z, pActor->mRot.z);
    pActor->mRot.y = RP_vecEy;
    PSVECCrossProduct(pActor->mRot.y, pActor->mRot.z, pActor->mRot.x);
}

void RPTnkWeaponManager::CreateInstance() {
    sInstance = new RPTnkWeaponManager();
}

void RPTnkWeaponManager::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkWeaponManager::~RPTnkWeaponManager() {}

void RPTnkWeaponManager::Create() {
    RPTnkShellPool::CreateInstance();
    mpShellPool = RPTnkShellPool::sInstance;
    mpShellPool->Init(32);
    RPTnkMinePool::CreateInstance();
    mpMinePool = RPTnkMinePool::sInstance;
    mpMinePool->Init(16);
}

void RPTnkWeaponManager::Load() {
    mpShellPool->Load();
    mpMinePool->Load();
}

void RPTnkWeaponManager::Reset() {
    mpShellPool->Reset();
    mpMinePool->Reset();
}

void RPTnkWeaponManager::Calculate() {}

void RPTnkWeaponManager::ApplyVelocity() {
    mpShellPool->ApplyVelocityAll();
    mpMinePool->ApplyVelocityAll();
}

void RPTnkWeaponManager::ClearHitFlags() {
    mpShellPool->ClearForcesAll();
    mpMinePool->ClearForcesAll();
}

void RPTnkWeaponManager::CheckCollisions() {
    mpShellPool->CollideAllWithField();
    mpMinePool->CollideAllWithField();
}

void RPTnkWeaponManager::IntegrateAll() {
    mpShellPool->IntegrateAll();
    mpMinePool->IntegrateAll();
}

void RPTnkWeaponManager::CalcAll() {
    mpShellPool->CalcAll();
    mpMinePool->CalcAll();
}

void RPTnkWeaponManager::TickAge() {
    mpShellPool->TickAge();
    mpMinePool->TickAge();
}

void RPTnkWeaponManager::CalcModelMtx() {
    mpShellPool->CalcModelMtxAll();
    mpMinePool->CalcModelMtxAll();
}

void RPTnkMinePool::Destroy() { DestroyInstance(); }

void RPTnkWeaponManager::Exit() {
    mpShellPool->Destroy();
    mpMinePool->Destroy();
}

void RPTnkWeaponManager::DeactivateAll() {
    mpShellPool->KillAll();
    mpMinePool->KillAll();
}

// The signed/unsigned mix of the packed parameter decides its scratch registers.
void RPTnkWeaponManager::FireShell(f32 speed, RPTnkTank* pTank, nw4r::math::VEC3* pPos,
                                   nw4r::math::VEC3* pDir, s32 slot, s32 bounces, s32 smoke,
                                   s32 unk) {
    if (!RPTnkField::sInstance->IsInside(*pPos)) {
        return;
    }
    if (!RPTnkField::sInstance->IsBlockAtPos(*pPos)) {
        s32 smokeBits = smoke << 8;
        s32 bounceBits = bounces << 4;
        u32 param = bounceBits | ((unk << 12) | smokeBits);
        RPTnkActor* pShell = mpShellPool->Spawn(*pPos, slot | param);
        if (pShell != NULL) {
            nw4r::math::VEC3 vel = *pDir;
            PSVECNormalize(*pDir, *pDir);
            vel.x *= speed;
            vel.y *= speed;
            vel.z *= speed;
            SetDirection(pShell, *pDir);
            SetVelocity(pShell, vel);
            pShell->SetOwnerTank(pTank);
        }
    }
}

void RPTnkWeaponManager::LayMine(f32 speed, RPTnkTank* pTank, nw4r::math::VEC3* pPos,
                                 const nw4r::math::VEC3* pTarget, u32 param) {
    if (RPTnkField::sInstance->IsInside(*pPos)) {
        RPTnkActor* pMine = mpMinePool->Spawn(*pPos, param);
        if (pMine != NULL) {
            nw4r::math::VEC3 dir;
            dir.x = pTarget->x - pPos->x;
            dir.y = 0.0f;
            dir.z = pTarget->z - pPos->z;
            f32 len = PSVECMag(dir);
            if (len != 0.0f) {
                dir.x *= speed / len;
                dir.z *= speed / len;
            }
            pMine->SetTargetPos(nw4r::math::VEC3(*pTarget));
            SetVelocity(pMine, dir);
            pMine->SetOwnerTank(pTank);
        }
    }
}

// STAND-IN: dead-stripped; .sdata2 (0.5f lbl_804403E0, 0.0f 804403E4) is the same without it,
// but LayMine's code is not (5 of 124 words): the 0.5f literal must already exist when LayMine
// is compiled, so LayMine loads it before `stw r31` in its prologue, as the original does.
extern "C" f32 Tnk_weaponPoolOrderHalf(f32 x) { return x * 0.5f; }
