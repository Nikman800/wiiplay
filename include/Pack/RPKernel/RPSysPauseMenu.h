#ifndef RP_KERNEL_PAUSE_MENU_H
#define RP_KERNEL_PAUSE_MENU_H

#include <nw4r/lyt.h>
#include <Pack/types_pack.h>

//! @addtogroup rp_kernel
//! @{

// Forward declarations
class RPCPadCursor;
class RPSysLayout;
class RPSysLytBounding;
class RPSysLytAnmObj;
class RPSysLytResAccessor;
class RPSysLytTextBox;

/**
 * @brief Library that handles pause menu layout.
 * @details Wii Play: no vtable (the constructor stores 0 at offset 0, which load fills with the
 * resource accessor; the destructor is called directly by the pause manager). Size 0x88, verified
 * (Sys_Pause_createSingleton). pause.arc / pause_00.brlyt with three buttons and an RPCPadCursor.
 * Members are from Wii Play's code (ledger Lyt_PauseMenu).
 */

class RPSysPauseMenu {
public:
    RPSysPauseMenu();
    ~RPSysPauseMenu(); // Wii Play: not virtual

public:
    void draw();

    // Wii Play: called by RPSysPauseMgr [M names]
    void load();
    void open();
    /**
     * @brief Updates the menu; returns the button chosen (0 resume, 1 retry, 2 quit) or
     * another value while the menu is open
     */
    u8 calc();

    //! Wii Play: player who opened the menu [M]
    void setPlayer(u8 player) {
        mUNK_0x80 = player;
    }

private:
    // Wii Play [M names]
    void resetButtonAnims();
    void updateButtonHighlight();
    void updateSelect();
    void updateInput();
    bool isPointing();

private:
    /**
     * @brief One menu button (Wii Play)
     */
    struct Button {
        u8 mUNK_0x0;                  // at 0x0 highlighted
        u8 mUNK_0x1;                  // at 0x1 highlighted last frame
        nw4r::lyt::Pane* mPane;       // at 0x4
        RPSysLytBounding* mBounding;  // at 0x8
        RPSysLytTextBox* mTextBox;    // at 0xC
        RPSysLytAnmObj* mAnmObjs[3];  // at 0x10
    };

    RPSysLytResAccessor* mResAccessor; // at 0x0 Wii Play
    RPSysLayout* mLayout; // at 0x4
    RPSysLytAnmObj* mAnmObj_0x04; // at 0x8 pause_00_in_00
    RPSysLytAnmObj* mAnmObj_0x08; // at 0xC pause_00_out_00
    s32 mState; // at 0x10 Wii Play [M]
    nw4r::lyt::Pane* mPane_0x14; // at 0x14 N_center_00
    nw4r::lyt::Pane* mPane_0x18; // at 0x18 N_bg_00
    Button mButtons[3]; // at 0x1C Wii Play
    RPCPadCursor* mUtlCursor; // at 0x70 Wii Play: RPCPadCursor (RPUtlLytFrameCursor in Wii Sports?)
    s32 mUNK_0x74; // at 0x74
    bool mUNK_0x78; // at 0x78
    u8 mUNK_0x79; // at 0x79
    u8 mUNK_0x7A; // at 0x7A
    u8 mUNK_0x7B; // at 0x7B
    bool mUNK_0x7C; // at 0x7C
    u8 mUNK_0x7D; // at 0x7D button under the pointer
    u8 mUNK_0x7E; // at 0x7E button chosen with the D-pad
    u8 mUNK_0x7F; // at 0x7F
    u8 mUNK_0x80; // at 0x80
    s32 mUNK_0x84; // at 0x84
};

//! @}

#endif
