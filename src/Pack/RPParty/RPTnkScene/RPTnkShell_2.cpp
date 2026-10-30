#include <Pack/RPParty/RPTnkScene/RPTnkShell.h>
#include <Pack/RPParty/RPTnkScene/RPTnkField.h>
#include <Pack/RPParty/RPTnkScene/RPTnkMine.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTankManager.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

extern RPAxes RP_mtx33Identity;
extern nw4r::math::VEC3 RP_vecEy;

static RPTnkShell::StateFunc sShellStateFuncs[] = {
    &RPTnkShell::StateResolveHits,
};

static inline void* GetShellModel(RPGrpModelResManager* pManager,
                                  RPGrpHandle file, const char* pName) {
    return pManager->GetPtr(RPGrpModelResManager::Type_ResMdl, file, pName);
}

static inline void AccumulateShellExtrema(nw4r::math::VEC3* accum,
                                          const nw4r::math::VEC3& vec, f32 scale) {
    f32 x, y, z;
    x = scale * vec.x;
    z = scale * vec.z;
    y = scale * vec.y;
    accum[0].x = accum[0].x > x ? x : accum[0].x;
    accum[0].y = accum[0].y > y ? y : accum[0].y;
    accum[0].z = accum[0].z > z ? z : accum[0].z;
    accum[1].x = accum[1].x < x ? x : accum[1].x;
    accum[1].y = accum[1].y < y ? y : accum[1].y;
    accum[1].z = accum[1].z < z ? z : accum[1].z;
}

static inline nw4r::math::VEC3 ShellHitCenter(const nw4r::math::VEC3& pos,
                                            const nw4r::math::VEC3& offset) {
    nw4r::math::VEC3 center;
    center.x = pos.x + offset.x;
    center.y = pos.y + offset.y;
    center.z = pos.z + offset.z;
    return center;
}

static inline void SetShellHitKind(u32* pKind, s32 slot) {
    u32 kind = 3;
    if (slot == 1) {
        kind = 2;
    }
    *pKind = kind;
}

static inline f32 ShellRadiusSum(f32 shellRadius, f32 otherRadius) {
    return shellRadius + otherRadius;
}

static inline void SetShellVector(nw4r::math::VEC3* out, f32 x, f32 y, f32 z) {
    out->x = x;
    out->y = y;
    out->z = z;
}

RPTnkTank* RPTnkShell::GetOwnerTank() const {
    return mpOwnerTank;
}

s32 RPTnkShell::GetOwnerSlot() const {
    return mOwnerSlot;
}

void RPTnkShell::SetOwnerTank(RPTnkTank* pTank) {
    mpOwnerTank = pTank;
}

extern "C" f32 Tnk_getShellRadius() {
    return 6.0f;
}

RPTnkShell::RPTnkShell() {
    mStateFuncs[0] = sShellStateFuncs[0];
}

RPTnkShell::~RPTnkShell() {}

void RPTnkShell::OnSpawn() {
    mScale.x = 1.5238097f;
    mHitRadius = 1.0f;
    mScale.y = 1.5238097f;
    mScale.z = 1.5238097f;
    mRot = RP_mtx33Identity;
    mHitTank = false;
    mHitFlags[0] = false;
    mHitFlags[1] = false;
    mHitFlags[2] = false;
    mHitFlags[3] = false;
    mBouncesLeft = 1;
    mpOwnerTank = NULL;
    mState = 0;
}

void RPTnkShell::ApplySpawnParam(u32 param) {
    mOwnerSlot = param & 0xF;
    mIsEnemy = mOwnerSlot >= 2;
    mBouncesLeft = (param >> 4) & 0xF;
    mSmokeType = (param >> 8) & 7;
    mpSmokeEffects[mSmokeType]->create();
    unkBC = (param >> 12) & 0xF;
}

