// RPPhysSphere: rigid sphere physics (Billiards balls).
// Functions in reverse address order. The collision functions keep their ledger names
// (extern "C"): linked Billiards units call them by these names.
#pragma auto_inline off
#include <egg/math.h>
#pragma auto_inline on

#include <Pack/RPParty/RPPartyCommon/RPPhysSphere.h>

#include <egg/geom.h>

#include <float.h>

extern "C" EGG::Vector3f RP_mtx33MultVec(const EGG::Matrix34f& rMtx, const EGG::Vector3f& rVec);

namespace {

// Access to RPPhysSphere's protected members from the ledger's free functions.
struct PhysSphereView : public RPPhysSphere {
    const EGG::Vector3f& Vel() const {
        return static_cast<const EGG::Vector3f&>(mVec1C);
    }
    const EGG::Vector3f& AngVel() const {
        return static_cast<const EGG::Vector3f&>(mAngVel);
    }
    const EGG::Matrix34f& Rot() const {
        return static_cast<const EGG::Matrix34f&>(mRotMtx);
    }
    const EGG::Matrix34f& RotT() const {
        return static_cast<const EGG::Matrix34f&>(mRotMtxT);
    }
    const EGG::Vector3f& Pos() const {
        return static_cast<const EGG::Vector3f&>(mPos);
    }

    using RPPhysSphere::mAngVel;
    using RPPhysSphere::mInertia;
    using RPPhysSphere::mpContactA;
    using RPPhysSphere::mpContactB;
    using RPPhysSphere::mpContactC;
    using RPPhysSphere::mParam;
    using RPPhysSphere::mRotMtx;
    using RPPhysSphere::mRotMtxT;
    using RPActorBase::mAxisX;
    using RPActorBase::mAxisY;
    using RPActorBase::mAxisZ;
    using RPActorBase::mPos;
    using RPActorBase::mVec1C;
};

inline PhysSphereView* View(RPPhysSphere* pSphere) {
    return static_cast<PhysSphereView*>(pSphere);
}

inline void Transpose33(EGG::Matrix34f& rDst, EGG::Matrix34f& rSrc) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            rDst(i, j) = rSrc(j, i);
        }
    }
}

inline bool IsTiny(const EGG::Vector3f& rVec) {
    return rVec.squaredLength() <= FLT_EPSILON;
}

// One table contact
struct ContactInput {
    EGG::Vector3f mParam; // at 0x0 restitution, friction, friction while sliding
    EGG::Vector3f mPoint; // at 0xC contact normal
    f32 mDepth;           // at 0x18
    f32 mSpinLimit;       // at 0x1C
};

// Position, velocity and angular velocity change from a collision
struct SphereImpulse {
    EGG::Vector3f mPos;    // at 0x0
    EGG::Vector3f mVel;    // at 0xC
    EGG::Vector3f mAngVel; // at 0x18
};

// One resolved contact
struct ContactResult {
    bool mHit;          // at 0x0
    EGG::Vector3f mPos; // at 0x4
    f32 mSpeed;         // at 0x10
};

struct SphereCollideParam {
    f32 mRestitution;      // at 0x0
    f32 mFriction;         // at 0x4
    RPPhysSphere* mpOther; // at 0x8
};

} // namespace

extern "C" s32 RP_PhysSphere_checkContact(RPPhysSphere* pSphere, const nw4r::math::VEC3* pNormal,
                                          EGG::Vector3f* pVel);

RPPhysSphere::~RPPhysSphere() {}

RPPhysSphere::RPPhysSphere(const RPPhysSphereParam& rParam) {
    mParam.unk10 = FLT_EPSILON;
    mParam.restSpeed = FLT_EPSILON;
    mParam.unk18 = 0.0f;
    mParam.unk1C = 0.0f;
    mParam = rParam;
    static_cast<EGG::Vector3f&>(mAngVel) = EGG::Vector3f::zero;
    mInertia = 0.4f * mParam.mass * mParam.radius * mParam.radius;
    mPos = EGG::Vector3f::zero;
    mVec1C = EGG::Vector3f::zero;
    mAxisX = EGG::Vector3f(1.0f, 0.0f, 0.0f);
    mAxisY = EGG::Vector3f(0.0f, 1.0f, 0.0f);
    mAxisZ = EGG::Vector3f(0.0f, 0.0f, 1.0f);
    ResetMatrices();
    mpContactA = new RPVec3Range;
    mpContactB = new RPVec3Range;
    mpContactC = new RPVec3Range;
}

