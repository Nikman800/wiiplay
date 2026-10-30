#ifndef RP_PARTY_BIL_LYT_H
#define RP_PARTY_BIL_LYT_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpDrawObject.h>
#include <Pack/RPParty/RPBilScene/RPBilObject.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_bil
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;

/*
 * The three layouts below (and RPBilHud) are layout objects that are also their own draw
 * object: IRPGrpDrawObject at 0x8 (mpNext, vtable at 0xC), appended to the renderer at
 * construction. vtable layout (CodeWarrior): RPLytObjBase part 0x8-0x18 (Draw not
 * overridden), IRPGrpDrawObject part at 0x1C (dtor and UserDraw thunks -8), then the
 * destructor at 0x30 and UserDraw at 0x34. RPBilGameMgr loads them with the scene-local
 * archive.
 */

/**
 * @brief Foul message window (billiard_01) (singleton RPBilMsgWindow::sInstance)
 * @details vtable 8035E4D0. Size 0x24, verified (Bil_MsgWindow_create). ShowFoul opens it in
 * the closed state and sets T_msgText_01 to message 0x8FD with the foul's text index
 * (4, 2, 3, 1 for foul types 0-3). 4-state machine from sStateFuncs.
 */
class RPBilMsgWindow : public RPLytObjBase, public IRPGrpDrawObject {
public:
    static RPBilMsgWindow* sInstance; // 8043CA40

    enum EState {
        STATE_CLOSED, //!< no functions
        STATE_OPENING,
        STATE_OPEN,
        STATE_CLOSE,

        STATE_NUM
    };

public:
    RPBilMsgWindow();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10

    virtual ~RPBilMsgWindow(); // at 0x30
    virtual void UserDraw();   // at 0x34

    void Close();
    void ShowFoul(s32 foul);

    RPStateMachine<RPBilMsgWindow>* GetStateMachine() const {
        return mpStateMachine;
    }

    static void CreateInstance();
    static void DestroyInstance();

private:
    void OpeningEnter();
    void OpeningCalc();
    void OpenEnter();
    void OpenCalc();
    void CloseEnter();
    void CloseCalc();

private:
    s32 mFoul;                                        // at 0x10
    RPStateMachine<RPBilMsgWindow>* mpStateMachine;   // at 0x14
    RPSysLayout* mpLayout;                            // at 0x18
    RPSysLytAnmObj* mpAnmObjs[2];                     // at 0x1C inAll, outAll

    /**
     * @brief State functions, 803278E0 (copied to the stack by Bil_MsgWindow_create)
     */
    static const RPBilStateFuncs<RPBilMsgWindow> sStateFuncs[STATE_NUM];
};

//! Ledger Bil_MsgSM (size 0x28)
typedef RPStateMachine<RPBilMsgWindow> RPBilMsgSeq;

/**
 * @brief Pull / push stroke guide messages (billiard_05) (singleton RPBilStrokeGuide::sInstance)
 * @details vtable 8035E628. Size 0x24, verified (Bil_StrokeGuide_create). SetMode shows the
 * pull message (0), the push message (1) or neither (-1, which also stops waitMsg_00).
 */
class RPBilStrokeGuide : public RPLytObjBase, public IRPGrpDrawObject {
public:
    static RPBilStrokeGuide* sInstance; // 8043CA58

    enum EMode {
        MODE_NONE = -1,
        MODE_PULL,
        MODE_PUSH,

        MODE_NUM
    };

public:
    RPBilStrokeGuide();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10

    virtual ~RPBilStrokeGuide(); // at 0x30
    virtual void UserDraw();     // at 0x34

    void SetMode(s32 mode);

    static void CreateInstance();
    static void DestroyInstance();

private:
    s32 mMode;                    // at 0x10 EMode
    RPSysLayout* mpLayout;        // at 0x14
    RPSysLytAnmObj* mpAnmObj;     // at 0x18 waitMsg_00
    nw4r::lyt::Pane* mpMsgPanes[MODE_NUM]; // at 0x1C N_msgL_00 (pull), N_msgR_00 (push)
};

/**
 * @brief Number tag following the targeted ball (billiard_07) (singleton RPBilBallNumberTag::sInstance)
 * @details vtable 8035E680. Size 0x54, verified (Bil_BallNumberTag_create). Show requests the
 * tag for a ball (or a switch to another ball while shown); the number_01-09 animations pick
 * the digit. 4-state machine from sStateFuncs (idle, in, shown, out).
 */
