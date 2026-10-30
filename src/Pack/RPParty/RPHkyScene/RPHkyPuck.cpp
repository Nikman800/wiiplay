// Laser Hockey puck: physics against the rink and the paddles, goals, hit effects and sounds.
// Unit 0x80216E8C-0x802191DC (dtk/game_units.tsv). With -inline deferred CodeWarrior emits a
// file's functions in reverse order, so the source runs from the last address to the first.
#include <Pack/RPParty/RPHkyScene/RPHkyPuck.h>
#include <Pack/RPParty/RPHkyScene/RPHkySound.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPHkyScene/RPHkyBgmCtrl.h>
#include <Pack/RPParty/RPHkyScene/RPHkyField.h>
#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPHkyScene/RPHkyHud.h>
#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>
#include <Pack/RPParty/RPHkyScene/RPHkyPaddle.h>
#include <Pack/RPParty/RPHkyScene/RPHkyPhase.h>

#include <egg/gfxe.h>

#include <cmath>

f32 RPHkyPuck::sRadius = 16.0f;
f32 RPHkyPuck::sSpeed = 0.65f;
f32 RPHkyPuck::sSpeedInit = 0.6f;
f32 RPHkyPuck::sSpeedStep = 0.025f;
f32 RPHkyPuck::sSpeedMax = 1.5f;
f32 RPHkyPuck::sFriction = 0.9998f;
f32 RPHkyPuck::sWallBounce = 1.0f;
f32 RPHkyPuck::sPaddleBounce = 1.0f;
f32 RPHkyPuck::sWallSpinFactor = 0.1f;
f32 RPHkyPuck::sPaddleSpinFactor = 0.1f;
f32 RPHkyPuck::sSpinImpulseScale = -10.0f;
f32 RPHkyPuck::sSpinMax = 0.9f;
f32 RPHkyPuck::sSpinDamping = 0.9998f;
f32 RPHkyPuck::sGravity = -0.003f;
f32 RPHkyPuck::sGoalSlope = 1.0f;
f32 RPHkyPuck::sGoalDepth = -10.0f;
f32 RPHkyPuck::sSpinFactor = -0.0f;
s32 RPHkyPuck::sHitCooldown = 9;
f32 RPHkyPuck::sPaddleHitScale = 1.0f;
f32 RPHkyPuck::sPaddlePushScale = 1.0f;
f32 RPHkyPuck::sWallAngleLimit = 180.0f;

u16 RPHkyPuck::sDebugHold;
f32 RPHkyPuck::sDebugHoldX;
f32 RPHkyPuck::sDebugHoldZ;

// NONMATCHING-DEBT: the original copies a 3x3 matrix as three 12-byte rows with word moves,
// not as one 36-byte struct. A POD row copy needs RPSimpleModel::mRot as rows of a POD vector
// (RPAxes/Vec, shared by ~30 units); `VEC3 = VEC3` per row copies as floats (43/64).
static inline void CopyRows(nw4r::math::MTX33& rDst, const nw4r::math::VEC3* pSrc) {
    struct Row {
        f32 v[3];
    };

    for (int i = 0; i < 3; i++) {
        *reinterpret_cast<Row*>(rDst.m[i]) = *reinterpret_cast<const Row*>(&pSrc[i]);
    }
}

inline void RPHkyPuck::ResetPushes() {
    mPushMin.x = mPushMin.y = mPushMin.z = 0.0f;
    mPushMax.x = mPushMax.y = mPushMax.z = 0.0f;
    mVelPushMin.x = mVelPushMin.y = mVelPushMin.z = 0.0f;
    mVelPushMax.x = mVelPushMax.y = mVelPushMax.z = 0.0f;
}

inline void RPHkyPuck::AddPush(const nw4r::math::VEC3& rPush) {
    if (rPush.x < mPushMin.x) {
        mPushMin.x = rPush.x;
    } else if (rPush.x > mPushMax.x) {
        mPushMax.x = rPush.x;
    }

    if (rPush.y < mPushMin.y) {
        mPushMin.y = rPush.y;
    } else if (rPush.y > mPushMax.y) {
        mPushMax.y = rPush.y;
    }

    if (rPush.z < mPushMin.z) {
        mPushMin.z = rPush.z;
    } else if (rPush.z > mPushMax.z) {
        mPushMax.z = rPush.z;
    }
}

