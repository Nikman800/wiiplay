#ifndef RP_PARTY_COW_HUD_H
#define RP_PARTY_COW_HUD_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPActor.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_cow
//! @{

// Forward declarations
class RPSysEffect;
class RPSysKokeshiIcon;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytTextBox;

/**
 * @brief Time, score and result bar (cowMain_00.brlyt; cowMain_01.brlyt when EGG::Screen::sTVMode
 * is 1)
 * @details vtable 8035D728. Size 0xD0, verified (RPCowGame constructor). Eleven animations:
 * in, out, score x2, time bonus x2, add total, total, out 1P, in result, out result.
 * @note Name is a project convention (ledger Cow_MainHud).
 */
class RPCowMainHud : public RPActor {
public:
    RPCowMainHud();
    virtual ~RPCowMainHud(); // at 0x38

    virtual void LoadResource();      // at 0x44 ledger init
    virtual void OnStart();           // at 0x48 ledger reset
    virtual void OnStop();            // at 0x4C ledger exit (empty)
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void DrawScreen(u8 pass); // at 0x58

    void Show();
    void ShowTimeout();
    void SetPlayerScore(s32 player, s32 score, s32 add);
    void SetTimeWarning(bool warning);
    void SetTime(f32 seconds);
    void PlayTotal();
    bool IsAnimIdle() const;
    void PlayTimeBonus();
    void PlayOutResult();
    void PlayInResult();
    void PlayOut1P();
    void PlayOut();

private:
    RPSysLayout* mpLayout;                // at 0x24
    RPSysLytAnmObj* mpAnmObjs[11];        // at 0x28
    char unk54[0x58 - 0x54];
    /**
     * @brief N_time_00, N_time_02, N_timeBonus_00, N_result_00, N_player_00/01, N_score_02,
     * P_line_00 (first name from a string in .rodata)
     */
    nw4r::lyt::Pane* mpPanes[9];          // at 0x58
    char unk7C[0x80 - 0x7C];
    /**
     * @brief T_timer_00-03, T_comma_00-03, T_player00_00/01, T_score00/01_00-02,
     * T_timeBonus_00/01
     */
    RPSysLytTextBox* mpTextBoxes[18];     // at 0x80
    s32 mAnmState;                        // at 0xC8 [M]
    s32 mTimer;                           // at 0xCC [M]
};

/**
 * @brief "Perfect" banner when a whole scarecrow group is knocked (cowPerfect_00.brlyt)
 * @details vtable 8035D788. Size 0xB0, verified (RPCowGame constructor).
 * @note Name is a project convention (ledger Cow_PerfectMsg).
 */
class RPCowPerfectMsg : public RPActor {
public:
    RPCowPerfectMsg();
    virtual ~RPCowPerfectMsg(); // at 0x38

    virtual void LoadResource();      // at 0x44 ledger init
    virtual void OnStart();           // at 0x48 ledger reset
    virtual void OnStop();            // at 0x4C ledger exit (empty)
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void DrawScreen(u8 pass); // at 0x58

    void Show(s32 player, s32 score, s32 bonusLevel);

private:
    RPSysLayout* mpLayout;               // at 0x24
    RPSysLytAnmObj* mpAnmObjs[3];        // at 0x28 Perfect_00-02
    nw4r::lyt::Pane* mpPanes[21];        // at 0x34
    RPSysLytTextBox* mpTextBoxes[6];     // at 0x88 N_perfectL* (score digits)
    s32 mStates[2];                      // at 0xA0 per player: 1 while the banner is shown [M]
    /**
     * @brief Cow_2d_perfect_1p_blue, or Cow_2d_perfect_vs_blue/red in two-player mode
     */
    RPSysEffect* mpEffects[2];           // at 0xA8
};

/**
 * @brief Explanation messages (collision, movement and input hints; cowEx_00.brlyt)
 * @details vtable 8035D7E8. Size 0x70, verified (RPCowGame constructor).
 * @note Name is a project convention (ledger Cow_ExMsg).
 */
