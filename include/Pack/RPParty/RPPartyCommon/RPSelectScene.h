#ifndef RP_PARTY_COMMON_SELECT_SCENE_H
#define RP_PARTY_COMMON_SELECT_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPActorBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>
#include <Pack/RPSystem/RPPartySystemData.h>

#include <egg/math.h>
#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPCPadCursor;
class RPGameInfoLyt;
class RPMenuFrame;
class RPRankCompleteLyt;
class RPSelectGameLyt;
class RPSysKokeshi;
class RPSysMessage;

/**
 * @brief Game-select screen: game board, game info, player dolls, crowd, back button
 * (singleton sInstance)
 * @details vtable 803583B8. Size 0xE0, verified (Party_SelectScene_create). The constructor
 * also creates RPDollMgr, RPCrowdMgr and RPMsgBoard. 16 states (table 8031D5E0): 0 idle,
 * 1 start (menu BGM, unlock / all-medal Message Board posts) and wait for the save, 2 dialog,
 * 3 unlock message, 4 rank complete, 5 open select, 6 select, 7 decide game, 8 back press,
 * 9 show game info, 10 wait info open, 11 game info, 12 info confirm, 13 back button press,
 * 14 return to select, 15 finish. UserDraw draws the layouts on view 3 during draw step 5.
 * vtable layout (CodeWarrior): RPActorBase slots to 0x74, new virtuals from 0x78, UserDraw
 * primary slot at 0x84.
 * @note Wii Play-only framework class (ledger Party_SelectScene); the name is a project
 * convention.
 */
class RPSelectScene : public RPActorBase {
public:
    static void CreateInstance();
    static void DestroyInstance();
    /**
     * @brief Model resources of the select scene: the dolls' (RPDollMgr::GetResourceNum) (ledger Party_getConst12) [L]
     */
    static s32 GetResourceNum();
    static RPSelectScene* GetInstance() {
        return sInstance;
    }

    virtual ~RPSelectScene(); // at 0x8

    virtual void Load();      // at 0x78
    virtual void Reset();     // at 0x7C
    virtual void Calculate(); // at 0x80

    /**
     * @brief IRPGrpDrawObject override (ledger Party_SelectScene_draw) [H]
     */
    virtual void UserDraw(); // at 0x84

    RPSysKokeshi* GetDollAvatar(s32 player) const;

private:
    RPSelectScene();

    void HandleDollClicks(s32 player);

    void EnterStart();
    void CalcWaitSaveLoad();
    void EnterDialog();
    void CalcWaitDialog();
    void EnterUnlockGame();
    void CalcWaitUnlockMsg();
    void EnterRankComplete();
    void CalcRankComplete();
    void EnterOpenSelect();
    void CalcWaitSelectOpen();
    void EnterSelect();
    void CalcSelect();
    void EnterDecideGame();
    void CalcDecideGame();
    void EnterBackPress();
    void CalcBackHighlight();
    void EnterShowGameInfo();
    void CalcShowGameInfo();
    void EnterWaitInfoOpen();
    void CalcWaitInfoOpen();
    void EnterGameInfo();
    void CalcGameInfo();
    void EnterInfoConfirm();
    void CalcWaitInfoConfirm();
    void EnterBackButtonPress();
    void CalcWaitBackButton();
    void EnterReturnToSelect();
    void CalcGotoSelect();
    void EnterFinish();

public:
    RPStateMachine<RPSelectScene>* mpStateMachine; // at 0x60
    s32 mPlayerNum;                                // at 0x64 from RP_sceneInfoInstance + 0x4
    s32 unk68;                                     // at 0x68 -1 after Reset [L]
    s32 mCursor;                                   // at 0x6C game slot under the cursor
    s32 mLastGame;                                 // at 0x70 from RP_sceneInfoInstance + 0x28
    s32 mDecidedGame;                              // at 0x74
    s32 mPadCursor;                                // at 0x78 9 = none [M]
    s32 unk7C;                                     // at 0x7C 9 after Reset [L]
    bool mUsePad;                                  // at 0x80
    s32 mBackHoldTimer;                            // at 0x84 [M] counts to 60
    s32 mUnlockGame;                               // at 0x88 [M]
    s32 mCompleteRank;                             // at 0x8C passed to RPRankCompleteLyt::Open
    EGG::Vector2f mPointerPos;                     // at 0x90 active cursor position
    EGG::Vector2f mPrevPointerPos;                 // at 0x98 previous frame's mPointerPos
    RPPartySystemData mSystemData;                 // at 0xA0 save settings copy
    RPSelectGameLyt* mpSelectGameLyt;              // at 0xC0
    RPGameInfoLyt* mpGameInfoLyt;                  // at 0xC4
    RPRankCompleteLyt* mpRankCompleteLyt;          // at 0xC8
    RPMenuFrame* mpMenuFrame;                      // at 0xCC
    RPCPadCursor* mpCursor;                        // at 0xD0
    RPSysMessage* mpMessage;                       // at 0xD4 party_message.bmg
    u32 mSpeakerFlag;                              // at 0xD8 RPSysPlayerMgr::getSpeakerFlag(-1)
    bool unkDC;                                    // at 0xDC 1 after Reset [L]

    static RPSelectScene* sInstance; // 8043C6E8
};

//! Ledger Party_SelectSceneSeq (size 0x28)
typedef RPStateMachine<RPSelectScene> RPSelectSceneSeq;

//! @}

#endif
