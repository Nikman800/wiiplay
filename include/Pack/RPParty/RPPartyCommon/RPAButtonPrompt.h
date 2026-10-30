#ifndef RP_PARTY_COMMON_A_BUTTON_PROMPT_H
#define RP_PARTY_COMMON_A_BUTTON_PROMPT_H
#include <Pack/types_pack.h>

#include <nw4r/lyt.h>

#include <egg/core.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;

/**
 * @brief Blinking "A" prompt (AButton.arc) attached under a message window pane
 * @details No vtable. Size 0x24, verified (allocated by the RPMessageWindow constructor).
 * Show() re-parents the root pane under the given pane, places it and plays the in animation.
 * @note Wii Play-only framework class (ledger Lyt_AButtonPrompt); the name is a project
 * convention.
 */
class RPAButtonPrompt {
public:
    void Load(EGG::Archive* pArchive, EGG::Heap* pHeap);
    void Reset();
    void Calculate();
    void Draw();
    void Show(nw4r::lyt::Pane* pParent, f32 x, f32 y);
    void PlayOut();
    BOOL IsShown();
    BOOL IsHidden();

    RPSysLayout* mpLayout;          // at 0x0 AButton.brlyt
    RPSysLytAnmObj* mpInAnmObj;     // at 0x4
    RPSysLytAnmObj* mpOutAnmObj;    // at 0x8
    RPSysLytAnmObj* mpBlinkAnmObj;  // at 0xC
    nw4r::lyt::Pane* mpRootPane;    // at 0x10
    nw4r::lyt::Pane* mpParentPane;  // at 0x14 pane the root is appended to
    nw4r::lyt::Pane* mpButtonPane;  // at 0x18 N_AButton_00
    s32 mState;                     // at 0x1C 0 hidden, 1 in, 2 blinking, 3 out
    bool mDrawEnable;               // at 0x20
    bool mVisible;                  // at 0x21
};

//! @}

#endif