class RPCowExMsg : public RPActor {
public:
    RPCowExMsg();
    virtual ~RPCowExMsg(); // at 0x38

    virtual void LoadResource();      // at 0x44 ledger init
    virtual void OnStart();           // at 0x48 ledger reset
    virtual void OnStop();            // at 0x4C ledger exit (empty)
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void DrawScreen(u8 pass); // at 0x58

    void Hide(s32 idx);
    void ShowCollisionMsg(s32 idx);
    void ShowMoveMsg(s32 idx);
    void ShowInputMsg(s32 idx);

private:
    // unbindAnmObj + bindAnmObjToPane; the out animation of message msg (defined in
    // RPCowHud_5.cpp) [H]
    inline void BindAnm(u32 anm, s32 pane);
    inline void PlayOut(s32 msg);

    RPSysLayout* mpLayout;               // at 0x24
    RPSysLytAnmObj* mpAnmObjs[6];        // at 0x28 in/out per message
    nw4r::lyt::Pane* mpPanes[3];         // at 0x40
    RPSysLytTextBox* mpTextBoxes[3];     // at 0x4C T_ex_00-02
    s32 mStates[3];                      // at 0x58 per message: 1 input, 2 move, 3 collision [M]
    s32 mTimers[3];                      // at 0x64 frames until the message goes out [M]
};

/**
 * @brief "Knock down the scarecrows" order banner (cowOrder_00.brlyt)
 * @details vtable 8035D848. Size 0x44, verified (RPCowGame constructor).
 * @note Name is a project convention (ledger Cow_OrderMsg).
 */
class RPCowOrderMsg : public RPActor {
public:
    RPCowOrderMsg();
    virtual ~RPCowOrderMsg(); // at 0x38

    virtual void LoadResource();      // at 0x44 ledger init
    virtual void OnStart();           // at 0x48 ledger reset
    virtual void OnStop();            // at 0x4C ledger exit (empty)
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void DrawScreen(u8 pass); // at 0x58

    void Show();
    /**
     * @brief RPCowTargetMarker::Show / RPCowBalloon::Hide for every player: called on the
     * order message (slot 21 actor in r3) when the order is shown / hidden, `this` unused
     * (ledger Cow_showAllTargetMarkers / Cow_hideAllBalloons; defined in RPCowHud_6.cpp) [M]
     */
    void ShowAllTargetMarkers();
    void HideAllBalloons();

private:
    RPSysLayout* mpLayout;               // at 0x24
    RPSysLytAnmObj* mpAnmObjs[2];        // at 0x28 in, out
    nw4r::lyt::Pane* mpRootPane;         // at 0x30
    RPSysLytTextBox* mpTextBoxes[2];     // at 0x34 T_order_00/01
    s32 mState;                          // at 0x3C
    s32 mTimer;                          // at 0x40 180 frames
};

/**
 * @brief "Start" banner (cowStart_00.brlyt)
 * @details vtable 8035D8A8. Size 0x38, verified (RPCowGame constructor).
 * @note Name is a project convention (ledger Cow_StartMsg).
 */
class RPCowStartMsg : public RPActor {
public:
    RPCowStartMsg();
    virtual ~RPCowStartMsg(); // at 0x38

    virtual void LoadResource(); // at 0x44 ledger init
    virtual void OnStart();      // at 0x48 ledger reset
    /**
     * @brief Same function as OnActivate in the vtable (ledger calc) [H]
     */
    virtual void OnStop();            // at 0x4C
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void DrawScreen(u8 pass); // at 0x58

    void Show();

private:
    RPSysLayout* mpLayout;      // at 0x24
    RPSysLytAnmObj* mpAnmObj;   // at 0x28
    nw4r::lyt::Pane* mpPane;    // at 0x2C [M]
    s32 mState;                 // at 0x30 [M]
    s32 mTimer;                 // at 0x34 [M]
};

/**
 * @brief "Complete" banner at the goal (cowGoal_00.brlyt)
 * @details vtable 8035D908. Size 0x48, verified (RPCowGame constructor).
 * @note Name is a project convention (ledger Cow_GoalMsg).
 */
