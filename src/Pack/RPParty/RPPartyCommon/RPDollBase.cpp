#include <Pack/RPParty/RPPartyCommon/RPDollBase.h>

#include <Pack/RPGraphics/RPGrpCamera.h>
#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpModelAnm.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPKernel/RPSysKokeshi.h>
#include <Pack/RPParty/RPPartyCommon/RPJointMtxBuffer.h>
#include <Pack/RPParty/RPPartyCommon/RPInterpCamera.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

using EGG::Vector3f;

f32 RPDollBase::sLookInStep = 0.08f;
f32 RPDollBase::sLookOutStep = 0.06f;
f32 RPDollBase::sTurnSpeed = 0.08f;
f32 RPDollBase::sHitRadiusA = 2.0f;
f32 RPDollBase::sHitRadiusB = 3.0f;
f32 RPDollBase::sHitRadiusC = 1.6f;
f32 RPDollBase::sHitRadiusD = 1.5f;
f32 RPDollBase::sHitRadiusE = 1.5f;

inline f32 DollClamp(f32 value, f32 lo, f32 hi) {
    if (value < lo) {
        value = lo;
    } else if (value > hi) {
        value = hi;
    }
    return value;
}

inline f32 DollAngle(const Vector3f& a, const Vector3f& b) {
    f32 dot = a.dot(b);
    return EGG::Mathf::abs(EGG::Mathf::atan2(a.cross(b).length(), dot));
}

inline bool DollNearZero(const Vector3f& value) {
    return value.squaredLength() <= EGG::Mathf::epsilon();
}

inline bool DollDifferent(const Vector3f& a, const Vector3f& b) {
    bool different = false;
    if (a.x != b.x || a.y != b.y || a.z != b.z) {
        different = true;
    }
    return different;
}

extern "C" Vector3f RP_mtx33MultVec(const EGG::Matrix34f&, const Vector3f&);

RPDollBase::RPDollBase(s32 index) {
    mIndex = index;
}

void RPDollBase::Load() {
    RPSysResourceManager::LoadGameCommonArchive(-1, NULL);
    mpKokeshi->LoadResource(NULL);

    for (int i = 0; i < JOINT_NUM; i++) {
        const char* names[JOINT_NUM] = {
            "head", "hip", "arm_l1", "arm_r1", "arm_l2", "arm_r2", "wrist_l",
            "wrist_r", "leg_l1", "leg_r1", "leg_l2", "leg_r2", "ankle_l", "ankle_r"
        };
        mJointIds[i] = mpKokeshi->GetBodyModel()->GetJointIndex(names[i]);
    }

    mpJointMtxBuffer = new RPJointMtxBuffer(mpKokeshi->GetBodyModel());
    for (int i = 0; i < 3; i++) {
        Vector3f offset;
        const int joints[3] = {2, 4, 6};
        const nw4r::math::MTX34& mtx = mpJointMtxBuffer->GetLocalMtx(mJointIds[joints[i]]);
        offset.set(mtx._03, mtx._13, mtx._23);
        mBoneLengths[i] = offset.length();
    }

    mpKokeshi->ApplyLightTexture(1);
    mpKokeshi->GetBodyModel()->SetDrawScene(1);
    mpKokeshi->GetNigaoeModel()->SetDrawScene(1);
}

void RPDollBase::Reset() {
    ResetAxes();
    for (int i = 0; i < JOINT_NUM; i++) {
        mJointPos[i] = static_cast<const Vector3f&>(mPos);
    }
    mHeadPos = Vector3f::zero;
    mHeadAxisY = Vector3f(0.0f, 0.0f, 1.0f);
    mHeadAxisX = Vector3f(0.0f, 1.0f, 0.0f);
    mIsLooking = false;
    mLookWeight = 0.0f;
    mLookBlend = 0.0f;
    mLookSpeed = sTurnSpeed;
    mLookDir = mAxisZ;
    for (int i = 0; i < ANM_NUM; i++) {
        mAnmIdx[i] = -1;
        mAnmWeight[i] = 0.0f;
        mAnmWeightStep[i] = 0.0f;
    }
}

