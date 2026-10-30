#ifndef RP_PARTY_COMMON_SELECT_GAME_LYT_H
#define RP_PARTY_COMMON_SELECT_GAME_LYT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <egg/math.h>
#include <nw4r/lyt.h>
#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPSysEffect;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytTextBox;

/**
 * @brief Game-select board with the nine game icons, medals and unlock state
 * (ptSelectGame.arc, ptSelectGame_00.brlyt)
 * @details vtable 80358398. Size 0x53C, verified (Party_SelectScene_ctor). State machines
 * (all RPStateMachine<RPSelectGameLyt>):
 * - main, 6 states (table 8031CFA0): 0 hidden, 1 in, 2 idle, 3 decide, 4 out, 5 done
 * - one per game slot, 6 states (table 8031D030): 0 highlighted, 1 highlight, 2 idle,
 *   3 unhighlight, 4 decide, 5 none
 * - one per game for the locked-icon animation, 4 states (table 8031D0C0): 0 idle (ledger
 *   Party_clearCurrentFlagB7 / Party_activateCurrentIfFlag), 1 start, 2 loop, 3 end
 * - the explanation text, 2 states (ledger Party_activateIfPending, Party_restartPaneAnim,
 *   Party_updateSelectionMessage)
 * Seven medal effects per game sparkle on the highlighted icon. Calculate sets mCurSlot /
 * mCurLocked to the slot whose machine is running.
 * @note Wii Play-only framework class (ledger Party_SelectGameLyt); the name is a project
 * convention.
 */
class RPSelectGameLyt : public RPLytObjBase {
public:
    static const int GAME_NUM = 9;
    static const int EFFECT_NUM = 7;

public:
    explicit RPSelectGameLyt(s32 playerNum);

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    void Open();
    void Close();
    /**
     * @brief Starts the decide animation of the game in the slot (playerFlag: remote speakers)
     */
    bool DecideGame(s32 slot, u32 playerFlag);
    bool IsSelectable() const;
    /**
     * @brief Slot whose icon boundings contain the point (pictures, then buttons, then covers
     * of locked games); 9 = none
     */
    s32 HitTestGame(const EGG::Vector2f& rPos);
    /**
     * @brief Slot next to the slot in the direction of the pad button (Party_selectGameNeighbors)
     */
    s32 GetNeighborGame(s32 slot, u32 button);
    bool HighlightGame(s32 slot);
    void UnhighlightAll();
    void UnhighlightOthers(s32 slot);
    nw4r::lyt::Pane* GetGameIconPane(s32 slot, s32 idx);
    EGG::Vector2f GetGameIconPos(s32 slot);
    /**
     * @brief Player doll position: N_pPositionC_00 for one player, else N_pPositionL/R_00
     */
    EGG::Vector2f GetPlayerPanePos(s32 playerNum, s32 player);
    void PlayCursorSe(s32 slot, bool rumble);
    f32 GetAnmLength(s32 idx);
    void StartMedalEffects(s32 game);
    void StopMedalEffects(s32 game);
    void RequestLockedAnm(s32 slot);
    void StopLockedAnm(s32 slot);
    /**
     * @brief Shows the record of the game (9 = none) in T_p1Score_00 [M]
     */
    void RequestExpChange(s32 game);

private:
    void AnimateIcons();

    void EnterIn();
    void CalcIn();
    void EnterIdle();
    void CalcIdle();
    void EnterDecide();
    void CalcDecide();
    void EnterOut();
    void CalcOut();

    void EnterSlotHighlighted();
    void CalcSlotHighlighted();
    void EnterSlotHighlight();
    void CalcSlotHighlight();
    void EnterSlotIdle();
    void CalcSlotIdle();
    void EnterSlotUnhighlight();
    void CalcSlotUnhighlight();
    void EnterSlotDecide();
    void CalcSlotDecide();

