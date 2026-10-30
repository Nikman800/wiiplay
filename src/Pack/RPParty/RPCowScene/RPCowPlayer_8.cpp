// Charge! player effects (RPCowPlayerEffects): the cow's particle effects. Unit
// 0x802B547C-0x802B66D4 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPCowScene/RPCowPlayer_8.h>

#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPCowScene/RPCowCowModel.h>

#include <nw4r/math.h>

// Column of a 3x4 matrix (ledger RP_Mtx34_getColumn, RPCollisionShape.cpp) [M]
extern "C" EGG::Vector3f RP_Mtx34_getColumn(const nw4r::math::MTX34& rMtx, int col);

void RPCowPlayerEffects::SpawnEffectAtMtx(const EGG::Vector3f& rPos, const char* pName) {
    nw4r::math::MTX34 mtx = mMtx;
    mtx._03 = rPos.x;
    mtx._13 = sEffectY;
    mtx._23 = rPos.z;
    RPSysEffect effect(pName, 0, 0);
    effect.create();
    effect.setMtx(mtx);
    effect.update();
}

void RPCowPlayerEffects::SpawnLandSmoke(bool big, f32 power) {
    EGG::Vector3f pos = RP_Mtx34_getColumn(mMtx, 3);
    pos.y = 0.0f;
    EGG::Vector3f dir = RP_Mtx34_getColumn(mMtx, 2);
    dir.y = 0.0f;
    RPSysEffect effect = big ? RPSysEffect("Cow_landsmoke_big", 0, 0)
                             : RPSysEffect("Cow_landsmoke", 0, 0);
    effect.create();
    effect.setPos(pos);
    effect.setSpecDir(dir);
    effect.setPowerSpecDir(power);
    effect.update();
}

void RPCowPlayerEffects::EmitWool(f32 rate, const EGG::Vector3f& rPos,
                                  const EGG::Vector3f& rDir) {
    mCrashActive[mWoolIdx] = true;
    mCrashPos[mWoolIdx].x = rPos.x;
    mCrashPos[mWoolIdx].y = sEffectY;
    mCrashPos[mWoolIdx].z = rPos.z;
    mCrashVel[mWoolIdx].x = rDir.x;
    mCrashVel[mWoolIdx].y = 0.0f;
    mCrashVel[mWoolIdx].z = rDir.z;
    mpCrashEffects[mWoolIdx]->fade();
    mpCrashEffects[mWoolIdx]->create();
    mpCrashEffects[mWoolIdx]->setPos(mCrashPos[mWoolIdx]);
    nw4r::math::VEC2 scale(rate, rate);
    mpCrashEffects[mWoolIdx]->setParticleScale(scale);
    mpCrashEffects[mWoolIdx]->update();
    if (++mWoolIdx >= 4) {
        mWoolIdx = 0;
    }
}

RPCowPlayerEffects::RPCowPlayerEffects(RPCowCowModel* pCowModel) : mpCowModel(pCowModel) {
    mpEffects[0] = new RPSysEffect("Cow_cow_speedsmoke01l", 0, 0);
    mpEffects[1] = new RPSysEffect("Cow_cow_speedsmoke01r", 0, 0);
    mpEffects[2] = new RPSysEffect("Cow_cow_jump_ok", 0, 0);
    mpEffects[3] = new RPSysEffect("Cow_cow_damage01", 0, 0);
    mpEffects[4] = new RPSysEffect("Cow_landsmoke", 0, 0);
    mpEffects[5] = new RPSysEffect("Cow_landsmoke", 0, 0);
    mpDashEffects[0] = new RPSysEffect("Cow_dashsmk_front", 0, 0);
    mpDashEffects[1] = new RPSysEffect("Cow_dashsmk_front", 0, 0);
    mpDashEffects[2] = new RPSysEffect("Cow_dashsmk_back", 0, 0);
    mpDashEffects[3] = new RPSysEffect("Cow_dashsmk_back", 0, 0);
    for (int i = 0; i < 4; i++) {
        mpCrashEffects[i] = new RPSysEffect("Cow_vs_crash01", 0, 0);
    }
    for (int i = 0; i < 4; i++) {
        mpWeedEffects[i] = new RPSysEffect("Cow_dashsmk_weed", 0, 0);
        mpWoolEffects[i] = new RPSysEffect("Cow_slip_wool", 0, 0);
        mpGreenWoolEffects[i] = new RPSysEffect("Cow_slip_wool_green", 0, 0);
    }
}