void RPPhysSphere::Update() {
    static_cast<EGG::Vector3f&>(mPrevPos) = static_cast<EGG::Vector3f&>(mPos);
    if (IsAtRest(false)) {
        static_cast<EGG::Vector3f&>(mVec1C) = EGG::Vector3f::zero;
        static_cast<EGG::Vector3f&>(mAngVel) = EGG::Vector3f::zero;
    }
    static_cast<EGG::Vector3f&>(mVec1C) -= static_cast<EGG::Vector3f&>(mVec1C) * mParam.unk18;
    static_cast<EGG::Vector3f&>(mAngVel) -= static_cast<EGG::Vector3f&>(mAngVel) * mParam.unk18;
    static_cast<EGG::Vector3f&>(mVec1C).y -= mParam.unk8;
    static_cast<EGG::Vector3f&>(mPos) += static_cast<EGG::Vector3f&>(mVec1C);

    EGG::Matrix34f mtx;
    mtx(0, 0) = mAxisX.x;
    mtx(1, 0) = mAxisX.y;
    mtx(2, 0) = mAxisX.z;
    mtx(0, 1) = mAxisY.x;
    mtx(1, 1) = mAxisY.y;
    mtx(2, 1) = mAxisY.z;
    mtx(0, 2) = mAxisZ.x;
    mtx(1, 2) = mAxisZ.y;
    mtx(2, 2) = mAxisZ.z;
    EGG::Vector3f axis = RP_mtx33MultVec(mtx, static_cast<EGG::Vector3f&>(mAngVel));
    f32 angle = axis.normalise();
    if (angle > FLT_EPSILON) {
        EGG::Matrix34f rot;
        rot.setAxisRotation(axis, angle);
        EGG::Vector3f x;
        EGG::Vector3f y;
        EGG::Vector3f z;
        x = RP_mtx33MultVec(rot, mAxisX);
        y = RP_mtx33MultVec(rot, mAxisY);
        z = RP_mtx33MultVec(rot, mAxisZ);
        z.normalise();
        if (IsTiny(z)) {
            z = mAxisZ;
        }
        y.normalise();
        if (IsTiny(y)) {
            y = mAxisY;
        }
        x = y.cross(z);
        x.normalise();
        if (IsTiny(x)) {
            x = EGG::Vector3f(-z.z, 0.0f, z.x);
            x.normalise();
            y = z.cross(x);
            x = y.cross(z);
            x.normalise();
        } else {
            y = z.cross(x);
        }
        mAxisX = x;
        mAxisY = y;
        mAxisZ = z;
    }
    ResetMatrices();
}

void RPPhysSphere::ResetMatrices() {
    EGG::Matrix34f& rot = static_cast<EGG::Matrix34f&>(mRotMtx);
    rot.setBase(0, mAxisX);
    rot.setBase(1, mAxisY);
    rot.setBase(2, mAxisZ);
    rot.setBase(3, EGG::Vector3f::zero);
    Transpose33(static_cast<EGG::Matrix34f&>(mRotMtxT), rot);
    static_cast<EGG::Matrix34f&>(mRotMtxT).setBase(3, EGG::Vector3f::zero);
}