inline void RPHkyPuck::AddVelocityPush(const nw4r::math::VEC3& rPush) {
    if (rPush.x < mVelPushMin.x) {
        mVelPushMin.x = rPush.x;
    } else if (rPush.x > mVelPushMax.x) {
        mVelPushMax.x = rPush.x;
    }

    if (rPush.y < mVelPushMin.y) {
        mVelPushMin.y = rPush.y;
    } else if (rPush.y > mVelPushMax.y) {
        mVelPushMax.y = rPush.y;
    }

    if (rPush.z < mVelPushMin.z) {
        mVelPushMin.z = rPush.z;
    } else if (rPush.z > mVelPushMax.z) {
        mVelPushMax.z = rPush.z;
    }
}

inline void RPHkyPuck::AddSpinPush(f32 push) {
    if (push < mSpinPushMin) {
        mSpinPushMin = push;
    } else if (push > mSpinPushMax) {
        mSpinPushMax = push;
    }
}

RPHkyPuck::~RPHkyPuck() {}

RPHkyPuckTrail::~RPHkyPuckTrail() {}

void RPHkyPuck::Create() {
    mpTrail = new RPHkyPuckTrail();
    mpTailEffect = new RPSysEffect("Hky_balltail", 0, 0);
    ResetState();

    if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
        sSpeedInit = 0.65f;
        sSpeedStep = 0.02f;
    } else {
        sSpeedInit = 0.5f;
        sSpeedStep = 0.02f;
    }
}

void RPHkyPuck::Load() {
    RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/Hck_pack.brres");
    RPSimpleModel::LoadModel("Hck_pack", file, 1, 0, 0x108);
    mpModel->SetVisible(false);
    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);

    RPGrpHandle anmFile = RPSimpleModel::LoadResource("/G3D/Hck_anim.brres");
    const char* pName = "Hck_pack_hit";
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle anm = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResAnmClr,
        pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmClr, anmFile, pName));
    mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Clr, 0, anm);
}

void RPHkyPuck::Reset() {
    mPos = sVecZero;
    mScale = sVecOne;
    ResetState();
    sSpeed = sSpeedInit;
    mActive = false;
}

void RPHkyPuck::Calculate() {
    if (mActive && mPos.y < sRadius) {
        mpTailEffect->create();
        mActive = false;
    }

    mpTailEffect->setPos(mPos);
    mpTailEffect->update();
    CalcModelMtx();
    mpTrail->set3DPosition(mPos);
}

void RPHkyPuck::Exit() {}

void RPHkyPuck::ResetState() {
    mInPlay = false;
    for (int i = 0; i < 4; i++) {
        mHitState[i] = 0;
        mHitSpeed[i] = 0.0f;
    }
    for (int i = 0; i < 3; i++) {
        mHitCooldown[i] = 0;
    }

    ResetPushes();
    mSpin = mSpinPushMax = mSpinPushMin = 0.0f;

    CopyRows(mRot, sMtxIdentity);
    mVelocity = sVecZero;
    mPos.x = mPos.z = 0.0f;
    mPos.y = 300.0f;
    sSpeed = sSpeedInit;
}

void RPHkyPuck::SetInPlay() {
    mInPlay = true;
}

void RPHkyPuck::ApplyVelocity() {
    ResetPushes();
    mSpinPushMin = 0.0f;
    mSpinPushMax = 0.0f;
    mPos.x += mVelocity.x;
    mPos.y += mVelocity.y;
    mPos.z += mVelocity.z;

    if (sDebugHold) {
        mPos.x = sDebugHoldX;
        mPos.y = 0.0f;
        mPos.z = sDebugHoldZ;
        mVelocity = sVecZero;
    }
}

// Wii Play: a stack effect at the hit point, and the wall's light-up animation
// NONMATCHING-DEBT: kept as a macro. As the inline member RPHkyPuck::PlayWallHit (declared in
// RPHkyPuck.h, never defined) CollideField is 520/530 words: at each RPSysEffect constructor
// call the name argument is set up before the object address.
#define PLAY_WALL_HIT(name, pos, anim)                                                           \
    {                                                                                              \
        RPSysEffect effect(name, 0, 0);                                                            \
        effect.create();                                                                           \
        nw4r::math::VEC3 p;                                                                        \
        p.x = pos.x; p.y = pos.y; p.z = pos.z;                                                     \
        effect.setPos(p);                                                                          \
        effect.setScale(mScale);                                                                   \
        effect.update();                                                                           \
        RPHkyGame::GetInstance()->GetField()->PlayWallAnim(anim);                                  \
    }