void RPTnkShell::StateResolveHits() {
    mVelocity.y = 0.0f;
    nw4r::math::VEC3 direction = mVelocity;
    f32 speed = nw4r::math::VEC3Len(&direction);
    if (speed != 0.0f) {
        f32 invSpeed = 1.0f / speed;
        direction.x *= invSpeed;
        direction.y *= invSpeed;
        direction.z *= invSpeed;
        mRot.z.x = direction.x;
        mRot.z.y = direction.y;
        mRot.z.z = direction.z;
        mRot.y.x = RP_vecEy.x;
        mRot.y.y = RP_vecEy.y;
        mRot.y.z = RP_vecEy.z;
        PSVECCrossProduct(mRot.y, mRot.z, mRot.x);
    }

    nw4r::math::MTX34 mtx;
    mtx._00 = mScale.x * mRot.x.x;
    mtx._01 = mScale.y * mRot.y.x;
    mtx._02 = mScale.z * mRot.z.x;
    mtx._03 = mPos.x;
    mtx._10 = mScale.x * mRot.x.y;
    mtx._11 = mScale.y * mRot.y.y;
    mtx._12 = mScale.z * mRot.z.y;
    mtx._13 = mPos.y;
    mtx._20 = mScale.x * mRot.x.z;
    mtx._21 = mScale.y * mRot.y.z;
    mtx._22 = mScale.z * mRot.z.z;
    mtx._23 = mPos.z;
    for (s32 i = 0; i < 4; i++) {
        mpSmokeEffects[i]->setMtx(mtx);
        mpSmokeEffects[i]->update();
    }

    if (mHitFlags[2]) {
        if (--mBouncesLeft <= -1) {
            {
                RPSysEffect effect("Tnk_shell_bomb", 0, 0);
                effect.create();
                nw4r::math::VEC3 scale;
                nw4r::math::VEC3 pos(mPos.x, mPos.y, mPos.z);
                effect.setPos(pos);
                SetShellVector(&scale, mScale.x, mScale.y, mScale.z);
                effect.setScale(scale);
                effect.update();
            }
            mpSound->startSound(static_cast<unsigned int>(0x11), 0);
            Kill();
            return;
        }
        if (mHitFlags[3]) {
            mBouncesLeft = -1;
            {
                RPSysEffect effect("Tnk_shell_bomb", 0, 0);
                effect.create();
                nw4r::math::VEC3 scale;
                nw4r::math::VEC3 pos(mPos.x, mPos.y, mPos.z);
                effect.setPos(pos);
                SetShellVector(&scale, mScale.x, mScale.y, mScale.z);
                effect.setScale(scale);
                effect.update();
            }
            mpSound->startSound(static_cast<unsigned int>(0x11), 0);
            Kill();
            return;
        }
        if (unkBC == 7) {
            mpSound->startSound(static_cast<unsigned int>(0xF), 0);
        } else {
            mpSound->startSound(static_cast<unsigned int>(0x10), 0);
        }
        {
            RPSysEffect effect("Tnk_shell_crash", 0, 0);
            effect.create();
            nw4r::math::VEC3 scale;
            nw4r::math::VEC3 pos(mPos.x, mPos.y, mPos.z);
            effect.setPos(pos);
            SetShellVector(&scale, mScale.x, mScale.y, mScale.z);
            effect.setScale(scale);
            effect.update();
        }
    }

    if (mHitTank || mHitFlags[1]) {
        mBouncesLeft = -1;
        Kill();
        return;
    }
    if (mHitFlags[0]) {
        mBouncesLeft = -1;
        Kill();
        return;
    }
    switch (unkBC) {
    case 3:
        mpSound->holdSound(static_cast<unsigned int>(5), 0);
        break;
    case 7:
        mpSound->holdSound(static_cast<unsigned int>(9), 0);
        break;
    case 9:
        mpSound->holdSound(static_cast<unsigned int>(0xB), 0);
        break;
    }
}

void RPTnkShell::Create() {
    RPTnkActor::Create();
    mpSmokeEffects[0] = new RPSysEffect("Tnk_shellsmoke01", 0, 0);
    mpSmokeEffects[1] = new RPSysEffect("Tnk_shellsmoke02", 0, 0);
    mpSmokeEffects[2] = new RPSysEffect("Tnk_shellsmoke03", 0, 0);
    mpSmokeEffects[3] = new RPSysEffect("Tnk_shellsmoke03", 0, 0);
}

void RPTnkShell::Load() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive(-1, NULL);
    RPGrpModelResManager* pManager = RPGrpModelResManager::GetCurrent();
    void* pFile = RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/shell.brres", NULL);
    RPGrpHandle file = pManager->CreateData(RPGrpModelResManager::Type_ResFile, pFile, 0);
    void* pModel = GetShellModel(pManager, file, "shell");
    u32 model = pManager->CreateData(RPGrpModelResManager::Type_ResMdl, pModel, 0);
    mpModel = RPGrpModel::Construct(model, 1, 0, 0);
    mpModel->SetDrawGroup(0);
    mpModel->SetDrawScene(0);
    mpModel->SetUnkCFlags(0x12);
}

void RPTnkShell::ApplyVelocity() {
    mPos.x += mVelocity.x;
    mPos.y += mVelocity.y;
    mPos.z += mVelocity.z;
}

