#ifndef RP_PARTY_COMMON_JOINT_MTX_BUFFER_H
#define RP_PARTY_COMMON_JOINT_MTX_BUFFER_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpModelCallback.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Model callback that records every joint's local and world matrix
 * @details vtable 80357348. Size 0x14, verified (Party_DollBase, Fsh_Fish, Fsh_Rod,
 * Bil_Table). The constructor registers itself as the model's callback, calculates the model
 * once with an identity matrix to fill mpLocalMtx, then unregisters.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPJointMtxBuffer : public IRPGrpModelCallback {
public:
    explicit RPJointMtxBuffer(RPGrpModel* pModel);
    virtual ~RPJointMtxBuffer(); // at 0x8

    /**
     * @brief Stores the joint's local matrix and moves the callback to the next joint [H]
     */
    virtual void EditJointLocalMtx(RPGrpModel* pModel, u16 idx, nw4r::math::MTX34* pMtx,
                                   nw4r::math::VEC3* pScale); // at 0xC
    /**
     * @brief Copies every joint's world matrix into mpWorldMtx [H]
     */
    virtual void EditWorldMtx(RPGrpModel* pModel); // at 0x14

    // Wii Play: out of line (ledger Fsh_getNodeMtx, 801BAFD0)
    const nw4r::math::MTX34& GetLocalMtx(u16 idx) const;

    // Wii Play: out of line (ledger RP_JointMtxBuffer_getWorldMtx, 801BAFC0)
    const nw4r::math::MTX34& GetWorldMtx(u16 idx) const;

private:
    s32 mJointNum;                  // at 0x4
    u16 mCurJoint;                  // at 0x8
    nw4r::math::MTX34* mpLocalMtx;  // at 0xC
    nw4r::math::MTX34* mpWorldMtx;  // at 0x10
};

//! @}

#endif
