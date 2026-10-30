#ifndef RP_PARTY_TNK_LYT_H
#define RP_PARTY_TNK_LYT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysMessage;

/**
 * @name Tanks! layouts (tank.arc)
 * @details Owned by RPTnkLayoutManager. Each loads one layout and its animations from
 * tank.arc in Load; the empty slot 0x18 is the layout manager's Exit call. Number texts are
 * formatted as full-width digits (Lyt_formatSignedIntFullWidth).
 */
//! @{

/**
 * @brief Stage fades with mission, enemy count, lives and bonus texts (tnkfade.brlyt)
 * @details vtable 8035C240. Size 0x6C, verified (RPTnkLayoutManager::Create). Animations
 * in, out, inLife, outLife, inBonus, outBonus, addLife_00/01, inMission, inEnemy, loop.
 */
class RPTnkFade : public RPLytObjBase {
public:
    RPTnkFade();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPTnkFade();                    // at 0x1C

    /**
     * @brief Starts the fade in: with bonus, the extra life is awarded first; fast plays the
     * fade at rate 15
     */
    void Start(bool bonus, bool fast);
    void UpdateStates();

    bool IsDone() const { return mDone; }

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[11];  // at 0xC
    s32 mState;                     // at 0x38
    s32 mTimer;                     // at 0x3C
    s32 mLives;                     // at 0x40
    wchar_t mLivesText[4];          // at 0x44
    s32 mMission;                   // at 0x4C stage + 1
    wchar_t mMissionText[4];        // at 0x50
    s32 mEnemyNum;                  // at 0x58
    wchar_t mEnemyText[4];          // at 0x5C
    bool mFast;                     // at 0x64
    bool mBonus;                    // at 0x65
    bool mDone;                     // at 0x66 set when the fade out has finished
    s32 mFrame;                     // at 0x68 frames since Reset
};

/**
 * @brief In-game HUD: score, stage and enemy counters, player balloons (tnkUI_00)
 * @details vtable 8035C260. Size 0x4C, verified. Animations in, point1P, point2P, out.
 */
class RPTnkHud : public RPLytObjBase {
public:
    RPTnkHud();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPTnkHud();                     // at 0x1C

    void PlayIn();
    void PlayOut();
    void PlayScoreAnim(s32 player);
    void PlaceBalloonP1(f32 angle);
    void PlaceBalloonP2(f32 angle);
    void SetShowBalloons(bool show) { mShowBalloons = show; }

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[4];   // at 0xC
    s32 mState;                     // at 0x1C
    wchar_t mScoreText[2][4];       // at 0x20 1P, 2P
    wchar_t mStageText[4];          // at 0x30
    wchar_t mEnemyText[4];          // at 0x38
    f32 mBalloonX;                  // at 0x40 [L]
    f32 mBalloonY;                  // at 0x44 [L]
    bool mShowBalloons;             // at 0x48 [M]
};

/**
 * @brief Hit marker popup at a destroyed tank (tnkHit_00)
 * @details vtable 8035C280. Size 0x1C, verified. 16 are kept in a ring by the layout manager.
 */
class RPTnkHitPopup : public RPLytObjBase {
public:
    RPTnkHitPopup();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPTnkHitPopup();                // at 0x1C

    /**
     * @brief Shows the number at a screen position (clamped to [-0.9, 0.9] in place), with
     * the 1P (type 0) or 2P pane
     */
    void Start(nw4r::math::VEC2 rScreenPos, s32 num, s32 type);

    bool IsActive() const { return mActive; }

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[1];   // at 0xC hit_00
    wchar_t mNumText[4];            // at 0x10
    bool mActive;                   // at 0x18
};

/**
 * @brief Control hint balloon (tnkEx_00)
 * @details vtable 8035C2A0. Size 0x28, verified. Animations in, out, outAll.
 */
class RPTnkExMsg : public RPLytObjBase {
public:
    RPTnkExMsg();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPTnkExMsg();                   // at 0x1C

    void Show(s32 msg, f32 x);
    void PlayOut();
    void PlayOutAll();

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[3];   // at 0xC
    s32 mState;                     // at 0x18
    u32 unk1C;                      // at 0x1C [L]
    u32 unk20;                      // at 0x20 [L]
    bool unk24;                     // at 0x24 [L]
};

/**
 * @brief "Start!" message (tnkMsg_01)
 * @details vtable 8035C348. Size 0x10, verified.
 */
class RPTnkStartMsg : public RPLytObjBase {
public:
    RPTnkStartMsg();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPTnkStartMsg();                // at 0x1C

    void Play();

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[1];   // at 0xC start_00
};

/**
 * @brief "Mission cleared" message with destroyed-tank totals (tnkMsg_02)
 * @details vtable 8035C368. Size 0x30, verified. Animations inCleare, inTotal.
 */
class RPTnkClearMsg : public RPLytObjBase {
public:
    RPTnkClearMsg();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPTnkClearMsg();                // at 0x1C

    void Show();
    void ShowTotal();
    void Hide();

    bool IsActive() const { return mActive; }

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[2];   // at 0xC
    s32 mState;                     // at 0x14
    s32 mTimer;                     // at 0x18
    bool mActive;                   // at 0x1C
    wchar_t mTotalText[2][4];       // at 0x1E [M]
};

