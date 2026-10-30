#ifndef RP_PARTY_COMMON_FACE_CTRL_H
#define RP_PARTY_COMMON_FACE_CTRL_H
#include <Pack/types_pack.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPSysKokeshi;

/**
 * @brief Random blinking for a Mii face (RFL expressions)
 * @details No vtable, empty out-of-line constructor (RPLeadCursor.cpp). Size 0x18, verified (Party_Common_ctor,
 * Party_Doll_ctor, Party_CrowdMii_ctor). Reset reads the Mii's current expression and picks a
 * random blink interval and burst length for it (per-expression tables built on the stack);
 * Calculate switches to the blink expression (5) for 4 frames, then back to mExpression.
 * Random numbers come from the shared RP_lcgSeed.
 * @note Wii Play-only framework class (ledger Party_FaceCtrl); the name is a project
 * convention.
 */
class RPFaceCtrl {
public:
    RPFaceCtrl();

    void Reset(RPSysKokeshi* pKokeshi);
    void Calculate(RPSysKokeshi* pKokeshi);

private:
    // Picks mInterval and mBlinkNum for an expression (inlined into Reset and Calculate) [H]
    void InitBlink(s32 expression);

private:
    s32 mTimer;      // at 0x0 frames since the last blink
    s32 mInterval;   // at 0x4 frames between blinks, -1 = never blink
    s32 mBlinkTimer; // at 0x8 frames left with the eyes closed
    s32 mBlinkCount; // at 0xC blinks done in the current burst
    s32 mBlinkNum;   // at 0x10 blinks per burst
    s32 mExpression; // at 0x14 RFLExpression restored after a blink
};

//! @}

#endif
