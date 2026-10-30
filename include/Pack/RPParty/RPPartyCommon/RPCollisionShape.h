#ifndef RP_PARTY_COMMON_COLLISION_SHAPE_H
#define RP_PARTY_COMMON_COLLISION_SHAPE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPBBox.h>
#include <Pack/RPParty/RPPartyCommon/RPColliderList.h>

#include <egg/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Sphere (type 1) or box (type 2) RPCollider with accumulated push-out offsets
 * @details vtable 803575B8 (shares RPCollider's pointer at 0x10). Size 0xD8, verified (every
 * Cow object's init). No virtual destructor.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPCollisionShape : public RPCollider {
public:
    RPCollisionShape();

    virtual EGG::Vector3f& GetPos();             // at 0x8
    virtual EGG::Vector3f& GetRot();             // at 0xC
    virtual f32 GetRadius() const;               // at 0x10
    /**
     * @brief Dispatches on mType to the sphere/box/point tests (RP_CollisionShape_checkSphere) [M]
     */
    virtual bool CheckHit(f32 radius, const EGG::Vector3f& rPos,
                          const EGG::Vector3f& rPrevPos, EGG::Vector3f* pPosOfs,
                          EGG::Vector3f* pRotOfs, u16* pFlags); // at 0x14
    virtual void Draw();                                    // at 0x18 debug shapes

    /**
     * @brief Takes position and rotation from the identity matrix, zeroes the rest, type 1 [H]
     */
    void Reset();
    void AccumPosOffset(const EGG::Vector3f& rOfs) {
        mPosOfs.Extend(rOfs);
    }
    void AccumRotOffset(const EGG::Vector3f& rOfs) {
        mRotOfs.Extend(rOfs);
    }
    /**
     * @brief Adds the summed offsets to mPos/mRot, copies mPos to mPrevPos, resets both
     * accumulators [H]
     */
    void ApplyOffsets();
    // Non-virtual reads of the positions (Cow_Transform_getDelta)
    const EGG::Vector3f& GetCurrentPos() const {
        return mPos;
    }
    const EGG::Vector3f& GetPrevPos() const {
        return mPrevPos;
    }
    void GetMtx(EGG::Matrix34f* pMtx) const; // [M]
    /**
     * @brief Sets the radius and its square [M]
     */
    void SetRadius(f32 radius) {
        mRadius = radius;
        unk74 = radius * radius;
    }
    EGG::Vector3f& GetVec40() {
        return mVec40;
    }
    EGG::Vector3f& GetVec4C() {
        return mVec4C;
    }

protected:
    u32 unk14;               // at 0x14 [L] reset to 0
    s32 mType;               // at 0x18 1 sphere, 2 box
    EGG::Vector3f mPos;      // at 0x1C
    EGG::Vector3f mPrevPos;  // at 0x28
    EGG::Vector3f mRot;      // at 0x34
    EGG::Vector3f mVec40;    // at 0x40 [L] RPY of the identity matrix on reset
    EGG::Vector3f mVec4C;    // at 0x4C [L] reset to zero
    EGG::Vector3f mUp;       // at 0x58 [L] matrix column 1 in modes 1-3
    EGG::Vector3f mForward;  // at 0x64 [L] matrix column 2 in modes 1-3
    f32 mRadius;             // at 0x70
    f32 unk74;               // at 0x74 [L]
    RPBBox mPosOfs;          // at 0x78
    RPBBox mRotOfs;          // at 0xA8
};

//! @}

#endif