/**
 * @brief "Mission failed" message (tnkMsg_03)
 * @details vtable 8035C388. Size 0x20, verified. Animations in, out.
 */
class RPTnkFailedMsg : public RPLytObjBase {
public:
    RPTnkFailedMsg();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPTnkFailedMsg();               // at 0x1C

    void Show();

    bool IsActive() const { return mActive; }

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[2];   // at 0xC
    s32 mState;                     // at 0x14
    s32 mTimer;                     // at 0x18
    bool mActive;                   // at 0x1C
};

/**
 * @brief "Mission complete" message after the last stage (tnkMsg_04)
 * @details vtable 8035C3A8. Size 0x20, verified. Animations in, out.
 */
class RPTnkCompleteMsg : public RPLytObjBase {
public:
    RPTnkCompleteMsg();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPTnkCompleteMsg();             // at 0x1C

    void Show();

    bool IsActive() const { return mActive; }

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[2];   // at 0xC
    s32 mState;                     // at 0x14
    s32 mTimer;                     // at 0x18
    bool mActive;                   // at 0x1C
};

/**
 * @brief Result list: destroyed tanks per enemy type and totals (tnkResult_00)
 * @details vtable 8035C3F0. Size 0xE0, verified. Animations inList, inScore, inTotal, win1P,
 * win2P, outList, outTotal, upTotal. Rows are revealed one by one with a sound.
 */
class RPTnkResultLyt : public RPLytObjBase {
public:
    RPTnkResultLyt();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPTnkResultLyt();               // at 0x1C

    void Start();
    void ShowTotals();
    void HideAllPanes();
    void CalcSequence1P();
    void CalcSequence2P();
    void RevealRow1P(s32 type);
    void RevealRow2P(s32 type);
    // Wii Play: the row position is independent of the enemy type.
    void RevealRow1P(s32 type, s32 row);
    void RevealRow2P(s32 type, s32 row);
    void CloseList1P();
    void CloseList2P();
    void PlayRowSound();
    void PlayRowSound(s32 row);
    void SetCountTexts1P();
    void SetCountTexts2P();

    // Wii Play: unk3D is set when the closing animation of the list has finished
    bool IsFinished() const { return unk3D; }

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[8];   // at 0xC
    s32 mState;                     // at 0x2C
    s32 mTimer;                     // at 0x30 [M]
    s32 mRow;                       // at 0x34 rows revealed (9 = all)
    s32 unk38;                      // at 0x38 [L]
    bool unk3C;                     // at 0x3C [L]
    bool unk3D;                     // at 0x3D [L]
    wchar_t mCountText[2][9][4];    // at 0x3E per player and enemy type
    wchar_t mTotalText[2][4];       // at 0xCE
};

/**
 * @brief Plays the control hint balloon's out animation (ledger Tnk_hideExMsg; called by
 * RPTnkGameManager)
 */
void RPTnkHideExMsg();

//! @}

/**
 * @brief Owns every Tanks! layout and the BMG message (singleton Tnk_layoutManagerInstance)
 * @details vtable 8035C1F0. Size 0x70, verified (Tnk_LayoutManager_createInstance). Draw and
 * Calculate pick the message layouts from RPTnkGameManager's state.
 */
class RPTnkLayoutManager : public RPSceneObj {
public:
    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkLayoutManager(); // at 0x8

    virtual void Create();    // at 0xC
    /**
     * @brief Creates the party_message.bmg message and loads every layout [H]
     */
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Exit();      // at 0x1C

    void Draw2D();
    /**
     * @brief Starts the next hit popup of the ring (two players only, at most 16 at once)
     */
    void SpawnHitPopup(const nw4r::math::VEC2& rScreenPos, s32 num, s32 type);

    RPSysMessage* GetMessage() const { return mpMessage; }
    void DecHitPopupNum() { mHitPopupNum--; }
    RPTnkExMsg* GetExMsg() const { return mpExMsg; }
    RPTnkHud* GetHud() const { return mpHud; }
    RPTnkStartMsg* GetStartMsg() const { return mpStartMsg; }
    RPTnkClearMsg* GetClearMsg() const { return mpClearMsg; }
    RPTnkFailedMsg* GetFailedMsg() const { return mpFailedMsg; }
    RPTnkCompleteMsg* GetCompleteMsg() const { return mpCompleteMsg; }
    RPTnkResultLyt* GetResultLyt() const { return mpResultLyt; }
    RPTnkFade* GetFade() const { return mpFade; }

    static RPTnkLayoutManager* sInstance; // Tnk_layoutManagerInstance

private:
    RPSysMessage* mpMessage;          // at 0x4
    RPTnkHud* mpHud;                  // at 0x8
    RPTnkExMsg* mpExMsg;              // at 0xC
    RPTnkStartMsg* mpStartMsg;        // at 0x10
    RPTnkClearMsg* mpClearMsg;        // at 0x14
    RPTnkFailedMsg* mpFailedMsg;      // at 0x18
    RPTnkCompleteMsg* mpCompleteMsg;  // at 0x1C
    RPTnkResultLyt* mpResultLyt;      // at 0x20
    RPTnkHitPopup* mpHitPopups[16];   // at 0x24 ring buffer
    u32 mHitPopupNum;                 // at 0x64
    u32 mHitPopupNext;                // at 0x68
    RPTnkFade* mpFade;                // at 0x6C
};

//! @}

#endif
