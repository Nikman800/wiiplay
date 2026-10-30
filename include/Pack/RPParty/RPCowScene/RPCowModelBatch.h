#ifndef RP_PARTY_COW_MODEL_BATCH_H
#define RP_PARTY_COW_MODEL_BATCH_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpModelCallback.h>
#include <Pack/RPParty/RPPartyCommon/RPActor.h>

#include <egg/math.h>
#include <nw4r/math.h>

//! @addtogroup rp_cow
//! @{

// Forward declarations
class RPGrpModel;

/**
 * @brief Draws one model many times: each frame the course objects add their matrices and
 * the model callback hands them out as joint matrices (instancing for the far levels of
 * detail)
 * @details vtable 8035D458 (IRPGrpModelCallback part at 0x24, sub-vtable at 0x60). Size 0x38,
 * verified (RPCowGame constructor). The generic batch draws the round shadow
 * (cow_maru_shadow_128, up to 128 instances).
 * vtable layout (CodeWarrior): RPActor part to 0x5C, IRPGrpModelCallback part from 0x60
 * (EditJointLocalMtx thunk at 0x6C), EditJointLocalMtx own slot at 0x7C.
 * @note Name is a project convention (ledger Cow_ModelBatch).
 */
class RPCowModelBatch : public RPActor, public IRPGrpModelCallback {
public:
    RPCowModelBatch();
    virtual ~RPCowModelBatch(); // at 0x38

    /**
     * @brief Ledger init: model 0x1B, 128 matrices [H]
     */
    virtual void LoadResource(); // at 0x44
    virtual void OnStart();      // at 0x48 ledger reset: clears mNum
    virtual void OnStop();       // at 0x4C ledger exit (empty)
    virtual void OnActivate();   // at 0x50 ledger calc
    virtual void Calculate();    // at 0x54 ledger entryDraw

    /**
     * @brief IRPGrpModelCallback override (ledger getInstanceMtx): copies instance idx - 1 to
     * the joint matrix, or scales the joint far away when there is none [M]
     */
    virtual void EditJointLocalMtx(RPGrpModel* pModel, u16 idx,
                                   nw4r::math::MTX34* pMtx,
                                   nw4r::math::VEC3* pScale); // at 0x7C

    /**
     * @brief Appends a matrix; false when full [H]
     */
    bool AddInstance(const EGG::Matrix34f& rMtx);

protected:
    RPGrpModel* mpModel;        // at 0x28
    EGG::Matrix34f* mpMtxs;     // at 0x2C mCapacity entries
    s32 mNum;                   // at 0x30
    s32 mCapacity;              // at 0x34
};

/**
 * @brief Model batch of the far trees (model 0x16, 64 instances)
 * @details vtable 8035D4D8. Size 0x38 [H] (allocated with 0x38 and the RPCowModelBatch
 * constructor, vtable pointers replaced inline).
 * @note Name is a project convention (ledger Cow_TreeBatch).
 */
class RPCowTreeBatch : public RPCowModelBatch {
public:
    RPCowTreeBatch();

    // Declared before the destructor: LoadResource is the key function (vtable and thunks in
    // its unit), the destructor sits in a unit of its own.
    virtual void LoadResource(); // at 0x44 ledger init
    virtual ~RPCowTreeBatch(); // at 0x38
};

/**
 * @brief Model batch of the far hurdles
 * @details vtable 8035D558. Size 0x38 [H] (same construction as RPCowTreeBatch).
 * @note Name is a project convention (ledger Cow_HurdleBatch).
 */
class RPCowHurdleBatch : public RPCowModelBatch {
public:
    RPCowHurdleBatch();

    // Declared before the destructor: LoadResource is the key function (vtable and thunks in
    // its unit), the destructor sits in a unit of its own.
    virtual void LoadResource(); // at 0x44 ledger init
    virtual ~RPCowHurdleBatch(); // at 0x38
};

/**
 * @brief Model batch of the far scarecrows
 * @details vtable 8035D5D8. Size 0x38 [H] (same construction as RPCowTreeBatch).
 * @note Name is a project convention (ledger Cow_KakasiBatch).
 */
class RPCowKakasiBatch : public RPCowModelBatch {
public:
    RPCowKakasiBatch();

    // Declared before the destructor: LoadResource is the key function (vtable and thunks in
    // its unit), the destructor sits in a unit of its own.
    virtual void LoadResource(); // at 0x44 ledger init
    virtual ~RPCowKakasiBatch(); // at 0x38
};

//! @}

#endif
