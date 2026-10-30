#ifndef RP_PARTY_COMMON_RESULT_MENU_H
#define RP_PARTY_COMMON_RESULT_MENU_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <nw4r/lyt.h>
#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPCPadCursor;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytBounding;

/**
 * @brief Two-button "play again / quit" menu after a minigame (ptVsResult.arc,
 * ptGOMenu_00.brlyt)
 * @details vtable 80358340. Size 0x58, verified (Party_ResultMenu_ctor). Runs its own inline
 * state machine (states 0 hidden, 1 in, 2 idle, 3 push, 4 out, 5 done; enter/calc table
 * 8031C8D8 called through pointers to member functions). Button texts are
 * party_message.bmg 0x1F55.
 * @note Wii Play-only framework class (ledger Party_ChoiceLyt); the name is a project
 * convention.
 */
class RPChoiceLyt : public RPLytObjBase {
public:
    static const int BUTTON_NUM = 2;

public:
    RPChoiceLyt();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10

    void Open();
    void Decide();
    void DrawLayer1C();
    void DrawLayer1A();
    bool SetCursor(s32 button);
    s32 HitTest(const nw4r::math::VEC2& rPos) const;
    nw4r::lyt::Pane* GetButtonPane(s32 button) const;
    void PlayCursorSe(bool rumble);

    s32 GetState() const {
        return mState;
    }

private:
    void EnterIn();
    void CalcIn();
    void EnterPush();
    void CalcPush();
    void EnterOut();
    void CalcOut();

private:
    s32 mState;                                      // at 0x8
    s32 mNextState;                                  // at 0xC
    bool mIsChanging;                                // at 0x10
    s32 mStateTimer;                                 // at 0x14
    s32 mCursor;                                     // at 0x18 -1 = none
    s32 mPrevCursor;                                 // at 0x1C
    RPSysLayout* mpLayout;                           // at 0x20
    RPSysLytAnmObj* mpInAnmObj;                      // at 0x24
    RPSysLytAnmObj* mpOutAnmObj;                     // at 0x28
    RPSysLytAnmObj* mpButtonAnmObjs[BUTTON_NUM][2];  // at 0x2C active / push per button
    nw4r::lyt::Pane* mpChoicesPane;                  // at 0x3C N_choices_00
    nw4r::lyt::Pane* mpChoicePanes[BUTTON_NUM];      // at 0x40 N_choicesF_00, N_choicesS_00
    nw4r::lyt::Pane* mpButtonPanes[BUTTON_NUM];      // at 0x48 B_choicesF_00, B_choicesS_00
    RPSysLytBounding* mpBoundings[BUTTON_NUM];       // at 0x50
};

/**
 * @brief Win counts of a two-player session (ptVsResult.arc)
 * @details vtable 80358320. Size 0x2C, verified (Party_ResultMenu_ctor). Inline state machine
 * like RPChoiceLyt (table 8031C7A8: 0 hidden, 1 in, 2 shown, 3 out, 4 done).
 * @note Wii Play-only framework class (ledger Party_VsResultLyt); the name is a project
 * convention.
 */
class RPVsResultLyt : public RPLytObjBase {
public:
    RPVsResultLyt();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10

    void Open(const s32* pWins);
    void Close();
    void DrawLayer1C();
    void DrawLayer1A();

    s32 GetState() const {
        return mState;
    }

private:
    void EnterIn();
    void CalcIn();
    void EnterOut();
    void CalcOut();

private:
    s32 mWins[RP_PARTY_MAX_PLAYERS]; // at 0x8
    s32 mState;                      // at 0x10
    s32 mNextState;                  // at 0x14
    bool mIsChanging;                // at 0x18
    s32 mStateTimer;                 // at 0x1C
    RPSysLayout* mpLayout;           // at 0x20
    RPSysLytAnmObj* mpAnmObjs[2];    // at 0x24 in, out
};

/**
 * @brief Result menu shown after every minigame round (RPCommon + 0x1C)
 * @details vtable 80358308 (IRPSysHostIOSocket). Size 0x30, verified (Party_Common_ctor).
 * Minigames call Start(playerNum, winner) and poll GetChoice. Five states (table 8031C730):
 * 0 hidden, 1 show, 2 select, 3 decide, 4 done. For two players Show also updates the scene
 * info's win counts and streak. The cursor follows the pointer, or the +Control Pad (mUsePad).
 * @note Wii Play-only framework class (ledger Party_ResultMenu); the name is a project
 * convention.
 */
class RPResultMenu : public IRPSysHostIOSocket {
public:
    RPResultMenu();
    virtual ~RPResultMenu(); // at 0x8

    void Load();
    void Reset();
    void Calculate();
    void Start(s32 playerNum, s32 winner);
    /**
     * @brief Chosen button once the menu is done, else -1 [H]
     */
    s32 GetChoice() const;
    s32 GetState() const {
        return mpStateMachine->GetState();
    }
    void DrawLayer1C();
    void DrawLayer1A();

private:
    void EnterShow();
    void CalcShow();
    void EnterSelect();
    void CalcSelect();
    void EnterDecide();
    void CalcDecide();

private:
    RPStateMachine<RPResultMenu>* mpStateMachine; // at 0x4
    s32 mChoice;                                  // at 0x8 -1 = none
    s32 mLastChoice;                              // at 0xC
    s32 mHitButton;                               // at 0x10 button under the pointer, -1 none
    s32 mPrevHitButton;                           // at 0x14
    bool mUsePad;                                 // at 0x18
    s32 mPlayerNum;                               // at 0x1C
    s32 mWinner;                                  // at 0x20
    RPChoiceLyt* mpChoiceLyt;                     // at 0x24
    RPCPadCursor* mpCursor;                       // at 0x28
    RPVsResultLyt* mpVsResultLyt;                 // at 0x2C
};

//! Ledger Party_ResultMenuSeq (size 0x28)
typedef RPStateMachine<RPResultMenu> RPResultMenuSeq;

//! @}

#endif
