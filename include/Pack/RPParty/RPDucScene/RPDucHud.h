#ifndef RP_PARTY_DUC_HUD_H
#define RP_PARTY_DUC_HUD_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <egg/core.h>
#include <nw4r/math.h>

//! @addtogroup rp_duc
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysMessage;

/**
 * @brief Point popup shown where a target was hit (duckhunt.arc, duckhuntPoint_00.brlyt)
 * @details vtable 8035AD10. Size 0x1C, verified (Duc_Hud_init).
 * @note Name is a project convention (ledger Duc_PointPopup).
 */
class RPDucPointPopup : public RPLytObjBase {
public:
    RPDucPointPopup();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPDucPointPopup();              // at 0x1C

    void Show(nw4r::math::VEC2 screenPos, s32 points, s32 player, s32 effect);

private:
    RPSysLayout* mpLayout;        // at 0x8
    RPSysLytAnmObj* mpAnmObjs[1]; // at 0xC inOutAll
    wchar_t mNumText[4];        // at 0x10
    bool mActive;               // at 0x18
};

/**
 * @brief Score and hit counter bar (duckhunt.arc, duckhuntScore_00.brlyt)
 * @details vtable 8035AD30. Size 0x90, verified (Duc_Hud_init). Twelve animations: in/out,
 * score L/R switch, hit L/R switch (two each), bonus L/R in, bonus L/R out.
 * @note Name is a project convention (ledger Duc_ScoreLayout).
 */
class RPDucScoreLayout : public RPLytObjBase {
public:
    RPDucScoreLayout();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPDucScoreLayout();             // at 0x1C

    void ShowHitCounter();
    void HideHitCounter();
    void ShowBonus(s32 player, s32 bonus);
    void PlayScoreAnim(s32 player, s32 type);

private:
    void UpdateHitCounterP1();
    void UpdateHitCounterP2();
    void PlayScoreAnimP1(s32 type);
    void PlayScoreAnimP2(s32 type);
    void UpdateAnimStates();

    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[12];  // at 0xC
    wchar_t mScoreText[2][4];       // at 0x3C 1P, 2P
    s32 mState;                     // at 0x4C
    wchar_t mHitText[2][4];         // at 0x50 1P, 2P
    wchar_t mBonusText[2][8];       // at 0x60 1P, 2P
    s32 mBonusState[2];             // at 0x80
    s32 mBonusTimer[2];             // at 0x88 120 frames
};

/**
 * @brief Round start banner with the round number and scores (duckhuntStart_00.brlyt)
 * @details vtable 8035AD50. Size 0x30, verified (Duc_Hud_init).
 * @note Name is a project convention (ledger Duc_StartLayout).
 */
class RPDucStartLayout : public RPLytObjBase {
public:
    RPDucStartLayout();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPDucStartLayout();             // at 0x1C

    void PlayIn();
    void PlayOut();
    void DrawMiiIcons();

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[2];   // at 0xC in, out
    s32 mState;                     // at 0x14
    wchar_t mRoundText[4];          // at 0x18
    wchar_t mScoreText[2][4];       // at 0x20 1P, 2P
};

/**
 * @brief Final score panel (duckhuntEnd_00.brlyt)
 * @details vtable 8035B0D0. Size 0x38, verified (Duc_Hud_init). Animations: in, out,
 * switch 1P, switch 2P. mDone is RPDucHud::IsEndAnimDone.
 * @note Name is a project convention (ledger Duc_EndLayout).
 */
class RPDucEndLayout : public RPLytObjBase {
public:
    RPDucEndLayout();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPDucEndLayout();               // at 0x1C

    void PlayIn();
    void PlayOut();
    /**
     * @brief Hides the single-player result (state 3); inlined into Calculate, unused
     * elsewhere (dead-stripped) [M]
     */
    void PlayOutSingle();
    /**
     * @brief Hides the message pane (state 4); inlined into Calculate, unused elsewhere [M]
     */
    void PlayOutMsg();
    void DrawMiiIcons();
    void UpdateAnimStates();
    bool IsDone() const { return mDone; }

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[4];   // at 0xC
    s32 mState;                     // at 0x1C
    s32 mTimer;                     // at 0x20 180 frames
    bool unk24;                     // at 0x24 [L]
    bool mDone;                     // at 0x25
    wchar_t mScoreText[2][4];       // at 0x26 1P, 2P
};

/**
 * @brief Owns the Shooting Range layouts and a ring of 32 point popups
 * (singleton Duc_hudInstance)
 * @details vtable 8035AB60. Size 0x9C, verified. Messages from party_message.bmg.
 * @note Name is a project convention (ledger Duc_Hud).
 */
class RPDucHud : public RPSceneObj {
    friend class RPDucPointPopup;

public:
    virtual ~RPDucHud(); // at 0x8

    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    /**
     * @brief Calls VF_0x18 on every layout [M]
     */
    virtual void Exit(); // at 0x1C

    static void CreateInstance();
    static void DestroyInstance();

    static RPDucHud* sInstance;

    RPSysMessage* GetMessage() const { return mpMessage; }
    RPDucEndLayout* GetEndLayout() const { return mpEndLayout; }

    void DrawLayouts();
    bool IsEndAnimDone() const;
    void SetHitCounterVisible(bool visible);
    void SetStartBannerVisible(bool visible);
    void ShowBonus(s32 player, s32 bonus);
    void SpawnScorePopup(nw4r::math::VEC2 screenPos, s32 points, u32 player, s32 effect,
                         bool combo);

private:
    RPSysMessage* mpMessage;           // at 0x4
    RPDucScoreLayout* mpScoreLayout;   // at 0x8
    RPDucStartLayout* mpStartLayout;   // at 0xC
    RPDucEndLayout* mpEndLayout;       // at 0x10
    RPDucPointPopup* mpPopups[32];     // at 0x14
    u32 mPopupNum;                     // at 0x94 active popups
    u32 mPopupNext;                    // at 0x98 ring index
};

//! @}

#endif