// Wii Play: fill row 2 first, as for paddle contacts. Although unused here, the basis stores
// survive optimization; their element offsets determine the lifetime-split stack layout.
void RPHkyPuck::CollideField() {
    RPHkyField* pField = RPHkyGame::GetInstance()->GetField();
    pField->ResetPuckWallIter();

    nw4r::math::VEC3 normal;
    f32 depth;
    while (pField->TestCircleCollision(sRadius, mPos, &normal, &depth)) {
        nw4r::math::VEC3 push;
        push.x = depth * normal.x;
        push.y = 0.0f;
        push.z = depth * normal.z;
        AddPush(push);

        nw4r::math::VEC3 hitPos;
        nw4r::math::MTX33 basis;
        hitPos.x = mPos.x + push.x - normal.x * sRadius;
        hitPos.y = 0.25f * sRadius;
        hitPos.z = mPos.z + push.z - normal.z * sRadius;
        basis._20 = normal.x;
        basis._21 = 0.0f;
        basis._22 = normal.z;
        basis._00 = -normal.z;
        basis._01 = 0.0f;
        basis._02 = normal.x;
        basis._10 = 0.0f;
        basis._11 = 1.0f;
        basis._12 = 0.0f;

        nw4r::math::VEC3 vel;
        vel.x = mVelocity.x - normal.z * mSpin;
        vel.y = 0.0f;
        vel.z = mVelocity.z + normal.x * mSpin;
        f32 vn = -vel.x * normal.x - vel.z * normal.z;
        nw4r::math::VEC3 tangent;
        tangent.x = vel.x + vn * normal.x;
        tangent.y = 0.0f;
        tangent.z = vel.z + vn * normal.z;
        f32 tangentLen = std::sqrtf(tangent.x * tangent.x + tangent.z * tangent.z);
        if (vn <= 0.0f) {
            vn = 0.0f;
        }

        f32 maxTangent = vn * sWallSpinFactor;
        if (tangentLen > maxTangent) {
            f32 scale = maxTangent / tangentLen;
            tangent.x *= scale;
            tangent.z *= scale;
        }

        vn *= 1.0f + sWallBounce;
        push.x = vn * normal.x - tangent.x;
        push.y = 0.0f;
        push.z = vn * normal.z - tangent.z;
        AddVelocityPush(push);
        AddSpinPush(sSpinImpulseScale * (-tangent.x * normal.z + tangent.z * normal.x));

        if (mInPlay) {
            RegisterHit(mPos.x < 0.0f ? Hit_WallLeft : Hit_WallRight, NULL);
        }

        if ((f32)std::fabs(mPos.x) > sWallAngleLimit) {
            if (mHitCooldown[2] == 0) {
                switch (pField->GetHitWallIndex()) {
                case 0:
                    if (mPos.x < 0.0f) {
                        PLAY_WALL_HIT("Hky_wallhit_r", hitPos, 0)
                    } else {
                        PLAY_WALL_HIT("Hky_wallhit_o", hitPos, 1)
                    }
                    break;
                case 1:
                    if (mPos.x < 0.0f) {
                        PLAY_WALL_HIT("Hky_wallhit_b", hitPos, 2)
                    } else {
                        PLAY_WALL_HIT("Hky_wallhit_g", hitPos, 3)
                    }
                    break;
                case 2:
                    PLAY_WALL_HIT("Hky_wallhit_o", hitPos, 1)
                    break;
                case 3:
                    PLAY_WALL_HIT("Hky_wallhit_g", hitPos, 3)
                    break;
                case 4:
                    PLAY_WALL_HIT("Hky_wallhit_b", hitPos, 2)
                    break;
                case 5:
                    PLAY_WALL_HIT("Hky_wallhit_r", hitPos, 0)
                    break;
                case 6:
                case 7:
                    PLAY_WALL_HIT("Hky_wallhit_y", hitPos, 5)
                    break;
                case 8:
                case 9:
                    PLAY_WALL_HIT("Hky_wallhit_p", hitPos, 4)
                    break;
                }
            }
            mHitCooldown[2] = 1;
        }
    }
}

