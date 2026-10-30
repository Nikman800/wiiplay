#ifndef RP_PARTY_COMMON_LEAD_CURSOR_H
#define RP_PARTY_COMMON_LEAD_CURSOR_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;
class RPSysMessage;

/**
 * @brief Off-screen pointer guide (leadCursor.arc, leadCursor_00.brlyt)
 * @details No vtable. Size 0x88, verified (Party_LeadCursor_ctor). Two guides (one per screen
 * side), each with a four-state machine: 0 idle, 1 in, 2 loop, 3 out. The arrow and line
 * pictures take the colour of the player shown; the text is party_message.bmg 0x2382.
 * @note Wii Play-only framework class (ledger Party_LeadCursorLyt); the name is a project
 * convention.
 */
class RPLeadCursorLyt {
public:
    static const int SIDE_NUM = 2;
    static const int PICTURE_NUM = 6;

public:
    explicit RPLeadCursorLyt(s32 playerNum);

    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Show(s32 player, s32 side);
    void Hide(s32 player, s32 side);

private:
    void EnterIdle();
    void CalcIdle();
    void EnterIn();
    void CalcIn();
    void EnterLoop();
    void CalcLoop();
    void EnterOut();
    void CalcOut();

private:
    RPStateMachine<RPLeadCursorLyt>* mpStateMachines[SIDE_NUM]; // at 0x0
    bool mShowRequest[SIDE_NUM];                           // at 0x8
    bool mHideRequest[SIDE_NUM];                           // at 0xA
    s32 mPlayerNum;                                        // at 0xC
    s32 mPlayer[SIDE_NUM];                                 // at 0x10 player shown, -1 none
    s32 mRequestPlayer[SIDE_NUM];                          // at 0x18 player passed to Show
    s32 mCurSide;                                          // at 0x20 side being calculated
    RPSysLayout* mpLayout;                                 // at 0x24
    RPSysLytAnmObj* mpAnmObjs[SIDE_NUM][3];                // at 0x28 in / roop / out
    nw4r::lyt::Pane* mpBasePanes[SIDE_NUM];                // at 0x40 N_base_01, N_base_00
    nw4r::lyt::TextBox* mpTextBoxes[SIDE_NUM];             // at 0x48 T_mes_01, T_mes_00
    nw4r::lyt::Picture* mpPictures[SIDE_NUM][PICTURE_NUM]; // at 0x50 arrows and lines
    RPSysLytResAccessor* mpResAccessor;                    // at 0x80
    RPSysMessage* mpMessage;                               // at 0x84 party_message.bmg
};

//! Ledger Party_LeadCursorSeq (size 0x28)
typedef RPStateMachine<RPLeadCursorLyt> RPLeadCursorSeq;

/**
 * @brief Shows the off-screen guide once a player's pointer has been off one side of the
 * screen long enough (RPCommon + 0x24)
 * @details vtable 803586D0 (IRPSysHostIOSocket). Size 0x28, verified (Party_Common_ctor).
 * Per player and side a counter runs while the remote reports that side (controller + 0x924
 * is 1 or 2) and the guide is shown once it passes mTimers[side]. Minigames set the timers
 * (120 frames after Reset) and disable players.
 * @note Wii Play-only framework class (ledger Party_LeadCursor); the name is a project
 * convention.
 */
class RPLeadCursor : public IRPSysHostIOSocket {
public:
    explicit RPLeadCursor(s32 playerNum);
    virtual ~RPLeadCursor(); // at 0x8

    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void SetPlayerEnabled(s32 player, bool enabled);
    void SetAllEnabled(bool enabled);
    void SetTimer(s32 frames, s32 side);
    void SetTimers(s32 frames);

private:
    s32 mPlayerNum;                                                 // at 0x4
    s32 mCounters[RP_PARTY_MAX_PLAYERS][RPLeadCursorLyt::SIDE_NUM]; // at 0x8
    s32 mTimers[RPLeadCursorLyt::SIDE_NUM];                         // at 0x18
    bool mEnabled[RP_PARTY_MAX_PLAYERS];                            // at 0x20
    RPLeadCursorLyt* mpLayout;                                      // at 0x24

    static s32 sDebugTimer;     // 8043B480 [M]
    static bool sUseDebugTimer; // 8043C710 [M] debug: force both timers to sDebugTimer
};

//! @}

#endif
