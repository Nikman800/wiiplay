#ifndef RP_PARTY_BIL_BREAK_LOTTERY_H
#define RP_PARTY_BIL_BREAK_LOTTERY_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPBilScene/RPBilObject.h>

#include <egg/math.h>

#include <nw4r/math.h>

//! @addtogroup rp_bil
//! @{

/**
 * @brief Spins the cue ball, carrying the players' Mii faces, to choose who breaks (singleton
 * RPBilBreakLottery::sInstance)
 * @details vtable 8035E450. Size 0x38, verified (Bil_BreakLottery_create). Start puts the
 * faces on the cue ball (RPBilBallMgr::SetCueBallFace) and requests STATE_WAIT. The state runs
 * inline from a local table (no RPStateMachine): when mIsChanging, mNextState becomes mState,
 * the timer is cleared and the enter runs, then the calc. SpinEnter draws the winner with the
 * game's random generator and precomputes the start angle (pi for player 1) so that 240
 * frames of decelerating spin end on the winner's face.
 */
// Derives from the three bases directly (an RPBilScreen level adds a null check to the destructor)
class RPBilBreakLottery : public RP_DEBUG_STUB_1,
                          public IRPSysHostIOSocket,
                          public IRPGrpDrawObject {
public:
    static RPBilBreakLottery* sInstance; // 8043CA34

    enum EState {
        STATE_NONE,
        STATE_WAIT,
        STATE_SPIN,
        STATE_DONE,
        STATE_END, //!< no functions

        STATE_NUM
    };

public:
    RPBilBreakLottery();
    virtual ~RPBilBreakLottery(); // at 0x8

    virtual void Load();      // at 0x34 empty
    virtual void Reset();     // at 0x38
    virtual void Calculate(); // at 0x3C
    virtual void Exit();      // at 0x40

    virtual void UserDraw(); // at 0x44 empty

    void Start(s32 playerNum);

    s32 GetWinner() const {
        return mWinner;
    }
    s32 GetState() const {
        return mState;
    }

    static void CreateInstance();
    static void DestroyInstance();

private:
    void WaitEnter();
    void WaitCalc();
    void SpinEnter();
    void SpinCalc();
    void DoneEnter();
    void DoneCalc();

private:
    s32 mState;                   // at 0x10
    s32 mNextState;               // at 0x14
    bool mIsChanging;             // at 0x18
    s32 mTimer;                   // at 0x1C
    s32 mPlayerNum;               // at 0x20
    s32 mWinner;                  // at 0x24
    f32 mAngle;                   // at 0x28
    EGG::Vector3f mBaseAxisY;     // at 0x2C cue ball Y axis at SpinEnter (rotated by mAngle)

    // State functions: a local array in Calculate (initializer copy at 80327518)

public:
    static s32 sSpinFrames;
    static f32 sSpinDecay;
};

//! @}

#endif