void RPTnkShell::ClearForcesStep() {
    RPTnkActor::ClearForcesStep();
    mHitTank = false;
    mHitFlags[0] = false;
    mHitFlags[1] = false;
    mHitFlags[2] = false;
    mHitFlags[3] = false;
}

void RPTnkShell::CollideWithField() {
    struct HitResult {
        RPTnkTank* pTank;
        bool enemy;
        s32 index;
        u32 unkC;
        f32 distance;
    } result;
    struct HitInfo {
        u32 kind;
        f32 distance;
    } hit;

    mHitRadius = 6.0f;
    RPTnkField::sInstance->CollideActor(this, 3);
    mHitRadius = 6.0f;
    if (RPTnkTankManager::sInstance->FindShellHit(this, &result) &&
        (mAge >= 4 || mpOwnerTank != result.pTank)) {
        mHitTank = true;
        if (mOwnerSlot == 0) {
            hit.kind = 1;
        } else {
            SetShellHitKind(&hit.kind, mOwnerSlot);
        }
        hit.distance = result.distance;
        HitInfo copy = hit;
        RPTnkTankManager::sInstance->RecordShellHit(result.enemy, result.index, &copy);
    }
    mHitRadius = 6.0f;
    if (RPTnkMinePool::sInstance->CheckShellHit(this)) {
        mHitFlags[1] = true;
    }
    if (RPTnkShellPool::sInstance->CollideShellWithShells(this)) {
        mHitFlags[0] = true;
    }
}

void RPTnkShell::Calculate() {
    (this->*mStateFuncs[mState])();
    mpSound->set3DPosition(mPos);
}

void RPTnkShell::Kill() {
    RPTnkActor::Kill();
    for (s32 i = 0; i < 4; i++) {
        mpSmokeEffects[i]->fade();
    }
}

void RPTnkShell::StopEffects() {
    for (s32 i = 0; i < 4; i++) {
        mpSmokeEffects[i]->fade();
    }
}

BOOL RPTnkShell::TestHit(RPTnkActor* pOther, f32* pDist) {
    f32 radius, otherRadius;
    f32 dx, dz;
    const nw4r::math::VEC3& otherPos = pOther->*(&RPTnkShell::mPos);
    const nw4r::math::VEC3& otherOffset = pOther->*(&RPTnkShell::mHitOffsetAlt);
    nw4r::math::VEC3 center = ShellHitCenter(otherPos, otherOffset);
    otherRadius = pOther->*(&RPTnkShell::mHitRadius);
    radius = ShellRadiusSum(Tnk_getShellRadius(), otherRadius);
    dx = center.x - (mPos.x + mHitOffset.x);
    radius *= radius;
    dx *= dx;
    if (dx > radius) {
        return FALSE;
    }
    dz = center.z - (mPos.z + mHitOffset.z);
    dz *= dz;
    if (dz > radius) {
        return FALSE;
    }
    if (dx + dz > radius) {
        return FALSE;
    }
    if (pDist != NULL) {
        *pDist = nw4r::math::FSqrt(dx + dz);
    }
    return TRUE;
}

void RPTnkShell::OnHitWall(f32 depth, const nw4r::math::VEC3* pNormal, s32,
                          const nw4r::math::VEC3&) {
    f32 dot = nw4r::math::VEC3Dot(&mVelocity, pNormal);
    f32 speed = nw4r::math::VEC3Len(&mVelocity);

    if (dot < 0.0f) {
        nw4r::math::VEC3 reverse;
        reverse.x = -mVelocity.x;
        reverse.y = 0.0f;
        reverse.z = -mVelocity.z;
        nw4r::math::VEC3Normalize(&reverse, &reverse);
        f32 distance = nw4r::math::VEC3Dot(&reverse, pNormal);
        if (distance != 0.0f) {
            distance = depth / distance;
        }
        if (distance > speed) {
            AccumulateShellExtrema(mPosAccum, *pNormal, depth);
        } else {
            AccumulateShellExtrema(mPosAccum, reverse, distance);
        }
    } else {
        AccumulateShellExtrema(mPosAccum, *pNormal, depth);
    }
    f32 reflect = -2.0f * dot;
    if (reflect > 0.0f) {
        AccumulateShellExtrema(mVelAccum, *pNormal, reflect);
    }
    mHitFlags[2] = true;
    if (mAge <= 6) {
        mHitFlags[3] = true;
    }
}

void RPTnkShell::PlayHitSound() {
    mpSound->startSound(static_cast<unsigned int>(0x11), 0);
}