void RPCowPlayerEffects::ResetWool() {
    mWoolIdx = 0;
    for (int i = 0; i < 4; i++) {
        mCrashActive[mWoolIdx] = false;
    }
}

void RPCowPlayerEffects::Calc() {
    nw4r::math::MTX34 rootMtx = mpCowModel->mRootMtx;
    nw4r::math::MTX34 headMtx = mpCowModel->mHeadMtx;

    if (mSpeedSmoke) {
        mpEffects[0]->create();
        mpEffects[1]->create();
    } else {
        mpEffects[0]->fade();
        mpEffects[1]->fade();
    }
    mpEffects[0]->setMtx(headMtx);
    mpEffects[0]->update();
    mpEffects[1]->setMtx(headMtx);
    mpEffects[1]->update();

    if (mJump) {
        mpEffects[2]->create();
    } else {
        mpEffects[2]->fade();
    }
    mpEffects[2]->setMtx(rootMtx);
    mpEffects[2]->update();

    if (mDamage) {
        mpEffects[3]->create();
    } else {
        mpEffects[3]->fade();
    }
    mpEffects[3]->setMtx(headMtx);
    mpEffects[3]->update();

    for (int i = 0; i < 4; i++) {
        if (mCrashActive[mWoolIdx]) {
            mCrashPos[mWoolIdx] += mCrashVel[mWoolIdx];
            mpCrashEffects[mWoolIdx]->setPos(mCrashPos[mWoolIdx]);
            mpCrashEffects[mWoolIdx]->update();
        }
    }

    EGG::Vector3f footPos[4];
    footPos[0] = mpCowModel->mFootPos[0];
    footPos[1] = mpCowModel->mFootPos[1];
    footPos[2] = mpCowModel->mFootPos[2];
    footPos[3] = mpCowModel->mFootPos[3];

    // Dash effect parameters: the "Max" values (at full speed) grow with mDashRate; each
    // parameter is then interpolated by mSpeed from its value at rest. Written out (no inline
    // lerp helper): an inline changed the float register colouring.
    int frontLifeMax = 10;
    int backLifeMax = 20;
    int weedLifeMax = 20;
    f32 frontRateMax = 0.15f + (0.3f - 0.15f) * mDashRate;
    f32 backRateMax = 0.09f + (0.5f - 0.09f) * mDashRate;
    f32 weedRateMax = 0.2f + (0.3f - 0.2f) * mDashRate;
    f32 frontPowerMax = 4.0f;
    f32 backPowerMax = 4.0f;
    f32 weedPowerMax = 2.0f;
    f32 frontScaleMax = 1.2f + (1.5f - 1.2f) * mDashRate;
    f32 backScaleMax = 1.2f + (1.5f - 1.2f) * mDashRate;

    int frontLife = 5 + mSpeed * (frontLifeMax - 5);
    int backLife = 15 + mSpeed * (backLifeMax - 15);
    int weedLife = 10 + mSpeed * (weedLifeMax - 10);
    f32 frontRate = 0.01f + mSpeed * (frontRateMax - 0.01f);
    f32 backRate = 0.01f + mSpeed * (backRateMax - 0.01f);
    f32 weedRate = 0.02f + mSpeed * (weedRateMax - 0.02f);
    f32 frontPower = (u16)(0.5f + mSpeed * (frontPowerMax - 0.5f));
    f32 backPower = (u16)(0.5f + mSpeed * (backPowerMax - 0.5f));
    f32 weedPower = (u16)(1.0f + mSpeed * (weedPowerMax - 1.0f));
    f32 frontScale = 0.5f + mSpeed * (frontScaleMax - 0.5f);
    f32 backScale = 0.3f + mSpeed * (backScaleMax - 0.3f);

    nw4r::math::MTX34 mtx = mMtx;

    for (int i = 0; i < 4; i++) {
        mtx._03 = footPos[i].x;
        mtx._13 = 0.0f;
        mtx._23 = footPos[i].z;

        if (mSpeed > 0.1f && footPos[i].y < 0.8f) {
            mpDashEffects[i]->create();
        } else {
            mpDashEffects[i]->fade();
        }
        switch (i) {
        case 0:
        case 1: {
            mpDashEffects[i]->setLife(frontLife);
            mpDashEffects[i]->setEmitRatio(frontRate);
            mpDashEffects[i]->setPowerSpecDir(frontPower);
            nw4r::math::VEC2 scale(frontScale, frontScale);
            mpDashEffects[i]->setParticleScale(scale);
            break;
        }
        case 2:
        case 3: {
            mpDashEffects[i]->setLife(backLife);
            mpDashEffects[i]->setEmitRatio(backRate);
            mpDashEffects[i]->setPowerSpecDir(backPower);
            nw4r::math::VEC2 scale(backScale, backScale);
            mpDashEffects[i]->setParticleScale(scale);
            break;
        }
        }
        mpDashEffects[i]->setMtx(mtx);
        mpDashEffects[i]->update();

        if (mSpeed > 0.1f && footPos[i].y < 0.8f && mOnWeed) {
            mpWeedEffects[i]->create();
        } else {
            mpWeedEffects[i]->fade();
        }
        mpWeedEffects[i]->setLife(weedLife);
        mpWeedEffects[i]->setEmitRatio(weedRate);
        mpWeedEffects[i]->setPowerSpecDir(weedPower);
        mpWeedEffects[i]->setMtx(mtx);
        mpWeedEffects[i]->update();

        if (mSpeed > 0.1f && footPos[i].y < 0.8f && !mOnWeed) {
            if (mSlip < 0.0f) {
                mpWoolEffects[i]->create();
                mpWoolEffects[i]->setSpecDir(RP_Mtx34_getColumn(mtx, 0));
            } else if (mSlip > 0.0f) {
                mpWoolEffects[i]->create();
                mpWoolEffects[i]->setSpecDir(-RP_Mtx34_getColumn(mtx, 0));
            } else if (mSlip2 > 0.0f) {
                mpWoolEffects[i]->create();
            } else {
                mpWoolEffects[i]->fade();
            }
        } else {
            mpWoolEffects[i]->fade();
        }
        mpWoolEffects[i]->setLife(weedLife);
        mpWoolEffects[i]->setEmitRatio(weedRate);
        mpWoolEffects[i]->setPowerSpecDir(weedPower);
        mpWoolEffects[i]->setMtx(mtx);
        mpWoolEffects[i]->update();

        if (mSpeed > 0.1f && footPos[i].y < 0.8f && mOnWeed) {
            if (mSlip < 0.0f) {
                mpGreenWoolEffects[i]->create();
                mpGreenWoolEffects[i]->setSpecDir(RP_Mtx34_getColumn(mtx, 0));
            } else if (mSlip > 0.0f) {
                mpGreenWoolEffects[i]->create();
                mpGreenWoolEffects[i]->setSpecDir(-RP_Mtx34_getColumn(mtx, 0));
            } else if (mSlip2 > 0.0f) {
                mpGreenWoolEffects[i]->create();
            } else {
                mpGreenWoolEffects[i]->fade();
            }
        } else {
            mpGreenWoolEffects[i]->fade();
        }
        mpGreenWoolEffects[i]->setLife(weedLife);
        mpGreenWoolEffects[i]->setEmitRatio(weedRate);
        mpGreenWoolEffects[i]->setPowerSpecDir(weedPower);
        mpGreenWoolEffects[i]->setMtx(mtx);
        mpGreenWoolEffects[i]->update();
    }
}

const f32 RPCowPlayerEffects::sEffectY = 6.0f;