    void EnterLockedIdle();
    void CalcLockedIdle();
    void EnterLockedStart();
    void CalcLockedStart();
    void EnterLockedLoop();
    void CalcLockedLoop();
    void EnterLockedEnd();
    void CalcLockedEnd();

    void EnterExpIdle();
    void CalcExpIdle();
    void EnterExpChange();
    void CalcExpChange();

private:
    s32 mPlayerNum;                                          // at 0x8
    s32 mCurSlot;                                            // at 0xC
    s32 mDecidedGame;                                        // at 0x10 -1 = none
    s32 mCurLocked;                                          // at 0x14
public:
    RPStateMachine<RPSelectGameLyt>* mpStateMachine;         // at 0x18
private:
    RPStateMachine<RPSelectGameLyt>* mpSlotStates[GAME_NUM]; // at 0x1C
    EGG::Vector2f mIconPos[GAME_NUM];                        // at 0x40 from the icon panes
    EGG::Vector2f mIconOffset[GAME_NUM];                     // at 0x88 [M]
    EGG::Vector2f mIconVelocity[GAME_NUM];                   // at 0xD0 [M]
    s32 mIconTimer[GAME_NUM];                                // at 0x118 [M] icon wobble
    s32 mIconTimerEnd[GAME_NUM];                             // at 0x13C [M]
    RPStateMachine<RPSelectGameLyt>* mpLockedStates[GAME_NUM]; // at 0x160
    bool mLockedRequest[GAME_NUM];                           // at 0x184
    bool mLockedStop[GAME_NUM];                              // at 0x18D
    RPStateMachine<RPSelectGameLyt>* mpExpState;             // at 0x198
    bool mExpPending;                                        // at 0x19C
    s32 mHighlighted;                                        // at 0x1A0 9 = none
    s32 mExpNext;                                            // at 0x1A4 [M] requested game
    bool mUnlocked[GAME_NUM];                                // at 0x1A8
    s32 mMedals[GAME_NUM];                                   // at 0x1B4 single player only
    RPSysLayout* mpLayout;                                   // at 0x1D8
    RPSysLytAnmObj* mpAnmObjs[2];                            // at 0x1DC in / out
    RPSysLytAnmObj* mpSlotAnmObjs[GAME_NUM][3];              // at 0x1E4 offOn / onOff / push
    RPSysLytAnmObj* mpExpAnmObjs[GAME_NUM][2];               // at 0x250 inExp / outEep
    RPSysLytAnmObj* mpScoreAnmObjs[1];                       // at 0x298 changeScore
    nw4r::lyt::Pane* mpCenterPane;                           // at 0x29C N_center_00
    nw4r::lyt::Pane* mpGamePosPanes[GAME_NUM];               // at 0x2A0 N_gamesPoAd_NN
    /**
     * @brief N_game_NN, N_gamePic_NN, N_gameButton_NN, N_gameCover_NN (shown when locked)
     */
    nw4r::lyt::Pane* mpIconPanes[GAME_NUM][4];               // at 0x2C4
    nw4r::lyt::Pane* mpIconBoundings[GAME_NUM][4];           // at 0x354 B_game*_NN at [1]-[3]
    nw4r::lyt::Pane* mpPlayerPosPanes[3];                    // at 0x3E4 N_pPositionC/L/R_00
    nw4r::lyt::Pane* mpRankPanes[GAME_NUM];                  // at 0x3F0 N_rank_NN
    nw4r::lyt::Pane* mpExpPanes[GAME_NUM];                   // at 0x414 N_gameExp_NN
    RPSysLytTextBox* mpScoreTextBox;                         // at 0x438 T_p1Score_00
    RPSysEffect* mpMedalEffects[GAME_NUM][EFFECT_NUM];       // at 0x43C
    u32 mDecidePlayerFlag;                                   // at 0x538
};

//! Ledger Party_SelectGameSeq (size 0x28)
typedef RPStateMachine<RPSelectGameLyt> RPSelectGameSeq;

//! @}

#endif
