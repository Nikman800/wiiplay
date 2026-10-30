// Fishing rod (RPFshRod): the rod model, the line (an RPCurve drawn through the fsh_rope joints),
// the hook's splash and effects, and the rod's sound actor.
// Unit 0x80209118-0x8020AE5C, including the compiler-generated adjustment thunks.
// Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshRod.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayer.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayerMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshStage.h>
#include <Pack/RPParty/RPPartyCommon/RPActorBase.h>
#include <Pack/RPParty/RPPartyCommon/RPCurve.h>
#include <Pack/RPParty/RPPartyCommon/RPJointMtxBuffer.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>

f32 RPFshRod::sModelScale = 1.0f;
f32 RPFshRod::sSpacing = 200.0f;
f32 RPFshRod::sRopeGravity = -2.3f;
f32 RPFshRod::sRopeDamping = 0.5f;
f32 RPFshRod::sRopeStiffness = 1.0f;
f32 RPFshRod::sRopeLength = 10.0f;
f32 RPFshRod::sRestRate = 0.4f;
f32 RPFshRod::sFollowRate = 0.1f;
f32 RPFshRod::sMaxBend = 6.0f;
f32 RPFshRod::sHookShadowHeight = 300.0f;
const char* RPFshRod::sTipJointName = "rod_end";
const char* RPFshRod::sGripJointName = "grip";
const char* RPFshRod::sRopeJointNames[ROPE_JOINT_NUM] = {
    "rope_joint_01", "rope_joint_02", "rope_joint_03",  "rope_joint_04",
    "rope_joint_05", "rope_joint_06", "rope_joint_end", "hook"};

EGG::Vector3f RPFshRod::sGripPos(0.0f, 40.0f, 40.0f);
EGG::Vector3f RPFshRod::sRestDir(0.0f, 1.0f, -1.0f);
f32 RPFshRod::sSplashBend = 0.1f * sMaxBend;

// Wii Play: both destructors come after the constructor in the binary
RPFshRod::~RPFshRod() {}

RPFshRodSnd::~RPFshRodSnd() {}

RPFshRod::RPFshRod(RPFshPlayer* pOwner) : mpOwner(pOwner) {
    mGripOffset = EGG::Vector3f::zero;
    mTipOffset = EGG::Vector3f::zero;
    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));

    const char* effectNames[EFFECT_NUM] = {"Fsh_hook_drop",   "Fsh_hook_tail_blue",
                                           "Fsh_hook_tail_red", "Fsh_hook_bubble",
                                           "Fsh_rope_ripple", "Fsh_rope_trail"};
    for (int i = 0; i < EFFECT_NUM; i++) {
        mpEffects[i] = new RPSysEffect(effectNames[i], 0, 0);
    }

    mpSnd = new RPFshRodSnd(RPSndAudioMgr::instance()->getPlayer());
    mLengthScale = 1.0f;
}

