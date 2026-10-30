#ifndef RP_PARTY_COMMON_BODY_JOINTS_H
#define RP_PARTY_COMMON_BODY_JOINTS_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/IRPSysKokeshiBodyManager.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Wii Play's Mii body definition: 12 body types, body/clothes file tables and the head
 * and hand joint matrices (joints head_set, hand_l / wrist_l, hand_r / wrist_r)
 * @details vtable 80357918; implements IRPSysKokeshiBodyManager (the ten slots match its order).
 * No members: size 0x4 [M] (the constructor RP_Cls80357918_ctor only stores the vtable; no
 * allocation was found). Body file names at 803578C8 (per body type and sex), clothes names at
 * 80357748, clothes file name at 8043B380. Body types 0 and 1 use the hand joints, others the
 * wrist joints.
 * @note Wii Play-only framework class (ledger Mii_BodyJoints); the name is a project convention.
 */
class RPBodyJoints : public IRPSysKokeshiBodyManager {
public:
    RPBodyJoints() {}

    virtual u16 GetFriendClothesStartIndex() { // at 0x8
        return 12;
    }
    virtual u16 GetFriendClothesIndexRange(); // at 0xC returns 12
    virtual const char* GetBodyFileName(u16 bodyType, RFLSex sex); // at 0x10
    virtual u16 GetBodyModelIndex(u16 bodyType, RFLSex sex, u32 height,
                                  u32 build) { // at 0x14
        return 0;
    }
    virtual const char* GetClothesFileName(); // at 0x18
    virtual void GetClothesName(u16 clothesType, RFLSex sex, const char** ppNameA,
                                const char** ppNameB); // at 0x1C
    virtual void GetHeadJointMtx(u16 bodyType, RPGrpModel* pBodyModel,
                                 EGG::Matrix34f* pMtx); // at 0x20
    virtual void GetLeftHandJointMtx(u16 bodyType, RPGrpModel* pBodyModel,
                                     EGG::Matrix34f* pMtx); // at 0x24
    virtual void GetRightHandJointMtx(u16 bodyType, RPGrpModel* pBodyModel,
                                      EGG::Matrix34f* pMtx); // at 0x28
    virtual ~RPBodyJoints(); // at 0x2C
};

//! @}

#endif
