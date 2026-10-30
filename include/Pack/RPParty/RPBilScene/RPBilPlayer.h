#ifndef RP_PARTY_BIL_PLAYER_H
#define RP_PARTY_BIL_PLAYER_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPBilScene/RPBilObject.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <egg/core.h>
#include <egg/math.h>

#include <nw4r/math.h>

//! @addtogroup rp_bil
//! @{

// Forward declarations
class RPGrpModel;
class RPSysEffect;
class RPBilPlayer;

// Original ABI entries; retain the existing public declarations used by linked units.
extern "C" bool CalcPointerRay__11RPBilPlayerFv(RPBilPlayer*);
extern "C" void UpdateArrowModel__11RPBilPlayerFv(RPBilPlayer*, s32);

/**
 * @brief A Billiards player: Wii Remote pointer and swing input, the cue, the aim arrow and
 * the pointer cursors (2 made by RPBilPlayerMgr)
 * @details vtable 8035DDC8. Size 0x68CC, verified (Bil_PlayerMgr_create). The constructor
 * builds the 9-state machine from sStateFuncs, the 3D sound player and the
 * Bil_camera_arrow_2d effect. Calculate (only for the current player, outside state 0) sets the
 * remote's rumble/motor parameters for the state, scales the pointer's distance parameter, and
 * records every new core status in a 200-entry ring buffer before running the state machine.
 * Load builds bad_cue, bad_arrow_00/01 and the five bad_cursor1-5 models. Score statistics
 * (0x6874-0x6880) feed RPBilResult::ComputeScores.
 */
class RPBilPlayer : public RPActorBase {
public:
    enum EState {
        STATE_NONE,
        STATE_PLACE_BALL,  //!< cue ball in hand
        STATE_AIM,
        STATE_ADDRESS,     //!< cue lined up, waiting for the swing
        STATE_STROKE,
        STATE_ROTATE_VIEW,
        STATE_AFTER_SHOT,
        STATE_WAIT,
        STATE_INACTIVE,    //!< the other player's turn (no functions)

        STATE_NUM
    };

    enum {
        STATUS_HISTORY_NUM = 200,
        ROTATE_HISTORY_NUM = 10,
        ARROW_MODEL_NUM = 2,
        CURSOR_MODEL_NUM = 5
    };

public:
    explicit RPBilPlayer(s32 index);
    virtual ~RPBilPlayer(); // at 0x8

    virtual void Load();      // at 0x78
    virtual void Reset();     // at 0x7C
    virtual void Calculate(); // at 0x80
    virtual void Exit();      // at 0x84

    /**
     * @brief IRPGrpDrawObject::UserDraw override: for the current player while addressing,
     * striking or after the shot, captures the 2D screen (pass 0, step 3) and draws it back as
     * a quad (pass 7, step 5) [M]
     */
    virtual void UserDraw(); // at 0x88

    void SetCursorsVisible(bool visible);
    void Rumble();
    void UpdateCursorModel();
    void UpdateArrowModel(s32 index);
    void UpdateCueModel();
    void CalcCueFromCamera();
    void UpdateMaxStat(s32 value);
    void IncFoulCount();          //!< clamped to 9
    void IncShotCount();          //!< clamped to 99
    void AddScore(s32 score);
    void SetChannel(s32 channel);

    s32 GetChannel() const {
        return mChannel;
    }
    s32 GetState() const {
        return mpStateMachine->GetState();
    }
    void ChangeState(s32 state) {
        mpStateMachine->ChangeState(state);
    }
    s32 GetScore() const {
        return mScore;
    }
    s32 GetShotCount() const {
        return mShotCount;
    }
    s32 GetFoulCount() const {
        return mFoulCount;
    }
    s32 GetMaxStat() const {
        return mMaxStat;
    }
    void CalcPointerRay();
    void UpdatePointerTarget();

private:
    /**
     * @brief Shows the aim marker where the camera's aim line meets an object ball (hidden when
     * the camera is moving or the hit is on the cue ball) [M]
     */
    void UpdateAimMarker();
    friend bool CalcPointerRay__11RPBilPlayerFv(RPBilPlayer*);
    friend void UpdateArrowModel__11RPBilPlayerFv(RPBilPlayer*, s32);

