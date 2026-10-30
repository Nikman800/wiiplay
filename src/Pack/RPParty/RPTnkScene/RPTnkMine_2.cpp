// RPTnkMine and its sound actor (0x80266C14-0x80267F4C); the pool is in RPTnkMine.cpp.
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


RPTnkTank* RPTnkMine::GetOwnerTank() const {
    return mpOwnerTank;
}

BOOL RPTnkMine::IsExploding() const {
    return mState == 2;
}

s32 RPTnkMine::GetOwnerSlot() const {
    return mOwnerSlot;
}

void RPTnkMine::SetOwnerTank(RPTnkTank* pTank) {
    mpOwnerTank = pTank;
}

void RPTnkMine::SetTargetPos(const nw4r::math::VEC3& rPos) {
    mTargetPos = rPos;
}

RPTnkMineSoundActor::~RPTnkMineSoundActor() {}

RPTnkMine::RPTnkMine() {
    mStateFuncs[0] = &RPTnkMine::StateMoveToTarget;
    mStateFuncs[1] = &RPTnkMine::StateArmed;
    mStateFuncs[2] = &RPTnkMine::StateExplode;
}

RPTnkMine::~RPTnkMine() {}

void RPTnkMine::OnSpawn() {
    mHitRadius = 1.0f;
    mHitOffsetAlt.x = 0.0f;
    mHitOffsetAlt.y = 0.0f;
    mHitOffsetAlt.z = 16.0f;
    mPos.y = 0.0f;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    mRot = RP_mtx33Identity;
    mTimer = 0;
    unkC4 = 9.0f;
    mBlastRadius = 9.0f;
    mHitFlags[0] = false;
    mHitFlags[1] = false;
    mHitFlags[2] = false;
    mHitFlags[3] = false;
    mHitFlags[5] = false;
    mHitFlags[4] = false;
    mFuseLit = false;
    mpOwnerTank = NULL;
    mState = 0;
}

void RPTnkMine::ApplySpawnParam(u32 param) {
    mOwnerSlot = param & 0xF;
    mIsEnemy = mOwnerSlot >= 2;
    mpModel->GetModelAnm()->Cancel(RPGrpModelAnm::Anm_Clr, 0);
    mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
}

void RPTnkMine::StateMoveToTarget() {
    f32 speed = nw4r::math::VEC3Len(&mVelocity);
    if (speed == 0.0f) {
        mState = 1;
    } else {
        nw4r::math::VEC3 dir;
        dir.x = mTargetPos.x - mPos.x;
        dir.y = mTargetPos.y - mPos.y;
        dir.z = mTargetPos.z - mPos.z;
        f32 dist = nw4r::math::VEC3Len(&dir);
        if (dist == 0.0f) {
            mState = 1;
        } else {
            nw4r::math::VEC3Normalize(&dir, &dir);
            if (dist <= speed) {
                mVelocity.x = dist * dir.x;
                mVelocity.y = dist * dir.y;
                mVelocity.z = dist * dir.z;
                mState = 1;
            } else {
                mVelocity.x = speed * dir.x;
                mVelocity.y = speed * dir.y;
                mVelocity.z = speed * dir.z;
            }
        }
    }
    if (mHitFlags[3]) {
        mVelocity = RP_vecZero;
        mState = 1;
    }
    if (mHitFlags[0]) {
        Kill();
    } else if (mHitFlags[4]) {
        Kill();
    }
}

void RPTnkMine::StateArmed() {
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    if (mHitFlags[4]) {
        Kill();
    } else if (mHitFlags[1] || mHitFlags[2]) {
        Explode();
    } else if (!mFuseLit) {
        if (mAge >= 480) {
            mTimer = 120;
            mFuseLit = true;
        } else if (mHitFlags[0]) {
            mTimer = 20;
            mFuseLit = true;
        }
    } else {
        if (--mTimer <= 0) {
            Explode();
        }
        if (mOwnerSlot == 0) {
            RPTnkTankManager::sInstance->GetPlayerTank(0)->HoldMineFuseSe();
        } else if (mOwnerSlot == 1) {
            RPTnkTankManager::sInstance->GetPlayerTank(1)->HoldMineFuseSe();
        }
    }
}

void RPTnkMine::StateExplode() {
    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    if (mTimer >= 6) {
        unkC4 = 80.0f;
        mBlastRadius = 80.0f;
    } else {
        f32 t = static_cast<f32>(mTimer) / 6.0f;
        f32 radius = t * 72.0f + 8.0f;
        unkC4 = radius;
        mBlastRadius = radius;
    }
    if (++mTimer >= 40) {
        Kill();
    }
}

void RPTnkMine::Create() {
    RPTnkActor::Create();
    mpMineSound = new RPTnkMineSoundActor;
}