extern "C" BOOL RP_PhysSphere_collideB(RPPhysSphere* pSphere, const SphereCollideParam* pParam,
                                       SphereImpulse* pImpulse, SphereImpulse* pOtherImpulse,
                                       ContactResult* pResult) {
    PhysSphereView* pOther = View(pParam->mpOther);
    PhysSphereView* p = View(pSphere);
    EGG::Vector3f diff = static_cast<const EGG::Vector3f&>(pOther->GetPos()) - p->Pos();
    EGG::Vector3f dir = diff;
    f32 dist = dir.normalise();
    if (IsTiny(dir)) {
        dir = EGG::Vector3f(1.0f, 0.0f, 0.0f);
    }
    f32 radiusSum = p->mParam.radius + pOther->mParam.radius;
    if (radiusSum <= dist) {
        pImpulse->mPos = EGG::Vector3f::zero;
        pOtherImpulse->mPos = EGG::Vector3f::zero;
        pImpulse->mVel = EGG::Vector3f::zero;
        pOtherImpulse->mVel = EGG::Vector3f::zero;
        pImpulse->mAngVel = EGG::Vector3f::zero;
        pOtherImpulse->mAngVel = EGG::Vector3f::zero;
        if (pResult != NULL) {
            pResult->mPos = EGG::Vector3f::zero;
            pResult->mSpeed = 0.0f;
        }
        return FALSE;
    }

    f32 overlap = radiusSum - dist;
    pImpulse->mPos = -dir * overlap * 0.5f;
    pOtherImpulse->mPos = dir * overlap * 0.5f;
    EGG::Vector3f arm = dir * p->mParam.radius;
    EGG::Vector3f otherArm = -dir * pOther->mParam.radius;
    if (pResult != NULL) {
        pResult->mPos = p->Pos() + arm + pImpulse->mPos;
    }
    EGG::Vector3f angVel = RP_mtx33MultVec(p->Rot(), p->AngVel());
    EGG::Vector3f otherAngVel = RP_mtx33MultVec(pOther->Rot(), pOther->AngVel());
    EGG::Vector3f normalVel = dir * p->Vel().dot(dir);
    EGG::Vector3f tangentVel = p->Vel() - normalVel;
    EGG::Vector3f otherNormalVel =
        -dir * static_cast<const EGG::Vector3f&>(pOther->GetVec1C()).dot(-dir);
    EGG::Vector3f otherTangentVel =
        static_cast<const EGG::Vector3f&>(pOther->GetVec1C()) - otherNormalVel;
    EGG::Vector3f spinVel = angVel.cross(arm);
    EGG::Vector3f otherSpinVel = otherAngVel.cross(otherArm);
    EGG::Vector3f relNormalVel = otherNormalVel - normalVel;
    EGG::Vector3f normal = dir;
    f32 zero = 0.0f;
    f32 impulse = (1.0f + pParam->mRestitution) * -relNormalVel.length() /
                  (zero + (zero + (1.0f / p->mParam.mass + 1.0f / pOther->mParam.mass)));
    EGG::Vector3f slideDir = (tangentVel + spinVel) - (otherTangentVel + otherSpinVel);
    slideDir.normalise();
    EGG::Vector3f linImpulse = normal * impulse + slideDir * pParam->mFriction * impulse;
    EGG::Vector3f otherLinImpulse = -normal * impulse + slideDir * pParam->mFriction * impulse;
    EGG::Vector3f dVel = linImpulse / p->mParam.mass;
    EGG::Vector3f otherDVel = otherLinImpulse / pOther->mParam.mass;
    EGG::Vector3f dAngVel = arm.cross(linImpulse) / p->mInertia;
    EGG::Vector3f otherDAngVel = otherArm.cross(otherLinImpulse) / pOther->mInertia;
    EGG::Plane3f plane(dir, EGG::Vector3f::zero);
    EGG::Vector3f slideVel = plane.project(dVel);
    f32 slide = slideVel.length();
    if (slide <= FLT_EPSILON) {
        slide = FLT_EPSILON;
    }
    EGG::Vector3f otherSlideVel = plane.project(otherDVel);
    f32 otherSlide = otherSlideVel.length();
    if (otherSlide <= FLT_EPSILON) {
        otherSlide = FLT_EPSILON;
    }
    EGG::Vector3f surfVel = dVel + dAngVel.cross(arm);
    surfVel = plane.project(surfVel);
    EGG::Vector3f otherSurfVel = otherDVel + otherDAngVel.cross(otherArm);
    otherSurfVel = plane.project(otherSurfVel);

    f32 surfSpeed = surfVel.length();
    f32 speed = (tangentVel + spinVel).length();
    if (surfSpeed > speed) {
        if (surfSpeed <= FLT_EPSILON) {
            dVel = -surfVel + dir * dVel.dot(dir);
            dAngVel *= plane.project(dVel).length() / slide;
        } else {
            dVel -= surfVel * (1.0f - speed / surfSpeed);
            dAngVel *= plane.project(dVel).length() / slide;
        }
    }
    f32 otherSurfSpeed = otherSurfVel.length();
    f32 otherSpeed = (otherTangentVel + otherSpinVel).length();
    if (otherSurfSpeed > otherSpeed) {
        if (otherSurfSpeed <= FLT_EPSILON) {
            otherDVel = -otherSurfVel - dir * dVel.dot(-dir);
            otherDAngVel *= plane.project(otherDVel).length() / otherSlide;
        } else {
            otherDVel -= otherSurfVel * (1.0f - otherSpeed / otherSurfSpeed);
            otherDAngVel *= plane.project(otherDVel).length() / otherSlide;
        }
    }
    pImpulse->mVel = dVel;
    pOtherImpulse->mVel = otherDVel;
    pImpulse->mAngVel = RP_mtx33MultVec(p->RotT(), dAngVel);
    pOtherImpulse->mAngVel = RP_mtx33MultVec(pOther->RotT(), otherDAngVel);
    if (pResult != NULL) {
        pResult->mSpeed = p->mParam.mass * pImpulse->mVel.length();
    }
    return TRUE;
}