void RPHkyPuck::UpdatePhysics() {
    nw4r::math::MTX34 rot;
    nw4r::math::MTX33 rotated;
    nw4r::math::VEC3 floorNormal;
    nw4r::math::VEC3 push;
    nw4r::math::VEC3 side;
    nw4r::math::VEC3 velPush;
    nw4r::math::VEC3 posPush;
    f32 floorY;

    mVelocity.x *= sFriction;
    mVelocity.z *= sFriction;
    mVelocity.y += sGravity;

    // The goal mouths slope up towards the goal lines
    if (-RPHkyField::sWidth + RPHkyField::cCorner - 32.0f <= mPos.x &&
        mPos.x <= 32.0f + (RPHkyField::sWidth - RPHkyField::cCorner)) {
        floorY = 0.25f * sRadius;
        floorNormal.z = 0.0f;
        floorNormal.x = 0.0f;
        floorNormal.y = 1.0f;
    } else if (mPos.x < 0.0f) {
        floorY = 0.25f * sRadius + sGoalSlope * (RPHkyField::sWidth + mPos.x);
        floorNormal.x = -sGoalSlope;
        floorNormal.y = 1.0f;
        floorNormal.z = 0.0f;
        PSVECNormalize(floorNormal, floorNormal);
        RPHkyGame::GetInstance()->GetField()->PlayWallAnim(4);
    } else {
        floorY = 0.25f * sRadius + sGoalSlope * (RPHkyField::sWidth - mPos.x);
        floorNormal.x = sGoalSlope;
        floorNormal.y = 1.0f;
        floorNormal.z = 0.0f;
        PSVECNormalize(floorNormal, floorNormal);
        RPHkyGame::GetInstance()->GetField()->PlayWallAnim(5);
    }

    if (mPos.y < floorY) {
        mPos.y = floorY;
        f32 vn = -mVelocity.x * floorNormal.x - mVelocity.y * floorNormal.y -
                 mVelocity.z * floorNormal.z;
        if (vn > 0.0f) {
            push.x = vn * floorNormal.x;
            push.y = vn * floorNormal.y;
            push.z = vn * floorNormal.z;
            AddVelocityPush(push);
        }
    }

    PSVECAdd(mVelPushMin, mVelPushMax, velPush);
    PSVECAdd(mVelocity, velPush, mVelocity);
    PSVECAdd(mPushMin, mPushMax, posPush);
    PSVECAdd(mPos, posPush, mPos);

    f32 spin = mSpin * sSpinDamping;
    mSpin = spin + (mSpinPushMin + mSpinPushMax);
    if (mSpin > sSpinMax) {
        mSpin = sSpinMax;
    } else if (mSpin < -sSpinMax) {
        mSpin = -sSpinMax;
    }

    // Spin curves the path
    side.x = -mVelocity.z;
    side.z = mVelocity.x;
    if (std::sqrtf(side.x * side.x + side.z * side.z) != 0.0f) {
        f32 curve = mSpin * sSpinFactor;
        side.x *= curve;
        side.z *= curve;
        mVelocity.x += side.x;
        mVelocity.z += side.z;
    }

    f32 speedSq = mVelocity.x * mVelocity.x + mVelocity.z * mVelocity.z;
    if (speedSq >= sSpeed * sSpeed) {
        f32 scale = sSpeed / std::sqrtf(speedSq);
        mVelocity.x *= scale;
        mVelocity.z *= scale;
    }

    // Tilt towards the floor and turn by the spin
    mRot._10 += 0.01f * (floorNormal.x - mRot._10);
    mRot._11 += 0.01f * (floorNormal.y - mRot._11);
    mRot._12 += 0.01f * (floorNormal.z - mRot._12);
    PSVECNormalize((Vec*)mRot.m[1], (Vec*)mRot.m[1]);
    PSVECCrossProduct((Vec*)mRot.m[1], (Vec*)mRot.m[2], (Vec*)mRot.m[0]);
    PSVECNormalize((Vec*)mRot.m[0], (Vec*)mRot.m[0]);
    PSVECCrossProduct((Vec*)mRot.m[0], (Vec*)mRot.m[1], (Vec*)mRot.m[2]);

    PSMTXRotRad(rot, 'Y', 0.017453292f * mSpin);

    rotated._00 = mRot._00 * rot._00 + mRot._10 * rot._10 + mRot._20 * rot._20;
    rotated._10 = mRot._00 * rot._01 + mRot._10 * rot._11 + mRot._20 * rot._21;
    rotated._20 = mRot._00 * rot._02 + mRot._10 * rot._12 + mRot._20 * rot._22;
    rotated._01 = mRot._01 * rot._00 + mRot._11 * rot._10 + mRot._21 * rot._20;
    rotated._11 = mRot._01 * rot._01 + mRot._11 * rot._11 + mRot._21 * rot._21;
    rotated._21 = mRot._01 * rot._02 + mRot._11 * rot._12 + mRot._21 * rot._22;
    rotated._02 = mRot._02 * rot._00 + mRot._12 * rot._10 + mRot._22 * rot._20;
    rotated._12 = mRot._02 * rot._01 + mRot._12 * rot._11 + mRot._22 * rot._21;
    rotated._22 = mRot._02 * rot._02 + mRot._12 * rot._12 + mRot._22 * rot._22;
    mRot = rotated;
}

