#ifndef RP_PARTY_FSH_OBJECT_H
#define RP_PARTY_FSH_OBJECT_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpDrawObject.h>
#include <Pack/RPGraphics/IRPGrpModelCallback.h>
#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPKernel/RP_DEBUG_STUB_1.h>
#include <Pack/RPParty/RPPartyCommon/RPActorBase.h>

//! @addtogroup rp_fsh
//! @{

/**
 * @brief One row of a Fishing state-function table: enter and calc member functions
 * @details The tables live in .rodata (one per owner) and are copied to the stack by the owner's
 * constructor, which passes each row to RPStateMachine<T>::SetStateFuncs (or, for the owners
 * that run their state inline, are indexed by the current state in Calculate). Each member
 * function pointer is 12 bytes (CodeWarrior: this-delta, vtable offset -1, function).
 * Size 0x18. Name is a project convention.
 */
template <typename T> struct RPFshStateFuncs {
    typedef void (T::*Func)();

    Func mEnter; // at 0x0
    Func mCalc;  // at 0xC
};

/**
 * @brief State-function table row with an exit function as well (RPFshFish only)
 * @details Size 0x24. Name is a project convention.
 */
template <typename T> struct RPFshStateFuncsEx {
    typedef void (T::*Func)();

    Func mEnter; // at 0x0
    Func mCalc;  // at 0xC
    Func mExit;  // at 0x18
};

/**
 * @brief Fishing actor driven by the Fishing game (player, result-screen fish)
 * @details Abstract: no vtable of its own exists in the binary, but RPFshPlayer (vtable
 * 80358B50) and RPFshResultFish (vtable 80358E98) both append these four virtuals right after
 * RPActorBase's slots, and the managers call them through these slots.
 * Name is a project convention.
 */
class RPFshActor : public RPActorBase {
public:
    virtual void Create() = 0;    // at 0x78
    virtual void Reset() = 0;     // at 0x7C
    virtual void Calculate() = 0; // at 0x80
    virtual void Exit() = 0;      // at 0x84
};

/**
 * @brief Fishing actor that also edits its model's joints (fish, rod)
 * @details Abstract: RPFshFish (vtable 803587E0) and RPFshRod (vtable 80358C10) share this slot
 * layout: RPActorBase's slots up to 0x74, the IRPGrpModelCallback part at 0x78 (callback object
 * at +0x60, destructor thunk -0x60 at 0x80), then these four virtuals from 0x94.
 * Name is a project convention.
 */
class RPFshModelActor : public RPActorBase, public IRPGrpModelCallback {
public:
    virtual void Create() = 0;    // at 0x94
    virtual void Reset() = 0;     // at 0x98
    virtual void Calculate() = 0; // at 0x9C
    virtual void Exit() = 0;      // at 0xA0
};

/**
 * @brief Fishing layout screen registered as a draw object (result, points, score, start)
 * @details Abstract: RPFshResultLayout, RPFshPointPopup, RPFshScoreLayout and
 * RPFshStartLayout share this slot layout. vtable layout (CodeWarrior): RP_DEBUG_STUB_1 part
 * (dtor) at 0x8, IRPSysHostIOSocket part at 0xC (thunk -4), IRPGrpDrawObject part at 0x20
 * (thunk -8; the draw object is at +0x8 and is appended to the renderer by the constructor),
 * then these four virtuals from 0x34. Every derived class overrides UserDraw, which appends its
 * primary slot at 0x44.
 * Name is a project convention.
 */
class RPFshLayout : public RP_DEBUG_STUB_1,
                    public IRPSysHostIOSocket,
                    public IRPGrpDrawObject {
public:
    virtual void Create() = 0;    // at 0x34
    virtual void Reset() = 0;     // at 0x38
    virtual void Calculate() = 0; // at 0x3C
    virtual void Exit() = 0;      // at 0x40
};

/**
 * @brief Small Fishing layout that is itself the draw object (messages, yank prompt, arrow)
 * @details Abstract: RPFshMsg00, RPFshMsg01, RPFshYankPrompt and RPFshAimArrow derive
 * directly from IRPGrpDrawObject (mpNext at 0x0, vtable at 0x4) and add these three virtuals
 * after DebugDraw; the prompt and the arrow add Exit at 0x20.
 * Name is a project convention.
 */
class RPFshDrawObj : public IRPGrpDrawObject {
public:
    virtual void Create() = 0;    // at 0x14
    virtual void Reset() = 0;     // at 0x18
    virtual void Calculate() = 0; // at 0x1C
};

//! @}

#endif
