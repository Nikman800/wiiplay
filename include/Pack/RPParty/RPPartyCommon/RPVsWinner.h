#ifndef RP_PARTY_COMMON_VS_WINNER_H
#define RP_PARTY_COMMON_VS_WINNER_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <egg/math.h>
#include <nw4r/lyt.h>
#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPGrpModel;
class RPSysEffect;
class RPSysLayout;
class RPSysLytAnmObj;

/**
 * @brief Two-player winner banner (ptVsResult.arc, ptVsWinner_00.brlyt)
 * @details vtable 80358378. Size 0x24, verified (Party_VsWinner_ctor). Five states (table
 * 8031CD80): 0 hidden, 1 in, 2 shown, 3 out, 4 done. The player and trophy position panes
 * place the 3D models.
 * @note Wii Play-only framework class (ledger Party_VsWinnerLyt); the name is a project
 * convention.
 */
class RPVsWinnerLyt : public RPLytObjBase {
public:
    RPVsWinnerLyt();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10

    void Open();
    void Close();
    void DrawLayer1C();
    void DrawLayer1A();
    EGG::Vector2f GetPlayerPanePos(s32 player) const;
    EGG::Vector2f GetTrophyPanePos() const;

private:
    void EnterIn();
    void CalcIn();
    void EnterShow();
    void CalcShow();
    void EnterOut();
    void CalcOut();

public:
    RPStateMachine<RPVsWinnerLyt>* mpStateMachine;         // at 0x8
private:
    RPSysLayout* mpLayout;                                 // at 0xC
    RPSysLytAnmObj* mpAnmObjs[2];                          // at 0x10 in, out
    nw4r::lyt::Pane* mpPlayerPosPanes[RP_PARTY_MAX_PLAYERS]; // at 0x18 N_pLPosition_00/R
    nw4r::lyt::Pane* mpTrophyPosPane;                      // at 0x20 N_tPosition_00
};

//! Ledger Party_VsWinnerLytSeq (size 0x28)
typedef RPStateMachine<RPVsWinnerLyt> RPVsWinnerLytSeq;

/**
 * @brief Two-player winner screen: win_1P / win_2P / tie model, Bcm_win effects
 * (RPCommon + 0x20)
 * @details vtable 80358360 (IRPSysHostIOSocket). Size 0xD0, verified (Party_Common_ctor).
 * Minigames call Start(winner) (-1 = tie, model 2). Six states (table 8031CC18): 0 idle,
 * 1 in, 2 show, 3 close, 4 save, 5 done. Each player Mii and the trophy are placed by axes,
 * position and scale computed from the layout panes.
 * @note Wii Play-only framework class (ledger Party_VsWinner); the name is a project
 * convention.
 */
class RPVsWinner : public IRPSysHostIOSocket {
public:
    static const int MODEL_NUM = 3;
    static const int EFFECT_NUM = 2;

public:
    RPVsWinner();
    virtual ~RPVsWinner(); // at 0x8

    void Load();
    void Reset();
    void Calculate();
    void Start(s32 winner);
    s32 GetState() const {
        return mpStateMachine->GetState();
    }
    // Wii Play: RPFshResultLayout [H]
    bool IsStateEntered(s32 state) const {
        return mpStateMachine->GetState() == state && mpStateMachine->GetTimer() == 1;
    }
    void DrawLayer1C();
    void DrawLayer1A();
    void UpdatePositions();

private:
    void EnterIn();
    void CalcIn();
    void EnterShow();
    void CalcShow();
    void EnterClose();
    void CalcClose();
    void EnterSave();
    void CalcSave();

private:
    RPStateMachine<RPVsWinner>* mpStateMachine;          // at 0x4
    s32 mWinner;                                         // at 0x8 -1 = tie
    RPVsWinnerLyt* mpLayout;                             // at 0xC
    bool unk10;                                          // at 0x10
public:
    EGG::Vector3f mPlayerAxisZ[RP_PARTY_MAX_PLAYERS];    // at 0x14
    EGG::Vector3f mPlayerAxisY[RP_PARTY_MAX_PLAYERS];    // at 0x2C
    EGG::Vector3f mPlayerAxisX[RP_PARTY_MAX_PLAYERS];    // at 0x44
    EGG::Vector3f mPlayerPos[RP_PARTY_MAX_PLAYERS];      // at 0x5C
    f32 mPlayerScale[RP_PARTY_MAX_PLAYERS];              // at 0x74
private:
    nw4r::math::VEC3 mTrophyAxisZ;                       // at 0x7C
    nw4r::math::VEC3 mTrophyAxisY;                       // at 0x88
    nw4r::math::VEC3 mTrophyAxisX;                       // at 0x94
    nw4r::math::VEC3 mTrophyPos;                         // at 0xA0
    nw4r::math::VEC3 unkAC;                              // at 0xAC copied from 8043A43C [L]
    s32 mModelIdx;                                       // at 0xB8 winner, 2 for a tie
    RPGrpModel* mpModels[MODEL_NUM];                     // at 0xBC win_1P, win_2P, tie
    RPSysEffect* mpEffects[EFFECT_NUM];                  // at 0xC8 Bcm_win_fubuki, Bcm_win_spot01

public:
    static f32 sPlayerScale;
    static f32 sXOffsets[2]; // degrees
    static bool sFlag;
    static EGG::Vector2f sPlayerOffsets[2];
    static EGG::Vector3f sTrophyVec;
    static EGG::Vector2f sTrophyOffset;
};

//! Ledger Party_VsWinnerSeq (size 0x28)
typedef RPStateMachine<RPVsWinner> RPVsWinnerSeq;

//! @}

#endif
