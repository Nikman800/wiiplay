#ifndef RP_SYSTEM_PLAYER_SELECT_SCENE_H
#define RP_SYSTEM_PLAYER_SELECT_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPSystem/RPSysScene.h>

//! @addtogroup rp_system
//! @{

// Forward declarations
class RPCPadCursor;
class RPKokeshiMgr;
class RPLogoModel;
class RPMenuBgModel;
class RPMenuFrame;
class RPMiiSelect;
class RPPlayerNumSelect;
class RPSysLayout;
class RPSysMessage;
class RPWCursor;

/**
 * @brief Player setup menu scene: player count, Mii select, player settings, confirmation
 * @details vtable 803579A8. Size 0x8C, verified (RPSysSceneCreator::create, inlined
 * constructor). menuCommon.arc / menuBg_00.brlyt. Configure creates model scenes 0-3, the menu
 * background and logo models (or the RPPtMenuBg singleton in the other menu mode), the frame
 * layout (RPMenuFrame), both cursors, RPPlayerNumSelect, RPKokeshiMgr, RPMiiSelect, the
 * RPPlayerSettingPanel and RPPlayerConfirm singletons and two 2D views.
 * The constructor is inline (RPSysSceneCreator::create). Its code is in
 * RPParty/RPPartyCommon/RPMenuScene.cpp (ledger Menu_MenuScene).
 */
class RPSysPlayerSelectScene : public RPSysScene {
public:
    virtual ~RPSysPlayerSelectScene(); // at 0x8

    virtual void Configure();    // at 0x44
    virtual void LoadResource(); // at 0x48
    virtual void Reset();        // at 0x4C
    virtual void Calculate();    // at 0x50
    virtual void Exit();         // at 0x54
    virtual void UserDraw();     // at 0x5C

private:
    void CalcPlayerCountSelect();
    void CalcMiiSelect();
    void CalcWaitMiiLoad();
    void CalcPlayerSettings();
    void CalcConfirm();
    void CalcStartGame();
    void CalcPaused();
    // Commits the players' Miis and sets nextState; kind 0 first opens system window 0x122/0x123
    void CommitPlayerMii(s32 nextState, s32 kind);
    // Wii Play: plays the controller-change SE (this is unused)
    void PlayControllerChangeSe(s32 player, BOOL ignoreActive, BOOL useActive);
    // Sets the frame caption (msgId, or the pack/scene specific title)
    void SetTitleMessage(u32 msgId);
    // Loads the players' saved Miis; false if one is missing
    bool LoadPlayerMiis();
    void StartMiiSelect();
    void StartPlayerSettings();

private:
    s32 mState;                           // at 0x44
    s32 mNextState;                       // at 0x48 state after the system window (16)
    bool mWindowShown;                    // at 0x4C pause: the disconnect window is open
    bool mSaved;                          // at 0x4D confirm: the player data was saved
    bool mLoadOldMii[4];                  // at 0x4E per player count - 1: try the saved Miis
    bool mOldMiiLoaded;                   // at 0x52 the saved Miis were loaded
    bool mFrameShown;                     // at 0x53
    s32 mPlayer;                          // at 0x54 player being set up
    s32 mMenuMode;                        // at 0x58 RPSysProjectLocal pack (0: Sports Pack)
    u32 mPauseTimer;                      // at 0x5C
    s32 mAdviceMode;                      // at 0x60 -1 after configure, passed to the confirm
    RPSysMessage* mpMessage;              // at 0x64
    RPMenuBgModel* mpBgModel;             // at 0x68 mode 0 only
    RPLogoModel* mpLogoModel;             // at 0x6C mode 0 only
    RPSysLayout* mpBgLayout;              // at 0x70 menuBg_00.brlyt, mode 0 only
    RPMenuFrame* mpFrame;                 // at 0x74
    RPCPadCursor* mpCPadCursor;           // at 0x78
    RPWCursor* mpWCursor;                 // at 0x7C
    RPPlayerNumSelect* mpPlayerNumSelect; // at 0x80
    RPMiiSelect* mpMiiSelect;             // at 0x84
    RPKokeshiMgr* mpKokeshiMgr;           // at 0x88
};

//! @}

#endif