void RPHkyPuck::CheckGoal(s32* pScore0, s32* pScore1) {
    for (int i = 0; i < 4; i++) {
        if (mHitState[i] == 1) {
            switch (i) {
            case 0:
                mpTrail->StartSoundBySpeed(2, mHitSpeed[i]);
                break;
            case 1:
                mpTrail->StartSoundBySpeed(3, mHitSpeed[i]);
                break;
            case 2:
                mpTrail->StartSoundBySpeed(0, mHitSpeed[i]);
                break;
            case 3:
                mpTrail->StartSoundBySpeed(1, mHitSpeed[i]);
                break;
            }

            mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, 0.0f);

            sSpeed += sSpeedStep;
            if (sSpeed > sSpeedMax) {
                sSpeed = sSpeedMax;
            }
        }

        if (mHitState[i] != 0) {
            if (++mHitState[i] >= sHitCooldown) {
                mHitState[i] = 0;
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        if (mHitCooldown[i] != 0) {
            if (++mHitCooldown[i] >= sHitCooldown) {
                mHitCooldown[i] = 0;
            }
        }
    }

    if (mPos.y >= sGoalDepth) {
        return;
    }

    {
        f32 x;
        if (mPos.x >= 0.0f) {
            if (*pScore0 < 99) {
                (*pScore0)++;
            }
            x = 5.0f * sRadius;

            RPSndAudioMgr::instance()->startSystemSe(4U);
            RPSndAudioMgr::instance()->startSystemSeWithRemote(7U, 1);
            RPHkyGame::GetInstance()->GetBgmCtrl()->Request(RPHkyBgmCtrl::REQUEST_1);
            RPHkyHud::GetInstance()->GetScoreLyt()->PlayGoalSwitch(0);

            nw4r::math::VEC3 pos = sVecZero;
            nw4r::math::VEC3 scale(1.0f, 1.0f, 1.0f);
            if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
                RPSysEffect effect("Hky_goal_right_wd", 0, 0);
                effect.create();
                effect.setPos(pos);
                effect.setScale(scale);
                effect.update();
            } else {
                RPSysEffect effect("Hky_goal_right_st", 0, 0);
                effect.create();
                effect.setPos(pos);
                effect.setScale(scale);
                effect.update();
            }
        } else {
            if (*pScore1 < 99) {
                (*pScore1)++;
            }
            x = 5.0f * -sRadius;

            RPSndAudioMgr::instance()->startSystemSe(5U);
            RPSndAudioMgr::instance()->startSystemSeWithRemote(6U, 2);
            RPHkyGame::GetInstance()->GetBgmCtrl()->Request(RPHkyBgmCtrl::REQUEST_2);
            RPHkyHud::GetInstance()->GetScoreLyt()->PlayGoalSwitch(1);

            nw4r::math::VEC3 pos = sVecZero;
            nw4r::math::VEC3 scale(1.0f, 1.0f, 1.0f);
            if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
                RPSysEffect effect("Hky_goal_left_wd", 0, 0);
                effect.create();
                effect.setPos(pos);
                effect.setScale(scale);
                effect.update();
            } else {
                RPSysEffect effect("Hky_goal_left_st", 0, 0);
                effect.create();
                effect.setPos(pos);
                effect.setScale(scale);
                effect.update();
            }
        }

        ResetState();
        mPos.x = x;
        RPHkyGame::GetInstance()->SetState(2);
        RPHkyGame::GetInstance()->GetGoalPhase()->Start();
        mpModel->SetVisible(false);
        mpTailEffect->fade();
    }
}

