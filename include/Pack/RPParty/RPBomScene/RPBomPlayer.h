#ifndef RP_PARTY_BOM_PLAYER_H
#define RP_PARTY_BOM_PLAYER_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpModelCallback.h>
#include <Pack/RPParty/RPPartyCommon/RPAxes.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_bom
//! @{

// Forward declarations
class RPBomPlayerSound;
class RPGrpModel;
class RPSysEffect;
class RPSysKokeshi;

/**
 * @brief One player's posing Mii (and the bubble around it after a miss)
 * @details vtable 8035ECA4 (the IRPGrpModelCallback vtable is the primary one: no constructor
 * chain, the destructor only frees). Size 0x164, verified (Bom_PlayerManager_createPlayers).
 * The Mii is an RPSysKokeshi built with the RP_Base8034F478 constructor (0xB4 bytes) and the
 * bom_pose_anim / ira_pose_C / ira_pose_win / ira_pose_lose animations; the pose (0-2) is a
 * frame range of bom_pose_anim (pose * 20) changed with the B / A buttons (mPoseCooldown 10
 * frames). The position follows the player's RPBomCursor; after a hit it follows the bubble
 * (RPBomPanelPool::GetPanelTransform of mPanelIdx).
 * Two PTMF tables copied from Bom_playerStateFuncs (14 file-static PTMFs in RPBomPlayer.cpp,
 * copied one by one): mMoveFuncs (run by CalcState) and mStateFuncs (run by CalcMove), both
 * indexed by mState: 0 idle, 1 posing (follow cursor), 2 hit wait (on a
 * matched bubble), 3 inside the start bubble (level intro), 4 angry (ira), 5 win, 6 result.
 * @note Name is a project convention (ledger Bom_Player).
 */
class RPBomPlayer : public IRPGrpModelCallback {
public:
    typedef void (RPBomPlayer::*StateFunc)();

public:
    RPBomPlayer();
    virtual ~RPBomPlayer(); // at 0x8

    /**
     * @brief IRPGrpModelCallback::EditWorldMtx override on the Mii model: copies its hip node
     * into mEffectMtx and moves the three pose-change effects there (ledger
     * attachEffectsToNode) [H]
     */
    virtual void EditWorldMtx(RPGrpModel* pModel); // at 0x14

    /**
     * @brief Creates the pose-change effects, the Mii (RPSysKokeshi) and the sound actor
     * (remote flag 1 or 2 by player) [H]
     */
    void Create();
    void Load();
    void Reset();
    void CalcMove();
    void CalcState();
    void UpdateModel();
    void StartPosing(s32 pose, f32 scale);
    void RestartPosing();
    void RestartPosing2();
    void SetStateIdle();
    void SetStateIra(BOOL playAnim);
    void SetStateWin(BOOL playAnim);
    void SetStateResult();
    void PlayResultAnim(BOOL win);
    void PlayHitSound(s32 colorType);
    void TriggerPoseEffects();

    void MoveNop0();
    void MoveFollowCursor1();
    void MoveAlongBubble2();
    void MoveAlongBubble3();
    void MoveNop4();
    void MoveFollowCursor5();
    void MoveNop6();
    void StateNop0();
    void StatePosing();
    void StateHitWait();
    void StateNop3();
    void StateNop4();
    void StateNop5();
    void StateNop6();

public:
    nw4r::math::VEC3 mPos;            // at 0x4
    nw4r::math::VEC3 mScale;          // at 0x10
    RPAxes mRot;                      // at 0x1C
    f32 mBaseScale;                   // at 0x40 13.0 after Reset (level data 0x18/0x1C)
    s32 mHitResult;                   // at 0x44 set by RPBomPanel::PostCalc: 1 matched, 2 start bubble
    s32 mPanelIdx;                    // at 0x48 hit bubble, -1 for none
    s32 mTimer;                       // at 0x4C hit wait (10 frames)
    s32 mPose;                        // at 0x50 0-2
    f32 mPoseFrame;                   // at 0x54 mPose * 20
    bool mPoseReverse;                // at 0x58 last change was backwards (anim rate -2)
    bool mInBubble;                   // at 0x59 bom_bubble model shown
    u32 unk5C;                        // at 0x5C [L] cleared by Reset, never read
    s32 mPoseCooldown;                // at 0x60
    s32 mFrame;                       // at 0x64 frames while not idle
    s32 mPlayer;                      // at 0x68
    s32 mState;                       // at 0x6C
    f32 mPrevAnmFrame;                // at 0x70 for TriggerPoseEffects (frames 13, 32, 51)
    nw4r::math::MTX34 mEffectMtx;     // at 0x74
    RPSysEffect* mpPoseEffects[3];    // at 0xA4 Bom_change_pose01-03
    RPSysKokeshi* mpKokeshi;          // at 0xB0
    RPGrpModel* mpBubbleModel;        // at 0xB4 bom_bubble
    RPBomPlayerSound* mpSound;        // at 0xB8
    StateFunc mMoveFuncs[7];          // at 0xBC
    StateFunc mStateFuncs[7];         // at 0x110
};

// (0, -1, 0), built by RPBomPlayer.cpp's static initializer
extern nw4r::math::VEC3 Bom_vecDown;
extern nw4r::math::VEC3 RP_vecEy;

/**
 * @brief Owner of the players (singleton RPBomPlayerManager::sInstance)
 * @details vtable 8035ED50. Size 0x1C, verified (Bom_PlayerManager_createInstance). Load /
 * Reset run all created players; the other loops run the active ones (1 or 2 by game mode).
 * @note Name is a project convention (ledger Bom_PlayerManager).
 */
class RPBomPlayerManager : public RPSceneObj {
public:
    static RPBomPlayerManager* sInstance; // 8043CA90

    virtual ~RPBomPlayerManager(); // at 0x8

    virtual void Load();  // at 0x10
    virtual void Reset(); // at 0x14

    /**
     * @brief Allocates num players (called by RPBomScene with 2) [H]
     */
    void CreatePlayers(s32 num);
    void CalcMove();
    void CalcState();
    void UpdateModels();
    void StartAllPosing(f32 scale, s32 pose);
    void RestartAllPosing();
    void RestartAllPosing2();
    void SetAllIdle();
    void SetAllStateResult();
    void SetPlayerStateWin(s32 player, BOOL playAnim);
    void SetStateIraByMask(u32 mask);
    /**
     * @brief Result animations: winner 0 / 1 or 2 for a draw (two players), or win / lose
     * for the single player [M]
     */
    void PlayResultAnims(s32 result);

    static void CreateInstance();
    static void DestroyInstance();

public:
    s32 mPlayerNum;              // at 0x4
    s32 mActiveNum;              // at 0x8
    RPBomPlayer* mpPlayers[4];   // at 0xC
};

//! @}

#endif