void RPDollBase::Calculate() {
    if (mIsLooking) {
        mLookWeight += sLookInStep;
        mLookBlend += mLookSpeed;
    } else {
        mLookWeight -= sLookOutStep;
    }
    mLookWeight = DollClamp(mLookWeight, 0.0f, 1.0f);
    mLookBlend = DollClamp(mLookBlend, 0.0f, 1.0f);

    RPGrpModelAnm* anm = mpKokeshi->GetBodyModel()->GetModelAnm();
    f32 total = 0.0f;
    for (int i = 0; i < ANM_NUM; i++) {
        if (mAnmIdx[i] >= 0) {
            mAnmWeight[i] += mAnmWeightStep[i];
            mAnmWeight[i] = EGG::Mathf::clamp2(mAnmWeight[i], 0.0f, 1.0f);
            total += mAnmWeight[i];
        }
    }
    for (int i = 0; i < ANM_NUM; i++) {
        if (mAnmIdx[i] >= 0) {
            if (mAnmWeight[i] <= 0.0f) {
                mAnmIdx[i] = -1;
                anm->RemoveBlendAnm(RPGrpModelAnm::Anm_Chr, i, 0);
            } else if (total > EGG::Mathf::epsilon()) {
                anm->SetBlendWeight(RPGrpModelAnm::Anm_Chr, i, mAnmWeight[i] / total, 0);
            }
        }
    }
}

void RPDollBase::LookAt(const nw4r::math::VEC2& screen, f32 distance, f32 maxAngle) {
    mIsLooking = true;
    mLookTo = ProjectLookTarget(screen, distance, maxAngle);
    mLookFrom = mLookDir;
    mLookBlend = 0.0f;
}

void RPDollBase::LookAtTarget(const nw4r::math::VEC3& target, f32 maxAngle) {
    mIsLooking = true;
    mLookTo = ClampLookDir(target, maxAngle);
    mLookFrom = mLookDir;
    mLookBlend = 0.0f;
}

Vector3f RPDollBase::ProjectLookTarget(const nw4r::math::VEC2& screen, f32 distance,
                                       f32 maxAngle) {
    RPGrpView* view = RPGrpRenderer::GetCurrent()->GetView(2);
    RPGrpCamera* camera = view->GetCamera();
    Vector3f point = static_cast<const Vector3f&>(mPos);
    Vector3f plane = point;
    Vector3f normal;
    const nw4r::math::VEC3 normalValue = camera->GetTargetRef() - camera->GetPosition();
    normal.set(normalValue.x, normalValue.y, normalValue.z);
    normal.normalise();
    f32 screenX = screen.x;
    f32 screenY = screen.y;
    const RPGrpScreen* viewScreen = view->GetScreen();
    camera->GetScreenToPlane(&point, plane, normal, viewScreen, screenX, screenY);
    point -= normal * distance;
    return ClampLookDir(point, maxAngle);
}

Vector3f RPDollBase::ClampLookDir(const nw4r::math::VEC3& target, f32 maxAngle) {
    Vector3f result = (static_cast<const Vector3f&>(target) - static_cast<const Vector3f&>(mHeadPos))
                   + static_cast<const Vector3f&>(mHeadAxisX) * sHitRadiusA;
    result.normalise();
    Vector3f right = static_cast<const Vector3f&>(mHeadAxisX);
    Vector3f forward = static_cast<const Vector3f&>(mHeadAxisY);
    f32 angle = DollAngle(forward, result);
    if (angle > maxAngle) {
        Vector3f axis;
        axis = forward.cross(result);
        axis.normalise();
        if (angle > 2.5132742f) {
            if (DollNearZero(axis)) {
                goto useRight;
            }
            if (DollAngle(axis, right) < EGG::Mathf::pi_half()) {
                goto useRight;
            }
            goto useNegative;
        useRight:
            axis = right;
            goto rotate;
        useNegative:
            axis = -right;
        }
    rotate:
        EGG::Matrix34f rotation;
        rotation.setAxisRotation(axis, maxAngle - angle);
        result = RP_mtx33MultVec(rotation, result);
    }
    return result;
}