// Wii Play: RPHkyPaddle's push accumulators, updated the same way as the puck's
inline void RPHkyPuck::AddPaddlePush(RPHkyPaddle* pPaddle, const nw4r::math::VEC3& rPush) {
    if (rPush.x < pPaddle->mPushMin.x) {
        pPaddle->mPushMin.x = rPush.x;
    } else if (rPush.x > pPaddle->mPushMax.x) {
        pPaddle->mPushMax.x = rPush.x;
    }

    if (rPush.y < pPaddle->mPushMin.y) {
        pPaddle->mPushMin.y = rPush.y;
    } else if (rPush.y > pPaddle->mPushMax.y) {
        pPaddle->mPushMax.y = rPush.y;
    }

    if (rPush.z < pPaddle->mPushMin.z) {
        pPaddle->mPushMin.z = rPush.z;
    } else if (rPush.z > pPaddle->mPushMax.z) {
        pPaddle->mPushMax.z = rPush.z;
    }
}

inline void RPHkyPuck::AddPaddleVelocityPush(RPHkyPaddle* pPaddle, const nw4r::math::VEC3& rPush) {
    if (rPush.x < pPaddle->mVelPushMin.x) {
        pPaddle->mVelPushMin.x = rPush.x;
    } else if (rPush.x > pPaddle->mVelPushMax.x) {
        pPaddle->mVelPushMax.x = rPush.x;
    }

    if (rPush.y < pPaddle->mVelPushMin.y) {
        pPaddle->mVelPushMin.y = rPush.y;
    } else if (rPush.y > pPaddle->mVelPushMax.y) {
        pPaddle->mVelPushMax.y = rPush.y;
    }

    if (rPush.z < pPaddle->mVelPushMin.z) {
        pPaddle->mVelPushMin.z = rPush.z;
    } else if (rPush.z > pPaddle->mVelPushMax.z) {
        pPaddle->mVelPushMax.z = rPush.z;
    }
}

