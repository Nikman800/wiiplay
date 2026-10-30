#ifndef RP_PARTY_COMMON_TITLE_LYT_H
#define RP_PARTY_COMMON_TITLE_LYT_H
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
 * @brief Wii Play title screen layouts (ptTitle.arc: background, logo, info text, bubbles)
 * @details vtable 80358748 (IRPSysHostIOSocket). Size 0x248, verified (constructed by
 * RPCommonMessage::Configure). State machines (all RPStateMachine<RPTitleLyt>, enter/calc
 * table 8031E9F8):
 * - main, 7 states: 0 hidden, 1 wait ready, 2 in, 3 loop (wait for A+B), 4 out, 5 closed
 * - one per button (A, B), 3 states: 0 idle, 1 press
 * - one per bubble (2 x 10), 2 states: 0 idle, 1 bubble
 * Japanese and other-language panes (groups japan_00 / notJapan_00) are shown by region; the
 * A/B prompt text is party_message.bmg 0x1771. Plays RP_COMMON_BGM_PARTY_01.
 * @note Wii Play-only framework class (ledger Party_TitleLyt); the name is a project
 * convention.
 */
class RPTitleLyt : public IRPSysHostIOSocket {
public:
    static const int BUTTON_NUM = 2;
    static const int BUBBLE_GROUP_NUM = 2;
    static const int BUBBLE_NUM = 10;

public:
    RPTitleLyt();
    virtual ~RPTitleLyt(); // at 0x8

    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Open();
    // Wii Play: pressed = the button is held (RPCommonMessage::CalcWaitInput)
    void OnButtonPress(s32 button, bool pressed);

    // Wii Play: main state machine queries (RPCommonMessage::CalcWaitInput) [H]
    s32 GetState() const {
        return mpStateMachine->GetState();
    }
    bool IsStateSettled(s32 state) const {
        return mpStateMachine->IsStateSettled(state);
    }

private:
    void EnterWaitReady();
    void CalcWaitReady();
    void EnterIn();
    void CalcIn();
    void EnterLoop();
    void CalcWaitPress();
    void EnterOut();
    void CalcOut();
    void EnterClosed();
    void CalcClosed();
    void EnterButtonIdle();
    void CalcButtonIdle();
    void EnterButtonPress();
    void CalcButtonPress();
    void EnterBubble();
    void CalcBubble();

    // Wii Play: inlined into Calculate and OnButtonPress [H]
    void StartBubble(s32 group);

private:
    RPStateMachine<RPTitleLyt>* mpStateMachine;                    // at 0x4
    RPStateMachine<RPTitleLyt>* mpButtonStates[BUTTON_NUM];        // at 0x8
    s32 mCurIdx;                                                   // at 0x10 button / group
    bool mButtonsReady;                                            // at 0x14
    RPStateMachine<RPTitleLyt>* mpBubbleStates[BUBBLE_GROUP_NUM][BUBBLE_NUM]; // at 0x18
    s32 mCurBubble;                                                // at 0x68
    s32 mBubbleAnmIdx[BUBBLE_GROUP_NUM][BUBBLE_NUM];               // at 0x6C
    u32 unkBC;                                                     // at 0xBC
    s32 mBubbleTimers[BUBBLE_GROUP_NUM];                           // at 0xC0
    RPSysLayout* mpLayouts[3];                                     // at 0xC8 bg, logo, info
    RPSysLayout* mpBubbleLayouts[BUBBLE_GROUP_NUM][BUBBLE_NUM];    // at 0xD4
    /**
     * @brief inBg, loopBg (bg); inLogo, loopLogo (logo); inInfo, waitInfo, actOthers (info)
     */
    RPSysLytAnmObj* mpAnmObjs[7];                                  // at 0x124
    RPSysLytAnmObj* mpButtonAnmObjs[BUTTON_NUM];                   // at 0x140 actABButton
    RPSysLytAnmObj* mpBubbleAnmObjs[BUBBLE_GROUP_NUM][BUBBLE_NUM][3]; // at 0x148 bubble_00-02
    nw4r::lyt::Pane* mpButtonPanes[BUTTON_NUM];                    // at 0x238 N_A/BButtonJ or E
    RPSysLytResAccessor* mpResAccessor;                            // at 0x240
    RPSysMessage* mpMessage;                                       // at 0x244 party_message.bmg
};

//! Ledger Party_TitleSeq (size 0x28)
typedef RPStateMachine<RPTitleLyt> RPTitleSeq;

//! @}

#endif