// Strings differ only in label names (@stringBase0 vs lbl_8031F468). The scale locals are declared
// before the y locals and assigned after them: that gives the original f3 (scale) / f4 (y).
void RPFshRod::Create() {
    if (mpOwner->GetPlayerIndex() % 2 == 0) {
        // Wii Play: the (handle, int) overload is called with the model name
        mpRodModel = RPGrpModel::Construct(1, "fsh_rod_B", 1, 0, 0);
    } else {
        mpRodModel = RPGrpModel::Construct(2, "fsh_rod_R", 1, 0, 0);
    }
    mpRopeModel = RPGrpModel::Construct(3, "fsh_rope", 1, 0, 0);

    mpRodJointMtx = new RPJointMtxBuffer(mpRodModel);
    mpRopeJointMtx = new RPJointMtxBuffer(mpRopeModel);
    mpRopeModel->SetCallback(this);
    mpRodModel->SetDrawScene(1);
    mpRopeModel->SetDrawScene(0);

    mTipJoint = mpRodModel->GetJointIndex(sTipJointName);
    mGripJoint = mpRodModel->GetJointIndex(sGripJointName);
    for (int i = 0; i < ROPE_JOINT_NUM; i++) {
        mRopeJoints[i] = mpRopeModel->GetJointIndex(sRopeJointNames[i]);
    }

    const nw4r::math::MTX34& rGrip = mpRodJointMtx->GetWorldMtx(mGripJoint);
    f32 scale;
    f32 y;
    mGripOffset.x = rGrip._03;
    y = rGrip._13;
    mGripOffset.y = y;
    mGripOffset.z = rGrip._23;
    mGripOffset = EGG::Vector3f(-mGripOffset.x, y, -mGripOffset.z);
    scale = sModelScale;
    mGripOffset *= scale;

    const nw4r::math::MTX34& rTip = mpRodJointMtx->GetWorldMtx(mTipJoint);
    f32 tipScale;
    f32 tipY;
    mTipOffset.x = rTip._03;
    tipY = rTip._13;
    mTipOffset.y = tipY;
    mTipOffset.z = rTip._23;
    mTipOffset = EGG::Vector3f(-mTipOffset.x, tipY, -mTipOffset.z);
    tipScale = sModelScale;
    mTipOffset *= tipScale;

    f32 segLengths[ROPE_JOINT_NUM - 1];
    for (int i = 0; i < ROPE_JOINT_NUM - 1; i++) {
        const nw4r::math::MTX34& rJoint = mpRopeJointMtx->GetLocalMtx(mRopeJoints[i + 1]);
        EGG::Vector3f offset(rJoint._03, rJoint._13, rJoint._23);
        segLengths[i] = sModelScale * offset.length();
    }

    f32 lineParam;
    f32 hookParam = 0.8f * sRopeLength;
    lineParam = 0.5f * hookParam;
    f32 params[ROPE_JOINT_NUM] = {lineParam, lineParam, lineParam, lineParam,
                                  lineParam, lineParam, lineParam, hookParam};
    RPCurveDesc desc = {ROPE_JOINT_NUM, sRopeGravity, sRopeDamping, sRopeStiffness,
                        segLengths,     params};
    mpRope = new RPCurve(desc);
}

void RPFshRod::Reset() {
    mIsHookInWater = false;
    mHookPos = EGG::Vector3f::zero;
    mUseKcl = true;
    mFollowRest = true;
    mPrevHookTarget = EGG::Vector3f::zero;
    SetFrontDir(CalcRestDir());

    f32 offset = mpOwner->GetPlayerIndex() -
                 (RPFshPlayerMgr::GetInstance()->GetPlayerNum() - 1.0f) / 2.0f;
    mGripPos = sGripPos;
    mGripPos.x += sSpacing * offset;
    SetPosFromGrip(mGripPos);

    EGG::Vector3f down(0.0f, -1.0f, 0.0f);
    RPCurveLine line = {GetTipPos(), down};
    mpRope->SamplePoints(line, NULL);

    mHookAxes[0] = EGG::Vector3f(0.0f, 0.0f, 1.0f);
    mHookAxes[1] = EGG::Vector3f(0.0f, 1.0f, 0.0f);
    mHookAxes[2] = EGG::Vector3f(1.0f, 0.0f, 0.0f);
    mDropFrame = 0;
    mLengthScale = 1.0f;
}

void RPFshRod::Exit() {}

void RPFshRod::Calculate() {
    mpRope->SetUnk4(sRopeGravity);
    mpRope->SetUnk8(sRopeDamping);
    mPrevHookTarget = mpRope->GetTargetPoint(ROPE_JOINT_NUM - 1);

    if (mFollowRest) {
        EGG::Vector3f dir = mFrontDir * (1.0f - sFollowRate) + CalcRestDir() * sFollowRate;
        dir.normalise();
        SetFrontDir(dir);
    }

    RPKclHolder* pKcl = NULL;
    if (mUseKcl) {
        pKcl = RPFshStage::GetInstance()->GetKcl();
    }

    RPCurveAnchor anchor = {GetTipPos(), EGG::Vector3f::zero, pKcl};
    mpRope->Build(anchor);
    unkD4 = sRopeLength;
}