class RPBilBallNumberTag : public RPLytObjBase, public IRPGrpDrawObject {
public:
    static RPBilBallNumberTag* sInstance; // 8043CA68

    enum EState {
        STATE_IDLE,
        STATE_IN,
        STATE_SHOWN,
        STATE_OUT,

        STATE_NUM
    };

    enum {
        ANM_NUM = 11 //!< in, out, number_01 .. number_09
    };

public:
    RPBilBallNumberTag();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10

    virtual ~RPBilBallNumberTag(); // at 0x30
    virtual void UserDraw();       // at 0x34

    void Hide();
    void Show(s32 ball);

    static void CreateInstance();
    static void DestroyInstance();

private:
    void IdleEnter();
    void IdleCalc();
    void InEnter();
    void InCalc();
    void ShownEnter();
    void ShownCalc();
    void OutEnter();
    void OutCalc();

private:
    RPStateMachine<RPBilBallNumberTag>* mpStateMachine; // at 0x10
    bool mShowRequest;                                  // at 0x14
    bool mHideRequest;                                  // at 0x15
    bool mSwitchRequest;                                // at 0x16
    s32 mBall;                                          // at 0x18 -1 after Reset
    s32 mNextBall;                                      // at 0x1C
    RPSysLayout* mpLayout;                              // at 0x20
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM];                 // at 0x24
    nw4r::lyt::Pane* mpNumberPane;                      // at 0x50 N_number_00

    /**
     * @brief State functions, 80328118 (copied to the stack by Bil_BallNumberTag_create)
     */
    static const RPBilStateFuncs<RPBilBallNumberTag> sStateFuncs[STATE_NUM];
};

//! Ledger Bil_TagSM (size 0x28)
typedef RPStateMachine<RPBilBallNumberTag> RPBilTagSeq;

/**
 * @brief Score popup layout (billiard_06): "+N" score or a ball background
 * @details vtable 8035E5C0 (Draw overridden, no destructor). Size 0x28, verified
 * (Bil_ScorePopup_create). 4-state machine from sStateFuncs (closed, opening, open, close).
 */
class RPBilScorePopupLyt : public RPLytObjBase {
public:
    enum EState {
        STATE_CLOSED, //!< no functions
        STATE_OPENING,
        STATE_OPEN,
        STATE_CLOSE,

        STATE_NUM
    };

public:
    RPBilScorePopupLyt();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    void Hide();
    /**
     * @brief Opens from the closed state; type 1 hides N_score_00, otherwise T_scoreText_00
     * shows the signed score [H]
     */
    void Show(s32 type, s32 player, s32 score);

    // Read by RPBilScorePopup [H]
    bool IsClosed() const {
        return mpStateMachine->IsStateSettled(STATE_CLOSED);
    }
    bool IsOpen() const {
        return mpStateMachine->IsStateSettled(STATE_OPEN);
    }
    f32 GetBallBgY() const {
        return mBallBgY;
    }

private:
    void OpeningEnter();
    void OpeningCalc();
    void OpenEnter();
    void OpenCalc();
    void CloseEnter();
    void CloseCalc();

private:
    s32 mType;                                          // at 0x8
    s32 mPlayer;                                        // at 0xC
    f32 mBallBgY;                                       // at 0x10 N_ballBg_00 translate y
    RPStateMachine<RPBilScorePopupLyt>* mpStateMachine; // at 0x14
    RPSysLayout* mpLayout;                              // at 0x18
    RPSysLytAnmObj* mpAnmObjs[2];                       // at 0x1C inAll, outAll
    u32 unk24;                                          // at 0x24 [L]

    /**
     * @brief State functions, 80327E58 (copied to the stack by the constructor)
     */
    static const RPBilStateFuncs<RPBilScorePopupLyt> sStateFuncs[STATE_NUM];
};

//! Ledger Bil_PopupLytSM (size 0x28)
typedef RPStateMachine<RPBilScorePopupLyt> RPBilPopupLytSeq;

/**
 * @brief Score popup controller (singleton RPBilScorePopup::sInstance)
 * @details vtable 8035E5E0 (at 0x4). Size 0x18 [H] (Bil_ScorePopup_create allocates 0x18).
 * Start opens the layout; the 4-state machine (closed, open, show for 90 frames, close) runs
 * the ball icons (RPBilBallMgr::ShowPocketedIcons) between.
 */
