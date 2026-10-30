#ifndef RP_PARTY_HKY_LYT_H
#define RP_PARTY_HKY_LYT_H
#include <Pack/types_pack.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_hky
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;
class RPSysKokeshiIcon;

/**
 * @name Laser Hockey layouts (airHockey.arc)
 * @details Plain structs owned by RPHkyHud: no vtable and no constructor (Create zeroes the
 * members). Load creates the archive accessor, the layout and its animation objects; Calculate
 * advances a small state machine (mState: 0 init, 1 idle, then one state per playing animation)
 * and calls RPSysLayout::calc. Number texts are swprintf'd and turned into full-width digits
 * (each character - 0x120).
 */
//! @{

/**
 * @brief Start countdown "3, 2, 1, start" (airHockeyMsg_00.brlyt)
 * @details Size 0x20, verified (Hky_createHud). States 2-4 count, 5 star; leaving state 4 starts
 * the game BGM, hides the balloons and brings in the time and score layouts.
 */
class RPHkyStartMsgLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void Start();
    bool IsDone() const;

    s32 GetState() const {
        return mState;
    }

private:
    enum {
        ANM_COUNT_3, //!< count_00
        ANM_COUNT_2, //!< count_01
        ANM_COUNT_1, //!< count_02
        ANM_STAR,    //!< star_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    s32 mTimer;                         // at 0x4 [L] zeroed on every state change
    RPSysLytResAccessor* mpResAccessor; // at 0x8
    RPSysLayout* mpLayout;              // at 0xC
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x10
};

/**
 * @brief Player "1P / 2P" balloons with the players' Mii icons (airHockeyBalloon_00.brlyt)
 * @details Size 0x34, verified (Hky_createHud). Create builds one RPSysKokeshiIcon (0x60 x 0x60)
 * per player.
 */
class RPHkyBalloonLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void Show();
    void RequestOut();

private:
    struct Icon {
        RPSysKokeshiIcon* mpIcon; // at 0x0
        nw4r::lyt::Pane* mpPane;  // at 0x4 icon picture pane
    };

    enum {
        ANM_IN,   //!< inAll_00
        ANM_WAIT, //!< waitAll_00
        ANM_OUT,  //!< outAll_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    bool mOutRequested;                 // at 0x4
    RPSysLytResAccessor* mpResAccessor; // at 0x8
    RPSysLayout* mpLayout;              // at 0xC
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x10
    nw4r::lyt::Pane* mpCenterPanes[2];  // at 0x1C N_1pCenter_00, N_2pCenter_00
    Icon mIcons[2];                     // at 0x24
};

/**
 * @brief Remaining time (airHockeyTime_00.brlyt), single player only
 * @details Size 0x30, verified (Hky_createHud). Shows ceil(timer / 60) seconds, plays the
 * countdown jingles under 30/20/10/5 seconds and flashes at 10, 5, 4, 3, 2, 1 and 0 seconds.
 */
class RPHkyTimeLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void PlayIn();
    void PlayOut();
    void SetNumber(s32 seconds) DECOMP_DONT_INLINE;

private:
    enum {
        ANM_IN,         //!< inAll_00
        ANM_OUT,        //!< outAll_00
        ANM_SWITCH_BG,  //!< switchBG_00
        ANM_SWITCH_NUM, //!< switchNum_00
        ANM_LAST_NUM,   //!< lastNum_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    s32 mTimer;                         // at 0x4 [L]
    s32 mSeconds;                       // at 0x8 last shown value
    wchar_t mNumberText[4];             // at 0xC
    RPSysLytResAccessor* mpResAccessor; // at 0x14
    RPSysLayout* mpLayout;              // at 0x18
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x1C
};

/**
 * @brief Score board (airHockeyScore_00.brlyt), drawn on view 2
 * @details Size 0x30, verified (Hky_createHud). On a goal (state 3) the scoring side's number is
 * updated from RPHkyPaddle::mScore after the switch animation.
 */
class RPHkyScoreLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void PlayIn();
    void PlayOut();
    void PlayGoalSwitch(s32 side);
    void PlayResultSwitch();

private:
    enum {
        ANM_IN,            //!< inAll_00
        ANM_OUT,           //!< outAll_00
        ANM_SWITCH_GOAL,   //!< switchGoal_00
        ANM_SWITCH_RESULT, //!< switchResult_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    s32 mTimer;                         // at 0x4 [L]
    s32 mGoalSide;                      // at 0x8 0 left, 1 right
    wchar_t mScoreText[2][3];           // at 0xC
    RPSysLytResAccessor* mpResAccessor; // at 0x18
    RPSysLayout* mpLayout;              // at 0x1C
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x20
};

/**
 * @brief "Goal!" message with the new score (airHockeyMsg_01.brlyt)
 * @details Size 0x2C, verified (Hky_createHud). States 2 in, 3 score switch (L or R by
 * mSide), 4 out, 5-7 result variant.
 */
class RPHkyGoalMsgLyt {
public:
    enum Side {
        SIDE_LEFT = 0,
        SIDE_RIGHT = 1,
    };

    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void ShowGoal(s32 side);
    void StartResult();
    void PlayOut();
    bool IsOutDone() const;
    void SetScoreText(s32 side, s32 score) DECOMP_DONT_INLINE;

private:
    void CalcResultSwitch() DECOMP_DONT_INLINE;

private:
    enum {
        ANM_IN,       //!< inAll_00
        ANM_SWITCH_L, //!< switchScoreL_00
        ANM_SWITCH_R, //!< switchScoreR_00
        ANM_OUT,      //!< outAll_00

        ANM_NUM
    };

    RPSysLytAnmObj* GetSwitchAnmObj() const {
        return mpAnmObjs[mSide == SIDE_LEFT ? ANM_SWITCH_L : ANM_SWITCH_R];
    }

    s32 mState;                         // at 0x0
    s32 mSide;                          // at 0x4 scoring side
    wchar_t mScoreText[2][3];           // at 0x8
    RPSysLytResAccessor* mpResAccessor; // at 0x14
    RPSysLayout* mpLayout;              // at 0x18
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x1C
};

/**
 * @brief "Time up" message (airHockeyMsg_02.brlyt)
 * @details Size 0x18, verified (Hky_createHud).
 */
class RPHkyTimeUpMsgLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void PlayIn();
    void PlayOut();

private:
    enum {
        ANM_IN,  //!< inMsg_00
        ANM_OUT, //!< outMsg_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    u32 unk4;                           // at 0x4 [L] never accessed
    RPSysLytResAccessor* mpResAccessor; // at 0x8
    RPSysLayout* mpLayout;              // at 0xC
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x10
};

/**
 * @brief Two-player message (airHockeyMsg_03.brlyt, message 0xA2D)
 * @details Size 0x1C, verified (Hky_createHud). Shown at the start of a two-player game; the
 * switch animation repeats until it has played 6 times or the game reaches the result state.
 */
class RPHkyMsg03Lyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void PlayIn();

private:
    enum {
        ANM_IN,     //!< inMsg_00
        ANM_SWITCH, //!< switchMsg_00
        ANM_OUT,    //!< outMsg_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    s32 mSwitchCount;                   // at 0x4
    RPSysLytResAccessor* mpResAccessor; // at 0x8
    RPSysLayout* mpLayout;              // at 0xC
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x10
};

//! @}

//! @}

#endif
