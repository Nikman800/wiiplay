#ifndef RP_PARTY_FSH_SCORE_LAYOUT_H
#define RP_PARTY_FSH_SCORE_LAYOUT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshObject.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPSysKokeshiIcon;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;
class RPSysLytTextBox;

/**
 * @brief Per-player score and fish-count panels with the Mii icons (fishingScore_00.brlyt;
 * singleton Fsh_scoreLayout)
 * @details vtable 80358F68. Size 0xA4, verified (Fsh_ScoreLayout_new). One state machine per
 * player panel; the state functions work on panel mCurPlayer, which Calculate sets before
 * running each machine. The displayed score counts towards the real total in steps of 2.
 */
// Wii Play: direct bases avoid an extra intermediate-base destructor null check.
class RPFshScoreLayout : public RP_DEBUG_STUB_1,
                         public IRPSysHostIOSocket,
                         public IRPGrpDrawObject {
public:
    enum EState {
        STATE_HIDDEN,
        STATE_IN,
        STATE_SHOWN,
        STATE_GET, // fish count goes up
        STATE_OUT,

        STATE_NUM
    };

    enum {
        PLAYER_MAX = 2
    };

public:
    RPFshScoreLayout();
    virtual ~RPFshScoreLayout(); // at 0x8

    virtual void Create();    // at 0x34
    virtual void Reset();     // at 0x38
    virtual void Calculate(); // at 0x3C
    virtual void Exit();     // at 0x40

    virtual void UserDraw(); // at 0x44

    RPSysKokeshiIcon* GetIcon(s32 player) const {
        return mpIcons[player];
    }

    void AddFishCount(s32 player);
    void RequestShow(s32 player);
    void RequestHide(s32 player);
    void RequestHideAll();

    /**
     * @brief Points of the player's caught fish (bonus applied), clamped to [-999, 9999] [M]
     */
    s32 CalcTotalPoints(s32 player);
    /**
     * @brief Points of one fish: the type's points times sqrt(size), rounded up to sPointsUnit
     * (the "miss" fish gives its points as they are) [M]
     */
    static s32 CalcFishPoints(s32 type, f32 size);

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshScoreLayout* GetInstance() {
        return sInstance;
    }

private:
    void UpdateFishCount(s32 player);
    void UpdateTexts();

    void HiddenEnter();
    void HiddenCalc();
    void InEnter();
    void InCalc();
    void ShownEnter();
    void ShownCalc();
    void GetEnter();
    void GetCalc();
    void OutEnter();
    void OutCalc();

private:
    RPStateMachine<RPFshScoreLayout>* mpStateMachines[PLAYER_MAX]; // at 0x10
    s32 mPlayerNum;                    // at 0x18
    s32 mCurPlayer;                    // at 0x1C panel the state functions work on
    bool mShowRequest[PLAYER_MAX];     // at 0x20
    bool mHideRequest[PLAYER_MAX];     // at 0x22
    s32 mDispScore[PLAYER_MAX];        // at 0x24
    s32 mScoreDir[PLAYER_MAX];         // at 0x2C 1 counting up, -1 down, 0 settled
    s32 mDispFishCount[PLAYER_MAX];    // at 0x34
    s32 mFishCount[PLAYER_MAX];        // at 0x3C
    RPSysLayout* mpLayout;             // at 0x44
    nw4r::lyt::Pane* mpPanels[PLAYER_MAX];          // at 0x48 N_L_00 / N_R_00
    RPSysLytTextBox* mpScoreTexts[PLAYER_MAX];      // at 0x50 T_scoreL_00 / T_scoreR_00
    nw4r::lyt::Pane* mpScorePanes[PLAYER_MAX];      // at 0x58 N_scoreL_00 / N_scoreR_00
    RPSysLytTextBox* mpFishTexts[PLAYER_MAX][2];    // at 0x60 T_fishL_00/01, T_fishR_00/01
    nw4r::lyt::Pane* mpFacePanes[PLAYER_MAX][2];    // at 0x70 N_playerFace*
    /**
     * @brief Per player: inAll_00, outAll_00, Get_00
     */
    RPSysLytAnmObj* mpAnmObjs[PLAYER_MAX][3];       // at 0x80
    RPSysKokeshiIcon* mpIcons[PLAYER_MAX];          // at 0x98 96x96 Mii icons
    RPSysLytResAccessor* mpResAccessor;             // at 0xA0

    // The state functions are a local table in the constructor (8031FC58, .rodata)

    static s32 sScoreCountStep; // 8043B5C0 displayed score step per frame
    static s32 sPointsUnit;     // 8043B5C4 fish points are rounded up to this

    static RPFshScoreLayout* sInstance; // 8043C778
};

//! @}

#endif
