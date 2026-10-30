#ifndef RP_PARTY_BIL_RESULT_H
#define RP_PARTY_BIL_RESULT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPBilScene/RPBilObject.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_bil
//! @{

// Forward declarations
struct RPBilPlayerResult;
class RPBilResult;
class RPSysLayout;
class RPSysLytAnmObj;

/**
 * @brief One player's end-of-game score breakdown (two in RPBilResult)
 * @details Size 0x20. Filled by RPBilResult::ComputeScores from the player statistics, each
 * value clamped by a min/max pair of the table at 803273C0. Name is a project convention.
 */
struct RPBilPlayerResult {
    s32 mScore;        // at 0x0 pocketed numbers (two players), 0 for one player
    s32 mShotCount;    // at 0x4 one player only
    s32 mShotBonus;    // at 0x8 (27 - shots) * 3, one player only
    s32 mFoulCount;    // at 0xC
    s32 mFoulPenalty;  // at 0x10 fouls * -3
    s32 mMaxStat;      // at 0x14 [L] RPBilPlayer::mMaxStat
    s32 unk18;         // at 0x18 [L] always 0
    s32 mTotal;        // at 0x1C score + bonus + penalty + unk18
};

/**
 * @brief Billiards result layout (billiard_02: score sheet for one or two players)
 * @details vtable 8035E508. Size 0x6C, verified (Bil_Result_ctor). Two 5-state machines (one
 * per screen half: none, show, shown, hide, done; the constructor's local state table is
 * 803279C0), five animations (inAll, switch1pSum, switch2pSum, outUpperDisp, outDownDisp) and
 * the display panes. Code in RPBilResult_2.cpp.
 */
class RPBilResultLyt : public RPLytObjBase {
public:
    enum EState {
        STATE_NONE,
        STATE_SHOW,
        STATE_SHOWN,
        STATE_HIDE,
        STATE_DONE, //!< no functions

        STATE_NUM
    };

    enum {
        SIDE_NUM = 2,
        ANM_NUM = 5,
        FACE_PANE_NUM = 6
    };

public:
    RPBilResultLyt();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    /**
     * @brief Starts hiding one screen half (unless it is already hiding or hidden) [H]
     */
    void HideSide(s32 side);
    /**
     * @brief Both halves are past the show state [H]
     */
    bool IsShowDone() const;
    /**
     * @brief Shows the score sheet for 1 or 2 players and writes every RPBilPlayerResult value
     * into the text boxes [M]
     */
    void ShowScores(s32 playerNum);

private:
    friend class RPBilResult;

    void ShowEnter();
    void ShowCalc();
    void ShownEnter();
    void ShownCalc();
    void HideEnter();
    void HideCalc();

private:
    RPStateMachine<RPBilResultLyt>* mpStateMachines[SIDE_NUM]; // at 0x8 one per screen half
    s32 mSide;                                   // at 0x10 [L] half whose state functions run
    s32 mPlayerNum;                              // at 0x14 [L] set by ShowScores
    RPSysLayout* mpLayout;                       // at 0x18
    RPSysLytAnmObj* mpAnmObjs[ANM_NUM];          // at 0x1C
    nw4r::lyt::Pane* mpNomDisplayPane;           // at 0x30 N_nomDisplay_00
    nw4r::lyt::Pane* mpDisplayPanes[2];          // at 0x34 N_1pDisplay_00, N_2pDisplay_00
    nw4r::lyt::Pane* mpNomSidePanes[SIDE_NUM];   // at 0x3C N_nomDisplayU/D_00
    nw4r::lyt::Pane* mpSidePanes[2][SIDE_NUM];   // at 0x44 N_1p/2pDisplayU/D_00 [player][half]
    nw4r::lyt::Pane* mpNomFacePanes[2];          // at 0x54 N_nomFacePict_00/01
    nw4r::lyt::Pane* mpFacePanes[2][2];          // at 0x5C N_1p/2pFacePict_00/01
};

//! Ledger Bil_ResultLytSM (size 0x28)
typedef RPStateMachine<RPBilResultLyt> RPBilResultLytSeq;

/**
 * @brief Billiards end-of-game flow (singleton RPBilResult::sInstance)
 * @details vtable 8035E270. Size 0x60, verified (Bil_Result_create). 8-state machine from
 * sStateFuncs: summary (result layout), wait, branch, then the single-player record
 * (RPMedalResult) or the two-player winner (RPVsWinner), then the end menu. ComputeScores
 * (from RPBilGameMgr's game-over state) fills mResults and mWinner and starts the summary.
 * Wii Play: derives from the three bases directly (an RPBilScreen level would add a null check
 * to the destructor); the slot layout is RPBilScreen's.
 */
class RPBilResult : public RP_DEBUG_STUB_1,
                    public IRPSysHostIOSocket,
                    public IRPGrpDrawObject {
public:
    static RPBilResult* sInstance; // 8043CA08

    enum EState {
        STATE_NONE,
        STATE_SUMMARY,
        STATE_WAIT,
        STATE_BRANCH,
        STATE_1P_RECORD,
        STATE_WINNER,
        STATE_END_MENU,
        STATE_DONE, //!< no functions

        STATE_NUM
    };

public:
    RPBilResult();
    virtual ~RPBilResult(); // at 0x8

    virtual void Load();      // at 0x34
    virtual void Reset();     // at 0x38
    virtual void Calculate(); // at 0x3C
    virtual void Exit();      // at 0x40

    virtual void UserDraw(); // at 0x44

    /**
     * @brief Winning player, -1 on a draw or before the end [H]
     */
    s32 GetWinner() const;
    const RPBilPlayerResult* GetPlayerResult(s32 player) const;
    void ComputeScores();

    RPStateMachine<RPBilResult>* GetStateMachine() const {
        return mpStateMachine;
    }
    // The end menu's choice was "retry" (EndMenuCalc; RPBilGameMgr restarts billiards) [M]
    bool IsRetry() const {
        return unk18;
    }

    static void CreateInstance();
    static void DestroyInstance();

private:
    friend class RPStateMachine<RPBilResult>;

    void SummaryEnter();
    void SummaryCalc();
    void WaitEnter();
    void WaitCalc();
    void BranchEnter();
    void BranchCalc();
    void Record1pEnter();
    void Record1pCalc();
    void WinnerEnter();
    void WinnerCalc();
    void EndMenuEnter();
    void EndMenuCalc();

private:
    RPStateMachine<RPBilResult>* mpStateMachine; // at 0x10
    s32 mWinner;                                 // at 0x14
    bool unk18;                                  // at 0x18 [L] set by EndMenuCalc
    RPBilPlayerResult mResults[2];               // at 0x1C
    RPBilResultLyt* mpLyt;                       // at 0x5C

    /**
     * @brief State functions, 80327408 (copied to the stack by the constructor)
     */
    static const RPBilStateFuncs<RPBilResult> sStateFuncs[STATE_NUM];
};

//! Ledger Bil_ResultSM (size 0x28)
typedef RPStateMachine<RPBilResult> RPBilResultSeq;

//! @}

#endif
