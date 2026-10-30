#ifndef RP_PARTY_COMMON_PLAY_ADVICE_LYT_H
#define RP_PARTY_COMMON_PLAY_ADVICE_LYT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPPlayAdviceLyt;
class RPSysLayout;
class RPSysLytAnmObj;

/**
 * @brief Play advice popup with a blinking A button (playAdvice.arc, playAdvice_00.brlyt)
 * @details vtable 80358020. Size 0x28, verified (Menu_PlayerConfirm_ctor). Seven states:
 * hidden, in, A-button in, blink, decide, out, done [M].
 * @note Wii Play-only framework class (ledger Menu_PlayAdviceLyt); the name is a project
 * convention.
 */
class RPPlayAdviceLyt : public RPLytObjBase {
public:
    RPPlayAdviceLyt();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    void Show(s32 advice, bool visible);
    bool PressA(s32 playerFlag);

private:

    void InEnter();
    void InCalc();
    void AButtonInEnter();
    void AButtonInCalc();
    void BlinkEnter();
    void BlinkCalc();
    void DecideEnter();
    void DecideCalc();
    void OutEnter();
    void OutCalc();

public:
    RPStateMachine<RPPlayAdviceLyt>* mpStateMachine; // at 0x8
private:
    bool mVisible;                                   // at 0xC
    RPSysLayout* mpLayout;                           // at 0x10
    RPSysLytAnmObj* mpInAnmObj;                      // at 0x14
    RPSysLytAnmObj* mpAButtonInAnmObj;               // at 0x18
    RPSysLytAnmObj* mpBlinkAnmObj;                   // at 0x1C
    RPSysLytAnmObj* mpOutAnmObj;                     // at 0x20
    s32 mDecideTimer;                                // at 0x24 [M]
};

//! @}

#endif
