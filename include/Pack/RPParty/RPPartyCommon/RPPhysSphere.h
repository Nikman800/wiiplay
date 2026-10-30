#ifndef RP_PARTY_COMMON_PHYS_SPHERE_H
#define RP_PARTY_COMMON_PHYS_SPHERE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPActorBase.h>
#include <Pack/RPParty/RPPartyCommon/RPVec3Range.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Construction parameters of RPPhysSphere, copied to 0x60 [M]
 */
struct RPPhysSphereParam {
    u32 unk0;      // at 0x0 [L]
    f32 mass;      // at 0x4
    f32 unk8;      // at 0x8 [L]
    f32 radius;    // at 0xC
    f32 unk10;     // at 0x10 [L] position tolerance of IsAtRest
    f32 restSpeed; // at 0x14 IsAtRest speed threshold (angular: restSpeed / mass)
    f32 unk18;     // at 0x18 [L]
    f32 unk1C;     // at 0x1C [L]
};

/**
 * @brief Rigid sphere with linear and angular velocity (Billiards balls)
 * @details vtable 80357368; adds no virtuals to RPActorBase. Size 0x108, verified
 * (Bil_Ball). Inertia = 0.4 * mass * radius^2. mVec1C is the linear velocity.
 * @note Wii Play-only framework class; the name is a project convention. The ledger's
 * RP_RigidBody (isAtRest, resetMatrices) is this class.
 */
class RPPhysSphere : public RPActorBase {
public:
    explicit RPPhysSphere(const RPPhysSphereParam& rParam);
    virtual ~RPPhysSphere(); // at 0x8

    void Update();
    void ApplyImpulse(const nw4r::math::VEC3& rImpulse, const nw4r::math::VEC3& rPoint);
    /**
     * @brief Linear (*pLinear) and angular (*pAngular, body space) velocity change from an
     * impulse rImpulse at the surface point facing rPoint; a = linear factor, b = angular
     * factor, c = blend of the impulse-direction part toward the normal part [M]
     */
    void CalcImpulse(nw4r::math::VEC3* pLinear, nw4r::math::VEC3* pAngular,
                     const nw4r::math::VEC3& rPoint, const nw4r::math::VEC3& rImpulse, f32 a,
                     f32 b, f32 c);
    /**
     * @brief Surface velocity from the spin at the contact offset rPoint (ledger
     * RP_PhysSphere_calcSpinVelocity) [M]
     */
    EGG::Vector3f CalcSpinVelocity(const EGG::Vector3f& rPoint) const;
    void CalcWorldOffset();
    bool CheckContact(); // [L] signature
    void CollideA();     // [L] signature
    void CollideB();     // [L] signature
    /**
     * @brief Linear speed <= restSpeed, angular speed <= restSpeed / mass and, when
     * checkMoved, |mPrevPos - mPos| <= unk10 [H]
     */
    bool IsAtRest(bool checkMoved) const;
    void ResetMatrices();

    f32 GetRadius() const {
        return mParam.radius;
    }
    RPVec3Range& GetPosRange() {
        return *mpContactA;
    }
    RPVec3Range& GetVelRange() {
        return *mpContactB;
    }
    RPVec3Range& GetAngVelRange() {
        return *mpContactC;
    }

public:
    RPPhysSphereParam mParam;        // at 0x60
    nw4r::math::MTX34 mRotMtx;       // at 0x80 identity
    nw4r::math::MTX34 mRotMtxT;      // at 0xB0 [M] transposed copy of mRotMtx
    nw4r::math::VEC3 mAngVel;        // at 0xE0
    f32 mInertia;                    // at 0xEC
    nw4r::math::VEC3 mPrevPos;       // at 0xF0 [M]
    RPVec3Range* mpContactA;         // at 0xFC position push-outs of a step
    RPVec3Range* mpContactB;         // at 0x100 velocity changes of a step
    RPVec3Range* mpContactC;         // at 0x104 angular velocity changes of a step
};

//! @}

#endif
