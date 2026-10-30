// Charge! knitted cow extra (RPCowKnitCowEx): exA/exB placed from the course's object table
// (resource types 0x70007/0x70008), waiting or eating, knocked away by the cow.
// Unit 0x802B8DC4-0x802B9800 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPCowScene/RPCowFieldObj.h>

#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpModelAnm.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPCowScene/RPCowPlayer.h>
#include <Pack/RPParty/RPPartyCommon/RPPathFollower.h>
#include <Pack/RPParty/RPPartyCommon/RPResFile.h>
#include <Pack/RPUtility/RPUtlRandom.h>

#include <egg/math.h>
#include <nw4r/math.h>

bool RPCowKnitCowEx::IsEnabled(IRPActorPriority* pOther) {
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
            f32 sideSpeed = speed * (sSlideSide * dot);
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
            f32 sideSpeed = speed * (1.5f * dot);
            f32 height = speed * (1.5f - sFlyHeightMin) + sFlyHeightMin;
            f32 fwdSpeed = speed * (align * (sFlySpeedMax - sFlySpeedMin)) + sFlySpeedMin;
            mpCollision->GetRot().x =
                vel.x + fwd.x * fwdSpeed + side.x * sideSpeed;
            mpCollision->GetRot().y = height;
            mpCollision->GetRot().z =
                vel.z + fwd.z * fwdSpeed + side.x * sideSpeed;
            unkEC.y = speed * 0.10000001f + 0.05f;
            unkE0 = -perp;
            KnockFly();
        }
    }

    mpCollision->mpOwner = NULL;
    return true;
}

RPCowKnitCowEx::RPCowKnitCowEx() {}

RPCowKnitCowEx::~RPCowKnitCowEx() {}

void RPCowKnitCowEx::LoadResource() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    if (pGame->GetPlayerNum() <= 1) {
        if (GetPriority() == 0xB) {
            mpModel = RPGrpModel::Construct(0x2C, 2, 0, 0);
            mpPathObj = new RPPathObject(
                0.1f, 1.0f, pGame->GetResTable()->FindNthEntryOfType(0x70007, GetUnk1A()));
        } else {
            mpModel = RPGrpModel::Construct(0x2D, 2, 0, 0);
            mpPathObj = new RPPathObject(
                0.1f, 1.0f, pGame->GetResTable()->FindNthEntryOfType(0x70008, GetUnk1A()));
        }
        mpModel->SetDrawGroup(2);
        mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
        if (RPUtlRandom::getU32(2) != 0) {
            mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 0, 0x2E, 0);
        } else {
            mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, 0, 0x2F, 0);
        }
        mpModel->GetModelAnm()->SetBlendAnm(RPGrpModelAnm::Anm_Chr, 0, 0);
        mpModel->GetModelAnm()->SetBlendWeight(RPGrpModelAnm::Anm_Chr, 0, 1.0f, 0);
        mpLodModels[0] = mpModel;
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
        mpLodModels[3] = NULL;
        mpLodModels[4] = NULL;
        mpLodModels[5] = NULL;
        mpPathObj = new RPPathObject(
            0.1f, 1.0f, pGame->GetResTable()->FindNthEntryOfType(0x70007, GetUnk1A()));
    }

    mpShadowModel = RPGrpModel::Construct(0x1A, 2, 0, 0x100);
    mpShadowLodModels[0] = mpShadowModel;
    mpShadowLodModels[1] = NULL;
    mpShadowMaterial = mpShadowModel->GetMaterial("cow_maru_kage");
    mpShadowLodModels[2] = mpShadowMaterial;
    mpShadowLodModels[3] = NULL;
    mpShadowLodModels[4] = pGame->GetSlotActor(0x10);
    mpShadowLodModels[5] = pGame->GetSlotActor(0x10);

    mpCollision = new RPCollisionShape();
    mpCollision->SetRadius(sRadius);
}

void RPCowKnitCowEx::OnStart() {
    ResetTransform();
    mpCollision->Reset();
    mpCollision->GetPos() = mpPathObj->GetPos();
    mpCollision->GetVec40().y =
        nw4r::math::Atan2Rad(mpPathObj->GetVec20().x, mpPathObj->GetVec20().z);
    mpCollision->mpOwner = static_cast<IRPActorEnable*>(this);
    mpCollision->SetRadius(sRadius * mpPathObj->GetRadius());
    Cow_objectColliders.Append(mpCollision, 4);
    unkD4.x = sOfsX;
    unkD4.y = sOfsY;
    unkD4.z = sOfsZ;
}

void RPCowKnitCowEx::OnStop() {
    Cow_objectColliders.Remove(mpCollision);
}

// Named consts after the code (loaded, not folded). The zeros are written `= 0`: an integer
// initialiser puts them in .sbss2 as in the original, `= 0.0f` would put them in .sdata2.
const f32 RPCowKnitCowEx::sRadius = 8.0f;
const f32 RPCowKnitCowEx::sOfsX = 6.0f;
const f32 RPCowKnitCowEx::sOfsY = 8.0f;
const f32 RPCowKnitCowEx::sOfsZ = 1.0f;
const f32 RPCowKnitCowEx::sSlideSide = 0;
const f32 RPCowKnitCowEx::sSlideHeight = 3.0f;
const f32 RPCowKnitCowEx::sSlideSpeedMin = 20.0f;
const f32 RPCowKnitCowEx::sSlideSpeedMax = 40.0f;
const f32 RPCowKnitCowEx::sFlyHeightMin = 0;
const f32 RPCowKnitCowEx::sFlySpeedMin = 0;
const f32 RPCowKnitCowEx::sFlySpeedMax = 20.0f;
