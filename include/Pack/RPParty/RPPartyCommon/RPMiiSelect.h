// PROPOSED replacement for RPMiiSelect.h (that header is read-only while workers run).
// The coordinator should copy this file over RPMiiSelect.h, delete this file, and change the
// include in src/Pack/RPParty/RPPartyCommon/RPMiiSelect*.cpp back to RPMiiSelect.h.
#ifndef RP_PARTY_COMMON_MII_SELECT_H
#define RP_PARTY_COMMON_MII_SELECT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>

#include <egg/math/eggVector.h>

#include <nw4r/lyt.h>
#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPCPadCursor;
class RPKokeshiMgr;
class RPMenuFrame;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysMessage;
class RPWCursor;

/**
 * @brief Mii selection layout (playerSelect.arc, playerSelect_00.brlyt): face grid, page arrows,
 * buttons
 * @details vtable 80357B00. Size 0x100, verified (Menu_MiiSelectLyt_ctor). 46 animations.
 * Items: 0-2 buttons (P_button_0x), 3 right arrow, 4 left arrow, 5-14 faces (N_face_00-09).
 * Animations: 0 inFrame, 1 outFrame, 2-3 activeArrow, 4-13 activePlayer, 14-15 pushArrow,
 * 16-25 pushPlayer, 26-27 moveArrow, 28-29 inoutArrow, 30 slidePlayer, 31-33 activeButton_00,
 * 34-36 pushButton_00, 37-39 activeButton_01, 40-42 pushButton_01, 43 inoutText, 44 moveText,
 * 45 loopMark.
 * @note Wii Play-only framework class (ledger Menu_MiiSelectLyt); the name is a project
 * convention.
 */
class RPMiiSelectLyt : public RPLytObjBase {
public:
    static const int ITEM_NUM = 15;
    static const int ANIM_NUM = 46;

public:
    RPMiiSelectLyt();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    void DrawBackLayer();
    void SetPlayerTitle(RPSysMessage* pMessage, u32 player, bool cpu);
    void SetButtonTexts(RPSysMessage* pMessage);
    void PlaySceneOutAnim();
    bool IsSceneAnimPlaying();
    bool IsPointOnItem(s32 idx, const EGG::Vector2f& rPos);
    void SetItemState(s32 idx, s32 state, bool force);
    nw4r::lyt::Pane* GetItemPane(s32 idx);
    bool AreDecideAnimsDone();
    bool AreSelectAnimsDone();
    void SetFaceAllAnimFrame(s32 end);
    void PlayFaceAllAnim(s32 forward);
    bool IsFaceAllAnimPlaying();
    void SetArrowState(s32 arrow, s32 state, bool force);
    void PlayMessageAnim(s32 close);
    void PlayMessageMoveAnim();
    bool IsMessageAnimPlaying();
    nw4r::lyt::Pane* GetModelPane(s32 idx);
    nw4r::lyt::Pane* GetFacePane(s32 idx);
    EGG::Vector2f GetFacePanePos(s32 idx);
    void SetFaceState(s32 idx, s32 state);
    void SetButtonState(s32 idx, s32 state, bool force);
    void SetArrowBackVisible(s32 mode);

private:
    s32 mItemStates[ITEM_NUM];           // at 0x8 6 after construction
public:
    RPSysLayout* mpLayout;               // at 0x44
    RPSysLytAnmObj* mpAnmObjs[ANIM_NUM]; // at 0x48
};

/**
 * @brief Mii selection: pages of Mii faces by source, pointer and D-pad input, source tabs,
 * message prompts
 * @details No vtable. Size 0x3C, verified (Menu_MenuScene_configure allocates 0x3C with this
 * constructor). Sources (RPKokeshiMgr): 0 plaza Miis, 1 guests, 2-5 the Miis on each Wii Remote.
 * The layout items (RPMiiSelectLyt) are the three source tabs (0-2), the page arrows (3 next,
 * 4 previous) and ten faces (5-14).
 * @note Wii Play-only framework class (ledger Menu_MiiSelect); the name is a project convention.
 */
class RPMiiSelect {
public:
    //! A source's pages
    struct Page {
        bool mLoaded;  // at 0x0
        s32 mFirst;    // at 0x4 first Mii shown
        s32 mTotal;    // at 0x8 Miis in the source
        s32 mPage;     // at 0xC
        s32 mPageNum;  // at 0x10
        s32 mCount;    // at 0x14 Miis on this page
    };

    //! The chosen Mii
    struct Selection {
        s32 mSource; // at 0x0
        s32 mIndex;  // at 0x4
    };

    enum EMode {
        EMode_Plaza,
        EMode_Guest,
        EMode_Remote,
    };

public:
    RPMiiSelect(RPMenuFrame* pFrame, RPCPadCursor* pCPadCursor, RPWCursor* pWCursor,
                RPKokeshiMgr* pKokeshiMgr);

    void Load(EGG::Archive* pArchive, RPSysMessage* pMessage);
    void Start(s32 player);
    void Calculate(bool active);
    void DrawLayout();
    void DrawOverlay();
    Selection GetSelectedMii();

    void CalcEnter(bool active);
    void StartPageScroll();
    void CalcTabChange();
    void CalcFadeOut();
    void StartPage();
    void CalcFadeIn();
    void ShowMessage(s32 type);
    void OnMessageResult(bool yes);
    void CalcPointer(bool* pNoHit, bool* pOnBack);
    void CalcInput(bool noHit, bool onBack);
    void InitCategoryPages(s32 type, s32 chan);
    void AttachPageKokeshi(s32 dir);
    void InitCategory();
    bool IsItemEnabled(s32 item);
    s32 GetMiiIndexForItem(s32 source);
    s32 GetFirstFreeFace();
    s32 GetItemAfterPageChange();
    void PlayChoiceEffect(s32 player, s32 item);
    void PlayItemSe(s32 item, const EGG::Vector2f* pPos);

private:
    inline s32 GetRemoteSource();
    inline s32 GetSource();
    inline bool IsFirstPage();
    inline bool IsLastPage();
    inline bool IsMiiTaken(s32 item);
    inline void UpdateCursors();
    inline void ClearItems();

private:
    s32 mCursor;                  // at 0x0 current item
    s32 mMode;                    // at 0x4 EMode
    s32 mLastFace;                // at 0x8 last item that is not a page arrow
public:
    s32 mState;                   // at 0xC -1 after construction
private:
    s32 mMessageType;             // at 0x10
    s32 mLastPointed;             // at 0x14
    s32 mPlayer;                  // at 0x18
    u32 mChannel;                 // at 0x1C
    Page* mpPages;                // at 0x20 new[6]
    RPKokeshiMgr* mpKokeshiMgr;   // at 0x24
    RPSysMessage* mpMessage;      // at 0x28
    RPMenuFrame* mpFrame;         // at 0x2C
    RPCPadCursor* mpCPadCursor;   // at 0x30
    RPWCursor* mpWCursor;         // at 0x34
    RPMiiSelectLyt* mpLayout;     // at 0x38
};

//! @}

#endif
