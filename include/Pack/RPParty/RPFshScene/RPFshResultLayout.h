#ifndef RP_PARTY_FSH_RESULT_LAYOUT_H
#define RP_PARTY_FSH_RESULT_LAYOUT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshObject.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPFshResultFish;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;

/**
 * @brief Fishing result screen (fishingResult_00/01.brlyt; singleton Fsh_resultLayout)
 * @details vtable 80358E38. Size 0xE84, verified (Fsh_ResultLayout_new). Holds one
 * RPFshResultFish and one fishingResult_01 point label per caught-fish slot (2 players x 6 types
 * x 50, the labels 2 x 50). The state runs inline from sStateFuncs (enter/calc pairs): when
 * mIsChanging, mNextState moves to mState, mStateTimer is cleared and the enter is called.
 * RPFshGame changes the scene once mState reaches STATE_DONE (to scene 0x10 when mRetry, else
 * 0x1B).
 */
// Wii Play: direct bases avoid an extra intermediate-base destructor null check.
class RPFshResultLayout : public RP_DEBUG_STUB_1,
                         public IRPSysHostIOSocket,
                         public IRPGrpDrawObject {
public:
    enum EState {
        STATE_NONE,
        STATE_CAMERA_MOVE,
        STATE_INTRO,
        STATE_COUNT_POINTS,
        STATE_JUDGE,
        STATE_SINGLE_END,
        STATE_VERSUS_END,
        STATE_FINISH,
        STATE_DONE,

        STATE_NUM
    };

    enum {
        PLAYER_MAX = 2,
        FISH_TYPE_NUM = 6,
        FISH_MAX = 50
    };

public:
    RPFshResultLayout();
    virtual ~RPFshResultLayout(); // at 0x8

    /**
     * @brief Creates the result fish, fishingResult_00 (3 animations, panes, player names) and
     * 2 x 50 fishingResult_01 point labels [H]
     */
    virtual void Create();    // at 0x34
    virtual void Reset();     // at 0x38
    virtual void Calculate(); // at 0x3C
    virtual void Exit();      // at 0x40 empty, out of line

    /**
     * @brief IRPGrpDrawObject::UserDraw override (draw pass 4 of view 5) [M]
     */
    virtual void UserDraw(); // at 0x44

    /**
     * @brief Requests STATE_CAMERA_MOVE [H]
     */
    void Start();
    s32 GetState() const {
        return mState;
    }
    /**
     * @brief Past the camera move (the pond stops) [H]
     */
    bool IsShowing() const {
        return mState != STATE_NONE && mState != STATE_CAMERA_MOVE;
    }
    bool IsRetry() const {
        return mRetry;
    }

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshResultLayout* GetInstance() {
        return sInstance;
    }

private:
    void DrawFishLabels();
    f32 GetScrollSpeed() const;

    void CameraMoveEnter();
    void CameraMoveCalc();
    void IntroEnter();
    void IntroCalc();
    void CountPointsEnter();
    void CountPointsCalc();
    void JudgeEnter();
    void JudgeCalc();
    void SingleEndEnter();
    void SingleEndCalc();
    void VersusEndEnter();
    void VersusEndCalc();
    void FinishEnter();
    void FinishCalc();

private:
    s32 mState;                 // at 0x10 EState
    s32 mNextState;             // at 0x14
    bool mIsChanging;           // at 0x18
    s32 mStateTimer;            // at 0x1C
    bool mRetry;                // at 0x20 [M] chosen in the result menu
    s32 mBestScore;             // at 0x24 [M] -10000 before the judge
    s32 mWinner;                // at 0x28 [M] -1 on a draw
    bool mCountDone[PLAYER_MAX]; // at 0x2C [M]
    f32 mScroll[PLAYER_MAX];    // at 0x30 [M] clamped to mScrollMax
    RPFshResultFish* mpFish[PLAYER_MAX][FISH_TYPE_NUM][FISH_MAX]; // at 0x38
    RPSysLayout* mpLayout;      // at 0x998 fishingResult_00
    /**
     * @brief inAll_00, switchScore_00, outAll_00
     */
    RPSysLytAnmObj* mpAnmObjs[3];             // at 0x99C
    nw4r::lyt::Pane* mpPointPanes[PLAYER_MAX]; // at 0x9A8 N_pointL_00 / N_pointR_00
    f32 mScrollMax[PLAYER_MAX];                // at 0x9B0 [M] point pane height / screen scale
    nw4r::lyt::Pane* mpSidePanes[PLAYER_MAX];  // at 0x9B8 N_L_00 / N_R_00
    RPSysLayout* mpLabelLayouts[PLAYER_MAX][FISH_MAX];         // at 0x9C0 fishingResult_01
    nw4r::lyt::Pane* mpLabelPointPanes[PLAYER_MAX][FISH_MAX];  // at 0xB50
    nw4r::lyt::Pane* mpLabelPanes[PLAYER_MAX][FISH_MAX];       // at 0xCE0 [L]
    nw4r::lyt::Pane* mpFacePanes[PLAYER_MAX][2];               // at 0xE70 N_playerFace*
    RPSysLytResAccessor* mpResAccessor;                        // at 0xE80

    static f32 sScrollRange[PLAYER_MAX]; // 8043B5A8 label x per side (-0.2, 0.2)
    static f32 sLabelMinY;               // 8043B5B0 -0.7
    static f32 sLabelMaxY;               // 8043B5B4 0.7
    static f32 sLabelStep;               // 8043B5B8 0.25

    // The state functions are a local table in Calculate (8031F5D8, .rodata)

    static RPFshResultLayout* sInstance; // 8043C760
};

//! @}

#endif