extern "C" void RP_PhysSphere_collideA(RPPhysSphere* pSphere, s32 num,
                                       const ContactInput* pInputs, SphereImpulse* pImpulse,
                                       ContactResult* pResults, BOOL limitBounce) {
    PhysSphereView* p = View(pSphere);
    p->mpContactA->Reset();
    p->mpContactB->Reset();
    p->mpContactC->Reset();
    f32 zero = 0.0f;
    const ContactInput* pInput = pInputs;
    ContactResult* pResult = pResults;
    for (int i = 0; i < num; pInput++, pResult++, i++) {
        p->mpContactA->Extend(pInput->mPoint * pInput->mDepth);
        if (static_cast<const EGG::Vector3f&>(p->mVec1C).dot(pInput->mPoint) < 0.0f) {
            EGG::Vector3f dVelSum = EGG::Vector3f::zero;
            EGG::Vector3f dAngVelSum = EGG::Vector3f::zero;
            EGG::Plane3f plane(pInput->mPoint, EGG::Vector3f::zero);
            EGG::Vector3f linear = plane.project(static_cast<const EGG::Vector3f&>(p->mVec1C));
            s32 contact = RP_PhysSphere_checkContact(pSphere, &pInput->mPoint, NULL);
            EGG::Vector3f arm = -pInput->mPoint * p->mParam.radius;
            EGG::Vector3f angVel = RP_mtx33MultVec(p->Rot(), p->AngVel());
            EGG::Vector3f spin = angVel.cross(arm);
            f32 spinSpeed = spin.length();
            EGG::Vector3f contactVel = linear + spin;
            f32 contactSpeed = contactVel.length();
            EGG::Vector3f normalVel = pInput->mPoint * static_cast<const EGG::Vector3f&>(p->mVec1C).dot(pInput->mPoint);
            EGG::Vector3f relNormalVel = -normalVel;
            EGG::Vector3f normal = -pInput->mPoint;
            f32 impulse = (1.0f + pInput->mParam.x) * -relNormalVel.length() /
                          (1.0f / p->mParam.mass + zero);
            f32 friction = pInput->mParam.y;
            if (contact == 1) {
                friction = pInput->mParam.z;
            }
            EGG::Vector3f slideDir = EGG::Vector3f::zero;
            if (contact == 2) {
                slideDir = linear;
            } else {
                slideDir = contactVel;
            }
            slideDir.normalise();
            EGG::Vector3f linImpulse = normal * impulse + slideDir * friction * impulse;
            EGG::Vector3f dVel = linImpulse / p->mParam.mass;
            EGG::Vector3f dAngVel = arm.cross(linImpulse) / p->mInertia;
            EGG::Vector3f slideVel = plane.project(dVel);
            f32 slide = slideVel.length();
            if (slide <= FLT_EPSILON) {
                slide = FLT_EPSILON;
            }
            if (contact == 2) {
                dAngVel *= -0.4f;
            }
            EGG::Vector3f dAngVelLocal = RP_mtx33MultVec(p->RotT(), dAngVel);
            if (contact == 2) {
                EGG::Vector3f newVel = plane.project(static_cast<const EGG::Vector3f&>(p->mVec1C) + dVel);
                if (linear.dot(newVel) < 0.0f) {
                    dVel = -linear + pInput->mPoint * dVel.dot(pInput->mPoint);
                    dAngVelLocal *= plane.project(dVel).length() / slide;
                }
            }
            if (contact == 1) {
                EGG::Vector3f surfVel = dVel + dAngVel.cross(arm);
                surfVel = plane.project(surfVel);
                f32 surfSpeed = surfVel.length();
                if (surfSpeed > contactSpeed) {
                    if (surfSpeed <= FLT_EPSILON) {
                        dVel = -linear + pInput->mPoint * dVel.dot(pInput->mPoint);
                        dAngVelLocal *= plane.project(dVel).length() / slide;
                    } else {
                        EGG::Vector3f dVelSlide = plane.project(dVel);
                        dVel -= dVelSlide * (1.0f - contactSpeed / surfSpeed);
                        dAngVelLocal *= plane.project(dVel).length() / slide;
                    }
                }
            }
            EGG::Vector3f normalLocal = RP_mtx33MultVec(p->RotT(), pInput->mPoint);
            EGG::Vector3f normalSpin = normalLocal * p->AngVel().dot(normalLocal);
            if (contact == 0) {
                dVel += pInput->mPoint * static_cast<const EGG::Vector3f&>(p->mVec1C).dot(pInput->mPoint) - static_cast<const EGG::Vector3f&>(p->mVec1C);
                dAngVelLocal = normalSpin - p->AngVel();
            }
            EGG::Vector3f normalSpinDir = normalSpin;
            f32 normalSpinSpeed = normalSpinDir.normalise();
            if (normalSpinSpeed <= pInput->mSpinLimit) {
                dAngVelSum -= normalSpin;
            } else {
                dAngVelSum -= normalSpinDir * pInput->mSpinLimit;
            }
            dVelSum += dVel;
            dAngVelSum += dAngVelLocal;
            p->mpContactB->Extend(dVelSum);
            p->mpContactC->Extend(dAngVelSum);
            if (pResults != NULL) {
                pResult->mHit = true;
                pResult->mPos = p->Pos() + arm;
                pResult->mSpeed = p->mParam.mass * dVelSum.length();
            }
        } else if (pResults != NULL) {
            pResult->mHit = false;
        }
    }
    pImpulse->mPos = p->mpContactA->GetSum();
    pImpulse->mVel = p->mpContactB->GetSum();
    pImpulse->mAngVel = p->mpContactC->GetSum();
    if (limitBounce) {
        f32 vy = p->mVec1C.y + pImpulse->mVel.y;
        if (vy > 0.0f && vy <= p->mParam.unk8) {
            pImpulse->mVel.y = -p->mVec1C.y;
        }
    }
}

