#ifndef RP_PARTY_PNP_LYT_H
#define RP_PARTY_PNP_LYT_H
#include <Pack/types_pack.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_pnp
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;
class RPSysKokeshiIcon;

/**
 * @name Table Tennis layouts (pingpong.arc)
 * @details Plain structs owned by RPPnpHud, like the Laser Hockey ones: no vtable and no
 * constructor (Create zeroes the members), Load builds the layout and animation objects,
 * Calculate runs a small state machine (mState: 0 init, 1 idle, then one state per animation).
 * Numbers are swprintf'd and turned into full-width digits (each character - 0x120).
 * All sizes verified against Pnp_createHud.
 */
//! @{

/**
 * @brief "1P / 2P" balloons with the Mii icons (pingpongBalloon_00.brlyt, one layout per player)
 * @details Size 0x70. Each player's layout only draws that player's icon.
 */
class RPPnpBalloonLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void Show();

private:
    struct Icon {
        RPSysKokeshiIcon* mpIcon;  // at 0x0 one per player (0x60 x 0x60)
        nw4r::lyt::Pane* mpPane;   // at 0x4 drawn with alpha mode 0
        nw4r::lyt::Pane* mpPane2;  // at 0x8 drawn with alpha mode 1
    };

    s32 mStates[2];                          // at 0x0
    bool mOutRequested[2];                   // at 0x8
    RPSysLytResAccessor* mpResAccessor;      // at 0xC
    RPSysLayout* mpLayouts[2];               // at 0x10
    enum {
        ANM_IN,      //!< inAll_00
        ANM_UP_DOWN, //!< UpDownAll_00
        ANM_OUT,     //!< outAll_00

        ANM_NUM
    };

    RPSysLytAnmObj* mpAnmObjs[2][ANM_NUM];   // at 0x18
    nw4r::lyt::Pane* mpCenterPanes[2][2];    // at 0x30 N_1pCenter_00, N_2pCenter_00
    Icon mIcons[2][2];                       // at 0x40 [layout][slot], icons of the layout's player
};

/**
 * @brief Score board for the two-player game (pingpongScore_00.brlyt), drawn on views 2/3
 * @details Size 0x3C. Marks the server ("A") and receiver ("U") per side.
 */
class RPPnpScoreLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void PlayIn();
    void PlayOut();
    void PlayPointSwitch(s32 side);
    void SetScore(s32 side, s32 score);

private:
    void CalcSwitch();

private:
    enum {
        ANM_IN,         //!< inAll_00
        ANM_SWITCH_1P,  //!< switch1P_00
        ANM_SWITCH_2P,  //!< switch2P_00
        ANM_OUT,        //!< outAll_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    s32 mTimer;                         // at 0x4
    bool mIsOut;                        // at 0x8 PlayIn only runs after PlayOut
    s32 mSide;                          // at 0xC side of the last point
    wchar_t mScoreText[2][3];           // at 0x10
    RPSysLytResAccessor* mpResAccessor; // at 0x1C
    RPSysLayout* mpLayout;              // at 0x20
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x24
    nw4r::lyt::Pane* mpServerPanes[2];  // at 0x34 moved to the serving side (y = +-172)
};

/**
 * @brief "Server" marker (pingpongServer_00.brlyt, message 0x8DE), two players only
 * @details Size 0x2C. Show/Hide queue an animation in mPending that ApplyPending starts once
 * the current one is done.
 */
class RPPnpServerLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    /**
     * @brief Shows the marker on side's half, following player's racket
     */
    void Show(s32 side, s32 player);
    void Hide();

private:
    void ApplyPending();

private:
    enum {
        ANM_IN,      //!< inAll_00
        ANM_UP_DOWN, //!< upDownAll_00
        ANM_OUT,     //!< outAll_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0 previous animation
    s32 mAnim;                          // at 0x4 0 init, 1 none, 2 in, 3 out
    s32 mPending;                       // at 0x8 1 none
    s32 mSide;                          // at 0xC 0 upper (N_serverWU_00), else lower
    s32 mPlayer;                        // at 0x10 the layout follows this player
    RPSysLytResAccessor* mpResAccessor; // at 0x14
    RPSysLayout* mpLayout;              // at 0x18
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x1C
    nw4r::lyt::Pane* mpRootPane;        // at 0x28
};

/**
 * @brief Arrows over the players (pingpongArrow_00.brlyt, one layout per player)
 * @details Size 0x30. A player's arrow is drawn while RPPnpPlayer::mHidden is clear.
 */
class RPPnpArrowLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void Show();

private:
    bool mShown;                        // at 0x0
    RPSysLytResAccessor* mpResAccessor; // at 0x4
    RPSysLayout* mpLayouts[2];          // at 0x8
    enum {
        ANM_UP_DOWN, //!< arrowUpDown_00
        ANM_BLUE,    //!< arrowBlue_00
        ANM_RED,     //!< arrowRed_00

        ANM_NUM
    };

    RPSysLytAnmObj* mpAnmObjs[2][ANM_NUM]; // at 0x10
    nw4r::lyt::Pane* mpRootPanes[2];    // at 0x28
};