class RPCowGoalMsg : public RPActor {
public:
    RPCowGoalMsg();
    virtual ~RPCowGoalMsg(); // at 0x38

    virtual void LoadResource();      // at 0x44 ledger init
    virtual void OnStart();           // at 0x48 ledger reset
    virtual void OnStop();            // at 0x4C ledger exit (empty)
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void DrawScreen(u8 pass); // at 0x58

    void Show(s32 player);

private:
    RPSysLayout* mpLayout;          // at 0x24
    RPSysLytAnmObj* mpAnmObjs[3];   // at 0x28 (first name from a string in .rodata), out, inComplete
    nw4r::lyt::Pane* mpPanes[2];    // at 0x34
    RPSysEffect* mpEffect;          // at 0x3C Cow_2d_complete
    s32 mState;                     // at 0x40 [M]
    s32 mTimer;                     // at 0x44 [M]
};

/**
 * @brief Last ten seconds count down and the time-up message (cowLimit_00.brlyt)
 * @details vtable 8035D968. Size 0x74, verified (RPCowGame constructor).
 * @note Name is a project convention (ledger Cow_LimitCount).
 */
class RPCowLimitCount : public RPActor {
public:
    RPCowLimitCount();
    virtual ~RPCowLimitCount(); // at 0x38

    virtual void LoadResource();      // at 0x44 ledger init
    virtual void OnStart();           // at 0x48 ledger reset
    virtual void OnStop();            // at 0x4C ledger exit (empty)
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void DrawScreen(u8 pass); // at 0x58

    void SetCount(s32 count);
    void HideMessage();
    void Start();
    void ShowFinish();

private:
    RPSysLayout* mpLayout;          // at 0x24
    RPSysLytAnmObj* mpAnmObjs[3];   // at 0x28 count, in, out
    /**
     * @brief N_messageAll_00, N_message_00/01, N_countAll_00, N_count_00-09
     */
    nw4r::lyt::Pane* mpPanes[14];   // at 0x34
    s32 mState;                     // at 0x6C [M]
    s32 mTimer;                     // at 0x70 [M]
};

/**
 * @brief Two-player result with both scores and the winner (cowVsResult_00.brlyt)
 * @details vtable 8035D9C8. Size 0x5C, verified (RPCowGame constructor).
 * @note Name is a project convention (ledger Cow_VsResult).
 */
class RPCowVsResult : public RPActor {
public:
    RPCowVsResult();
    virtual ~RPCowVsResult(); // at 0x38

    virtual void LoadResource(); // at 0x44 ledger init
    virtual void OnStart();      // at 0x48 ledger reset
    /**
     * @brief Same function as OnActivate in the vtable (ledger calc) [H]
     */
    virtual void OnStop();            // at 0x4C
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void DrawScreen(u8 pass); // at 0x58

    void SetScore(s32 player, s32 score);
    void ShowWinner(s32 player);
    void PlayOut();
    void Show();

private:
    RPSysLayout* mpLayout;               // at 0x24
    RPSysLytAnmObj* mpAnmObjs[4];        // at 0x28 in, out, win1p, win2p
    nw4r::lyt::Pane* mpPanes[3];         // at 0x38 N_result_00, N_score_00/01
    RPSysLytTextBox* mpTextBoxes[4];     // at 0x44 T_score*
    s32 mState;                          // at 0x54 [M]
    s32 unk58;                           // at 0x58 [L]
};

/**
 * @brief Marker over the scarecrow group to aim for (cowOrder_01.brlyt), one per player
 * @details vtable 8035DA28. Size 0x40, verified (array stride in the RPCowGame constructor).
 * @note Name is a project convention (ledger Cow_TargetMarker).
 */
class RPCowTargetMarker : public RPActor {
public:
    RPCowTargetMarker();
    virtual ~RPCowTargetMarker(); // at 0x38