extern "C" s32 RP_PhysSphere_checkContact(RPPhysSphere* pSphere, const nw4r::math::VEC3* pNormal,
                                          EGG::Vector3f* pVel) {
    PhysSphereView* p = View(pSphere);
    EGG::Plane3f plane(static_cast<const EGG::Vector3f&>(*pNormal), EGG::Vector3f::zero);
    EGG::Vector3f linear = plane.project(p->Vel());
    EGG::Vector3f arm = -static_cast<const EGG::Vector3f&>(*pNormal) * p->mParam.radius;
    EGG::Vector3f angVel = RP_mtx33MultVec(p->Rot(), p->AngVel());
    EGG::Vector3f spin = angVel.cross(arm);
    EGG::Vector3f contact = linear + spin;
    if (pVel != NULL) {
        *pVel = contact;
    }
    f32 linearSpeed = linear.length();
    f32 spinSpeed = spin.length();
    if (linearSpeed <= p->mParam.restSpeed && spinSpeed <= p->mParam.restSpeed) {
        return 0;
    }
    f32 contactSpeed = contact.length();
    if (linearSpeed > p->mParam.restSpeed && spinSpeed > p->mParam.restSpeed &&
        contactSpeed <= p->mParam.restSpeed) {
        return 2;
    }
    return 1;
}

