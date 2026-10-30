#ifndef RP_PARTY_COMMON_CURSOR_TRAIL_H
#define RP_PARTY_COMMON_CURSOR_TRAIL_H
#include <Pack/types_pack.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Per-sample pointer data of one controller (position, direction, scale, flags)
 * @details vtable 80357338. Size 0x28, verified (Duc_Shot, Tnk_Cursor, Bom_Cursor). Update
 * fills the arrays from the controller's buffered samples; Reset sets flags 0xFF, positions to
 * (0, 0), directions to (1, 0) and scales to 1.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPCursorTrail {
public:
    RPCursorTrail(s32 channel, s32 sampleNum);

    void Update();
    void Reset();

    // Wii Play: RPDucShot::Reset/UpdateAim
    void SetChannel(s32 channel) {
        mChannel = channel;
    }
    s32 GetLatest() const {
        return mLatest;
    }
    const nw4r::math::VEC2& GetPos(s32 idx) const {
        return mpPos[idx];
    }
    // Wii Play: RPBomCursor::UpdateFromPointer
    s32 GetLatestDir() const {
        return mLatestDir;
    }
    const nw4r::math::VEC2& GetDir(s32 idx) const {
        return mpDir[idx];
    }

private:
    s32 mChannel;               // at 0x0
    s32 mSampleNum;             // at 0x4
    s32 mLatest;                // at 0x8 [M] newest sample, -1 none (written by Update)
    s32 mLatestDir;             // at 0xC [M] newest direction sample, -1 none
    s32 mLatestScale;           // at 0x10 [M] newest sample with dpd_valid_fg >= 2, -1 none
    s8* mpFlags;                // at 0x14 dpd_valid_fg per sample
    nw4r::math::VEC2* mpPos;    // at 0x18
    nw4r::math::VEC2* mpDir;    // at 0x1C
    f32* mpScale;               // at 0x20

public:
    virtual ~RPCursorTrail(); // at 0x8
};

//! @}

#endif
