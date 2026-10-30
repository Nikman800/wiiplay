#ifndef RP_PARTY_WLY_LYT_H
#define RP_PARTY_WLY_LYT_H
#include <Pack/types_pack.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_wly
//! @{

// Forward declarations
class RPSysKokeshiIcon;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;

/**
 * @name Find Mii layouts (wally.arc)
 * @details Plain classes owned by RPWlyLayoutSet: no vtable and no constructor (allocated
 * with operator new only; Init zeroes the members). Create builds the archive accessor, the
 * layout and its animation objects and sets the message texts (party_message.bmg); Calculate
 * advances a small state machine (mState: 0 init, 1 idle, then one state per playing
 * animation) and calls RPSysLayout::calc. Sizes verified (Wly_LayoutSet_createGlobal).
 */
//! @{

/**
 * @brief "Start" countdown (wallyStartCount_00.brlyt, message 0x962). Size 0x20.
 */
class RPWlyStartCountLyt {
public:
    void Create();
    void Init();
    void Reset();
    void Calculate();
    void Draw();
    void Destroy();

private:
    s32 mState;                         // at 0x0
    s32 mTimer;                         // at 0x4
    RPSysLytResAccessor* mpResAccessor; // at 0x8
    RPSysLayout* mpLayout;              // at 0xC
    RPSysLytAnmObj* mpCountAnmObjs[3];  // at 0x10 count_00..02
    RPSysLytAnmObj* mpStarAnmObj;       // at 0x1C star_00
};

/**
 * @brief Stage number, player scores and Mii icons (wallyDisplay_00.brlyt). Size 0x58.
 */
class RPWlyDisplayLyt {
public:
    struct PlayerIcon {
        RPSysKokeshiIcon* mpIcon;  // at 0x0 0x60 x 0x60
        nw4r::lyt::Pane* mpPane;   // at 0x4
        nw4r::lyt::Pane* mpPane2;  // at 0x8 [M]
    };

public:
    void Create();
    void Init();
    void Reset();
    void Calculate();
    void Draw();
    void Destroy();

    void PlayLevelIn();
    void PlayLevelOut();
    void PlayEndIn();
    void PlayWinnerScore();
    void SetPlayerScore(s32 player, s32 score);
    void StartScoreResult();
    void UpdateScoreResult();

public:
    s32 mState;                          // at 0x0
private:
    s32 mTimer;                          // at 0x4
    wchar_t mStageText[3];               // at 0x8 formatted stage number (3 digits)
    wchar_t mScoreText[2][4];            // at 0xE per player, full-width digits
    RPSysLytResAccessor* mpResAccessor;  // at 0x20
    RPSysLayout* mpLayout;               // at 0x24
    RPSysLytAnmObj* mpInLevelAnmObj;     // at 0x28 inLevel1_00
    RPSysLytAnmObj* mpOutLevelAnmObj;    // at 0x2C outLevel_00
    RPSysLytAnmObj* mpInLevelEndAnmObj;  // at 0x30 inLevel2End_00
    RPSysLytAnmObj* mpOutEndAnmObj;      // at 0x34 outEnd_00
    RPSysLytAnmObj* mpSwitchAnmObjs[2];  // at 0x38 switch1p_00, switch2p_00
    PlayerIcon mIcons[2];                // at 0x40
};

/**
 * @brief Final "end" message on the display layout (wallyDisplay_00.brlyt, message 0x967).
 * Size 0x50.
 */
class RPWlyDisplayEndLyt {
public:
    void Create();
    void Init();
    void Reset();
    void Calculate();
    void Draw();
    void Destroy();

    void PlayIn();
    void PlayOut();

private:
    s32 mState;                          // at 0x0
    s32 mTimer;                          // at 0x4
    char unk8[0x20 - 0x8];               // at 0x8
    RPSysLytResAccessor* mpResAccessor;  // at 0x20
    RPSysLayout* mpLayout;               // at 0x24
    RPSysLytAnmObj* mpInLevelAnmObj;     // at 0x28 inLevel1_00
    RPSysLytAnmObj* mpOutLevelAnmObj;    // at 0x2C outLevel_00
    RPSysLytAnmObj* mpInEndAnmObj;       // at 0x30 inLevel2End_00
    RPSysLytAnmObj* mpOutEndAnmObj;      // at 0x34 outEnd_00
    char unk38[0x50 - 0x38];             // at 0x38 [L] never accessed
};

