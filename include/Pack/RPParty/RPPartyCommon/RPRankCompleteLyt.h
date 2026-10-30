#ifndef RP_PARTY_COMMON_RANK_COMPLETE_LYT_H
#define RP_PARTY_COMMON_RANK_COMPLETE_LYT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPSysEffect;
class RPSysLayout;
class RPSysLytAnmObj;

/**
 * @brief "All medals of a rank" congratulation on the game-select screen (ptSelectGame.arc,
 * rankComplete_00.brlyt)
 * @details vtable 80358710. Size 0x30, verified (Party_SelectScene_ctor). Five states (table
 * 8031E760): 0 hidden, 1 in, 2 shown, 3 out, 4 done; plays RP_COMMON_BGM_RANK_COMPLETE and the
 * effects Bsm_com_fubuki and Bsm_com_fireworks_fubuki/slv/gld/pla.
 * @note Wii Play-only framework class (ledger Party_RankCompleteLyt); the name is a project
 * convention.
 */
class RPRankCompleteLyt : public RPLytObjBase {
public:
    static const int EFFECT_NUM = 5;

public:
    RPRankCompleteLyt();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    void Open(s32 rank);
    void Close();

private:
    void EnterIn();
    void CalcIn();
    void EnterShow();
    void CalcShow();
    void EnterOut();
    void CalcOut();

public:
    RPStateMachine<RPRankCompleteLyt>* mpStateMachine; // at 0x8
private:
    s32 mRank;                                         // at 0xC medal rank 1-3
    RPSysLayout* mpLayout;                             // at 0x10
    RPSysLytAnmObj* mpInAnmObj;                        // at 0x14
    RPSysLytAnmObj* mpOutAnmObj;                       // at 0x18
    RPSysEffect* mpEffects[EFFECT_NUM];                // at 0x1C
};

//! Ledger Party_RankCompleteSeq (size 0x28)
typedef RPStateMachine<RPRankCompleteLyt> RPRankCompleteSeq;

//! @}

#endif
