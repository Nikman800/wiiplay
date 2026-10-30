// Table Tennis table: model, net animation, ball bounces on the table top and the net.
// Unit 0x80226D04-0x802274C0 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpField.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPPnpScene/RPPnpBall.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>


// Table size units: the first objects of this unit's .sdata2 (read by the ball and the players
// too). Defined before Load, whose anmNames initializer is the next .sdata2 object (an aggregate
// initializer is emitted when its function is parsed). The code reads them only through the
// header's inline getters, which were parsed before these values, so they are loaded, not folded.
const f32 RPPnpTable::sLengthUnit = 34.0f;
const f32 RPPnpTable::sWidthUnit = 27.0f;
const f32 RPPnpTable::sNetHeightUnit = 2.08f;
const f32 RPPnpTable::sHeightUnit = 9.12f;

f32 RPPnpTable::sRestitution = 0.9f;
f32 RPPnpTable::sFriction = 0.2f;

nw4r::math::VEC3 RPPnpTable::sBounceScale(2.0f, 1.0f, 2.0f);

RPPnpTable::~RPPnpTable() {}

f32 RPPnpTable::GetBallRestHeight() const {
    return RPPnpGame::GetInstance()->GetBall()->GetRadius() / 2.0f + GetHeight();
}

void RPPnpTable::Create() {
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = 0.0f;
    mScale.x = 10.0f;
    mScale.y = 10.0f;
    mScale.z = 10.0f;
}

void RPPnpTable::Load() {
    RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/pin_table.brres");
    RPSimpleModel::LoadModel("pin_table", file, 1, 0, 0);
    mpModel->SetDrawGroup(2);

    const char* anmNames[2] = {"pin_table_a", "pin_table_b"};
    RPGrpHandle anms[2];

    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    for (u32 i = 0; i < 2; i++) {
        RPGrpHandle anm = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResAnmChr,
            pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmChr, file, anmNames[(u16)i]));
        anms[(u16)i] = anm;
    }

    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 2);
    for (u32 i = 0; i < 2; i++) {
        mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, i, anms[(u16)i]);
    }
}

void RPPnpTable::Reset() {}

void RPPnpTable::Calculate() {
    CalcModelMtx();
}

void RPPnpTable::Exit() {}

bool RPPnpTable::HandleBallCollision() {
    bool hit = false;

    RPPnpBall* pBall = RPPnpGame::GetInstance()->GetBall();
    nw4r::math::VEC3 prevPos = pBall->GetPrevPos();
    nw4r::math::VEC3 pos = pBall->GetPos();
    f32 radius = pBall->GetRadius();

    // The net
    f32 width = GetWidth();
    if (1.0815333f * (-width / 2.0f) <= pos.x && pos.x <= 1.0815333f * (width / 2.0f) &&
        -radius <= pos.z && pos.z <= radius && GetHeight() - radius <= pos.y &&
        pos.y <= radius + (GetHeight() + GetNetHeight())) {
        {
            nw4r::math::VEC3 velocity = pBall->GetVelocity();
            velocity.x *= sFriction;
            velocity.y *= sFriction;
            if (pos.y > 3.0f * radius / 4.0f + (GetHeight() + GetNetHeight())) {
                velocity.z *= sFriction;
            } else {
                velocity.z *= -sFriction;
            }
            pBall->SetVelocity(velocity);

            if (!(pos.y > 3.0f * radius / 4.0f + (GetHeight() + GetNetHeight()))) {
                if (velocity.z < 0.0f) {
                    pos.z = -radius;
                } else if (velocity.z > 0.0f) {
                    pos.z = radius;
                } else {
                    pos.z = prevPos.z;
                }
            }

            pBall->SetPos(pos);
            RPPnpGame::GetInstance()->GetBall()->OnEvent(6, 2);
            hit = true;

            RPGrpModelAnm* pAnm = mpModel->GetModelAnm();
            if (prevPos.z > 0.0f) {
                pAnm->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
            } else {
                pAnm->Start(RPGrpModelAnm::Anm_Chr, 1, 0.0f);
            }
        }
    }

    // The table top
    width = GetWidth();
    if (-width / 2.0f <= pos.x && pos.x <= width / 2.0f) {
        f32 length = GetLength();
        if (-length / 2.0f <= pos.z && pos.z <= length / 2.0f) {
            f32 top = RPPnpGame::GetInstance()->GetBall()->GetRadius() / 2.0f + GetHeight();
            nw4r::math::VEC3 prevOfs(0.0f, prevPos.y - top, 0.0f);
            nw4r::math::VEC3 ofs(0.0f, pos.y - top, 0.0f);
            if (nw4r::math::VEC3Dot(&prevOfs, &ofs) < 0.0f) {
                nw4r::math::VEC3 velocity = pBall->GetVelocity();
                velocity.y *= -sRestitution;
                if (pBall->GetState() == 3) {
                    velocity.y *= 1.5f;
                }
                pBall->SetVelocity(velocity);

                if (hit) {
                    pBall->ApplyReceiverSpin();
                }

                if (velocity.y > 0.0f) {
                    pos.y += radius;
                    if (pos.y < radius + GetHeight()) {
                        pos.y = 0.1f + (radius + GetHeight());
                    }
                } else {
                    pos.y -= radius;
                    if (pos.y > GetHeight() - radius) {
                        pos.y = GetHeight() - radius;
                    }
                }

                pBall->SetPos(pos);
                RPPnpGame::GetInstance()->GetBall()->OnEvent(pos.z > 0.0f ? 4 : 5, 2);
                hit = true;

                if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_RALLY) {
                    nw4r::math::VEC3 effectPos;
                    effectPos.x = pos.x;
                    effectPos.y =
                        RPPnpGame::GetInstance()->GetBall()->GetRadius() / 2.0f + GetHeight();
                    effectPos.z = pos.z;
                    nw4r::math::VEC3 effectScale(1.0f, 1.0f, 1.0f);
                    if (RPPnpGame::GetInstance()->GetBall()->IsHidden()) {
                        effectScale = sBounceScale;
                    }

                    RPSysEffect effect("Pnp_bound", 0, 0);
                    effect.create();
                    effect.setPos(effectPos);
                    effect.setScale(effectScale);
                    effect.update();
                }
            }
        }
    }

    return hit;
}
