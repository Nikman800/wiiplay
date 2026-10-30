#ifndef RP_PARTY_COMMON_RUMBLE_PATTERN_H
#define RP_PARTY_COMMON_RUMBLE_PATTERN_H
#include <Pack/types_pack.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Rumble on/off bit pattern of up to 1024 frames
 * @details No vtable. Size 0x8C, verified (array stride of the 16 slots at 804391F0, 4 per
 * controller). Build(frames, strength, ...) spreads "on" bits so that their density follows
 * the strength.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPRumblePattern {
public:
    RPRumblePattern();
    RPRumblePattern(s32 length, f32 strength, s32 attack); // [M] calls Build

    void Build(s32 length, f32 strength, s32 attack); // [M] parameter meaning

    /**
     * @brief Copies the pattern into the first free slot of the controller [H]
     */
    static void Add(s32 channel, const RPRumblePattern& rPattern);
    static void UpdateAll();
    static void ClearAll();

private:
    s32 mLength;       // at 0x0 frames
    s32 mRemaining;    // at 0x4 frames left; > 0 means the slot is in use
    u32 mBits[0x20];   // at 0x8
    RPRumblePattern* mpSelf; // at 0x88 [L] set to this by both constructors

    static RPRumblePattern sSlots[4][4]; // 804391F0
};

//! @}

#endif
