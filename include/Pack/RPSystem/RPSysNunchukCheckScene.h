#ifndef RP_SYSTEM_NUNCHUK_CHECK_SCENE_H
#define RP_SYSTEM_NUNCHUK_CHECK_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel.h>
#include <Pack/RPSystem/RPSysScene.h>

//! @addtogroup rp_system
//! @{

// Forward declarations
class RPFrameDown;
class RPSportsMsgPanel;
class RPSysKokeshiIcon;
class RPSysLayout;
class RPSysMessage;
namespace EGG {
class Screen;
} // namespace EGG

/**
 * @brief Pre-game screen that waits until every player holds the remote as shown (Sports
 * message, sports_message.bmg)
 * @details vtable 80357D08 (RPSysScene, then the IRPSysHostIOSocket part at 0x44, then
 * UserDraw). Size 0x98, verified (RPSysSceneCreator::create, inlined constructor).
 * A player is ready after holding the remote alone for 20 frames (or, for a game that needs
 * the Nunchuk, as soon as one is attached); the OK button of the RPFrameDown continues.
 * Wii Sports leftover: in Wii Play only the pack != Sports branches run.
 * Its code is in RPParty/RPPartyCommon/RPSportsMessageScene.cpp.
 */
struct RPSysNunchukCheckSportInfo;

class RPSysNunchukCheckScene : public RPSysScene, public IRPSysHostIOSocket {
public:
    static const int PLAYER_NUM = 4;

    enum EState {
        EState_Start,        //!< First frame: checks the controllers once
        EState_Check,        //!< Waiting for every player
        EState_AllReady,     //!< Everybody ready, short wait
        EState_StartGame,    //!< Change to the game scene
        EState_BackPressed,  //!< Back button pressed, waiting for its animation
        EState_GoBack,       //!< Change to the previous scene
        EState_OpenWindow,   //!< Opens the "quit?" system window
        EState_WaitWindow,   //!< Waits for its answer
        EState_OpenWindow2,  //!< Opens the second system window
        EState_WaitWindow2,  //!< Waits for its answer, then leaves
    };

public:
    virtual ~RPSysNunchukCheckScene(); // at 0x8

    virtual void Configure();    // at 0x44
    virtual void LoadResource(); // at 0x48
    virtual void Reset();        // at 0x4C
    virtual void Calculate();    // at 0x50
    virtual void Exit();         // at 0x54
    virtual void UserDraw();     // IRPGrpDrawObject

private:
    s32 GetSportID(s32 sceneID);
    bool IsNunchukGame();
    void CalcOkButton();
    void GoNext();
    void StartGame();
    void OnAllReady();
    void CheckPlayersReady();
    bool IsPlayerReady(u8 idx);
    void Start();
    void ResetState();

private:
    EGG::Screen* mpScreen;                    // at 0x48
    RPSysMessage* mpMessage;                  // at 0x4C sports_message.bmg
    s32 mState;                               // at 0x50
    u8 mPlayerIdx;                            // at 0x54 loop counter
    u8 mHoldFrames[PLAYER_NUM];               // at 0x55
    s16 mPlayerNum;                           // at 0x5A
    s32 mTimer;                               // at 0x5C
    bool mNeedNunchuk;                        // at 0x60
    bool mSkip;                               // at 0x61 everybody was ready on the first frame
    RPFrameDown* mpFrameDown;                 // at 0x64
    bool mBackHovered;                        // at 0x68
    RPSysLayout* mpLayout;                    // at 0x6C
    RPSportsMsgPanel* mpPanels[PLAYER_NUM];   // at 0x70
    nw4r::lyt::Pane* mpNunchukPanes[2];       // at 0x80 N_nunchuk_01 (remote only), N_nunchuk_00
    RPSysKokeshiIcon* mpIcons[PLAYER_NUM];    // at 0x88

public:
    static RPSysNunchukCheckSportInfo* spSportInfo; // the sport the scene was opened for
};

//! @}

#endif