    EState GetPreviousState() const {
        return static_cast<EState>(mpStateMachine->GetPrevState());
    }

    void PlaceBallEnter();
    void PlaceBallCalc();
    void AimEnter();
    void AimCalc();
    void AddressEnter();
    void AddressCalc();
    void StrokeEnter();
    void StrokeCalc();
    void RotateViewEnter();
    void RotateViewCalc();
    void AfterShotEnter();
    void AfterShotCalc();
    void WaitEnter();
    void WaitCalc();

private:
    s32 mIndex;                                     // at 0x60
    s32 mChannel;                                   // at 0x64 controller channel
    RPStateMachine<RPBilPlayer>* mpStateMachine;    // at 0x68
    s32 unk6C;                                      // at 0x6C [L] pointer-target state
    EGG::Vector3f mPointerTarget;                   // at 0x70 [M] pointed table position
    EGG::Vector2f mCueBallScreenPos;                // at 0x7C [M] cue ball on the screen
    f32 mCueBallScreenRadius;                       // at 0x84 [M] its radius on the screen
    EGG::Vector3f unk88;                            // at 0x88 [L] read by AfterShotCalc
    u8 mCursorAlpha;                                // at 0x94 [M] cursor model alpha
    EGG::Vector3f mCursorPos;                       // at 0x98 [L]
    bool mPointerValid;                             // at 0xA4
    nw4r::math::VEC2 mPointerPos;                   // at 0xA8
    f32 mPointerDist;                               // at 0xB0 1.0 after Reset
    bool mPrevPointerValid;                         // at 0xB4 previous frame's copy
    nw4r::math::VEC2 mPrevPointerPos;               // at 0xB8
    f32 mPrevPointerDist;                           // at 0xC0
    f32 unkC4;                                      // at 0xC4 [L]
    EGG::Vector3f mPointerRayDir;                   // at 0xC8 [M] pointer ray: normalised direction
    EGG::Vector3f mPointerRayOrigin;                // at 0xD4 [M] and the camera position
    s32 mStatusHead;                                // at 0xE0 ring buffer write index
    s32 mStatusNum;                                 // at 0xE4 entries (<= 200)
    EGG::CoreStatus mStatusHistory[STATUS_HISTORY_NUM]; // at 0xE8
    f32 unk6808;                                    // at 0x6808 [L] RotateView
    f32 unk680C;                                    // at 0x680C [L] RotateView
    s32 unk6810;                                    // at 0x6810 [L] RotateView
    EGG::Vector2f mRotateHistory[ROTATE_HISTORY_NUM]; // at 0x6814 [M] cleared by RotateViewEnter
    u32 unk6864;                                    // at 0x6864 [L] set by AimEnter
    EGG::Vector3f unk6868;                          // at 0x6868 [L] set by StrokeEnter
    s32 mScore;                                     // at 0x6874 sum of pocketed numbers
    s32 mShotCount;                                 // at 0x6878
    s32 mFoulCount;                                 // at 0x687C [M]
    s32 mMaxStat;                                   // at 0x6880 [L] best run (UpdateMaxStat)
    EGG::Vector3f mCuePos;                          // at 0x6884
    nw4r::math::VEC3 mCueDir;                       // at 0x6890 assigned from nw4r VEC3 math
    f32 mCuePull;                                   // at 0x689C [M]
    RPGrpModel* mpCueModel;                         // at 0x68A0 bad_cue
    RPGrpModel* mpArrowModels[ARROW_MODEL_NUM];     // at 0x68A4 bad_arrow_00/01
    s32 mTargetBall;                                // at 0x68AC -1 when none
    RPGrpModel* mpCursorModels[CURSOR_MODEL_NUM];   // at 0x68B0 bad_cursor3, 4, 5, 1, 2
    RPBilSound3D* mpSound;                          // at 0x68C4
    RPSysEffect* mpArrowEffect;                     // at 0x68C8 Bil_camera_arrow_2d

