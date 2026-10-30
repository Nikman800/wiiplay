// Charge! cloud (RPCowCloud): cloudA/cloudB placed from the course's object table (resource
// types 0x70009/0x7000A), knocked away by the cow. Unit 0x802B8508-0x802B8DC4
// (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPCowScene/RPCowFieldObj.h>

#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPCowScene/RPCowPlayer.h>
#include <Pack/RPParty/RPPartyCommon/RPPathFollower.h>
#include <Pack/RPParty/RPPartyCommon/RPResFile.h>

#include <egg/math.h>
#include <nw4r/math.h>

bool RPCowCloud::IsEnabled(IRPActorPriority* pOther) {
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

RPCowCloud::RPCowCloud() {}

RPCowCloud::~RPCowCloud() {}

void RPCowCloud::LoadResource() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    if (pGame->GetPlayerNum() <= 1) {
        if (GetPriority() == 0xB) {
            mpModel = RPGrpModel::Construct(0x40, 2, 0, 0);
            mpPathObj = new RPPathObject(
                0.1f, 1.0f, pGame->GetResTable()->FindNthEntryOfType(0x70009, GetUnk1A()));
        } else {
            mpModel = RPGrpModel::Construct(0x41, 2, 0, 0);
            mpPathObj = new RPPathObject(
                0.1f, 1.0f, pGame->GetResTable()->FindNthEntryOfType(0x7000A, GetUnk1A()));
        }
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
            0.1f, 1.0f, pGame->GetResTable()->FindNthEntryOfType(0x70009, GetUnk1A()));
    }

    mpShadowModel = NULL;
    mpShadowLodModels[0] = NULL;
    mpShadowLodModels[1] = NULL;
    mpShadowMaterial = NULL;
    mpShadowLodModels[2] = NULL;
    mpShadowLodModels[3] = NULL;
    mpShadowLodModels[4] = NULL;
    mpShadowLodModels[5] = NULL;

    mpCollision = new RPCollisionShape();
    mpCollision->SetRadius(sRadius);
}

void RPCowCloud::OnStart() {
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

void RPCowCloud::OnStop() {
    Cow_objectColliders.Remove(mpCollision);
}

// Named consts after the code (loaded, not folded). The zeros are written `= 0`: an integer
// initialiser puts them in .sbss2 as in the original, `= 0.0f` would put them in .sdata2.
const f32 RPCowCloud::sRadius = 8.0f;
const f32 RPCowCloud::sOfsX = 6.0f;
const f32 RPCowCloud::sOfsY = 8.0f;
const f32 RPCowCloud::sOfsZ = 1.0f;
const f32 RPCowCloud::sSlideSide = 0;
const f32 RPCowCloud::sSlideHeight = 3.0f;
const f32 RPCowCloud::sSlideSpeedMin = 20.0f;
const f32 RPCowCloud::sSlideSpeedMax = 40.0f;
const f32 RPCowCloud::sFlyHeightMin = 0;
const f32 RPCowCloud::sFlySpeedMin = 0;
const f32 RPCowCloud::sFlySpeedMax = 20.0f;