void RPFshRod::CalcHookWater() {
    f32 surface = RPFshStage::GetWaterSurfaceY();
    for (int i = 0; i < ROPE_JOINT_NUM - 1; i++) {
        EGG::Vector3f upper = mpRope->GetPoint(i);
        EGG::Vector3f lower = mpRope->GetPoint(i + 1);

        if (i == 0 && upper.y < surface) {
            mHookPos = upper;
            mHookPos.y = surface;
            break;
        }

        if (upper.y > surface && lower.y <= surface) {
            f32 dy = upper.y - lower.y;
            if (dy > 1.1920929e-7f) {
                f32 t = (upper.y - surface) / dy;
                mHookPos = upper * (1.0f - t) + lower * t;
                break;
            }
        }
    }

    bool wasInWater = mIsHookInWater;
    const EGG::Vector3f& hookPoint = mpRope->GetPoint(ROPE_JOINT_NUM - 1);
    if (RPFshStage::GetInstance()->IsInPond(hookPoint)) {
        if (!wasInWater) {
            if (mpOwner->GetState() == RPFshPlayer::STATE_WAIT) {
                mpSnd->startSound(4U);
            }

            mpEffects[4]->create();
            mpEffects[5]->create();
            if (mpOwner->GetPlayerIndex() == 0) {
                mpEffects[1]->create();
            } else {
                mpEffects[2]->create();
            }
            mpEffects[3]->create();
        } else {
            EGG::Vector3f move = mHookPos - mPrevHookPos;
            move.y = 0.0f;
            f32 dist = move.length();
            if (mpOwner->GetState() == RPFshPlayer::STATE_WAIT) {
                mpSnd->HoldSplash(dist);
            }
        }

        mPrevHookPos = mHookPos;
        mIsHookInWater = true;
        mpEffects[4]->setPos(mHookPos);
        mpEffects[5]->setPos(mHookPos);
        mpEffects[1]->setPos(mpRope->GetPoint(ROPE_JOINT_NUM - 1));
        mpEffects[2]->setPos(mpRope->GetPoint(ROPE_JOINT_NUM - 1));
        mpEffects[3]->setPos(mpRope->GetPoint(ROPE_JOINT_NUM - 1));
        mpEffects[0]->fade();
    } else {
        if (wasInWater) {
            if (mpOwner->GetState() == RPFshPlayer::STATE_WAIT) {
                mpSnd->startSound(5U);
            }
            mDropFrame = 0;
            mpEffects[0]->create();
        }

        mIsHookInWater = false;
        if (mDropFrame > sDropFrameMax) {
            mpEffects[0]->fade();
        } else {
            mpEffects[0]->setPos(mpRope->GetPoint(ROPE_JOINT_NUM - 1));
            mDropFrame++;
        }

        mpEffects[4]->fade();
        mpEffects[5]->fade();
        mpEffects[1]->fade();
        mpEffects[2]->fade();
        mpEffects[3]->fade();
    }

    if (mIsHookInWater != wasInWater) {
        RPSysEffect effect("Fsh_hook_water", 0, 0);
        effect.create();
        effect.setPos(mHookPos);
        effect.update();
    }

    mBend -= sSplashBend;
    EGG::Vector3f pull = mPrevHookTarget - mpRope->GetTargetPoint(ROPE_JOINT_NUM - 1);
    mBend += pull.y > 0.0f ? pull.y : -pull.y;
    if (mBend < 0.0f) {
        mBend = 0.0f;
    }
    if (mBend > sMaxBend) {
        mBend = sMaxBend;
    }

    CalcModelMtx();
    for (int i = 0; i < EFFECT_NUM; i++) {
        mpEffects[i]->update();
    }

    EGG::Vector3f hook = mpRope->GetPoint(ROPE_JOINT_NUM - 1);
    mpSnd->set3DPosition(hook);
}

