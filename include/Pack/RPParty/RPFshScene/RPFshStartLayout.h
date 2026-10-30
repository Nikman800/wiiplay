#ifndef RP_PARTY_FSH_START_LAYOUT_H
#define RP_PARTY_FSH_START_LAYOUT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshObject.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;
class RPSndRangeTable;

/**
 * @brief Fishing start sequence: camera intro, "ready", 3-2-1 countdown (fishingStart_00.brlyt;
 * singleton Fsh_startLayout)
 * @details vtable 80358FB0. Size 0x40, verified (Fsh_StartLayout_new). The other objects poll
 * the machine's state: the players and the fish only run once it is STATE_PLAY (4), and the
 * score/player managers treat states 0 and 1 as "not started".
 */
// Wii Play: direct bases avoid an extra intermediate-base destructor null check.
class RPFshStartLayout : public RP_DEBUG_STUB_1,
                         public IRPSysHostIOSocket,
                         public IRPGrpDrawObject {
public:
    enum EState {
        STATE_NONE,
        STATE_INTRO,
        STATE_WAIT_READY,
        STATE_COUNT,
        STATE_PLAY, // enter only

        STATE_NUM
    };

public:
    RPFshStartLayout();
    virtual ~RPFshStartLayout(); // at 0x8

    virtual void Create();    // at 0x34
    virtual void Reset();     // at 0x38
    virtual void Calculate(); // at 0x3C
    virtual void Exit();      // at 0x40 empty, out of line

    virtual void UserDraw(); // at 0x44

    s32 GetState() const {
        return mpStateMachine->GetState();
    }

    /**
     * @brief Past the intro (the players and the arrows run) [H]
     */
    bool IsStarted() const {
        return mpStateMachine->GetState() != STATE_NONE &&
               mpStateMachine->GetState() != STATE_INTRO;
    }

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshStartLayout* GetInstance() {
        return sInstance;
    }

private:
    void IntroEnter();
    void IntroCalc();
    void WaitReadyEnter();
    void WaitReadyCalc();
    void CountEnter();
    void CountCalc();
    void PlayEnter();

private:
    RPStateMachine<RPFshStartLayout>* mpStateMachine; // at 0x10
    s32 mTimeLimit;                     // at 0x14 [M] frames for RPFshTimer::Start
    s32 mTimerWindowStart;              // at 0x18 [L] start of the timer's window
    bool mReady;                        // at 0x1C [M] every player has cast
    bool mFarMsgShown;                  // at 0x1D [M] RPFshMsg01 (too far) shown instead of 00
    s32 mCount;                         // at 0x20 [M] countdown number
    RPSysLayout* mpLayout;              // at 0x24
    /**
     * @brief count_00, count_01, count_02, star_00
     */
    RPSysLytAnmObj* mpAnmObjs[4];       // at 0x28
    RPSysLytResAccessor* mpResAccessor; // at 0x38
    /**
     * @brief Countdown sounds by count (table 80359048, Fsh_getStartSoundTable)
     */
    RPSndRangeTable* mpCountSnd;        // at 0x3C

    // The state functions are a local table in the constructor (8031FE48, .rodata)

    static f32 sReadyDistFar;  // 8043B5C8 remote distance that opens the "too far" message
    static f32 sReadyDistNear; // 8043B5CC distance below which it closes again

    static RPFshStartLayout* sInstance; // 8043C780
};

//! @}

#endif
