#ifndef RP_PARTY_BOM_LYT_H
#define RP_PARTY_BOM_LYT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_bom
//! @{

// Forward declarations
class RPSysEffect;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysMessage;

/**
 * @name Pose Mii layouts (poseMan.arc)
 * @details RPLytObjBase subclasses owned by RPBomLayoutMgr; each adds a destructor after
 * VF_0x18 (the ledger's exit, empty in all of them). Load mounts poseMan.arc, builds the
 * layout and its animation objects; texts come from party_message.bmg
 * (RPBomLayoutMgr::mpMessage). Sizes verified (Bom_LayoutMgr_create).
 * @note Names are a project convention (ledger Bom_Lyt*).
 */
//! @{

/**
 * @brief Title and theme ("pose!") banner (poseManStart_00.brlyt, messages 0xAF0 / 0xAF1)
 * @details vtable 8035F7C8. Size 0x24. States: 0 hidden, 1 title in, 2 title, 3 theme in,
 * 4 theme (signal animation while a bubble is past z 0), 5 theme out.
 */
class RPBomLytStart : public RPLytObjBase {
public:
    RPBomLytStart();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18 empty
    virtual ~RPBomLytStart();                // at 0x1C

    void ShowTitle();
    void ShowTheme();
    void PlayThemeOut();
    void UpdateStates();

private:
    RPSysLayout* mpLayout;            // at 0x8
    RPSysLytAnmObj* mpAnmObjs[4];     // at 0xC inAll, upDownMsg, outAll, signal
public:
    s32 mState;                       // at 0x1C
private:
    bool mSignalOn;                   // at 0x20
    bool mThemeShown;                 // at 0x21
};

/**
 * @brief Score display (poseManScore_00.brlyt, messages 0xAF2 / 0xAF3)
 * @details vtable 8035F7E8. Size 0x5C. Eight animations: inAll, moveScoreL, switchScoreR,
 * moveScoreR, switchScoreL, switch1pSco, switch2pSco, outAll. ShowMoveLeft / ShowMoveRight
 * show a point change (N_moveL* / N_moveR*) that is added to the score when it ends.
 */
class RPBomLytScore : public RPLytObjBase {
public:
    RPBomLytScore();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18 empty
    virtual ~RPBomLytScore();                // at 0x1C

    void PlayIn();
    void PlayOut();
    void ShowMoveLeft(s32 points);
    void ShowMoveRight(s32 points);
    void PlayScorePlusEffect(s32 player);
    void UpdateAnims();

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[8];   // at 0xC
    wchar_t mScoreText[2][6];       // at 0x2C full-width scores per player
    u32 mState;                     // at 0x44
    wchar_t mMoveText[6];           // at 0x48
    bool mMoveLeft;                 // at 0x54
    bool mMoveRight;                // at 0x55
    s32 mMovePoints;                // at 0x58
};

/**
 * @brief Points popping up over a matched bubble (poseManPoint_00.brlyt, message 0xAF4)
 * @details vtable 8035F808. Size 0x1C. One of the 16 in RPBomLayoutMgr's ring; N_dobCenter_00
 * shows doubled points (two-player target bubbles).
 */
class RPBomLytPoint : public RPLytObjBase {
public:
    RPBomLytPoint();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18 empty
    virtual ~RPBomLytPoint();                // at 0x1C

    void Show(const nw4r::math::VEC2* pScreenPos, s32 points, s32 player, BOOL doubled);

private:
    RPSysLayout* mpLayout;        // at 0x8
    RPSysLytAnmObj* mpAnmObj;     // at 0xC inOutAll
    wchar_t mText[4];             // at 0x10
    bool mActive;                 // at 0x18
};

/**
 * @brief Game end message (poseManEnd_00.brlyt, message 0xAF6 or 0xB08 for all clear)
 * @details vtable 8035F828. Size 0x20. States: 0 shown, 1 hidden, 2 in, 3 out (after 180
 * frames).
 */