    virtual void LoadResource();      // at 0x44 ledger init
    virtual void OnStart();           // at 0x48 ledger reset
    virtual void OnStop();            // at 0x4C ledger exit (empty)
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void DrawScreen(u8 pass); // at 0x58

    void RestartAnim();
    void Show();

private:
    RPSysLayout* mpLayout;          // at 0x24
    RPSysLytAnmObj* mpAnmObjs[3];   // at 0x28 in, out, cursor
    nw4r::lyt::Pane* mpPane;        // at 0x34 [M]
    s32 mState;                     // at 0x38 [M]
    s32 unk3C;                      // at 0x3C [L]
};

/**
 * @brief Speech balloon with the player's Mii icon (cowBalloon_00.brlyt), one per player
 * @details vtable 8035DA88. Size 0x50, verified (array stride in the RPCowGame constructor).
 * @note Name is a project convention (ledger Cow_Balloon).
 */
class RPCowBalloon : public RPActor {
public:
    RPCowBalloon();

    /**
     * @brief Ledger create: the Mii icon [H]
     * @details Declared before the destructor so that it, not the destructor, is the key
     * function: the vtable and thunks are emitted with it (RPCowHud_12), while the
     * constructor and destructor sit in their own small units (RPCowHud_2, RPCowHud).
     */
    virtual void VF_0x3C(); // at 0x3C
    virtual ~RPCowBalloon(); // at 0x38
    /**
     * @brief Ledger destroy (empty) [H]
     */
    virtual void VF_0x40();           // at 0x40
    virtual void LoadResource();      // at 0x44 ledger init
    virtual void OnStart();           // at 0x48 ledger reset
    virtual void OnStop();            // at 0x4C ledger exit (empty)
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void DrawScreen(u8 pass); // at 0x58

    void Hide();
    void Show();

private:
    RPSysKokeshiIcon* mpIcon;       // at 0x24
    RPSysLayout* mpLayout;          // at 0x28
    RPSysLytAnmObj* mpAnmObjs[2];   // at 0x2C in, out
    nw4r::lyt::Pane* mpPanes[5];    // at 0x34 [M] N_balloon_00/01, N_player_00/01, ...
    s32 mState;                     // at 0x48 [M]
    s32 unk4C;                      // at 0x4C [L]
};

/**
 * @brief Steering angle / rotation display (cowAngle_00.brlyt)
 * @details vtable 8035DBD8. Size 0x58, verified (RPCowGame constructor).
 * @note Name is a project convention (ledger Cow_AngleDisp).
 */
class RPCowAngleDisp : public RPActor {
public:
    RPCowAngleDisp();
    virtual ~RPCowAngleDisp(); // at 0x38

    virtual void LoadResource();      // at 0x44 ledger init
    virtual void OnStart();           // at 0x48 ledger reset
    virtual void OnStop();            // at 0x4C ledger exit (empty)
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void DrawScreen(u8 pass); // at 0x58

    void SetAngle(s32 player, f32 angle);
    void PlaySwitch(s32 player);
    /**
     * @brief Ledger Cow_bindAnimsMode1 / Cow_bindAnimsMode2 (called by RPCowGame at the start
     * and at the end of play) [M]
     */
    void BindAnimsMode1();
    void BindAnimsMode2();

private:
    // Y position of the rotation counters (defined after the code in RPCowHud_13.cpp) [H]
    static const f32 sCountPosY;

    RPSysLayout* mpLayout;          // at 0x24
    /**
     * @brief Four animations (first name from a string in .rodata, out, switchCow ...)
     */
    RPSysLytAnmObj* mpAnmObjs[4];   // at 0x28
    /**
     * @brief N_1pAngleDisp_00, N_2pAngleDisp_00, N_1pRotateCnt_00, N_2pRotateCnt_00, ...
     */
    nw4r::lyt::Pane* mpPanes[6];    // at 0x38
    s32 mState;                     // at 0x50 1 shown, 2 going out [M]
    bool mSwitching[2];             // at 0x54 per player: switchCow animation playing [M]
};

//! @}

#endif
