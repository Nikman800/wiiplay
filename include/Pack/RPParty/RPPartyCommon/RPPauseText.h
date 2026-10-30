#ifndef RP_PARTY_COMMON_PAUSE_TEXT_H
#define RP_PARTY_COMMON_PAUSE_TEXT_H
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
 * @brief "Paused" text shown per player (ptPauseText.arc, ptPauseText_00.brlyt)
 * @details No vtable. Size 0x3C, verified (Party_PauseText_ctor). One four-state machine per
 * player: 0 idle (hidden), 1 in, 2 shown, 3 out. Show only takes effect while the scene info's
 * pause flag (RP_sceneInfoInstance + 0x2C) is set. Text is party_message.bmg 0x2383.
 * @note Wii Play-only framework class (ledger Party_PauseTextLyt); the name is a project
 * convention.
 */
class RPPauseTextLyt {
public:
    RPPauseTextLyt();

    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Show(s32 player);
    void Hide(s32 player);

private:
    void EnterIdle();
    void CalcIdle();
    void EnterIn();
    void CalcIn();
    void EnterShown();
    void CalcShown();
    void EnterOut();
    void CalcOut();

private:
    RPStateMachine<RPPauseTextLyt>* mpStateMachines[RP_PARTY_MAX_PLAYERS]; // at 0x0
    bool mShowRequest[RP_PARTY_MAX_PLAYERS];             // at 0x8
    bool mHideRequest[RP_PARTY_MAX_PLAYERS];             // at 0xA
    s32 mCurPlayer;                                      // at 0xC player being calculated
    RPSysLayout* mpLayout;                               // at 0x10
    RPSysLytAnmObj* mpAnmObjs[RP_PARTY_MAX_PLAYERS][2];  // at 0x14 in / out per player
    nw4r::lyt::Pane* mpPausePanes[RP_PARTY_MAX_PLAYERS]; // at 0x24 N_pause_00/01
    char unk2C[0x34 - 0x2C];
    RPSysLytResAccessor* mpResAccessor;                  // at 0x34
    RPSysMessage* mpMessage;                             // at 0x38 party_message.bmg
};

//! Ledger Party_PauseTextSeq (size 0x28)
typedef RPStateMachine<RPPauseTextLyt> RPPauseTextSeq;

/**
 * @brief Owner of the pause text layout (RPCommon + 0x28)
 * @details vtable 80358730 (IRPSysHostIOSocket). Size 0xC, verified (Party_Common_ctor).
 * Calculate shows or hides the text per player from mEnabled.
 * @note Wii Play-only framework class (ledger Party_PauseText); the name is a project
 * convention.
 */
class RPPauseText : public IRPSysHostIOSocket {
public:
    RPPauseText();
    virtual ~RPPauseText(); // at 0x8

    void Load();
    void Reset();
    void Calculate();
    void Draw();
    /**
     * @brief Sets mEnabled for both players (RPMedalResult/RPVsWinner pass false) [H]
     */
    void SetEnabled(bool enabled);
    // Wii Play: one player's pause text (RPHkyGame::Calculate)
    void SetEnabled(s32 player, bool enabled) {
        mEnabled[player] = enabled;
    }

private:
    bool mEnabled[RP_PARTY_MAX_PLAYERS]; // at 0x4
    RPPauseTextLyt* mpLayout;            // at 0x8
};

//! @}

#endif
