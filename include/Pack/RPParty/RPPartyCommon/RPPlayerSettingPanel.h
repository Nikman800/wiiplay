#ifndef RP_PARTY_COMMON_PLAYER_SETTING_PANEL_H
#define RP_PARTY_COMMON_PLAYER_SETTING_PANEL_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPCPadCursor.h>

#include <nw4r/lyt.h>
#include <nw4r/math.h>

#include <egg/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPAnimButton;
class RPAnimButtonTable;
class RPMenuFrame;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;
class RPSysMessage;
class RPSysLytTextBox;
class RPSysCoreController;
namespace EGG {
class Archive;
}

/**
 * @brief Player settings panel (playerSelect.arc, playerSelect_01.brlyt): six two-choice option
 * rows plus OK, written to the player record
 * @details vtable 80357F98. Size 0x610, verified (Menu_createPlayerSettingPanel; singleton
 * 8043C6B0). Embeds an RPCPadCursor and an RPWCursor (constructed in place by the create
 * function). Rows are RPAnimButtons looked up through two RPAnimButtonTables.
 * @note Wii Play-only framework class (ledger Menu_PlayerSettingPanel); the name is a project
 * convention.
 */
class RPPlayerSettingPanel {
public:
    static const int ROW_NUM = 6;
    static const int BUTTON_NUM = 13; // two per row, then OK
    static const int BUTTON_OK = 12;

public:
    static void CreateInstance();
    static void DestroyInstance();
    static RPPlayerSettingPanel* instance() {
        return spInstance;
    }

public:
    virtual ~RPPlayerSettingPanel(); // at 0x8
    /**
     * @brief Creates the resource accessor, clears the state, 5 selectable rows [H]
     */
    virtual void Init();                     // at 0xC
    virtual void Reset(RPMenuFrame* pFrame); // at 0x10 a non-NULL frame replaces mpFrame
    virtual void Calculate();                // at 0x14
    virtual void Draw();                     // at 0x18
    virtual void Exit();                     // at 0x1C

    /**
     * @brief Loads playerSelect.arc from pArchive; the cursors load from pCursorArchive
     */
    void Load(EGG::Archive* pArchive, EGG::Archive* pCursorArchive);
    void DrawCursors();
    void Open();
    bool IsConfirmed();
    bool IsCancelled();
    void SetOption(s32 row, u32 choice);
    u32 GetOption(s32 row);
    void SetPlayer(u32 controller, u32 player);
    void SetMode(s32 mode) {
        mMode = mode;
    }

private:
    void CalcMain();
    void CalcSelect();
    void SetHover(s32 id);
    void RefreshButtons();
    void StartIn();
    void CreateButtons(RPSysLayout* pLayout);
    void BindButtons(RPSysLayout* pLayout);
    void ApplyRowMode();
    void SetTexts();
    s32 HitTest(nw4r::math::VEC2 pos);
    s32 FilterButtonForMode(s32 id);
    s32 NavigateDpad(s32 id);
    s32 GetNextRow(s32 row, s32 oldRow);
    s32 ClampRowForMode(s32 row);
    bool UpdatePointer();
    void OnHoverChange(s32 id, nw4r::math::VEC2 pos);
    EGG::Vector2f ToNormalizedPos(nw4r::math::VEC2 pos);

    // Inline helpers (RPPlayerSettingPanel.cpp)
    inline RPSysCoreController* GetController();
    inline void PlayCursorSe(const char* pName);
    inline void DisableRow(s32 row, s32 anim);
    inline void PlayPushAnim(s32 id);
    inline void SetHoverId(s32 id);
    inline s32 GetNextRowButton(s32 row);

private:
    s32 mState;                                // at 0x4 0 in, 1 select, 2 cancelled, 3 confirmed
    RPSysLytTextBox* mpTextBoxes[BUTTON_NUM];  // at 0x8 button labels (titles reuse the slots)
    char unk3C[0x6C - 0x3C];
    s32 mSubState;                             // at 0x6C
    RPAnimButtonTable* mpOptionButtonTable;    // at 0x70
    RPAnimButton* mpOptionButtons[BUTTON_NUM]; // at 0x74
    RPAnimButtonTable* mpRowButtonTable;       // at 0xA8
    RPAnimButton* mpRowButtons[ROW_NUM];       // at 0xAC
    nw4r::lyt::Pane* mpRowPanes[7];            // at 0xC4
    RPSysLytAnmObj* mpSportsAnmObjs[5];        // at 0xE0
    s32 mTimer;                                // at 0xF4
    s32 mLastId;                               // at 0xF8 last hovered button
    s32 mCursorId;                             // at 0xFC D-pad selection
    s32 mHoverId;                              // at 0x100 -1 = none
    bool mPointerValid;                        // at 0x104
    bool mDpadMode;                            // at 0x105
    s32 mBackHover;                            // at 0x108 2 = pointer off the back button
    bool mHoverSound;                          // at 0x10C
    nw4r::lyt::Pane* mpTargetPane;             // at 0x110
    nw4r::lyt::Pane* mpScenePane;              // at 0x114 N_scene_00
    EGG::Vector2f mPointerPos;                 // at 0x118 (-5000, -5000) when off screen
    u32 unk120;                                // at 0x120
    u32 unk124;                                // at 0x124
    RPSysLayout* mpLayout;                     // at 0x128
    RPMenuFrame* mpFrame;                      // at 0x12C
    RPSysLytAnmObj* mpInAnmObj;                // at 0x130
    RPCPadCursor mCPadCursor;                  // at 0x134
    RPWCursor mWCursor;                        // at 0x38C
    RPSysMessage* mpMessage;                   // at 0x5DC
    RPSysLytResAccessor* mpResAccessor;        // at 0x5E0
    u8 mOptions[ROW_NUM][2];                   // at 0x5E4 1 = chosen
    s32 mMode;                                 // at 0x5F0 5 = every row, else one sport
    u32 mController;                           // at 0x5F4
    u32 mPlayer;                               // at 0x5F8
    nw4r::math::VEC2 mNormPointerPos;          // at 0x5FC
    s32 mColumn;                               // at 0x604
    s32 unk608;                                // at 0x608
    bool mPointerMoved;                        // at 0x60C

    static RPPlayerSettingPanel* spInstance;
    static const f32 sOffScreenPos; // -5000.0f

public:
    static const char* sRowAnimName;
    static const char* sSportsAnimName;
};

//! @}

#endif