EGG::Vector3f RPFshRod::GetTipPos() const {
    return mBasePos + mFrontDir * mTipOffset.z * mLengthScale + mUpDir * mTipOffset.y +
           mSideDir * mTipOffset.x;
}

EGG::Vector3f RPFshRod::GetGripPos() const {
    return mBasePos + mFrontDir * mGripOffset.z * mLengthScale + mUpDir * mGripOffset.y +
           mSideDir * mGripOffset.x;
}

void RPFshRod::SetPosFromGrip(const EGG::Vector3f& rGripPos) {
    mBasePos = rGripPos - (mFrontDir * mGripOffset.z * mLengthScale +
                                          mUpDir * mGripOffset.y + mSideDir * mGripOffset.x);
}

void RPFshRod::Translate(const nw4r::math::VEC3& rOffset) {
    mBasePos.x += rOffset.x;
    mBasePos.y += rOffset.y;
    mBasePos.z += rOffset.z;

    for (int i = 0; i < ROPE_JOINT_NUM; i++) {
        EGG::Vector3f pos = mpRope->GetPoint(i);
        pos.x += rOffset.x;
        pos.y += rOffset.y;
        pos.z += rOffset.z;
        mpRope->SetPoint(i, pos);
    }
}

const nw4r::math::VEC3& RPFshRod::GetRopePoint(s32 idx) const {
    return mpRope->GetPoint(idx);
}

void RPFshRod::SetAllRopeTargets(const nw4r::math::VEC3& rPos) {
    for (int i = 0; i < ROPE_JOINT_NUM; i++) {
        mpRope->SetTargetPoint(i, static_cast<const EGG::Vector3f&>(rPos));
    }
}

void RPFshRod::SetRopeTargetPoint(s32 idx, const nw4r::math::VEC3& rPos) {
    mpRope->SetTargetPoint(idx, static_cast<const EGG::Vector3f&>(rPos));
}

EGG::Vector3f RPFshRod::CalcRestDir() const {
    EGG::Vector3f dir = sRestDir;
    f32 offset = mpOwner->GetPlayerIndex() -
                 (RPFshPlayerMgr::GetInstance()->GetPlayerNum() - 1.0f) / 2.0f;
    if (RPFshPlayerMgr::GetInstance()->GetPlayerNum() % 2 == 1 && offset >= 0.0f) {
        offset += 1.0f;
    }
    dir.x -= sRestRate * offset;
    dir.normalise();
    return dir;
}

void RPFshRod::SetFrontDir(const nw4r::math::VEC3& rDir) {
    mFrontDir = static_cast<const EGG::Vector3f&>(rDir);
    UpdateAxes();
}

f32 RPFshRod::GetBendRatio() const {
    return mBend / sMaxBend;
}

