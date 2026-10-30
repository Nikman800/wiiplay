#ifndef RP_PARTY_COMMON_AB_PRESS_H
#define RP_PARTY_COMMON_AB_PRESS_H
#include <Pack/types_pack.h>

#include <nw4r/lyt.h>

#include <egg/core.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPKokeshiHead;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytTextBox;
class RPSysMessage;

/**
 * @brief "Hold A and B" check screen (ABPress.arc, ABPress_00.brlyt) shown before a game
 * @details No vtable, no constructor: RPPlayerConfirm allocates it with a plain new(0x160)
 * (size verified). start() does nothing when no player needs the check (the check function
 * Lyt_returnFalse_8018CA14 always returns 0 in Wii Play, so the screen is always shown).
 * @note Wii Play-only framework class (ledger Lyt_ABPress); the name is a project convention.
 */
class RPABPress {
public:
    static const int PLAYER_NUM = 4;

    struct AnimPair {
        RPSysLytAnmObj* mpInAnmObj;  // at 0x0
        RPSysLytAnmObj* mpOutAnmObj; // at 0x4
        nw4r::lyt::Pane* mpPane;     // at 0x8
    };

    /**
     * @brief Per-player panel N_player_NN
     */
    struct Player {
        nw4r::lyt::Pane* mpPane;            // at 0x0 N_player_NN
        nw4r::lyt::Picture* mpColor1;       // at 0x4 P_pColor1_NN
        nw4r::lyt::Picture* mpColor2;       // at 0x8 P_pColor2_NN
        RPSysLytAnmObj* mpLEDAnmObjs[4];    // at 0xC
        nw4r::lyt::Pane* mpLEDPane;         // at 0x1C N_LED_NN
        nw4r::lyt::Pane* mpOKPane;          // at 0x20 N_ok_NN
        RPSysLytAnmObj* mpOKInAnmObj;       // at 0x24
        RPSysLytAnmObj* mpOKInAnmObj2;      // at 0x28
        nw4r::lyt::Pane* mpLEDPanes[4];     // at 0x2C P_P<n>LED_NN
        u32 unk3C;                          // at 0x3C
        u8 unk40;                           // at 0x40 [L] cleared by Reset (byte store)
        RPSysLytTextBox* mpNameTextBox;     // at 0x44 T_pName_NN
        RPKokeshiHead* mpHead;              // at 0x48 cleared by load when its 4th argument is 0
    };

public:
    void Load(EGG::Archive* pArchive, RPSysMessage* pMessage, bool arg3);
    void Reset();
    void Calculate();
    void Draw();
    BOOL IsNeeded(s32 player);
    void Start(s32 player);
    bool IsDone(); // out of line in the binary (called, not inlined)

public:
    s32 mState;                     // at 0x0
    s32 mTimer;                     // at 0x4 -1 after reset
    RPSysLayout* mpLayout;          // at 0x8
    RPSysMessage* mpMessage;        // at 0xC
    s32 mPlayer;                    // at 0x10 -1 = all players
    bool mDone;                     // at 0x14
    AnimPair mAnims[2];             // at 0x18 [0] pane N_bg_00, [1] N_main_00
    Player mPlayers[PLAYER_NUM];    // at 0x30
};

//! @}

#endif
