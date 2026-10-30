#ifndef RP_PARTY_BIL_HUD_H
#define RP_PARTY_BIL_HUD_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpDrawObject.h>
#include <Pack/RPParty/RPBilScene/RPBilObject.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <nw4r/lyt.h>

//! @addtogroup rp_bil
//! @{

// Forward declarations
class RPSysKokeshiIcon;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytTextBox;

/**
 * @brief Billiards HUD (billiard_00): scores, instruction text, prompt and turn panel
 * (singleton RPBilHud::sInstance)
 * @details vtable 8035E498. Size 0xA0, verified (Bil_Hud_create). A layout object that is also
 * the draw object (IRPGrpDrawObject at 0x8, vtable at 0xC): vtable layout (CodeWarrior)
 * RPLytObjBase part 0x8-0x18 (Draw not overridden), IRPGrpDrawObject part at 0x1C (dtor and
 * UserDraw thunks -8), then the destructor at 0x30 and UserDraw at 0x34. Two 4-state machines
 * (idle, in, shown, out) run the instruction prompt and the per-player turn panel; one Mii
 * icon (RPSysKokeshiIcon, 0x60 px) per player.
 */
class RPBilHud : public RPLytObjBase, public IRPGrpDrawObject {
public:
    static RPBilHud* sInstance; // 8043CA38

    enum EState {
        STATE_IDLE,
        STATE_IN,
        STATE_SHOWN,
        STATE_OUT,

        STATE_NUM
    };

    enum {
        ANM_NUM = 8,
        GROUP_NUM = 3,
        PLAYER_MAX = 2
    };

public:
    RPBilHud();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10

    virtual ~RPBilHud(); // at 0x30

    /**
     * @brief IRPGrpDrawObject::UserDraw override: the layout and the Mii icons [M]
     */
    virtual void UserDraw(); // at 0x34

    void SetInstruction(s32 msg);
    void SetPlayerScore(s32 player, s32 score);
    void Set1pScore(s32 shots);
    void HideTurnPanel();
    void ShowTurnPanel(s32 player);
    void HidePrompt();
    void ShowPrompt();

    s32 GetInstruction() const {
        return mInstruction;
    }
    RPSysKokeshiIcon* GetKokeshiIcon(s32 player) const {
        return mpKokeshiIcons[player];
    }

    static void CreateInstance();
    static void DestroyInstance();

private:
    void PromptIdleEnter();
    void PromptIdleCalc();
    void PromptInEnter();
    void PromptInCalc();
    void PromptShownEnter();
    void PromptShownCalc();
    void PromptOutEnter();
    void PromptOutCalc();
    void TurnPanelIdleEnter();
    void TurnPanelIdleCalc();
    void TurnPanelInEnter();
    void TurnPanelInCalc();
    void TurnPanelShownEnter();
    void TurnPanelShownCalc();
    void TurnPanelOutEnter();
    void TurnPanelOutCalc();

private:
    s32 mPlayerNum;                                     // at 0x10 Bil_playerNum
    RPStateMachine<RPBilHud>* mpPromptStateMachine;     // at 0x14
    bool mShowPrompt;                                   // at 0x18 request
    bool mHidePrompt;                                   // at 0x19 request
    s32 m1pScore;                                       // at 0x1C [M] set by Set1pScore
    s32 mScores[PLAYER_MAX];                            // at 0x20 [M] SetPlayerScore
    s32 mInstruction;                                   // at 0x28 [M] message of the prompt
    RPStateMachine<RPBilHud>* mpTurnPanelStateMachine;  // at 0x2C
    s32 mTurnPlayer;                                    // at 0x30 [M]
    bool mHideTurnPanel;                                // at 0x34 request
    s32 mTurnPlayerShown;                               // at 0x38 [L] player of the shown panel
    RPSysLytAnmObj* mpTurnAnmObj;                       // at 0x3C [L] in/out message anm
    RPSysLayout* mpLayout;                              // at 0x40 billiard_00
    /**
     * @brief inAll, stText, loopText, in1pMessage, out1pMessage, in2pMessage, out2pMessage,
     * outAll
     */
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM];                 // at 0x44
    bool mLoopPrompt;                                   // at 0x64 [L] PromptShown
    nw4r::lyt::Group* mpGroups[GROUP_NUM];              // at 0x68 gCommon_00, g1Player_00, g2Players_00
    nw4r::lyt::Pane* mpMessagePanes[PLAYER_MAX];        // at 0x74 N_1pMessage_00, N_2pMessage_00
    nw4r::lyt::Pane* mpTextPane;                        // at 0x7C N_text_00
    RPSysLytTextBox* mpTextBoxes[PLAYER_MAX];           // at 0x80 T_text_00/01
    nw4r::lyt::Pane* mpFacePanes[PLAYER_MAX][2];        // at 0x88 N_1p/2pFacePict_00/01
    RPSysKokeshiIcon* mpKokeshiIcons[PLAYER_MAX];       // at 0x98

public:
    static wchar_t sEmptyText[4];
};

//! Ledger Bil_HudSM (size 0x28)
typedef RPStateMachine<RPBilHud> RPBilHudSeq;

//! @}

#endif