class RPBomLytEnd : public RPLytObjBase {
public:
    RPBomLytEnd();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18 empty
    virtual ~RPBomLytEnd();                  // at 0x1C

    void Show(BOOL allClear);

public:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[2];   // at 0xC inEnd, outEnd
    s32 mState;                     // at 0x14
    s32 mTimer;                     // at 0x18
    bool mDone;                     // at 0x1C read by RPBomGameManager
};

/**
 * @brief Level number (poseManLevel_00.brlyt, message 0xAF7)
 * @details vtable 8035F848. Size 0x28. States: 0 shown, 1 hidden, 2 in, 3 out (after 110
 * frames); N_levelMsg_01 instead of the number on the last level.
 */
class RPBomLytLevel : public RPLytObjBase {
public:
    RPBomLytLevel();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18 empty
    virtual ~RPBomLytLevel();                // at 0x1C

    void Show();

private:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[2];   // at 0xC in, out
    wchar_t mLevelText[4];          // at 0x14
    s32 mState;                     // at 0x1C
    s32 mTimer;                     // at 0x20
public:
    bool mDone;                     // at 0x24
};

/**
 * @brief "Ready" message (poseManReady_00.brlyt, message 0xAF8)
 * @details vtable 8035F868. Size 0x20. States: 0 waiting, 1 hidden, 2 in, 3 out (then shows
 * the level layout).
 */
class RPBomLytReady : public RPLytObjBase {
public:
    RPBomLytReady();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18 empty
    virtual ~RPBomLytReady();                // at 0x1C

    void Show();
    void PlayOut();

public:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[3];   // at 0xC in, wait, out
    s32 mState;                     // at 0x18 read by RPBomGameManager
    bool mDone;                     // at 0x1C
};

/**
 * @brief A player's pose selector with arrows (poseManAB_00.brlyt for player 1,
 * poseManAB_01.brlyt for player 2)
 * @details vtable 8035F888. Size 0xC8. Four copies of the ten animations
 * (Bom_poseSelAnmNames); ChangePose plays a frame animation toward the new pose and the
 * Bom_posechange_*p_a / _b effect on the arrow that was pressed. States: 0 shown, 1 hidden,
 * 2 in, 3 out.
 */
class RPBomLytPoseSel : public RPLytObjBase {
public:
    explicit RPBomLytPoseSel(s32 player);

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18 empty
    virtual ~RPBomLytPoseSel();              // at 0x1C

    void Show();
    void PlayOut();
    void SetPose(s32 pose);
    void ChangePose(s32 player, s32 pose, BOOL reverse);
    void PlayBlocked(s32 player, BOOL reverse);
    void PlayPoseFrameAnim(s32 pose, BOOL reverse);
    void UpdatePoseTransition();
    void UpdateEffectPositions();
    void ResetPanes();
    void ShowActivePose(s32 frame);
    void ShowInactivePose(s32 frame);

    // RPBomPlayer::StatePosing: poses change only while shown (0)
    s32 GetState() const {
        return mState;
    }

private:
    RPSysLayout* mpLayout;              // at 0x8
    RPSysLytAnmObj* mpAnmObjs[4][10];   // at 0xC
    s32 mPlayer;                        // at 0xAC
    s32 mPoseFrom;                      // at 0xB0 [M] arrows shown by it, -1
    s32 mPoseFrame;                     // at 0xB4 pose -> frame index {0, 2, 1}, -1
    s32 mState;                         // at 0xB8
    bool mDone;                         // at 0xBC
    RPSysEffect* mpEffects[2];          // at 0xC0 Bom_posechange_*p_a (forward), _b (reverse)
};

/**
 * @brief Level clear, bonus and final score (poseManClear_00.brlyt, messages 0xB06 / 0xB07)
 * @details vtable 8035F8D0. Size 0x7C. Seven animations: in, bonus, outBg, out, inScore,
 * outScore, blink. In two-player mode the root pane slides up (mSlideY) for the winner
 * display.
 */