void RPFshRod::CalcModelMtx() {
    UpdateAxes();

    EGG::Vector3f x = -mSideDir;
    EGG::Vector3f z = -mFrontDir;
    nw4r::math::MTX34 rodMtx;
    rodMtx._00 = x.x;
    rodMtx._10 = x.y;
    rodMtx._20 = x.z;
    rodMtx._01 = mUpDir.x;
    rodMtx._11 = mUpDir.y;
    rodMtx._21 = mUpDir.z;
    rodMtx._02 = z.x;
    rodMtx._12 = z.y;
    rodMtx._22 = z.z;
    rodMtx._03 = mBasePos.x;
    rodMtx._13 = mBasePos.y;
    rodMtx._23 = mBasePos.z;
    mpRodModel->GetModelEx()->setLocalMtx(&rodMtx);
    mpRodModel->SetBaseScale(sModelScale, sModelScale, sModelScale * mLengthScale);
    mpRodModel->Calc();
    mpRodModel->Entry();

    EGG::Vector3f dir = mpRope->GetPoint(mRopeJoints[0]) - mpRope->GetPoint(mRopeJoints[1]);
    dir.normalise();
    EGG::Vector3f ropeX(dir.z, dir.x, dir.y);
    EGG::Vector3f ropeZ = ropeX.cross(dir);
    ropeZ.normalise();
    ropeX = dir.cross(ropeZ);

    nw4r::math::MTX34 ropeMtx;
    ropeMtx._00 = ropeX.x;
    ropeMtx._10 = ropeX.y;
    ropeMtx._20 = ropeX.z;
    ropeMtx._01 = dir.x;
    ropeMtx._11 = dir.y;
    ropeMtx._21 = dir.z;
    ropeMtx._02 = ropeZ.x;
    ropeMtx._12 = ropeZ.y;
    ropeMtx._22 = ropeZ.z;
    const EGG::Vector3f& rRoot = mpRope->GetPoint(0);
    ropeMtx._03 = rRoot.x;
    ropeMtx._13 = rRoot.y;
    ropeMtx._23 = rRoot.z;
    mpRopeModel->GetModelEx()->setLocalMtx(&ropeMtx);
    mpRopeModel->SetBaseScale(sModelScale);
    mpRopeModel->Calc();
    mpRopeModel->Entry();
}

void RPFshRod::EditWorldMtx(RPGrpModel* pModel) {
    for (int i = 0; i < ROPE_JOINT_NUM; i++) {
        nw4r::math::MTX34* pMtx = pModel->ReferWorldMtx(mRopeJoints[i]);
        pMtx->_03 = mpRope->GetPoint(i).x;
        pMtx->_13 = mpRope->GetPoint(i).y;
        pMtx->_23 = mpRope->GetPoint(i).z;

        if (i >= ROPE_JOINT_NUM - 1) {
            continue;
        }

        EGG::Vector3f dir = mpRope->GetPoint(i + 1) - mpRope->GetPoint(i);
        dir.normalise();
        if ((dir.squaredLength() <= 1.1920929e-7f) != false) {
            continue;
        }

        EGG::Vector3f z(0.0f, 0.0f, 1.0f);
        EGG::Vector3f y = z.cross(dir);
        y.normalise();
        if ((y.squaredLength() <= 1.1920929e-7f) != false) {
            y = EGG::Vector3f(1.0f, 0.0f, 0.0f);
            z = dir.cross(y);
            z.normalise();
            y = z.cross(dir);
        } else {
            z = dir.cross(y);
        }

        pMtx->_00 = dir.x;
        pMtx->_10 = dir.y;
        pMtx->_20 = dir.z;
        pMtx->_01 = y.x;
        pMtx->_11 = y.y;
        pMtx->_21 = y.z;
        pMtx->_02 = z.x;
        pMtx->_12 = z.y;
        pMtx->_22 = z.z;

        if (i == ROPE_JOINT_NUM - 2) {
            mHookAxes[0] = z;
            mHookAxes[1] = y;
            mHookAxes[2] = dir;
        }
    }
}

void RPFshRod::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() == 0 && RPGrpRenderer::GetDrawStep() == 9 &&
        mpOwner->GetState() != RPFshPlayer::STATE_NONE) {
        nw4r::math::VEC3 hook = mpRope->GetPoint(ROPE_JOINT_NUM - 1);
        if (1.0f + (hook.y - RPFshStage::GetPondBottomY()) > 0.0f) {
            RPDrawShadowVolumeQuad(hook, unkD4, sHookShadowHeight);
        }
    }
}

// Wii Play: a named constant defined after the code, so it is loaded (first in .sdata2)
const f32 RPFshRod::sDropFrameMax = 70.0f;