bool RPPhysSphere::IsAtRest(bool checkMoved) const {
    if (static_cast<const EGG::Vector3f&>(mVec1C).length() > mParam.restSpeed) {
        return false;
    }
    if (static_cast<const EGG::Vector3f&>(mAngVel).length() >
        mParam.restSpeed / mParam.mass) {
        return false;
    }
    if (!checkMoved) {
        return true;
    }
    EGG::Vector3f moved = static_cast<const EGG::Vector3f&>(mPrevPos) -
                          static_cast<const EGG::Vector3f&>(mPos);
    if (moved.length() > mParam.unk10) {
        return false;
    }
    return true;
}


extern "C" void RP_PhysSphere_applyImpulse(RPPhysSphere* pSphere, const nw4r::math::VEC3& rPoint,
                                           const nw4r::math::VEC3& rImpulse, f32 a, f32 b,
                                           f32 c) {
    nw4r::math::VEC3 linear;
    nw4r::math::VEC3 angular;
    pSphere->CalcImpulse(&linear, &angular, rPoint, rImpulse, a, b, c);
    PhysSphereView* p = View(pSphere);
    p->mVec1C.x += linear.x;
    p->mVec1C.y += linear.y;
    p->mVec1C.z += linear.z;
    p->mAngVel.x += angular.x;
    p->mAngVel.y += angular.y;
    p->mAngVel.z += angular.z;
}

void RPPhysSphere::CalcImpulse(nw4r::math::VEC3* pLinear, nw4r::math::VEC3* pAngular,
                               const nw4r::math::VEC3& rPoint, const nw4r::math::VEC3& rImpulse,
                               f32 a, f32 b, f32 c) {
    const EGG::Vector3f& rImp = static_cast<const EGG::Vector3f&>(rImpulse);
    EGG::Vector3f arm =
        static_cast<const EGG::Vector3f&>(mPos) - static_cast<const EGG::Vector3f&>(rPoint);
    arm.normalise();
    EGG::Vector3f normal = arm;
    arm *= mParam.radius;
    EGG::Vector3f dir = rImpulse;
    dir.normalise();
    if (IsTiny(dir)) {
        return;
    }
    EGG::Vector3f normalPart = normal * rImp.dot(normal);
    EGG::Vector3f dirPart = dir * normalPart.dot(dir);
    dirPart += (normalPart - dirPart) * c;
    static_cast<EGG::Vector3f&>(*pLinear) = (dirPart * a) / mParam.mass;
    EGG::Vector3f tangent = rImp - normalPart;
    tangent *= b;
    const EGG::Matrix34f& rotT = static_cast<const EGG::Matrix34f&>(mRotMtxT);
    ResetMatrices();
    EGG::Vector3f tangentLocal = RP_mtx33MultVec(rotT, tangent);
    EGG::Vector3f armLocal =
        RP_mtx33MultVec(static_cast<const EGG::Matrix34f&>(mRotMtxT), -arm);
    static_cast<EGG::Vector3f&>(*pAngular) = armLocal.cross(tangentLocal) / mInertia;
}

EGG::Vector3f RPPhysSphere::CalcSpinVelocity(const EGG::Vector3f& rPoint) const {
    EGG::Matrix34f mtx;
    mtx(0, 0) = mAxisX.x;
    mtx(1, 0) = mAxisX.y;
    mtx(2, 0) = mAxisX.z;
    mtx(0, 1) = mAxisY.x;
    mtx(1, 1) = mAxisY.y;
    mtx(2, 1) = mAxisY.z;
    mtx(0, 2) = mAxisZ.x;
    mtx(1, 2) = mAxisZ.y;
    mtx(2, 2) = mAxisZ.z;
    mtx.setBase(3, EGG::Vector3f::zero);
    EGG::Vector3f result = RP_mtx33MultVec(mtx, static_cast<const EGG::Vector3f&>(mAngVel));
    return result.cross(rPoint);
}
