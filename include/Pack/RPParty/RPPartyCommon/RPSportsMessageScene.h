#ifndef RP_PARTY_COMMON_SPORTS_MESSAGE_SCENE_H
#define RP_PARTY_COMMON_SPORTS_MESSAGE_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPSystem/RPSysNunchukCheckScene.h>

#include <nw4r/lyt.h>

// The scene itself is RPSysNunchukCheckScene (vtable 80357D08 carries that name).

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPFrameDown;
class RPSysKokeshiIcon;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytTextBox;
class RPSysMessage;
namespace EGG {
class Screen;
} // namespace EGG

/**
 * @brief One player's panel of the "hold the remote like this" message screen
 * @details No vtable, no constructor: RPSysNunchukCheckScene::Configure allocates four with a
 * plain new(0x5C) (size verified). Init finds the panes, pictures (tinted with the player colour)
 * and text boxes of player slot n (name table RPSportsMsgPanel::sPaneNames). The scene drives the
 * rest through inline methods (defined in RPSportsMessageScene.cpp).
 * @note Wii Play-only (ledger Menu_SportsMsgPanel); the name is a project convention.
 */
class RPSportsMsgPanel {
public:
    enum {
        PANE_PLAYER, //!< N_player_0n
        PANE_OK,     //!< N_ok_0n
        PANE_LED,    //!< N_LED_0n
        PANE_P1LED,  //!< P_P1LED_0n .. P_P4LED_0n
        PANE_MAX = PANE_P1LED + 4
    };

    enum {
        ANM_IN_OK,       //!< nunchukCheck_00_inOK_00 (N_ok)
        ANM_IN_OK_2,     //!< nunchukCheck_00_inOK_00 (N_LED)
        ANM_OUT_OK,      //!< nunchukCheck_00_outOK_00 (N_ok)
        ANM_OUT_OK_2,    //!< nunchukCheck_00_outOK_00 (N_LED)
        ANM_LED,         //!< nunchukCheck_00_LED_00, one per P_PnLED
        ANM_MAX = ANM_LED + 4
    };

public:
    void Init(u32 player, RPSysLytResAccessor* pAccessor, RPSysLayout* pLayout,
              RPSysMessage* pMessage);

    void Reset();
    void Start(u8 channel, bool ready, const wchar_t* pName);
    void Calculate();

    bool IsReady() const {
        return mReady;
    }
    void SetReady(bool ready) {
        mReady = ready;
    }

private:
    s32 PlayIn(u32 anm, u32 pane);
    void SkipIn(u32 anm, u32 pane);
    void SetLED(u32 anm, u32 pane);

private:
    bool mActive;                          // at 0x0
    u8 mChannel;                           // at 0x1 controller channel (LED frame)
    s32 mTimer;                            // at 0x4 frames until the next OK animation
    bool mReady;                           // at 0x8 player holds the remote correctly
    bool mShownReady;                      // at 0x9 state the OK animation shows
    nw4r::lyt::Pane* mpPanes[PANE_MAX];    // at 0xC
    nw4r::lyt::Picture* mpColors[2];       // at 0x28
    RPSysLytAnmObj* mpAnmObjs[ANM_MAX];    // at 0x30
    RPSysLytTextBox* mpTextBox;            // at 0x50 T_pName
    RPSysLayout* mpLayout;                 // at 0x54
    RPSysMessage* mpMessage;               // at 0x58

public:
    static const char* sPaneNames[4][7];
};

//! @}

#endif