// The dead hitVel copy is read by the relative velocity (keeps it whole at 0x3C); hitPos and basis
// are written element by element, basis row 2 first (store order of the original)
void RPHkyPuck::CollidePaddle(RPHkyPaddle* pPaddle) {
    if (mPos.y > 0.75f * sRadius) {
        return;
    }

    pPaddle->ResetEdgeIter();

    nw4r::math::VEC3 normal;
    f32 depth;
    while (pPaddle->TestPuckCollision(sRadius, mPos, &normal, &depth)) {
        depth *= 0.5f;
        nw4r::math::VEC3 push;
        push.x = normal.x * depth;
        push.y = 0.0f;
        push.z = normal.z * depth;
        AddPush(push);
        push.x = -push.x;
        push.z = -push.z;
        AddPaddlePush(pPaddle, push);

        nw4r::math::VEC3 hitPos;
        nw4r::math::MTX33 basis;
        hitPos.x = mPos.x - push.x - normal.x * sRadius;
        hitPos.y = 0.25f * sRadius;
        hitPos.z = mPos.z - push.z - normal.z * sRadius;
        basis._20 = normal.x;
        basis._21 = 0.0f;
        basis._22 = normal.z;
        basis._00 = -normal.z;
        basis._01 = 0.0f;
        basis._02 = normal.x;
        basis._10 = 0.0f;
        basis._11 = 1.0f;
        basis._12 = 0.0f;

        nw4r::math::VEC3 hitVel;
        nw4r::math::VEC3 paddleVel = RPHkyPaddle::CalcHitVelocity(pPaddle);
        hitVel.x = paddleVel.x;
        hitVel.z = paddleVel.z;

        nw4r::math::VEC3 vel;
        vel.x = mVelocity.x - hitVel.x - normal.z * mSpin;
        vel.z = mVelocity.z - hitVel.z + normal.x * mSpin;
        f32 vn = -vel.x * normal.x - vel.z * normal.z;
        vn *= sPaddleHitScale;
        nw4r::math::VEC3 tangent;
        tangent.x = vel.x + vn * normal.x;
        tangent.z = vel.z + vn * normal.z;
        f32 tangentLen = std::sqrtf(tangent.x * tangent.x + tangent.z * tangent.z);
        if (vn < 0.0f) {
            vn = 0.0f;
        }

        f32 maxTangent = vn * sPaddleSpinFactor;
        if (tangentLen > maxTangent) {
            f32 scale = maxTangent / tangentLen;
            tangent.x *= scale;
            tangent.z *= scale;
        }

        if (pPaddle->mShapeType == RPHkyPaddle::Shape_Rect) {
            if ((pPaddle->mPlayerIndex == 0 && pPaddle->GetHitEdgeIndex() == 3) ||
                (pPaddle->mPlayerIndex == 1 && pPaddle->GetHitEdgeIndex() == 2)) {
                if (RPHkyGame::GetInstance()->IsMultiPlayer() || pPaddle->mPlayerIndex != 1) {
                    vn *= 0.05f;
                }
            } else if ((pPaddle->mPlayerIndex == 0 && pPaddle->GetHitEdgeIndex() == 2) ||
                       (pPaddle->mPlayerIndex == 1 && pPaddle->GetHitEdgeIndex() == 3)) {
                vn *= 1.0f + sPaddleBounce;
            } else {
                vn *= 10.0f;
            }
        } else {
            vn *= 1.0f + sPaddleBounce;
        }

        push.x = vn * normal.x - tangent.x;
        push.y = 0.0f;
        push.z = vn * normal.z - tangent.z;

        if (paddleVel.x * paddleVel.x + paddleVel.z * paddleVel.z > 0.5f) {
            nw4r::math::VEC3 paddlePos = pPaddle->mPos;
            if ((f32)std::fabs(mPos.x) < (f32)std::fabs(paddlePos.x) &&
                paddleVel.x * paddlePos.x <= 0.0f) {
                push.x += paddleVel.x * sPaddlePushScale;
                push.z += paddleVel.z * sPaddlePushScale;
            } else {
                push.x += 0.2f * (paddleVel.x * sPaddlePushScale);
                push.z += 0.2f * (paddleVel.z * sPaddlePushScale);
            }
        }

        AddVelocityPush(push);
        push.x = -push.x;
        push.z = -push.z;
        AddPaddleVelocityPush(pPaddle, push);
        AddSpinPush(sSpinImpulseScale * (-tangent.x * normal.z + tangent.z * normal.x));

        if (mInPlay) {
            RegisterHit(pPaddle->mPlayerIndex == 0 ? Hit_Paddle0 : Hit_Paddle1, pPaddle);
        }

        s32* pCooldown = &mHitCooldown[pPaddle->mPlayerIndex != 0];
        if (*pCooldown == 0) {
            nw4r::math::MTX34 mtx;
            mtx._00 = basis._00;
            mtx._10 = basis._01;
            mtx._20 = basis._02;
            mtx._01 = basis._10;
            mtx._11 = basis._11;
            mtx._21 = basis._12;
            mtx._02 = basis._20;
            mtx._12 = basis._21;
            mtx._22 = basis._22;
            mtx._03 = hitPos.x;
            mtx._13 = hitPos.y;
            mtx._23 = hitPos.z;

            RPSysEffect effect("Hky_paddlehit", 0, 0);
            effect.create();
            effect.setMtx(mtx);
            effect.setScale(mScale);
            effect.update();
        }
        *pCooldown = 1;
    }
}

void RPHkyPuck::RegisterHit(s32 idx, RPHkyPaddle* pPaddle) {
    switch (mHitState[idx]) {
    case 0:
        mHitState[idx] = 1;
        if (pPaddle != NULL) {
            mHitSpeed[idx] = nw4r::math::VEC3Len(&mVelocity);
            nw4r::math::VEC3 rel;
            nw4r::math::VEC3Sub(&rel, &mVelocity, &pPaddle->mVelocity);
            s32 frames = (s32)nw4r::math::VEC3Len(&rel) + 5;
            if (frames < 5) {
                frames = 5;
            } else if (frames > 10) {
                frames = 10;
            }
            pPaddle->RequestRumble(0, frames);
        }
        break;
    case 1:
        break;
    default:
        mHitState[idx] = 2;
        break;
    }
}

void RPHkyPuck::Serve() {
    mpModel->SetVisible(true);
    mActive = true;
    RPHkyGame::GetInstance()->GetBgmCtrl()->Request(RPHkyBgmCtrl::REQUEST_NONE);

    RPHkyGame* pGame = RPHkyGame::GetInstance();
    if (pGame->IsMultiPlayer() == TRUE &&
        (pGame->GetPaddle(0)->GetScore() == 7 || pGame->GetPaddle(1)->GetScore() == 7)) {
        pGame->GetBgmCtrl()->Request(RPHkyBgmCtrl::REQUEST_3);
    }
}

void RPHkyPuck::Hide() {
    mpModel->SetVisible(false);
    mpTailEffect->fade();
}