// Only string-pool symbol names differ; instructions and offsets match.
void RPTnkMine::Load() {
    struct ResourceAccess {
        static void* Get(RPGrpModelResManager* manager, RPGrpModelResManager::Type type,
                         const char* name, RPGrpHandle handle) {
            return manager->GetPtr(type, handle, name);
        }
    };
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pManager = RPGrpModelResManager::GetCurrent();
    void* pFile = RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/tnk_mine.brres");
    RPGrpHandle file = pManager->CreateData(RPGrpModelResManager::Type_ResFile, pFile);
    void* pModel = ResourceAccess::Get(pManager, RPGrpModelResManager::Type_ResMdl, "tnk_mine", file);
    RPGrpHandle model = pManager->CreateData(RPGrpModelResManager::Type_ResMdl, pModel);
    mpModel = RPGrpModel::Construct(model, 1, 0, 0x108);
    void* pAnm = ResourceAccess::Get(pManager, RPGrpModelResManager::Type_ResAnmClr, "tnk_mine", file);
    RPGrpHandle anm = pManager->CreateData(RPGrpModelResManager::Type_ResAnmClr, pAnm);
    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);
    mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Clr, 0, anm, 0);
    mpModel->SetDrawGroup(0);
    mpModel->SetDrawScene(0);
    mpModel->SetUnkCFlags(0x12);
}

void RPTnkMine::ApplyVelocity() {
    mPos.x += mVelocity.x;
    mPos.y += mVelocity.y;
    mPos.z += mVelocity.z;
}

void RPTnkMine::ClearForcesStep() {
    RPTnkActor::ClearForcesStep();
    mHitFlags[0] = false;
    mHitFlags[1] = false;
    mHitFlags[2] = false;
    mHitFlags[3] = false;
    mHitFlags[4] = false;
}

void RPTnkMine::CollideWithField() {
    struct HitResult {
        RPTnkTank* pTank;
        bool enemy;
        s32 index;
        u32 unkC;
        f32 dist;
    } result;

    mHitRadius = 17.1f;
    RPTnkField::sInstance->CollideActor(this, 0);
    switch (mState) {
    case 0:
    case 1:
        if (!mHitFlags[5]) {
            mHitRadius = 90.0f;
            s32 owner = mIsEnemy ? mOwnerSlot - 2 : mOwnerSlot;
            if (!RPTnkTankManager::sInstance->TestMineHitTank(this, owner, mIsEnemy)) {
                mHitFlags[5] = true;
            }
        } else {
            mHitRadius = 70.0f;
            if (RPTnkTankManager::sInstance->FindMineHit(this, &result)) {
                mHitFlags[0] = true;
            }
        }
        mHitRadius = 12.0f;
        if (static_cast<u32>(RPTnkShellPool::sInstance->IsObjectHit(this, TRUE)) == TRUE) {
            mHitFlags[1] = true;
        }
        mHitRadius = 12.0f;
        if (RPTnkMinePool::sInstance->IsInOtherExplosion(this)) {
            mHitFlags[2] = true;
        }
        break;
    case 2:
        if (mTimer < 20) {
            mHitRadius = mBlastRadius;
            if (RPTnkTankManager::sInstance->FindMineHit(this, &result)) {
                RPTnkTank::HitInfo hit;
                if (mOwnerSlot == 0) {
                    hit.mId = 1;
                } else {
                    s32 id = 3;
                    if (mOwnerSlot == 1) {
                        id = 2;
                    }
                    hit.mId = id;
                }
                hit.mDist = result.dist;
                RPTnkTank::HitInfo copy = hit;
                RPTnkTankManager::sInstance->RecordMineHit(result.enemy, result.index, &copy);
            }
        }
        break;
    }
}

void RPTnkMine::Calculate() {
    (this->*mStateFuncs[mState])();
    mpSound->set3DPosition(mPos);
}

void RPTnkMine::CalcModelMtx() {
    if (mState != 2) {
        nw4r::math::MTX34 mtx;
        mtx._00 = RP_vecOne.x * mRot.x.x;
        mtx._01 = RP_vecOne.y * mRot.y.x;
        mtx._02 = RP_vecOne.z * mRot.z.x;
        mtx._03 = mPos.x;
        mtx._10 = RP_vecOne.x * mRot.x.y;
        mtx._11 = RP_vecOne.y * mRot.y.y;
        mtx._12 = RP_vecOne.z * mRot.z.y;
        mtx._13 = mPos.y;
        mtx._20 = RP_vecOne.x * mRot.x.z;
        mtx._21 = RP_vecOne.y * mRot.y.z;
        mtx._22 = RP_vecOne.z * mRot.z.z;
        mtx._23 = mPos.z;
        nw4r::g3d::ScnObj* pScnObj = mpModel->GetScnObj();
        if (pScnObj != NULL) {
            pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
        }
        mpModel->SetBaseScale(mScale);
        if (mFuseLit) {
            mpModel->UpdateFrame();
        }
        mpModel->Calc();
        mpModel->Entry();
    }
}

