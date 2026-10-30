#ifndef RP_PARTY_BIL_OBJECT_H
#define RP_PARTY_BIL_OBJECT_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpDrawObject.h>
#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPKernel/RP_DEBUG_STUB_1.h>
#include <Pack/RPParty/RPBilScene/RPBilSound.h>
#include <Pack/RPParty/RPPartyCommon/RPActorBase.h>

//! @addtogroup rp_bil
//! @{

/**
 * @brief One row of a Billiards state-function table: enter and calc member functions
 * @details The tables live in .rodata (one per owner) and are copied to the stack by the
 * owner's constructor, which passes each row to RPStateMachine<T>::SetStateFuncs (RPBilBreakLottery
 * indexes its table inline). Rows of the unused state 0 (and of a trailing done state) are
 * NULL. Each member function pointer is 12 bytes (CodeWarrior: this-delta, vtable offset -1,
 * function). Size 0x18. Name is a project convention.
 */
template <typename T> struct RPBilStateFuncs {
    typedef void (T::*Func)();

    Func mEnter; // at 0x0
    Func mCalc;  // at 0xC
};

/**
 * @brief Billiards 3D actor (table, players, balls, ball manager, markers, ball icons)
 * @details Abstract: no vtable of its own exists in the binary, but every Billiards actor
 * vtable (RPBilTable 8035DCA8, RPBilPlayer 8035DDC8, RPBilBall 8035DE68, RPBilBallMgr 8035DEF8,
 * RPBilAimMarker 8035E1E0, RPBilCueGhost 8035E2B8, RPBilNextBallMarker 8035E3C0, RPBilBallIcon
 * 8035E538) appends these four virtuals right after RPActorBase's slots, and RPBilGameMgr calls
 * them through these slots. All but RPBilBallIcon then append their UserDraw override at 0x88.
 * Name is a project convention.
 */
class RPBilActor : public RPActorBase {
public:
    virtual void Load() = 0;      // at 0x78
    virtual void Reset() = 0;     // at 0x7C
    virtual void Calculate() = 0; // at 0x80
    virtual void Exit() = 0;      // at 0x84
};

/**
 * @brief Billiards 2D screen object registered as a draw object (break lottery, result)
 * @details Abstract: RPBilBreakLottery (vtable 8035E450) and RPBilResult (vtable 8035E270)
 * share this slot layout: RP_DEBUG_STUB_1 part (dtor) at 0x8, IRPSysHostIOSocket part at 0xC
 * (thunk -4), IRPGrpDrawObject part at 0x20 (thunk -8; the draw object is at +0x8 and is
 * appended to the renderer by the constructor), then these four virtuals from 0x34 and the
 * UserDraw override at 0x44. (RPBilPocketCamWindow has the same bases but Reset before Load.)
 * Name is a project convention.
 */
class RPBilScreen : public RP_DEBUG_STUB_1,
                    public IRPSysHostIOSocket,
                    public IRPGrpDrawObject {
public:
    virtual void Load() = 0;      // at 0x34
    virtual void Reset() = 0;     // at 0x38
    virtual void Calculate() = 0; // at 0x3C
    virtual void Exit() = 0;      // at 0x40
};

/**
 * @brief Small Billiards controller that is itself the draw object (score popup, hint)
 * @details Abstract: RPBilScorePopup (vtable 8035E5E0) and RPBilHint (vtable 8035E660) derive
 * directly from IRPGrpDrawObject (mpNext at 0x0, vtable at 0x4) and add these three virtuals
 * after DebugDraw; the score popup adds Exit at 0x20. Each owns an RPLytObjBase layout whose
 * Load it calls with the scene-local archive.
 * Name is a project convention.
 */
class RPBilDrawObj : public IRPGrpDrawObject {
public:
    virtual void Load() = 0;      // at 0x14
    virtual void Reset() = 0;     // at 0x18
    virtual void Calculate() = 0; // at 0x1C
};

//! @}

#endif
