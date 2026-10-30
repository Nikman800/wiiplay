#ifndef RP_GRAPHICS_MODEL_RECORD_H
#define RP_GRAPHICS_MODEL_RECORD_H
#include <Pack/types_pack.h>

#include <nw4r/math.h>

//! @addtogroup rp_graphics
//! @{

// Forward declarations
class RPGrpModel;

/**
 * @brief Model transformation recorder
 */
class RPGrpModelRecord {
private:
    // Wii Play: the members come first and the vtable sits at 0x44 (the vendored header put
    // the vtable at 0x0). Layout read from Wii Play's Calc (ledger Gfx_JointMtx_apply), the only
    // RPGrpModelRecord function left in Wii Play; size 0x48 kept from the vendored header [M].

    u16 mFlags;  // at 0x0 (bit 0: record, bit 1: play back) [M]
    f32 mFrame;  // at 0x4 (playback position, interpolated between frames) [M]
    //! Recorded frames: u16 count at +0x0, 0x34-byte frames at +0x4, vtable at +0x8 [M]
    void* mpFrames;              // at 0x8
    nw4r::math::MTX34 mRootMtx;  // at 0xC
    nw4r::math::MTX34* mpJointMtxArray; // at 0x3C (one per joint)
    u16 mRecordFrame;            // at 0x40

public:
    /**
     * @brief Constructor
     *
     * @param flags Bound type flags
     * @param pModel Owner model
     */
    RPGrpModelRecord(u16 frames, RPGrpModel* pModel);

    /**
     * @brief Destructor
     */
    virtual ~RPGrpModelRecord() {} // at 0x8

    /**
     * @brief Clears the recorded data
     */
    void Clear();

    /**
     * @brief Records the current frame, or applies the played-back frame to the model
     *
     * @param pModel Model to record
     */
    void Calc(RPGrpModel* pModel);
};

//! @}

#endif