class RPBilScorePopup : public RPBilDrawObj {
public:
    static RPBilScorePopup* sInstance; // 8043CA50

    enum EState {
        STATE_CLOSED, //!< no functions
        STATE_OPEN,
        STATE_SHOW,
        STATE_CLOSE,

        STATE_NUM
    };

public:
    RPBilScorePopup();
    virtual ~RPBilScorePopup(); // at 0x8

    virtual void UserDraw(); // at 0xC

    virtual void Load();      // at 0x14
    virtual void Reset();     // at 0x18
    virtual void Calculate(); // at 0x1C
    virtual void Exit();      // at 0x20 empty

    void Start(s32 type, s32 player);

    RPStateMachine<RPBilScorePopup>* GetStateMachine() const {
        return mpStateMachine;
    }

    static void CreateInstance();
    static void DestroyInstance();

private:
    void OpenEnter();
    void OpenCalc();
    void ShowEnter();
    void ShowCalc();
    void CloseEnter();
    void CloseCalc();

private:
    s32 mType;                                       // at 0x8
    s32 mPlayer;                                     // at 0xC player index (icons, A/B to close)
    RPStateMachine<RPBilScorePopup>* mpStateMachine; // at 0x10
    RPBilScorePopupLyt* mpLyt;                       // at 0x14

    /**
     * @brief State functions, 80327F48 (copied to the stack by Bil_ScorePopup_create)
     */
    static const RPBilStateFuncs<RPBilScorePopup> sStateFuncs[STATE_NUM];

public:
    static s32 sShowFrames;
};

//! Ledger Bil_ScorePopupSM (size 0x28)
typedef RPStateMachine<RPBilScorePopup> RPBilScorePopupSeq;

/**
 * @brief Hint text layout (billiard_04), faded by its alpha
 * @details vtable 8035E608 (Draw overridden, no destructor). Size 0x14, verified
 * (Bil_Hint_create). Calculate copies mAlpha to the pane's alpha. Both text boxes show
 * message 0x8FC.
 */
class RPBilHintLyt : public RPLytObjBase {
public:
    RPBilHintLyt();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    void SetAlpha(u8 alpha) {
        mAlpha = alpha;
    }

private:
    u8 mAlpha;                 // at 0x8
    RPSysLayout* mpLayout;     // at 0xC
    nw4r::lyt::Pane* mpPane;   // at 0x10 N_msg_00 [M]
};

/**
 * @brief Delayed hint controller (singleton RPBilHint::sInstance)
 * @details vtable 8035E660 (at 0x4). Size 0x18 [H] (Bil_Hint_create allocates 0x18).
 * Request arms the hint; after 180 idle frames it fades in (20 frames), stays 240 frames and
 * fades out (40 frames). 4-state machine from sStateFuncs.
 */
class RPBilHint : public RPBilDrawObj {
public:
    static RPBilHint* sInstance; // 8043CA60

    enum EState {
        STATE_IDLE,
        STATE_FADE_IN,
        STATE_SHOWN,
        STATE_FADE_OUT,

        STATE_NUM
    };

public:
    RPBilHint();
    virtual ~RPBilHint(); // at 0x8

    virtual void UserDraw(); // at 0xC

    virtual void Load();      // at 0x14
    virtual void Reset();     // at 0x18
    virtual void Calculate(); // at 0x1C

    void Hide();
    void Request(bool restart);
    void SetIdleTimer(s32 frames) {
        mIdleTimer = frames;
    }

    static void CreateInstance();
    static void DestroyInstance();

private:
    void IdleEnter();
    void IdleCalc();
    void FadeInEnter();
    void FadeInCalc();
    void ShownEnter();
    void ShownCalc();
    void FadeOutEnter();
    void FadeOutCalc();

private:
    bool mRequested;                           // at 0x8
    s32 mIdleTimer;                            // at 0xC [M]
    RPStateMachine<RPBilHint>* mpStateMachine; // at 0x10
    RPBilHintLyt* mpLyt;                       // at 0x14

    /**
     * @brief State functions, 803280B8 (copied to the stack by Bil_Hint_create)
     */
    static const RPBilStateFuncs<RPBilHint> sStateFuncs[STATE_NUM];

public:
    static s32 sDelayFrames;
    static s32 sFadeInFrames;
    static s32 sFadeOutFrames;
    static s32 sShownFrames;
};

//! Ledger Bil_HintSM (size 0x28)
typedef RPStateMachine<RPBilHint> RPBilHintSeq;

//! @}

#endif
