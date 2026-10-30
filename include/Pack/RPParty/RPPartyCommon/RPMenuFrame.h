#ifndef RP_PARTY_COMMON_MENU_FRAME_H
#define RP_PARTY_COMMON_MENU_FRAME_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <egg/math.h>

#include <nw4r/lyt.h>
#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytTextBox;
class RPSysMessage;
class RPSysTagProcessor;

/**
 * @brief Menu frame (menuCommon.arc, menuFrame_00.brlyt): frame, back button, back-button text,
 * caption, player name, training and party-game panels, each with its own state machine
 * @details vtable 80357A50. Size 0x288, verified (Menu_MenuScene_configure,
 * Party_SelectScene_ctor). The ledger's Menu_TPtGame00_ctor is this class's constructor (it
 * stores vtable 80357A50), so ledgers Menu_TPtGame00 and Menu_MenuFrame are one class.
 * State machines: frame 4 (hidden, in, shown, out), back button 7 (none, in, idle, activate,
 * deactivate, push, out), back text 2 (idle, change), caption 2 (idle, change), name 4 (hidden,
 * in, shown, out), training 5 and party game 5 (hidden, in, shown, change, out).
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPMenuFrame : public RPLytObjBase {
public:
    static const int TEXT_LEN = 0x20;

    typedef RPStateMachine<RPMenuFrame> StateMachine;

    /**
     * @brief A text change waiting for the panel's change animation
     */
    struct TextRequest {
        u32 id;                // at 0x0
        RPSysMessage* pMessage; // at 0x4
        u32 group;             // at 0x8
        u32 index;             // at 0xC
    };

public:
    RPMenuFrame(s32 mode);

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    BOOL ShowFrame();
    BOOL IsFrameNotEntering() const;
    void SetFramePaneVisible(s32 idx, bool visible);
    EGG::Vector2f GetLogoPos() const;

    BOOL ShowBackButton();
    BOOL HideBackButton();
    BOOL SetBackButtonActive(bool active, s32 player);
    void PushBackButton(u32 remote);
    BOOL IsBackPushDone() const;
    BOOL IsBackButtonIdle() const;
    BOOL IsPointOnBackButton(const EGG::Vector2f& rPos) const;

    void SetCaption(s32 id, RPSysMessage* pMessage, u32 group, u32 index, int argc, ...);
    void RequestCaptionChange(u32 id, BOOL force, RPSysMessage* pMessage, u32 group, u32 index,
                              int argc, ...);

    void ShowName(s32 player);
    void HideName();
    void SetPlayerNameText(RPSysMessage* pMessage, u32 group, u32 index, const wchar_t* pName);
    nw4r::lyt::Pane* GetModelPane() const;

    void ShowPtGame();
    void HidePtGame();
    void RequestPtGameText(u32 id, BOOL force, RPSysMessage* pMessage, u32 group, u32 index,
                           int argc, ...);

private:
    void EnterFrameHidden();
    void EnterFrameIn();
    void CalcFrameIn();
    void EnterFrameShown();
    void CalcFrameShown();
    void EnterFrameOut();
    void CalcFrameOut();

    void EnterBackIn();
    void CalcBackIn();
    void EnterBackIdle();
    void CalcBackIdle();
    void EnterBackActivate();
    void CalcBackActivate();
    void EnterBackDeactivate();
    void CalcBackDeactivate();
    void EnterBackPush();
    void CalcBackPush();
    void EnterBackOut();
    void CalcBackOut();

    void EnterBackTextIdle();
    void CalcBackTextIdle();
    void EnterBackTextChange();
    void CalcBackTextChange();

    void EnterCaptionIdle();
    void CalcCaptionIdle();
    void EnterCaptionChange();
    void CalcCaptionChange();

    void EnterNameHidden();
    void CalcNameHidden();
    void EnterNameIn();
    void CalcNameIn();
    void EnterNameShown();
    void CalcNameShown();
    void EnterNameOut();
    void CalcNameOut();

    void EnterTrainingHidden();
    void CalcTrainingHidden();
    void EnterTrainingIn();
    void CalcTrainingIn();
    void EnterTrainingShown();
    void CalcTrainingShown();
    void EnterTrainingChange();
    void CalcTrainingChange();
    void EnterTrainingOut();
    void CalcTrainingOut();

    void EnterPtGameHidden();
    void CalcPtGameHidden();
    void EnterPtGameIn();
    void CalcPtGameIn();
    void EnterPtGameShown();
    void CalcPtGameShown();
    void EnterPtGameChange();
    void CalcPtGameChange();
    void EnterPtGameOut();
    void CalcPtGameOut();

