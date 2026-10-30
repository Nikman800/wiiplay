#ifndef RP_PARTY_FSH_TIMER_H
#define RP_PARTY_FSH_TIMER_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpDrawObject.h>
#include <Pack/RPParty/RPFshScene/RPFshObject.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;
class RPSysLytTextBox;

/**
 * @brief Fishing time limit and the "time up" countdown (fishingTimeUp_00.brlyt; singleton
 * Fsh_timer)
 * @details vtable 80358E80 (destructor and UserDraw only). Size 0x3C, verified (Fsh_Timer_new).
 * The time limit is 60 seconds (Fsh_getTimeLimitSec). The players and the fish stop once the
 * machine is in STATE_LAST_SECOND with mFrame >= mFrameLimit, or in STATE_END.
 */
class RPFshTimer : public IRPGrpDrawObject {
public:
    enum EState {
        STATE_NONE,
        STATE_IDLE,
        STATE_COUNT,
        STATE_COUNTDOWN,
        STATE_LAST_SECOND,
        STATE_END,
        STATE_DONE,

        STATE_NUM
    };

public:
    RPFshTimer();
    virtual ~RPFshTimer(); // at 0x8

    virtual void UserDraw(); // at 0xC

    void Load();
    void Reset();
    void Calculate();
    void Start(s32 frames);

    static s32 GetTimeLimitSec(); // 60

    bool IsWindowEntered() const {
        return mWindowEntered;
    }
    bool IsNotStarted() const {
        return mpStateMachine->IsStateSettled(STATE_NONE);
    }
    bool IsTimeUp() const {
        return (mpStateMachine->IsStateOrNext(STATE_END) && mFrame >= mFrameLimit) ||
               mpStateMachine->IsStateOrNext(STATE_DONE);
    }
    s32 GetState() const {
        return mpStateMachine->GetState();
    }
    bool IsCounting() const {
        s32 state = mpStateMachine->GetState();
        bool counting = state == STATE_COUNT || state == STATE_LAST_SECOND;
        return counting || state == STATE_COUNTDOWN;
    }
    s32 GetFrame() const {
        return mFrame;
    }

    /**
     * @brief Stops the count while a caught fish is shown (state 5) [H]
     */
    void Pause() {
        mpStateMachine->ChangeState(STATE_END);
    }
    void Resume() {
        mpStateMachine->ChangeState(STATE_IDLE);
    }

    void SetWindow(s32 start, s32 end) {
        mWindowStart = start;
        mWindowEnd = end;
    }

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshTimer* GetInstance() {
        return sInstance;
    }

private:
    void CountFrame();
    void UpdateWindow();

    void IdleEnter();
    void IdleCalc();
    void CountEnter();
    void CountCalc();
    void CountdownEnter();
    void CountdownCalc();
    void LastSecondEnter();
    void LastSecondCalc();
    void EndEnter();
    void EndCalc();

private:
    RPStateMachine<RPFshTimer>* mpStateMachine; // at 0x8
    s32 mFrame;                         // at 0xC [M]
    s32 mFrameLimit;                    // at 0x10 [M]
    s32 mTotalFrame;                    // at 0x14 [M] frames given to Start
    bool mInWindow;                     // at 0x18 [M] mFrame in [mWindowStart, mWindowEnd]
    bool mWindowEntered;                // at 0x19 [M] set for one frame at mWindowStart
    s32 mWindowStart;                   // at 0x1C [L] -1 after Reset
    s32 mWindowEnd;                     // at 0x20 [L] -1 after Reset
    RPSysLayout* mpLayout;              // at 0x24
    RPSysLytAnmObj* mpAnmObjs[2];       // at 0x28 count_00, count_01
    RPSysLytTextBox* mpNumberTexts[2];  // at 0x30 T_number_00/01
    RPSysLytResAccessor* mpResAccessor; // at 0x38

    // The state functions are a local table in the constructor (8031F830, .rodata)

    static RPFshTimer* sInstance; // 8043C768
};

//! @}

#endif