/**
 * @brief Rally counter (pingpongCount_00.brlyt), single player
 * @details Size 0x28.
 */
class RPPnpCountLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void PlayIn();
    void PlayOut();
    void SetCount(s32 count);
    void SetNumber(s32 number);

private:
    enum {
        ANM_IN,     //!< inAll_00
        ANM_SWITCH, //!< switch_00
        ANM_OUT,    //!< outAll_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    wchar_t mNumberText[6];             // at 0x4
    s32 mCount;                         // at 0x10
    RPSysLytResAccessor* mpResAccessor; // at 0x14
    RPSysLayout* mpLayout;              // at 0x18
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x1C
};

/**
 * @brief Rally result (pingpongResult_00.brlyt)
 * @details Size 0x20.
 */
class RPPnpResultLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void Show();
    void PlayOut();
    /**
     * @brief Texts for RPPnpGame's rally count (unit and number panes by digit count)
     */
    void SetRallyText();

private:
    enum {
        ANM_IN,  //!< inAll_00
        ANM_OUT, //!< outAll_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    wchar_t mRallyText[6];              // at 0x4
    RPSysLytResAccessor* mpResAccessor; // at 0x10
    RPSysLayout* mpLayout;              // at 0x14
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x18
};

/**
 * @brief Start message (pingpongMsg_00.brlyt)
 * @details Size 0x14.
 */
class RPPnpStartMsgLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void Show();
    void PlayOut();

private:
    enum {
        ANM_IN,  //!< inAll_00
        ANM_OUT, //!< outAll_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    RPSysLytResAccessor* mpResAccessor; // at 0x4
    RPSysLayout* mpLayout;              // at 0x8
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0xC
};

/**
 * @brief Help / hint message (pingpongMsg_01.brlyt)
 * @details Size 0x28. SetMessage shows a message at once if idle, otherwise queues it.
 */
class RPPnpHelpMsgLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void SetMessage(s32 message);
    void RequestHide();

private:
    void ApplyText();

private:
    enum {
        ANM_IN,   //!< inAll_00
        ANM_WAIT, //!< waitAll_00
        ANM_OUT,  //!< outAll_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0 previous animation
    s32 mAnim;                          // at 0x4 0 init, 1 none, 2 in, 3 wait, 4 out
    s32 mPending;                       // at 0x8 1 none, 2 show, 4 hide
    s32 mMessage;                       // at 0xC 4 = none
    s32 mPrevMessage;                   // at 0x10
    RPSysLytResAccessor* mpResAccessor; // at 0x14
    RPSysLayout* mpLayout;              // at 0x18
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x1C
};

/**
 * @brief Fault message (pingpongMsg_02.brlyt, message 0x8CD; text by the ball state)
 * @details Size 0x18.
 */
class RPPnpFaultMsgLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void Show();
    bool IsDone() const;

private:
    enum {
        ANM_IN,  //!< inAll_00
        ANM_OUT, //!< outAll_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    s32 mTimer;                         // at 0x4
    RPSysLytResAccessor* mpResAccessor; // at 0x8
    RPSysLayout* mpLayout;              // at 0xC
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x10
};

/**
 * @brief "Serve here / over there" message (pingpongMsg_03.brlyt)
 * @details Size 0x1C.
 */
class RPPnpServeMsgLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void Show(s32 side);
    bool IsDone() const;

private:
    enum {
        ANM_IN,  //!< inAll_00
        ANM_OUT, //!< outAll_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    s32 mSide;                          // at 0x4 0 N_msgHere_00, else N_msgOver_00; 2 initially
    s32 mTimer;                         // at 0x8
    RPSysLytResAccessor* mpResAccessor; // at 0xC
    RPSysLayout* mpLayout;              // at 0x10
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x14
};

/**
 * @brief Point message for the scoring side (pingpongMsg_04.brlyt), two players
 * @details Size 0x1C.
 */
class RPPnpPointMsgLyt {
public:
    void Create();
    void Load();
    void Reset();
    void Calculate();
    void Draw();
    void Exit();

    void Show(s32 side);
    bool IsDone() const;

private:
    enum {
        ANM_IN,  //!< inAll_00
        ANM_OUT, //!< outAll_00

        ANM_NUM
    };

    s32 mState;                         // at 0x0
    s32 mSide;                          // at 0x4 2 initially
    s32 mTimer;                         // at 0x8
    RPSysLytResAccessor* mpResAccessor; // at 0xC
    RPSysLayout* mpLayout;              // at 0x10
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM]; // at 0x14
};

//! @}

//! @}

#endif