private:
    s32 mMode;                           // at 0x8
    StateMachine* mpFrameState;          // at 0xC
    EGG::Vector3f mAnchorPos[2];         // at 0x10 translations of two anchor panes
    nw4r::lyt::Pane* mpLogoPane;         // at 0x28 N_3Drogo_00
    StateMachine* mpBackState;           // at 0x2C
    s32 mBackPlayer;                     // at 0x30 controller that activated the button
    bool mBackActive;                    // at 0x34
    nw4r::lyt::Pane* mpBackBoundings[2]; // at 0x38 B_backButton_00/01
    StateMachine* mpBackTextState;       // at 0x40
    bool mBackTextRequested;             // at 0x44
    u32 mBackTextId;                     // at 0x48
    TextRequest mBackTextReq;            // at 0x4C
    wchar_t mBackText[TEXT_LEN];         // at 0x5C
    RPSysLytTextBox* mpBackTextBox;      // at 0x9C
    u32 mBackPushRemote;                 // at 0xA0
    StateMachine* mpCaptionState;        // at 0xA4
    bool mCaptionRequested;              // at 0xA8
    u32 mCaptionId;                      // at 0xAC
    TextRequest mCaptionReq;             // at 0xB0
    wchar_t mCaption[TEXT_LEN];          // at 0xC0
    RPSysLytTextBox* mpCaptionTextBox;   // at 0x100
    StateMachine* mpNameState;           // at 0x104
    bool mNameShowRequested;             // at 0x108
    bool mNameHideRequested;             // at 0x109
    bool mNameChangeRequested;           // at 0x10A
    s32 mNamePlayer;                     // at 0x10C
    s32 mNextNamePlayer;                 // at 0x110
    TextRequest mNameReq;                // at 0x114 (id unused)
    bool mNameRequested;                 // at 0x124
    wchar_t mPlayerName[TEXT_LEN + 1];   // at 0x126
    RPSysLytTextBox* mpPlayerTextBox;    // at 0x168 T_player_00
    RPSysLytTextBox* mpPlayerNameTextBox; // at 0x16C T_playerName_00
    nw4r::lyt::Pane* mpModelPane;        // at 0x170 N_3dmodel_00
    StateMachine* mpTrainingState;       // at 0x174
    bool mTrainingShowRequested;         // at 0x178
    bool mTrainingHideRequested;         // at 0x179
    bool mTrainingRequested;             // at 0x17A
    u32 mTrainingId;                     // at 0x17C
    TextRequest mTrainingReq;            // at 0x180
    wchar_t mTrainingText[TEXT_LEN];     // at 0x190
    RPSysLytTextBox* mpTrainingTextBox;  // at 0x1D0
    StateMachine* mpPtGameState;         // at 0x1D4
    bool mPtGameShowRequested;           // at 0x1D8
    bool mPtGameHideRequested;           // at 0x1D9
    bool mPtGameRequested;               // at 0x1DA
    u32 mPtGameId;                       // at 0x1DC
    TextRequest mPtGameReq;              // at 0x1E0
    wchar_t mPtGameText[TEXT_LEN];       // at 0x1F0
    RPSysLytTextBox* mpPtGameTextBox;    // at 0x230
    RPSysMessage* mpMessage;             // at 0x234 common_message.bmg
    RPSysTagProcessor* mpTagProcessor;   // at 0x238
    RPSysLayout* mpLayout;               // at 0x23C
    RPSysLytAnmObj* mpAnmObjs[18];       // at 0x240
};

//! @}

#endif
