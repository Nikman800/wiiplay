#ifndef RP_PARTY_COMMON_GAME_INFO_LYT_H
#define RP_PARTY_COMMON_GAME_INFO_LYT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;

/**
 * @brief Game information panel with the remote-hold demo (ptSelectGame.arc,
 * ptSelectGame_01.brlyt and the nine ptG*Remote*_00.brlyt layouts)
 * @details vtable 80358440. Size 0xA4, verified (Party_SelectScene_ctor). Six states (table
 * 8031DAC0): 0 hidden, 1 in, 2 idle, 3 push A (confirm), 4 out, 5 done. Open sets the title,
 * comment (single/two player) and step texts; EnterIn moves the game's N_remote_00 pane under
 * N_remoteP_00 and plays its remote animation.
 * @note Wii Play-only framework class (ledger Party_GameInfoLyt); the name is a project
 * convention.
 */
class RPGameInfoLyt : public RPLytObjBase {
public:
    static const int GAME_NUM = 9;

public:
    explicit RPGameInfoLyt(s32 playerNum);

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    void Open(s32 game);
    void Close();
    bool Confirm(u32 arg);

private:
    void EnterIn();
    void CalcIn();
    void EnterIdle();
    void CalcIdle();
    void EnterPushA();
    void CalcPushA();
    void EnterOut();
    void CalcOut();

public:
    RPStateMachine<RPGameInfoLyt>* mpStateMachine;  // at 0x8
private:
    s32 mPlayerNum;                                 // at 0xC
    s32 mGame;                                      // at 0x10
public:
    bool mConfirmed;                                // at 0x14
    u32 unk18;                                      // at 0x18 0xFFFF on confirm [L]
private:
    s32 mRemoteIdx;                                 // at 0x1C remote layout shown
    RPSysLayout* mpLayout;                          // at 0x20
    RPSysLytAnmObj* mpAnmObjs[3];                   // at 0x24 in, pushA, out
    RPSysLayout* mpRemoteLayouts[GAME_NUM];         // at 0x30
    RPSysLytAnmObj* mpRemoteAnmObjs[GAME_NUM];      // at 0x54
    char unk78[0x7C - 0x78];
    nw4r::lyt::Pane* mpRemotePanes[GAME_NUM];       // at 0x7C N_remote_00 of each remote layout
    u32 mConfirmArg;                                // at 0xA0
};

//! Ledger Party_GameInfoSeq (size 0x28)
typedef RPStateMachine<RPGameInfoLyt> RPGameInfoSeq;

//! @}

#endif
