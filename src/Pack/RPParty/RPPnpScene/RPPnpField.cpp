// Table Tennis room floor: bounces the ball off the floor, the barriers and the room walls.
// Unit 0x80224868-0x80225240 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpField.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPPnpScene/RPPnpBall.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>

// Compute the signed side offset before applying the table's translation.
static inline f32 BarrierEdgeOffset(f32 radius, f32 velocity, const f32& tableOffset, bool flip) {
    f32 edge;
    if (velocity < 0.0f) {
        edge = flip ? -(10.0f + radius) : 10.0f + radius;
    } else {
        edge = flip ? 10.0f + radius : -(10.0f + radius);
    }
    edge += 190.0f + tableOffset;
    return edge;
}

f32 RPPnpField::sFloorBounce = 0.5f;
f32 RPPnpField::sBarrierBounce = 0.5f;
f32 RPPnpField::sWallBounce = 0.5f;

RPPnpField::~RPPnpField() {}

void RPPnpField::Create() {
    mScale.x = 10.0f;
    mScale.y = 10.0f;
    mScale.z = 10.0f;
}

void RPPnpField::Load() {
    RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/pin_field.brres");
    RPSimpleModel::LoadModel("pin_field", file, 1, 0, 0);
    mpModel->SetDrawGroup(2);
    mpModel->SetUnk3(1);
    mpModel->SetUnk4(1);
    mpModel->SetUnkCFlags(4);
}

void RPPnpField::Reset() {}

void RPPnpField::Calculate() {
    CalcModelMtx();
}

void RPPnpField::Exit() {}

