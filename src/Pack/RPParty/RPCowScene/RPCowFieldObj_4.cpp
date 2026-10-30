// Charge! hurdle (RPCowHurdle): placed from the course's object table (resource type 0x70000);
// knocked away by a charging cow, otherwise jumped. Unit 0x802AD0DC-0x802ADAE4
// (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPCowScene/RPCowFieldObj.h>

#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPCowScene/RPCowPlayer.h>
#include <Pack/RPParty/RPPartyCommon/RPPathFollower.h>
#include <Pack/RPParty/RPPartyCommon/RPResFile.h>

#include <egg/math.h>
#include <nw4r/math.h>

bool RPCowHurdle::IsEnabled(IRPActorPriority* pOther) {
    if (pOther != NULL) {
        RPCowPlayer* pPlayer = static_cast<RPCowPlayer*>(pOther);
        if (pPlayer->GetUnkA8() == 1) {
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
                f32 fwdSpeed =
                    speed * (align * (sSlideSpeedMax - sSlideSpeedMin)) + sSlideSpeedMin;
                mpCollision->GetRot().x =
                    vel.x + fwd.x * fwdSpeed + side.x * sideSpeed;
                mpCollision->GetRot().y = height;
                mpCollision->GetRot().z =
                    vel.z + fwd.z * fwdSpeed + side.z * sideSpeed;
                unkEC.y = speed * 0.3f + 0.1f;
                unkE0 = RP_eggVecEy;
                if (dot < 0.0f) {
                    unkEC.y = -unkEC.y;
                }
                KnockSlide();
            } else {
                f32 sideSpeed = speed * (1.5f * dot);
                f32 height = speed * (sFlyHeightMax - sFlyHeightMin) + sFlyHeightMin;
                f32 fwdSpeed = speed * (align * (sFlySpeedMax - sFlySpeedMin)) + sFlySpeedMin;
                mpCollision->GetRot().x =
                    vel.x + fwd.x * fwdSpeed + side.x * sideSpeed;
                mpCollision->GetRot().y = height;
                mpCollision->GetRot().z =
                    vel.z + fwd.z * fwdSpeed + side.x * sideSpeed;
                unkEC.y = speed * 0.3f + 0.1f;
                unkE0 = RP_eggVecEy;
                if (dot < 0.0f) {
                    unkEC.y = -unkEC.y;
                }
                KnockFly();
            }
            mpCollision->mpOwner = NULL;
        } else {
            EGG::Vector3f objFwd = GetForwardDir();
            f32 facing = objFwd.dot(pPlayer->GetForwardDir());
            if (facing < 0.0f) {
                objFwd = -objFwd;
                facing = -facing;
            }
            EGG::Vector3f side(-objFwd.z, 0.0f, objFwd.x);
            EGG::Vector3f dir = mpCollision->GetPos() - pPlayer->GetPos();
            dir.normalise();
            f32 dot = side.dot(dir);
            mpCollision->GetRot().x = dir.x * sHurdleSpeed;
            mpCollision->GetRot().y = sHurdleHeight;
            mpCollision->GetRot().z = dir.z * sHurdleSpeed;
            unkEC.y = facing * (0.2f * -dot);
            unkE0 = RP_eggVecEy;
            KnockHurdle();
        }
    }

    return true;
}

RPCowHurdle::RPCowHurdle() {}

RPCowHurdle::~RPCowHurdle() {}

void RPCowHurdle::LoadResource() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    if (pGame->GetPlayerNum() <= 1) {
        mpModel = RPGrpModel::Construct(0xE, 2, 0, 0);
        mpModel->SetDrawGroup(2);
        mpLodModels[0] = mpModel;
        mpModel = RPGrpModel::Construct(0xF, 2, 0, 0);
        mpModel->SetDrawGroup(2);
        mpLodModels[1] = mpModel;
        mpLodModels[2] = mpModel;
        mpLodModels[3] = NULL;
        mpLodModels[4] = NULL;
        mpLodModels[5] = NULL;
    } else {
        mpModel = RPGrpModel::Construct(0xF, 2, 0, 0);
        mpModel->SetDrawGroup(2);
        mpLodModels[0] = mpModel;
        mpLodModels[1] = mpModel;
        mpLodModels[2] = mpModel;
        mpLodModels[3] = NULL;
        mpLodModels[4] = NULL;
        mpLodModels[5] = NULL;
    }

    mpShadowModel = RPGrpModel::Construct(0x1E, 2, 0, 0x100);
    mpShadowModel->SetUnk4(0);
    mpShadowLodModels[0] = mpShadowModel;
    mpShadowLodModels[1] = mpShadowModel;
    mpShadowMaterial = mpShadowModel->GetMaterial("cow_hurdle_kage");
    mpShadowLodModels[2] = mpShadowMaterial;
    mpShadowLodModels[3] = mpShadowMaterial;
    mpShadowLodModels[4] = NULL;
    mpShadowLodModels[5] = NULL;

    mpCollision = new RPCollisionShape();
    mpCollision->SetRadius(sRadius);

    mpPathObj = new RPPathObject(0.1f, 1.0f,
                                 pGame->GetResTable()->FindNthEntryOfType(0x70000, GetUnk1A()));
}

void RPCowHurdle::OnStart() {
    ResetTransform();
    mpCollision->Reset();
    mpCollision->GetPos() = mpPathObj->GetPos();
    mpCollision->GetVec40().y =
        nw4r::math::Atan2Rad(mpPathObj->GetVec20().x, mpPathObj->GetVec20().z);
    mpCollision->mpOwner = static_cast<IRPActorEnable*>(this);
    mpCollision->SetRadius(sRadius);
    Cow_objectColliders.Append(mpCollision, 4);
    unkD4.x = sOfsX;
    unkD4.y = sOfsY;
    unkD4.z = sOfsZ;
}

void RPCowHurdle::OnStop() {
    Cow_objectColliders.Remove(mpCollision);
}

// Named consts after the code (loaded, not folded). The zeros are written `= 0`: an integer
// initialiser puts them in .sbss2 as in the original, `= 0.0f` would put them in .sdata2.
const f32 RPCowHurdle::sRadius = 2.0f;
const f32 RPCowHurdle::sOfsX = 0;
const f32 RPCowHurdle::sOfsY = 0;
const f32 RPCowHurdle::sOfsZ = 1.0f;
const f32 RPCowHurdle::sSlideSide = 0;
const f32 RPCowHurdle::sSlideHeight = 3.0f;
const f32 RPCowHurdle::sSlideSpeedMin = 20.0f;
const f32 RPCowHurdle::sSlideSpeedMax = 40.0f;
const f32 RPCowHurdle::sFlyHeightMin = 1.0f;
const f32 RPCowHurdle::sFlyHeightMax = 2.0f;
const f32 RPCowHurdle::sFlySpeedMin = 0;
const f32 RPCowHurdle::sFlySpeedMax = 10.0f;
const f32 RPCowHurdle::sHurdleSpeed = 1.0f;
const f32 RPCowHurdle::sHurdleHeight = 1.0f;
