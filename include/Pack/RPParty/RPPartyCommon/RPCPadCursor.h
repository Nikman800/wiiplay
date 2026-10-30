#ifndef RP_PARTY_COMMON_CPAD_CURSOR_H
#define RP_PARTY_COMMON_CPAD_CURSOR_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>

#include <nw4r/lyt.h>
#include <nw4r/math.h>

#include <egg/core.h>
#include <egg/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPSysLayout;

/**
 * @brief Snapshot of the pane a layout cursor points at
 * @details Size 0x88 [H] (copied as a whole, the name with strcpy). Filled from the target
 * pane: size, global matrix scale/translation and base position (basePosition % 3, / 3).
 * @note Wii Play-only; the name is a project convention.
 */
struct RPCursorTarget {
    RPCursorTarget() : mSize(0.0f, 0.0f) {}

    bool mValid;                 // at 0x0
    char mName[100];             // at 0x1 pane name
    nw4r::lyt::Size mSize;       // at 0x68 (its copy constructor copies with lfs/stfs)
    nw4r::math::VEC2 mScale;     // at 0x70 global matrix [0][0], [1][1]
    nw4r::math::VEC3 mTrans;     // at 0x78 global matrix [0][3], [1][3]; z [L]
    u8 mBasePosH;                // at 0x84
    u8 mBasePosV;                // at 0x85
};

/**
 * @brief Corner-bracket cursor (cPadCursor.arc, panes N_cPadCursor_00..02) for D-pad selection
 * @details vtable 80357428. Size 0x258, verified (Lyt_PauseMenu, Lyt_SystemWindow, Menu_MenuScene,
 * Menu_PlayerConfirm, Party_ResultMenu, Party_SelectScene). The corners move from the previous
 * target to the new one; fading out takes 60 frames, fading in 120 frames. Uses
 * HealthCPadCursor.arc when the system language/region mode (lbl_8043C470 + 0xC) is 2 [L].
 * @note Wii Play-only framework class (ledger Lyt_CPadCursor); the name is a project convention.
 */
class RPCPadCursor : public RPLytObjBase {
    // Reads mPrevTarget (RPCPadCursor_2.cpp)
    friend void RP_getPointerScreenPos(EGG::Vector2f& rScreenPos, const RPCPadCursor& rCursor);

public:
    RPCPadCursor();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    void LoadWithParent(EGG::Archive* pArchive, EGG::Heap* pHeap);
    void SetTarget(nw4r::lyt::Pane* pPane);
    void SelectStyle(s32 style);
    BOOL IsActive() const; // out of line (RPCPadCursor_2.cpp)
    void SetActive(s32 active);
private:
    void UpdateCorners(s32 style);
    s32 CalcFadeAlpha();
    //! @brief Load has not found all three corner panes
    bool IsMissingPanes() const {
        for (s32 i = 0; i < 3; i++) {
            if (mpCornerPanes[i] == NULL) {
                return true;
            }
        }
        return false;
    }

private:
    s32 mActive;                          // at 0x8
    s32 mPrevActive;                      // at 0xC
    s32 mStyle;                           // at 0x10 index of the visible corner pane
    s32 unk14;                            // at 0x14 1 after reset / fade-out end [M]
    s32 mFadeState;                       // at 0x18 1 none, 2 fading out, 3 fading in
    s32 mFadeTimer;                       // at 0x1C
    RPCursorTarget mTarget;               // at 0x20 current target
    RPCursorTarget mPrevTarget;           // at 0xA8
    RPCursorTarget mNextTarget;           // at 0x130
    nw4r::lyt::Pane* mpCornerPanes[3];    // at 0x1B8 N_cPadCursor_00..02
    RPCursorTarget mDrawTarget;           // at 0x1C4 interpolated
    s32 mMoveTimer;                       // at 0x24C [M]
    s32 mMoveFrames;                      // at 0x250 35 after reset [M]
    RPSysLayout* mpLayout;                // at 0x254 cPadCursor_00.brlyt

public:
    static nw4r::math::VEC2 sDefaultSize;
    static nw4r::math::VEC2 sDefaultScale;
    static nw4r::math::VEC3 sDefaultTrans;
};

/**
 * @brief Window-pane cursor (cCursorCircle.arc, W_cursor_00) stretched over the target
 * @details vtable 80357480. Size 0x250, verified (Menu_MenuScene_configure). Same state layout as
 * RPCPadCursor up to 0x1B8, with one window pane instead of three corner panes.
 * @note Wii Play-only framework class (ledger Lyt_WCursor); the name is a project convention.
 */
/**
 * @brief Pointer position of the cursor's previous target in normalised screen units (x, y in
 * -1..1 of the half screen), zero when that target is not valid [M]
 */
void RP_getPointerScreenPos(EGG::Vector2f& rScreenPos, const RPCPadCursor& rCursor);

class RPWCursor : public RPLytObjBase {
public:
    RPWCursor();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    void SetTarget(nw4r::lyt::Pane* pPane);
    void SetActive(u32 active); // out of line (RPCPadCursor_3.cpp)

private:
    friend void UpdateWindow(RPWCursor& rCursor, s32 index);
    s32 CalcFadeAlpha();

private:
    s32 mActive;                 // at 0x8
    s32 mPrevActive;             // at 0xC
    s32 unk10;                   // at 0x10
    s32 unk14;                   // at 0x14
    s32 mFadeState;              // at 0x18
    s32 mFadeTimer;              // at 0x1C
    RPCursorTarget mTarget;      // at 0x20
    RPCursorTarget mPrevTarget;  // at 0xA8
    RPCursorTarget mNextTarget;  // at 0x130
    nw4r::lyt::Window* mpWindow; // at 0x1B8 W_cursor_00
    RPCursorTarget mDrawTarget;  // at 0x1BC
    s32 mMoveTimer;              // at 0x244 [M]
    s32 mMoveFrames;             // at 0x248 [M]
    RPSysLayout* mpLayout;       // at 0x24C cCursorCircle_00.brlyt

public:
    static nw4r::math::VEC2 sDefaultSize;
    static nw4r::math::VEC2 sDefaultScale;
    static nw4r::math::VEC3 sDefaultTrans;
};

//! @}

#endif