BOOL RPTnkMine::TestHit(RPTnkActor* pOther, f32* pDist) {
    f32 x, z;
    f32 mineRadius = 12.0f;
    nw4r::math::VEC3 center = pOther->GetHitCenter();
    f32 radius = mineRadius + GetActorHitRadius(pOther);
    x = center.x - (mPos.x + mHitOffset.x);
    f32 radius2 = radius * radius;
    x *= x;
    if (x > radius2) return FALSE;
    z = center.z - (mPos.z + mHitOffset.z);
    z *= z;
    if (z > radius2) return FALSE;
    if (x + z > radius2) return FALSE;
    if (pDist != NULL) *pDist = nw4r::math::FSqrt(x + z);
    return TRUE;
}

BOOL RPTnkMine::TestHitAlt(RPTnkActor* pOther, f32* pDist) {
    f32 x, z;
    f32 mineRadius = 12.0f;
    nw4r::math::VEC3 center = pOther->GetHitCenter();
    f32 radius = mineRadius + GetActorHitRadius(pOther);
    x = center.x - (mPos.x + mHitOffsetAlt.x);
    f32 radius2 = radius * radius;
    x *= x;
    if (x > radius2) return FALSE;
    z = center.z - (mPos.z + mHitOffsetAlt.z);
    z *= z;
    if (z > radius2) return FALSE;
    if (x + z > radius2) return FALSE;
    if (pDist != NULL) *pDist = nw4r::math::FSqrt(x + z);
    return TRUE;
}

#pragma peephole off
BOOL RPTnkMine::IsInBlastRadius(const RPTnkActor* pOther) const {
    if (mState != 2) {
        return FALSE;
    }
    nw4r::math::VEC3 center = pOther->GetHitCenter();
    f32 radius = mBlastRadius + GetActorHitRadius(pOther);
    f32 x = center.x - (mPos.x + mHitOffset.x);
    f32 radius2 = radius * radius;
    x *= x;
    if (x > radius2) {
        return FALSE;
    }
    f32 z = center.z - (mPos.z + mHitOffset.z);
    z *= z;
    if (z > radius2) {
        return FALSE;
    }
    return !(x + z > radius2);
}

BOOL RPTnkMine::IsInTriggerRange(const RPTnkActor* pOther) const {
    nw4r::math::VEC3 center = pOther->GetHitCenter();
    f32 radius = Tnk_getMineProximityRadius() + GetActorHitRadius(pOther);
    f32 x = center.x - (mPos.x + mHitOffset.x);
    f32 radius2 = radius * radius;
    x *= x;
    if (x > radius2) {
        return FALSE;
    }
    f32 z = center.z - (mPos.z + mHitOffset.z);
    z *= z;
    if (z > radius2) {
        return FALSE;
    }
    return !(x + z > radius2);
}
#pragma peephole reset

void RPTnkMine::Explode() {
    mFuseLit = true;
    mTimer = 0;
    {
        RPSysEffect effect("Tnk_mine_bomb", 0, 0);
        effect.create();
        nw4r::math::VEC3 scale;
        nw4r::math::VEC3 pos(mPos.x, mPos.y, mPos.z);
        effect.setPos(pos);
        scale.x = 1.6f;
        scale.y = 1.6f;
        scale.z = 1.6f;
        effect.setScale(scale);
        effect.update();
    }
    mpMineSound->startSound(static_cast<unsigned int>(0x16), 0);
    mState = 2;
}

void RPTnkMine::OnHitWall(f32 depth, const nw4r::math::VEC3* pNormal, s32 wall,
                         const nw4r::math::VEC3& rVel) {
    f32 x = depth * pNormal->x;
    f32 z = depth * pNormal->z;
    f32 y = depth * pNormal->y;
    mPosAccum[0].x = mPosAccum[0].x > x ? x : mPosAccum[0].x;
    mPosAccum[0].y = mPosAccum[0].y > y ? y : mPosAccum[0].y;
    mPosAccum[0].z = mPosAccum[0].z > z ? z : mPosAccum[0].z;
    mPosAccum[1].x = mPosAccum[1].x < x ? x : mPosAccum[1].x;
    mPosAccum[1].y = mPosAccum[1].y < y ? y : mPosAccum[1].y;
    mPosAccum[1].z = mPosAccum[1].z < z ? z : mPosAccum[1].z;
    f32 vx, vy, vz;
    f32 push = -nw4r::math::VEC3Dot(&mVelocity, pNormal);
    if (push > 0.0f) {
        vx = push * pNormal->x;
        vz = push * pNormal->z;
        vy = push * pNormal->y;
        mVelAccum[0].x = mVelAccum[0].x > vx ? vx : mVelAccum[0].x;
        mVelAccum[0].y = mVelAccum[0].y > vy ? vy : mVelAccum[0].y;
        mVelAccum[0].z = mVelAccum[0].z > vz ? vz : mVelAccum[0].z;
        mVelAccum[1].x = mVelAccum[1].x < vx ? vx : mVelAccum[1].x;
        mVelAccum[1].y = mVelAccum[1].y < vy ? vy : mVelAccum[1].y;
        mVelAccum[1].z = mVelAccum[1].z < vz ? vz : mVelAccum[1].z;
    }
    mHitFlags[3] = true;
}

extern "C" f32 Tnk_getMineProximityRadius() {
    return 80.0f;
}
