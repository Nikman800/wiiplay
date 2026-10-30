#ifndef RP_PARTY_COMMON_SIMPLE_MODEL_H
#define RP_PARTY_COMMON_SIMPLE_MODEL_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel/RP_DEBUG_STUB_1.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Object owning one RPGrpModel, placed by position, scale and rotation
 * @note Wii Play-only framework class (not in Wii Sports); the name is a project convention.
 * vtable 80357248. Size 0x44, verified against Wii Play's allocations.
 */
class RPSimpleModel : public RP_DEBUG_STUB_1 {
public:
    RPSimpleModel();
    virtual ~RPSimpleModel(); // at 0x8

    /**
     * @brief Registers a resource file from the scene's common archive; returns its handle [H]
     * @details Derived classes call it non-virtually (RPSimpleModel::LoadResource).
     */
    virtual RPGrpHandle LoadResource(const char* pName); // at 0xC

    /**
     * @brief Looks model pName up in resource file resFile and constructs it into mpModel
     * @details Calls RPGrpModel::Construct(handle, viewNo, typeOption, bufferOption) [H]
     */
    virtual void LoadModel(const char* pName, RPGrpHandle resFile, u8 viewNo, u32 typeOption,
                           u32 bufferOption); // at 0x10

    /**
     * @brief Per-frame update; empty here, overridden by every game object
     */
    // Wii Play: defined out of line in RPSimpleModel.cpp
    virtual void Calculate(); // at 0x14
    virtual void VF_0x18();   // at 0x18

    /**
     * @brief Builds the model matrix from mPos/mRot, copies mScale, updates the model [H]
     */
    void CalcModelMtx();

    // Wii Play: stores the RP_lcgSeed random seed (RP_setRandomSeed) [L name]
    static void SetRandomSeed(u32 seed);

    /**
     * @brief Unit statics set up by the RPSimpleModel unit's static initializer: zero vector and
     * the three axes (80439170, RP_vecZeroB...) [M owner and names]
     */
    static nw4r::math::VEC3 sVecZero; // 80439170
    static nw4r::math::VEC3 sVecEx;   // 8043917C
    static nw4r::math::VEC3 sVecEy;   // 80439188
    static nw4r::math::VEC3 sVecEz;   // 80439194
    static nw4r::math::VEC3 sVecOne;  // 804391A0 (RP_vecOneB) Wii Play: (1, 1, 1)
    /**
     * @brief Identity matrix as three row vectors, copy-constructed from sVecEx/Ey/Ez
     * @details Wii Play: the static initializer copies the axes word by word with no temporary
     * (a VEC3 array, not an MTX33); readers copy it as an MTX33 or row by row.
     */
    static nw4r::math::VEC3 sMtxIdentity[3]; // 804391AC (RP_mtx33IdentityB) Wii Play

    // Wii Play: RPWlyField scales the earth and reads its model
    const nw4r::math::VEC3& GetScale() const {
        return mScale;
    }
    void SetScale(const nw4r::math::VEC3& rScale) {
        mScale.x = rScale.x;
        mScale.y = rScale.y;
        mScale.z = rScale.z;
    }
    RPGrpModel* GetModel() const {
        return mpModel;
    }

protected:
    nw4r::math::VEC3 mPos;   // at 0x4
    nw4r::math::VEC3 mScale; // at 0x10
    /**
     * @brief Rotation basis (rows x, y, z), copied as one aggregate
     */
    nw4r::math::MTX33 mRot;  // at 0x1C
    RPGrpModel* mpModel;     // at 0x40
};

//! @}

#endif