/**
 * @brief Remaining time (wallyTime_00.brlyt, message 0x968). Size 0x38.
 * @details Shows ceil(timer / 60) as full-width digits, feeds RPWlyTimerSe and flashes at
 * 5, 4, 3, 2, 1 and 0 seconds; plus / minus animations for time bonuses.
 */
class RPWlyTimerLyt {
public:
    void Create();
    void Init();
    void Reset();
    void Calculate();
    void Draw();
    void Destroy();

    void PlayIn();
    void PlayOut();
    void PlayPlus();
    void PlayMinus();
    void UpdateWarnAnim();

private:
    s32 mState;                          // at 0x0
    s32 mTimer;                          // at 0x4
    nw4r::math::VEC2 mScale;             // at 0x8 target scale of N_timeD_00
    u32 unk10;                           // at 0x10
    wchar_t mNumberText[4];              // at 0x14
    RPSysLytResAccessor* mpResAccessor;  // at 0x1C
    RPSysLayout* mpLayout;               // at 0x20
    RPSysLytAnmObj* mpInAnmObj;          // at 0x24 inTime_00
    RPSysLytAnmObj* mpOutAnmObj;         // at 0x28 outTime_00
    RPSysLytAnmObj* mpSwitchAnmObj;      // at 0x2C switchTime_00
    RPSysLytAnmObj* mpPlusAnmObj;        // at 0x30 switchPlus_00
    RPSysLytAnmObj* mpMinusAnmObj;       // at 0x34 switchMinus_00
};

/**
 * @brief Question "find ..." (wallyQue_00.brlyt, message 0x960 by question type). Size 0x1C.
 */
class RPWlyQuestionLyt {
public:
    void Create();
    void Init();
    void Reset();
    void Calculate();
    void Draw();
    void Destroy();

    void ShowIn();
    void StartOut();
    bool IsOutDone() const;

private:
    s32 mState;                          // at 0x0
    bool mOutLocked;                     // at 0x4 [M] StartOut does nothing while set
    RPSysLytResAccessor* mpResAccessor;  // at 0x8
    RPSysLayout* mpLayout;               // at 0xC
    RPSysLytAnmObj* mpInAnmObj;          // at 0x10 inQue_00
    RPSysLytAnmObj* mpSwitchAnmObj;      // at 0x14 switchFaceWin_00
    RPSysLytAnmObj* mpOutAnmObj;         // at 0x18 outQue_00
};

/**
 * @brief Stage message (wallyMsg_00.brlyt, message 0x961). Size 0x1C.
 */
class RPWlyMessageLyt {
public:
    void Create();
    void Init();
    void Reset();
    void Calculate();
    void Draw();
    void Destroy();

    void Show();

private:
    s32 mState;                          // at 0x0
    s32 mTimer;                          // at 0x4
    RPSysLytResAccessor* mpResAccessor;  // at 0x8
    RPSysLayout* mpLayout;               // at 0xC
    RPSysLytAnmObj* mpInAnmObj;          // at 0x10 inMsg_00
    RPSysLytAnmObj* mpWaitAnmObj;        // at 0x14 waitMsg_00
    RPSysLytAnmObj* mpOutAnmObj;         // at 0x18 outMsg_00
};

/**
 * @brief Points (or time bonus in single player) popping up over a found avatar
 * (wallyPoint_00.brlyt / wallyTimePoint_00.brlyt, message 0x963). Size 0x44 (allocated in
 * RPWlyPointPopupSet::Init).
 */
class RPWlyPointPopup {
public:
    void Create();
    void Init();
    void Reset();
    void Calculate();
    void Draw();
    void Destroy();

    void Show(s32 player, s32 avatarIdx, s32 points);
    void UpdatePos();

    s32 GetState() const {
        return mState;
    }

private:
    s32 mMode;                           // at 0x0 RPWlyGameMgr::mMode (0: time popup)
    s32 mTimer;                          // at 0x4
    s32 mState;                          // at 0x8 1 idle, 2 showing
    s32 mPlayer;                         // at 0xC
    s32 mAvatarIdx;                      // at 0x10 -1 after Init
    s32 mPoints;                         // at 0x14
    wchar_t mNumberText[4];              // at 0x18
    RPSysLytResAccessor* mpResAccessor;  // at 0x20
    RPSysLayout* mpLayout;               // at 0x24
    RPSysLytAnmObj* mpAnmObj;            // at 0x28 inOutAll_00
    nw4r::lyt::TextBox* mpTextBoxes[4];  // at 0x2C T_numberU_00/01, T_numberD_00/01
    nw4r::lyt::Pane* mpCenterPanes[2];   // at 0x3C N_centerU_00 (plus), N_centerD_00 (minus)
};

