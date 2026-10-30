#ifndef RP_PARTY_COMMON_COLLISION_MODEL_H
#define RP_PARTY_COMMON_COLLISION_MODEL_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPColliderList.h>
#include <Pack/RPParty/RPPartyCommon/RPKcl.h>

#include <egg/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief RPCollider backed by KCL data, placed by position, rotation and a matrix (Cow field)
 * @details vtable 80357580 (pointer at 0xA4). Size 0xA8, verified (Cow_Game_load).
 * vtable layout (CodeWarrior): the first base RPKcl has no vtable, so RPCollisionModel gets its
 * own pointer after its members; the full vtable is [header] [RPCollider part at 0x8 (pointer
 * at 0x50), thunks -0x40, Draw not overridden] then the own copies GetPos 0x24, GetRot 0x28,
 * GetRadius 0x2C, CheckHit 0x30. No virtual destructor.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPCollisionModel : public RPKcl, public RPCollider {
    // Members are declared before the first own virtual so that the vtable pointer comes after
    // them (at 0xA4), as in the constructor.
protected:
    EGG::Vector3f mPos;   // at 0x54
    EGG::Vector3f mRot;   // at 0x60
    EGG::Matrix34f mMtx;  // at 0x6C
    f32 mScaleA;          // at 0x9C [L] initially 1.0
    f32 mScaleB;          // at 0xA0 [L] initially 1.0

public:
    RPCollisionModel();

    virtual EGG::Vector3f& GetPos();             // at 0x24
    virtual EGG::Vector3f& GetRot();             // at 0x28
    virtual f32 GetRadius() const;               // at 0x2C
    /**
     * @brief Sphere sweep against the KCL (RP_CollisionModel_sweepSphere) [M]
     */
    virtual bool CheckHit(f32 radius, const EGG::Vector3f& rPos,
                          const EGG::Vector3f& rPrevPos, EGG::Vector3f* pPosOfs,
                          EGG::Vector3f* pRotOfs, u16* pFlags); // at 0x30

    /**
     * @brief Sets the KCL resource, zeroes mPos/mRot, identity mMtx, scales 1 [H]
     */
    void Reset(void* pKclFile);
    bool CheckSphere(f32 radius, const EGG::Vector3f& rPos, EGG::Vector3f* pPosOfs,
                     EGG::Vector3f* pRotOfs, u16* pFlags);

};

//! @}

#endif
