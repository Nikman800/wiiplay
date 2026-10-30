// Charge! scarecrow (RPCowKakasi): placed alone or spawned by an RPCowKakasiGroup, knocked away
// by the cow. Unit 0x802AC118-0x802AC88C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPCowScene/RPCowFieldObj.h>

#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPCowScene/RPCowKakasiGroup.h>
#include <Pack/RPParty/RPCowScene/RPCowPlayer.h>
#include <Pack/RPParty/RPPartyCommon/RPPathFollower.h>

#include <egg/math.h>


bool RPCowKakasi::IsEnabled(IRPActorPriority* pOther) {
    if (pOther != NULL) {
        RPCowPlayer* pPlayer = static_cast<RPCowPlayer*>(pOther);
        f32 speed = pPlayer->GetSpeed();
        EGG::Vector3f vel = pPlayer->GetVelocity();
        EGG::Vector3f fwd = pPlayer->GetForwardDir();
        EGG::Vector3f side(-fwd.z, 0.0f, fwd.x);
        EGG::Vector3f dir = mpCollision->GetPos() - pPlayer->GetPos();
        dir.normalise();
        EGG::Vector3f perp(-dir.z, 0.0f, dir.x);
        f32 dot = side.dot(dir);
        f32 align = 1.0f - __fabsf(dot);

        if (align >= 0.5f && speed >= 0.5f) {
            f32 sideSpeed = sSlideSide * dot;
            f32 height = speed * (align * (sSlideHeight - 1.5f)) + 1.5f;
            f32 fwdSpeed = speed * (align * (sSlideSpeedMax - sSlideSpeedMin)) + sSlideSpeedMin;
            mpCollision->GetRot().x =
                vel.x + fwd.x * fwdSpeed + side.x * sideSpeed;
            mpCollision->GetRot().y = height;
            mpCollision->GetRot().z =
                vel.z + fwd.z * fwdSpeed + side.z * sideSpeed;
            unkEC.y = speed * (__fabsf(dot) * 0.1f) + 0.1f;
            unkE0 = -perp;
            KnockSlide();
        } else {
            f32 sideSpeed = sFlySide * dot;
            f32 height = speed * (sFlyHeightMax - sFlyHeightMin) + sFlyHeightMin;
            f32 fwdSpeed = speed * (align * (sFlySpeedMax - sFlySpeedMin)) + sFlySpeedMin;
            mpCollision->GetRot().x =
                vel.x + fwd.x * fwdSpeed + side.x * sideSpeed;
            mpCollision->GetRot().y = height;
            mpCollision->GetRot().z =
                vel.z + fwd.z * fwdSpeed + side.x * sideSpeed;
            unkEC.y = speed * 0.1f + 0.1f;
            unkE0 = -perp;
            KnockFly();
        }

        if (mGroup != NULL) {
            mGroup->OnKakasiHit(pPlayer->GetUnk1A());
        }
        pPlayer->AddScore(mScore, mpCollision->GetPos());
    }

    mpCollision->mpOwner = NULL;
    return true;
}

RPCowKakasi::RPCowKakasi() {}

RPCowKakasi::~RPCowKakasi() {}

void RPCowKakasi::LoadResource() {
    SetSoundMode();

    RPCowGame* pGame = RPCowGame::GetInstance();
    if (pGame->GetPlayerNum() <= 1) {
        mpModel = RPGrpModel::Construct(6, 2, 0, 0);
        mpModel->SetDrawGroup(2);
        mpLodModels[0] = mpModel;
        mpModel = RPGrpModel::Construct(7, 2, 0, 0);
        mpModel->SetDrawGroup(2);
        mpLodModels[1] = mpModel;
        mpLodModels[2] = mpModel;
        mpLodModels[3] = NULL;
        mpLodModels[4] = NULL;
        mpLodModels[5] = NULL;
    } else {
        mpModel = NULL;
        mpLodModels[0] = NULL;
        mpLodModels[1] = NULL;
        mpLodModels[2] = NULL;
        mpLodModels[3] = pGame->GetSlotActor(3);
        mpLodModels[4] = pGame->GetSlotActor(3);
        mpLodModels[5] = pGame->GetSlotActor(3);
    }

    mpShadowModel = RPGrpModel::Construct(0x1A, 2, 0, 0x100);
    mpShadowLodModels[0] = mpShadowModel;
    mpShadowLodModels[1] = NULL;
    mpShadowMaterial = mpShadowModel->GetMaterial("cow_maru_kage");
    mpShadowLodModels[2] = mpShadowMaterial;
    mpShadowLodModels[3] = NULL;
    mpShadowLodModels[4] = NULL;
    mpShadowLodModels[5] = pGame->GetSlotActor(0x10);

    mpCollision = new RPCollisionShape();
    mpCollision->SetRadius(sRadius);

    mpPathObj = new RPPathObject(0.1f, 1.0f, NULL);
}

void RPCowKakasi::OnStart() {
    ResetTransform();
    mpCollision->Reset();
    mpCollision->mpOwner = static_cast<IRPActorEnable*>(this);
    mpCollision->SetRadius(sRadius);
    unkD4.x = sOfsX;
    unkD4.y = sOfsY;
    unkD4.z = 0.5f;
    mGroup = NULL;
}

void RPCowKakasi::OnStop() {
    Cow_objectColliders.Remove(mpCollision);
}

// Named consts after the code (loaded, not folded). The zeros are written `= 0`: an integer
// initialiser puts them in .sbss2 as in the original, `= 0.0f` would put them in .sdata2.
const f32 RPCowKakasi::sRadius = 8.0f;
const f32 RPCowKakasi::sOfsX = 16.0f;
const f32 RPCowKakasi::sOfsY = 8.0f;
const f32 RPCowKakasi::sSlideSide = 0;
const f32 RPCowKakasi::sSlideHeight = 3.0f;
const f32 RPCowKakasi::sSlideSpeedMin = 20.0f;
const f32 RPCowKakasi::sSlideSpeedMax = 40.0f;
const f32 RPCowKakasi::sFlySide = 1.0f;
const f32 RPCowKakasi::sFlyHeightMin = 2.0f;
const f32 RPCowKakasi::sFlyHeightMax = 3.0f;
const f32 RPCowKakasi::sFlySpeedMin = 0;
const f32 RPCowKakasi::sFlySpeedMax = 10.0f;