s32 RPDollBase::HitTestPart(const nw4r::math::VEC2& screen, s32 player) const {
    RPGrpView* view = RPGrpRenderer::GetCurrent()->GetView(2);
    RPGrpCamera* camera = view->GetCamera();
    Vector3f world;
    view->GetScreenToWorld(&world, screen);
    Ray ray;
    ray.mDir = world - Vector3f(camera->GetPosition().x, camera->GetPosition().y,
                                    camera->GetPosition().z);
    ray.mDir.normalise();
    ray.mOrigin = static_cast<const Vector3f&>(camera->GetPosition());
    Vector3f head = static_cast<const Vector3f&>(mHeadPos)
                  + (static_cast<const Vector3f&>(mHeadAxisX) * sHitRadiusA) * mScale;
    if (RayHitSphere(ray, head, sHitRadiusB * mScale, NULL)) {
        return 0;
    }
    Vector3f torso = mJointPos[0] - mJointPos[1];
    Vector3f top = mJointPos[1] + torso * 0.75f;
    f32 bodyRadius = sHitRadiusC * mScale;
    if (RayHitCapsule(ray, bodyRadius, mJointPos[1], top, NULL)) {
        return 1;
    }
    const int joints[4][3] = {{2, 4, 6}, {3, 5, 7}, {8, 10, 12}, {9, 11, 13}};
    f32 radii[4] = {sHitRadiusD, sHitRadiusD,
                    sHitRadiusE, sHitRadiusE};
    int hit = -1;
    f32 radius;
    f32 best = 0.0f;
    for (int i = 0; i < 4; i++) {
        radius = radii[i];
        const int* row = joints[i];
        for (int j = 0; j < 2; j++) {
            f32 current;
            f32 scaledRadius = mScale * radius;
            if (RayHitJoints(ray, scaledRadius, row[j], row[j + 1], &current)) {
                if (hit == -1 || current < best) {
                    hit = i;
                    best = current;
                }
            }
        }
    }
    if (hit != -1) {
        const int parts[4] = {2, 3, 4, 5};
        return parts[hit];
    }
    return -1;
}

f32 RPDollBase::GetTurnSpeed() {
    return sTurnSpeed;
}

void RPDollBase::UpdateModelMatrices() {
    EGG::Matrix34f bodyMtx;
    bodyMtx.setBase(0, mAxisX);
    bodyMtx.setBase(1, mAxisY);
    bodyMtx.setBase(2, mAxisZ);
    bodyMtx.setBase(3, static_cast<const Vector3f&>(mPos));

    RPGrpModel* model = mpKokeshi->GetBodyModel();
    if (model->GetScnObj() != NULL) {
        model->GetScnObj()->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &bodyMtx);
    }
    model->SetBaseScale(mScale);
    model->UpdateFrame();
    model->Calc();
    model->Entry();

    model = mpKokeshi->GetNigaoeModel();
    EGG::Matrix34f headMtx;
    mpKokeshi->GetHeadJointMtx(&headMtx);
    headMtx.getBase(3, static_cast<Vector3f&>(mHeadPos));
    headMtx.getBase(0, static_cast<Vector3f&>(mHeadAxisX));
    headMtx.getBase(1, static_cast<Vector3f&>(mHeadAxisY));
    if (mLookWeight > 0.0f) {
        f32 t = RPRange::CalcEase(mLookBlend, 1);
        Vector3f look = static_cast<const Vector3f&>(mLookFrom) * (1.0f - t)
                      + static_cast<const Vector3f&>(mLookTo) * t;
        look.normalise();
        Vector3f up(0.0f, 1.0f, 0.0f);
        Vector3f right = up.cross(look);
        right.normalise();
        up = look.cross(right);
        Vector3f oldX, oldY, oldZ;
        Vector3f x, y, z;
        headMtx.getBase(0, oldX);
        headMtx.getBase(1, oldY);
        headMtx.getBase(2, oldZ);
        RPSlerpBasis(mLookWeight, &oldX, &oldY, &oldZ, up, look, right, x, y, z);
        headMtx.setBase(0, x);
        headMtx.setBase(1, y);
        headMtx.setBase(2, z);
    }
    headMtx.getBase(1, static_cast<Vector3f&>(mLookDir));
    if (model->GetScnObj() != NULL) {
        model->GetScnObj()->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &headMtx);
    }
    model->SetBaseScale(mScale);
    model->Calc();
    model->Entry();
}

void RPDollBase::StartBlendAnm(u16 index, f32 step) {
    int slot = -1;
    for (int i = 0; i < ANM_NUM; i++) {
        if (mAnmIdx[i] == -1) {
            slot = i;
            break;
        }
    }
    RPGrpModelAnm* anm = mpKokeshi->GetBodyModel()->GetModelAnm();
    anm->SetBlendAnm(RPGrpModelAnm::Anm_Chr, slot, mAnmIdx[0]);
    mAnmIdx[slot] = mAnmIdx[0];
    mAnmWeight[slot] = mAnmWeight[0];
    anm->Start(RPGrpModelAnm::Anm_Chr, index, 0.0f);
    mAnmIdx[0] = index;
    mAnmWeight[0] = 0.0f;
    mAnmWeightStep[0] = step;
    for (int i = 1; i < ANM_NUM; i++) {
        mAnmWeightStep[i] = -step;
    }
}