class RPBomLytClear : public RPLytObjBase {
public:
    RPBomLytClear();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18 empty
    virtual ~RPBomLytClear();                // at 0x1C

    void ShowBonus();
    void ShowFinalScore();
    void ShowWinner();
    void PlayOut();
    void PlayOutScore();
    void PlayScorePlusEffect();
    void SetClearText1P();
    void SetClearText2P();
    void SetBonusText2P();
    void UpdateSlide();
    void UpdateStates();
    void UpdateAnims();
    void PlayPlusEffect(s32 player);
    void ShowClearText1P();
    void ShowClearText2P();
    void ShowBonusText2P();

public:
    RPSysLayout* mpLayout;          // at 0x8
    RPSysLytAnmObj* mpAnmObjs[7];   // at 0xC
    wchar_t mScoreText[2][8];       // at 0x28
    wchar_t mBonusText[8];          // at 0x48
    wchar_t mBonusText2[8];         // at 0x58 formatted with RPBomLytClear::sBonusFormat
    s32 mState;                     // at 0x68
    s32 mSlideState;                // at 0x6C
    f32 mSlideY;                    // at 0x70 0 .. -69
    s32 mTimer;                     // at 0x74 180 after Reset
    bool mDone;                     // at 0x78 read by RPBomGameManager
    bool mScoreDone;                // at 0x79

    static wchar_t sBonusFormat[4];
};

/**
 * @brief Miss message (poseManMiss_00.brlyt)
 * @details vtable 8035F8F0. Size 0x14. SetMissText picks the 1P / 2P background and the
 * missing player's text.
 */
class RPBomLytMiss : public RPLytObjBase {
public:
    RPBomLytMiss();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18 empty
    virtual ~RPBomLytMiss();                 // at 0x1C

    void Show(s32 player);
    void SetMissText(s32 player);

private:
    RPSysLayout* mpLayout;      // at 0x8
    RPSysLytAnmObj* mpAnmObj;   // at 0xC inOutAll
    bool mShown;                // at 0x10
};

//! @}

/**
 * @brief Owner of every Pose Mii layout (singleton RPBomLayoutMgr::sInstance)
 * @details vtable 8035F7A0. Size 0x74, verified (Bom_LayoutMgr_createInstance). Create
 * allocates the layouts and a ring of 16 point popups; Load loads party_message.bmg and every
 * layout; Calculate / Draw run the layouts and the active popups (mPointNum ending at
 * mPointNext). Drawn by RPBomScene::UserDraw on view 3.
 * @note Name is a project convention (ledger Bom_LayoutMgr).
 */
class RPBomLayoutMgr : public RPSceneObj {
public:
    static RPBomLayoutMgr* sInstance; // 8043CAE0

    virtual ~RPBomLayoutMgr(); // at 0x8

    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Exit();      // at 0x1C
    /**
     * @brief Not in the vtable: slot 0x20 stays RPSceneObj's empty Draw [H]
     */
    void DrawLayouts();

    void ShowScoreMove(s32 player, s32 points);
    void SpawnScorePopup(const nw4r::math::VEC2* pScreenPos, s32 points, s32 player,
                         BOOL doubled);

    static void CreateInstance();
    static void DestroyInstance();

public:
    RPSysMessage* mpMessage;          // at 0x4 party_message.bmg
    RPBomLytStart* mpStart;           // at 0x8
    RPBomLytLevel* mpLevel;           // at 0xC
    RPBomLytReady* mpReady;           // at 0x10
    RPBomLytPoseSel* mpPoseSel[2];    // at 0x14 per player
    RPBomLytScore* mpScore;           // at 0x1C
    RPBomLytMiss* mpMiss;             // at 0x20
    RPBomLytClear* mpClear;           // at 0x24
    RPBomLytEnd* mpEnd;               // at 0x28
    RPBomLytPoint* mpPoints[16];      // at 0x2C
    s32 mPointNum;                    // at 0x6C active popups
    u32 mPointNext;                   // at 0x70 next ring slot (0-15)
};

//! @}

#endif