bool RPPnpField::CheckBallFloorHit() {
    bool hit = false;

    RPPnpBall* pBall = RPPnpGame::GetInstance()->GetBall();
    nw4r::math::VEC3 prevPos = pBall->GetPrevPos();
    nw4r::math::VEC3 pos = pBall->GetPos();
    f32 radius = pBall->GetRadius();

    // The floor
    nw4r::math::VEC3 prevOfs(0.0f, prevPos.y - mPos.y, 0.0f);
    nw4r::math::VEC3 ofs(0.0f, pos.y - mPos.y, 0.0f);
    if (nw4r::math::VEC3Dot(&prevOfs, &ofs) < 0.0f) {
        pos.y += radius;
        if (pos.y < mPos.y + radius) {
            pos.y = 0.1f + (mPos.y + radius);
        }

        nw4r::math::VEC3 velocity = pBall->GetVelocity();
        velocity.y = -velocity.y * sFloorBounce;
        pBall->SetVelocity(velocity);
        RPPnpGame::GetInstance()->GetBall()->OnEvent(0, 2);
        hit = true;
    }

    // The barriers around the three tables
    const f32 offsets[3] = {-500.0f, 0.0f, 500.0f};
    f32 margin = 7.5f * radius + 10.0f;
    for (int i = 0; i < 3; i++) {
        if (0.0f <= pos.y && pos.y <= 70.0f) {
            f32 halfLength = 190.0f + offsets[i];
            if (-halfLength - margin <= pos.x && pos.x <= margin + halfLength) {
                nw4r::math::VEC3 farOfs(0.0f, 0.0f, prevPos.z - 460.0f);
                nw4r::math::VEC3 farOfs2(0.0f, 0.0f, pos.z - 460.0f);
                if (nw4r::math::VEC3Dot(&farOfs, &farOfs2) < 0.0f) {
                    nw4r::math::VEC3 velocity = pBall->GetVelocity();
                    velocity.z = -velocity.z * sBarrierBounce;
                    pBall->SetVelocity(velocity);
                    if (velocity.z < 0.0f) {
                        pos.z = 460.0f - radius;
                    } else {
                        pos.z = 460.0f + radius;
                    }
                    RPPnpGame::GetInstance()->GetBall()->OnEvent(1, 2);
                    hit = true;
                }

                nw4r::math::VEC3 nearOfs(0.0f, 0.0f, 460.0f + prevPos.z);
                nw4r::math::VEC3 nearOfs2(0.0f, 0.0f, 460.0f + pos.z);
                if (nw4r::math::VEC3Dot(&nearOfs, &nearOfs2) < 0.0f) {
                    nw4r::math::VEC3 velocity = pBall->GetVelocity();
                    velocity.z = -velocity.z * sBarrierBounce;
                    pBall->SetVelocity(velocity);
                    if (velocity.z < 0.0f) {
                        pos.z = -460.0f - radius;
                    } else {
                        pos.z = -460.0f + radius;
                    }
                    RPPnpGame::GetInstance()->GetBall()->OnEvent(1, 2);
                    hit = true;
                }
            }

            if ((-460.0f - margin <= pos.z && pos.z <= -96.18f + margin) ||
                (96.18f - margin <= pos.z && pos.z <= 460.0f + margin)) {
                nw4r::math::VEC3 velocity = pBall->GetVelocity();

                f32 edge = BarrierEdgeOffset(radius, velocity.x, offsets[i], false);

                nw4r::math::VEC3 rightOfs(prevPos.x - edge, 0.0f, 0.0f);
                nw4r::math::VEC3 rightOfs2(pos.x - edge, 0.0f, 0.0f);
                if (nw4r::math::VEC3Dot(&rightOfs, &rightOfs2) < 0.0f) {
                    velocity.x = -velocity.x * sBarrierBounce;
                    pBall->SetVelocity(velocity);
                    if (velocity.x < 0.0f) {
                        pos.x = edge - radius;
                    } else {
                        pos.x = radius + edge;
                    }
                    RPPnpGame::GetInstance()->GetBall()->OnEvent(1, 2);
                    hit = true;
                }

                edge = BarrierEdgeOffset(radius, velocity.x, offsets[i], true);

                nw4r::math::VEC3 leftOfs(prevPos.x + edge, 0.0f, 0.0f);
                nw4r::math::VEC3 leftOfs2(pos.x + edge, 0.0f, 0.0f);
                if (nw4r::math::VEC3Dot(&leftOfs, &leftOfs2) < 0.0f) {
                    velocity.x = -velocity.x * sBarrierBounce;
                    pBall->SetVelocity(velocity);
                    if (velocity.x < 0.0f) {
                        pos.x = -edge - radius;
                    } else {
                        pos.x = radius + -edge;
                    }
                    RPPnpGame::GetInstance()->GetBall()->OnEvent(1, 2);
                    hit = true;
                }
            }
        }
    }

    // The room walls
    if (pos.z > 550.0f) {
        nw4r::math::VEC3 velocity = pBall->GetVelocity();
        velocity.z = -velocity.z * sWallBounce;
        pBall->SetVelocity(velocity);
        pos.z = 550.0f - radius;
        RPPnpGame::GetInstance()->GetBall()->OnEvent(3, 2);
        hit = true;
    }

    if (pos.z < -550.0f) {
        nw4r::math::VEC3 velocity = pBall->GetVelocity();
        velocity.z = -velocity.z * sWallBounce;
        pBall->SetVelocity(velocity);
        pos.z = -550.0f + radius;
        RPPnpGame::GetInstance()->GetBall()->OnEvent(3, 2);
        hit = true;
    }

    if (pos.x > 400.0f) {
        nw4r::math::VEC3 velocity = pBall->GetVelocity();
        velocity.x = -velocity.x * sWallBounce;
        pBall->SetVelocity(velocity);
        pos.x = 400.0f - radius;
        RPPnpGame::GetInstance()->GetBall()->OnEvent(3, 2);
        hit = true;
    }

    if (pos.x < -400.0f) {
        nw4r::math::VEC3 velocity = pBall->GetVelocity();
        velocity.x = -velocity.x * sWallBounce;
        pBall->SetVelocity(velocity);
        pos.x = -400.0f + radius;
        RPPnpGame::GetInstance()->GetBall()->OnEvent(3, 2);
        hit = true;
    }

    if (hit) {
        pBall->SetPos(pos);
    }

    return hit;
}