/**
 * @brief Ring of 20 point popups. Size 0x54.
 */
class RPWlyPointPopupSet {
public:
    static const int POPUP_NUM = 20;

public:
    void Create();
    void Init();
    void Reset();
    void Calculate();
    void Draw();
    void Destroy();

    void ShowNext(s32 player, s32 avatarIdx, s32 points);
    bool AreAllIdle() const;

private:
    s32 mNext;                             // at 0x0
    RPWlyPointPopup* mpPopups[POPUP_NUM];  // at 0x4
};

/**
 * @brief Single-player end score with the player's Mii icon (wallyEndScore_00.brlyt,
 * message 0x969). Size 0x20.
 */
class RPWlyEndScoreLyt {
public:
    void Create();
    void Init();
    void Reset();
    void Calculate();
    void Draw();
    void Destroy();

    void Show();
    void PlayOut();

private:
    s32 mState;                          // at 0x0
    RPSysLytResAccessor* mpResAccessor;  // at 0x4
    RPSysLayout* mpLayout;               // at 0x8
    RPSysLytAnmObj* mpInAnmObj;          // at 0xC inScore_00
    RPSysLytAnmObj* mpOutAnmObj;         // at 0x10 outScore_00
    RPSysKokeshiIcon* mpIcon;            // at 0x14 0x60 x 0x60
    nw4r::lyt::Pane* mpIconPanes[2];     // at 0x18 [M]
};

/**
 * @brief "Complete" message (wallyComp_00.brlyt, message 0x96B). Size 0x14.
 */
class RPWlyCompleteLyt {
public:
    void Create();
    void Init();
    void Reset();
    void Calculate();
    void Draw();
    void Destroy();

    void ShowAllClear();
    void PlayOut();

private:
    s32 mState;                          // at 0x0
    RPSysLytResAccessor* mpResAccessor;  // at 0x4
    RPSysLayout* mpLayout;               // at 0x8
    RPSysLytAnmObj* mpInAnmObj;          // at 0xC in_00
    RPSysLytAnmObj* mpOutAnmObj;         // at 0x10 out_00
};

//! @}

/**
 * @brief All Find Mii layouts (singleton RPWlyLayoutSet::sInstance)
 * @details vtable 8035C5D0 (destructor only). Size 0x28, verified (Wly_LayoutSet_createGlobal,
 * which also allocates every layout). Draw draws the display layout on view 5 and the others
 * on view 3 (draw step 5).
 */
class RPWlyLayoutSet {
public:
    RPWlyLayoutSet() {
        mpStartCount = new RPWlyStartCountLyt;
        mpDisplay = new RPWlyDisplayLyt;
        mpDisplayEnd = new RPWlyDisplayEndLyt;
        mpTimer = new RPWlyTimerLyt;
        mpQuestion = new RPWlyQuestionLyt;
        mpMessage = new RPWlyMessageLyt;
        mpPointPopups = new RPWlyPointPopupSet;
        mpEndScore = new RPWlyEndScoreLyt;
        mpComplete = new RPWlyCompleteLyt;
    }

    virtual ~RPWlyLayoutSet(); // at 0x8

    void Create();
    void Init();
    void Reset();
    void Calculate();
    void Draw();
    void Destroy();

    static void CreateInstance();  // Wly_LayoutSet_createGlobal
    static void DestroyInstance(); // Wly_LayoutSet_destroyGlobal

private:
    RPWlyStartCountLyt* mpStartCount;   // at 0x4
public:
    RPWlyDisplayLyt* mpDisplay;         // at 0x8
    RPWlyDisplayEndLyt* mpDisplayEnd;   // at 0xC
    RPWlyTimerLyt* mpTimer;             // at 0x10
    RPWlyQuestionLyt* mpQuestion;       // at 0x14
    RPWlyMessageLyt* mpMessage;         // at 0x18
    RPWlyPointPopupSet* mpPointPopups;  // at 0x1C
    RPWlyEndScoreLyt* mpEndScore;       // at 0x20
    RPWlyCompleteLyt* mpComplete;       // at 0x24

public:
    static RPWlyLayoutSet* sInstance; // 8043C960
};

//! @}

#endif