    /**
     * @brief State functions, 803270A8 (copied to the stack by the constructor)
     */
    static const RPBilStateFuncs<RPBilPlayer> sStateFuncs[STATE_NUM];

public:
    static f32 sRemoteParamIdleA;
    static f32 sRemoteParamIdleB;
    static f32 sRemoteParamAimA;
    static f32 sRemoteParamAimB;
    static f32 sRemoteParamPlaceA;
    static f32 sRemoteParamPlaceB;
    static f32 sPointerParamB;
    static f32 sPointerParamMax;
    static f32 sRemoteDistNear;
    static f32 sRemoteDistFar;
    static f32 sAddressDistOpen;
    static f32 sAddressDistClose;
    static f32 sPointerTargetRange;
    static s32 sAimHintFrames;
    static f32 sStrokeParam0;
    static f32 sStrokeParam1;
    static f32 sCuePullRate;
    static f32 sStrokeMaxPower;
    static f32 sStrokeParam2;
    static f32 sStrokeParam3;
    static f32 sStrokeParam4;
    static f32 sStrokeParam5;
    static f32 sStrokeParam6;
    static f32 sStrokeParam7;
    static f32 sStrokeParam8;
    static f32 sStrokeParam9;
    static f32 sCueDist;
    static f32 sCueParamA;
    static f32 sCueParamB;
    static u8 sAddressByte;
    static s32 sAfterShotFrames;
    static f32 sPointerParamMin;
    static f32 sRotateSpeedX;
    static f32 sRotateSpeedY;
    static f32 sPlayerConstD4;
    static f32 sPlayerConstD8;
    static f32 sAimTurnSlow;
    static f32 sAimTurnMid;
    static f32 sAimTurnFast;
    static f32 sStrokeParamD[2];

    static f32 sArrowParamA;
    static f32 sArrowParamB[2];
};

//! Ledger Bil_PlayerSM (size 0x28)
typedef RPStateMachine<RPBilPlayer> RPBilPlayerSeq;

/**
 * @brief Holder of the two Billiards players and the turn order (singleton RPBilPlayerMgr::sInstance)
 * @details vtable 8035DE58 (destructor only). Size 0x18 [H] (Bil_PlayerMgr_create allocates
 * 0x18 and constructs it inline). StartTurn moves the current player to STATE_AIM (or
 * STATE_PLACE_BALL after a foul), the other one to STATE_INACTIVE, counts the shot and updates
 * the HUD scores.
 */
class RPBilPlayerMgr {
public:
    static RPBilPlayerMgr* sInstance; // 8043C9F0

    static const int PLAYER_MAX = 2;

public:
    RPBilPlayerMgr();
    virtual ~RPBilPlayerMgr(); // at 0x8

    void Load();
    void Reset();
    void Calculate();
    void Exit();

    void SetCursorsVisible(bool visible);
    bool IsAorBTriggered() const;
    void UpdatePlayerMaxStat(s32 player, s32 value);
    void IncPlayerFoulCount(s32 player);
    void AddPlayerScore(s32 player, s32 score);
    void StartTurn(s32 player, bool ballInHand);
    void AssignChannels();

    s32 GetPlayerNum() const {
        return mPlayerNum;
    }
    s32 GetCurrentPlayer() const {
        return mCurrentPlayer;
    }
    RPBilPlayer* GetPlayer(s32 i) const {
        return mpPlayers[i];
    }
    bool GetUnkC() const {
        return unkC;
    }
    bool GetUnkD() const {
        return unkD;
    }
    bool GetUnkE() const {
        return unkE;
    }
    void SetUnkE(bool value) {
        unkE = value;
    }
    bool GetUnkF() const {
        return unkF;
    }
    void SetUnkF(bool value) {
        unkF = value;
    }

    static void CreateInstance();
    static void DestroyInstance();

private:
    void SetWaiting(bool waiting);

private:
    s32 mPlayerNum;                     // at 0x4 Bil_playerNum
    s32 mCurrentPlayer;                 // at 0x8
    bool unkC;                          // at 0xC [L] cleared by StartTurn
    bool unkD;                          // at 0xD [L] cleared by StartTurn
    bool unkE;                          // at 0xE [L]
    bool unkF;                          // at 0xF [L]
    RPBilPlayer* mpPlayers[PLAYER_MAX]; // at 0x10
};

//! @}

#endif