void RPDollBase::PlayAnm(u16 index, f32 blendFrames) {
    RPGrpModelAnm* anm = mpKokeshi->GetBodyModel()->GetModelAnm();
    if (mAnmIdx[0] < 0) {
        anm->Start(RPGrpModelAnm::Anm_Chr, index, 0.0f);
        mAnmIdx[0] = index;
        mAnmWeight[0] = 1.0f;
        return;
    }
    f32 step = 1.0f / blendFrames;
    if (index == mAnmIdx[0]) {
        return;
    }
    for (int i = 1; i < ANM_NUM; i++) {
        if (index == mAnmIdx[i]) {
            mAnmIdx[i] = -1;
            StartBlendAnm(index, step);
            return;
        }
    }
    for (int i = 0; i < ANM_NUM; i++) {
        if (mAnmIdx[i] == -1) {
            StartBlendAnm(index, step);
            return;
        }
    }
    int slot = -1;
    f32 smallest = 1.0f;
    for (int i = 0; i < ANM_NUM; i++) {
        if (mAnmWeight[i] <= smallest) {
            slot = i;
            smallest = mAnmWeight[i];
        }
    }
    mAnmIdx[slot] = -1;
    StartBlendAnm(index, step);
}

bool RPDollBase::IsAnmDone(u16 index) {
    RPGrpModelAnm* anm = mpKokeshi->GetBodyModel()->GetModelAnm();
    for (int i = 0; i < RPDollBase::ANM_NUM; i++) {
        if (index == mAnmIdx[i]) {
            f32 end = anm->GetNumFrame(RPGrpModelAnm::Anm_Chr, index) - 1.01f;
            return anm->GetFrame(RPGrpModelAnm::Anm_Chr, index) > end;
        }
    }
    return true;
}

bool RPDollBase::RayHitCapsule(const Ray& ray, f32 radius, Vector3f a, Vector3f b,
                               f32* distance) const {
    int count = 1;
    Vector3f delta = static_cast<const Vector3f&>(b) - static_cast<const Vector3f&>(a);
    f32 length = delta.length();
    if (DollDifferent(static_cast<const Vector3f&>(a), static_cast<const Vector3f&>(b))) {
        count = int(length / (0.25f * radius)) + 1;
    }
    bool hit = false;
    f32 best;
    for (int i = 0; i <= count; i++) {
        Vector3f center = static_cast<const Vector3f&>(a) + delta * (f32(i) / f32(count));
        f32 current;
        if (RayHitSphere(ray, center, radius, &current)) {
            if (!hit || current < best) {
                hit = true;
                best = current;
            }
        }
    }
    if (hit) {
        if (distance != NULL) {
            *distance = best;
        }
        return true;
    }
    return false;
}

bool RPDollBase::RayHitSphere(const Ray& ray, const Vector3f& center, f32 radius,
                              f32* distance) const {
    Vector3f delta = ray.mOrigin - center;
    Vector3f dir = ray.mDir;
    f32 a = dir.squaredLength();
    f32 b = 2.0f * delta.dot(dir);
    f32 c = delta.squaredLength() - radius * radius;
    f32 discr = b * b - 4.0f * a * c;
    if (discr < 0.0f) {
        return false;
    }
    f32 near = (-b - EGG::Mathf::sqrt(discr)) / (2.0f * a);
    if (near < 0.0f) {
        return false;
    }
    f32 far = (-b + EGG::Mathf::sqrt(discr)) / (2.0f * a);
    Vector3f point = delta + (dir * (near + far)) * 0.5f;
    Vector3f offset = point - center;
    if (distance != NULL) {
        *distance = offset.length();
    }
    return true;
}

void RPDollBase::EditWorldMtx(RPGrpModel* model) {
    for (int i = 0; i < JOINT_NUM; i++) {
        const nw4r::math::MTX34* mtx = model->ReferWorldMtx(mJointIds[i]);
        mJointPos[i].x = mtx->_03;
        mJointPos[i].y = mtx->_13;
        mJointPos[i].z = mtx->_23;
    }
}
