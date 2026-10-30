#include <Pack/RPParty/RPWlyScene/RPWlyCursor.h>

#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPGraphics/RPGrpScreen.h>

#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>

#include <egg/core/eggController.h>
#include <cmath>

// These operations have shared out-of-line copies in Wii Play.
extern "C" EGG::Vector2f getHorizon__Q23EGG10CoreStatusCFv(const EGG::CoreStatus*);
extern "C" EGG::Vector2f getDpdPos__Q23EGG10CoreStatusCFv(const EGG::CoreStatus*);
extern "C" EGG::Vector2f EGG_Vector2f_operator_minus(const EGG::Vector2f*,
                                                  const EGG::Vector2f*);
extern "C" EGG::Vector2f EGG_Vector2f_operator_mul(const EGG::Vector2f*, f32);

RPWlyCursor::RPWlyCursor(s32 player) {
    mVisible = false;
    mPlayer = player;
    switch (player) {
    case 0:
        mTargetPos.set(-0.4f, 0.0f);
        mPos.set(-0.4f, 0.0f);
        break;
    case 1:
        mTargetPos.set(0.4f, 0.0f);
        mPos.set(0.4f, 0.0f);
        break;
    }
    mAngle = 0.0f;
}

void RPWlyCursor::Create() {
    mpEffects[0] = new RPSysEffect(mPlayer == 0 ? "Wtd_cursor_normal_1p" :
                                               "Wtd_cursor_normal_2p", 6, 0);
    mpEffects[1] = new RPSysEffect(mPlayer == 0 ? "Wtd_cursor_normal_1p_blur" :
                                               "Wtd_cursor_normal_2p_blur", 6, 0);
    mpEffects[2] = new RPSysEffect("Wtd_cursor_normal_sd", 6, 0);
}

void RPWlyCursor::Init() {}

void RPWlyCursor::Reset() {
    mDistInit = false;
    mpEffects[1]->create();
    mpEffects[2]->create();
    mpEffects[0]->create();
}

void RPWlyCursor::SetEffectsVisible() {
    for (int i = 0; i < 3; i++) {
        bool visible = true;
        if (!mVisible && RPWlyGameMgr::sInstance->mState != 4) {
            visible = false;
        }
        mpEffects[i]->setDisableDraw(visible);
    }
}

void RPWlyCursor::Calculate() {
    EGG::CoreStatus status = *EGG::CoreControllerMgr::instance()
                                 ->getNthController(mPlayer)->getCoreStatus();
    if (status.getDPDValidFlag() > 0 && RPWlyGameMgr::sInstance->mState != 4) {
        EGG::Vector2f horizon = getHorizon__Q23EGG10CoreStatusCFv(&status);
        mAngle = EGG::Mathf::atan2(horizon.y, horizon.x);
        EGG::Vector2f pos = getDpdPos__Q23EGG10CoreStatusCFv(&status);
        mTargetPos.set(pos.x, -pos.y);
        EGG::Vector2f delta = EGG_Vector2f_operator_minus(&mTargetPos, &mPos);
        mPos += EGG_Vector2f_operator_mul(&delta, 0.5f);
        f32 x = mPos.x;
        if (x < -0.965f) {
            x = -0.965f;
        } else if (x > 0.965f) {
            x = 0.965f;
        }
        mPos.x = x;
        f32 y = mPos.y;
        if (y < -0.965f) {
            y = -0.965f;
        } else if (y > 0.965f) {
            y = 0.965f;
        }
        mPos.y = y;

        if (RPWlyGameMgr::sInstance->mMode == 0 && RPWlyGameMgr::sInstance->mState != 1) {
            f32 distance = status.dist;
            if (!mDistInit) {
                mDistMax = distance;
                mDist = distance;
                mDistMin = distance - 0.3f;
                mDistInit = true;
            }
            f32 step = (distance - mDist) * 0.2f;
            f32 magnitude = std::fabs(step);
            if (magnitude > 0.006666667f) {
                step = (step * 0.006666667f) / magnitude;
            }
            mDist += step;
            if (mDist < mDistMin) {
                mDistMin = distance;
                mDistRatio = 0.0f;
                mDistMax = distance + 0.3f;
                mDist = distance;
            } else if (mDist > mDistMax) {
                mDistMax = distance;
                mDistRatio = 1.0f;
                mDistMin = distance - 0.3f;
                mDist = distance;
            } else {
                mDistRatio = (mDist - mDistMin) / 0.3f;
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        EGG::Vector3f pos;
        EGG::Vector3f rotation(0.0f, 0.0f, mAngle);
        RPWlyGameMgr::sInstance->mpScreen->ConvertFromNormalCC(mPos.x, mPos.y,
                                                         &pos.x, &pos.y);
        pos.z = 0.0f;
        if (i == 2) {
            pos.x += 3.0f;
            pos.y -= 3.0f;
        }
        mpEffects[i]->setDisableCalcDraw(mVisible);
        mpEffects[i]->setPos(pos);
        mpEffects[i]->setParticleRotate(rotation);
        mpEffects[i]->update();
    }
}

void RPWlyCursor::Draw() {}
