#ifndef RP_PARTY_COMMON_PLAYER_NUM_SELECT_H
#define RP_PARTY_COMMON_PLAYER_NUM_SELECT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>

#include <nw4r/lyt.h>
#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPCPadCursor;
class RPMenuFrame;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysMessage;

/**
 * @brief Player count / remote entry layout (rimoconEntry.arc or rimoconEntry_pt.arc,
 * rimoconEntry_00.brlyt)
 * @details vtable 80357AE0. Size 0x10C, verified (Menu_PlayerNumSelect_ctor). 49 animations
 * (table at 8031963C): scene animations, per-button state animations and the entry animations.
 * @note Wii Play-only framework class (ledger Menu_RimoconEntryLyt); the name is a project
 * convention.
 */
class RPRimoconEntryLyt : public RPLytObjBase {
public:
    static const int BUTTON_NUM = 6;
    static const int ANIM_NUM = 49;

public:
    explicit RPRimoconEntryLyt(bool countSelect);

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    void BindSceneAnims();
    void PlaySceneAnim(s32 anim);
    bool IsSceneAnimPlaying() const;
    void PlayEntryOutAnim();
    bool IsEntryAnimPlaying() const;
    void SetButtonState(s32 button, s32 state);
    bool AreButtonAnimsDone() const;
    void ResetButton6Anim();
    nw4r::lyt::Pane* GetButtonPane(s32 button) const;
    bool IsPointOnButton(s32 button, const nw4r::math::VEC2& rPos) const;
    void SetTexts(RPSysMessage* pMessage);
    // Sets a remote's state (0 = none, 1 = connected); Calculate applies it on change
    void SetRemoteState(s32 player, s32 state);
    void SetRemoteIconAnim(s32 player, s32 state);
    void UpdateEntryAnims();
    void SetRemoteLampAnim(s32 player, s32 state);

public:
    bool mCountSelect;                   // at 0x8 player-count buttons [M]
private:
    s32 mButtonStates[BUTTON_NUM];       // at 0xC 3 after construction
    s32 mEntryAnims[4];                  // at 0x24 -1 after construction
    s32 unk34[4];                        // at 0x34 remote states (SetRemoteState)
    RPSysLayout* mpLayout;               // at 0x44
public:
    RPSysLytAnmObj* mpAnmObjs[ANIM_NUM]; // at 0x48
};

/**
 * @brief Player count selection (1-4 players) and remote entry
 * @details No vtable. Size 0x28, verified (Menu_MenuScene_configure allocates 0x28 with this
 * constructor).
 * @note Wii Play-only framework class (ledger Menu_PlayerNumSelect); the name is a project
 * convention.
 */
class RPPlayerNumSelect {
public:
    RPPlayerNumSelect(RPMenuFrame* pFrame, RPCPadCursor* pCursor);

    void Load(EGG::Archive* pArchive, RPSysMessage* pMessage);
    void Reset();
    void Calculate(bool active);
    void DrawLayout();
    void DrawCursor();
    void SetTitle(u32 msgId);
    void StartCountSelect();

private:
    void CalcPointer(bool* pCursorFree, bool* pOnBack);
    void CalcCountInput(bool cursorFree, bool onBack);
    void CalcEntryButtons(bool onButton);
    void CalcEntryPointer();

private:
    u8 mEntryMode;                   // at 0x0
public:
    s32 mPhase;                      // at 0x4
    s32 unk8;                        // at 0x8
    s32 mPlayerNum;                  // at 0xC
    s32 mState;                      // at 0x10
private:
    s32 unk14;                       // at 0x14
    RPSysMessage* mpMessage;         // at 0x18
    RPMenuFrame* mpFrame;            // at 0x1C
public:
    RPCPadCursor* mpCursor;          // at 0x20
    RPRimoconEntryLyt* mpLayout;     // at 0x24
};

//! @}

#endif
