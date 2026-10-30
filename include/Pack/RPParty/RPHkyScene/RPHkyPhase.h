#ifndef RP_PARTY_HKY_PHASE_H
#define RP_PARTY_HKY_PHASE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

//! @addtogroup rp_hky
//! @{

/**
 * @brief Game-flow phases owned by RPHkyGame (one per game state)
 * @details Each derives directly from RPSimpleModel and overrides only Calculate.
 * Sizes verified: Intro 0x50, Goal/TimeUp/Wait 0x4C, Result 0x54. RPHkyGame waits for mStep
 * to reach 2 (Result: 4) before changing state.
 */
class RPHkyIntroPhase : public RPSimpleModel { // state 0, vtable 80359430
public:
    virtual ~RPHkyIntroPhase(); // at 0x8
    virtual void Calculate();   // at 0x14

    // Called directly (not virtual) by RPHkyGame
    void Create();
    void Load();
    void Reset();
    void Draw();
    void Exit();

    // Wii Play: read by RPHkyGame::Calculate
    s32 GetStep() const {
        return mStep;
    }

private:
    s32 mStep;  // at 0x44 [M]
    s32 mTimer; // at 0x48 [M]
    u32 unk4C;  // at 0x4C
};

class RPHkyGoalPhase : public RPSimpleModel { // state 2, vtable 803594B0
public:
    virtual ~RPHkyGoalPhase(); // at 0x8
    virtual void Calculate();  // at 0x14

    // Called directly (not virtual) by RPHkyGame
    void Create();
    void Load();
    void Reset();
    void Draw();
    void Exit();

    // Wii Play: read by RPHkyGame::Calculate
    s32 GetStep() const {
        return mStep;
    }
    void Start();

private:
    s32 mStep;  // at 0x44 [M]
    s32 mTimer; // at 0x48 [M]
};

class RPHkyTimeUpPhase : public RPSimpleModel { // state 3, vtable 80359490
public:
    virtual ~RPHkyTimeUpPhase(); // at 0x8
    virtual void Calculate();    // at 0x14

    // Called directly (not virtual) by RPHkyGame
    void Create();
    void Load();
    void Reset();
    void Draw();
    void Exit();

    // Wii Play: read by RPHkyGame::Calculate
    s32 GetStep() const {
        return mStep;
    }
    void Start();

private:
    s32 mStep;  // at 0x44 [M]
    s32 mTimer; // at 0x48 [M]
};

class RPHkyWaitPhase : public RPSimpleModel { // state 4, vtable 80359450
public:
    static s32 sFrames; // 8043B698 (Hky_waitPhaseFrames) length of the pause, 80

    virtual ~RPHkyWaitPhase(); // at 0x8
    virtual void Calculate();  // at 0x14

    // Called directly (not virtual) by RPHkyGame
    void Create();
    void Load();
    void Reset();
    void Draw();
    void Exit();

    // Wii Play: read by RPHkyGame::Calculate
    s32 GetStep() const {
        return mStep;
    }
    void Start();

private:
    s32 mStep;  // at 0x44 [M]
    s32 mTimer; // at 0x48 [M]
};

class RPHkyResultPhase : public RPSimpleModel { // state 5, vtable 80359470
public:
    virtual ~RPHkyResultPhase(); // at 0x8
    virtual void Calculate();    // at 0x14

    // Called directly (not virtual) by RPHkyGame
    void Create();
    void Load();
    void Reset();
    void Draw();
    void Exit();

    // Wii Play: read by RPHkyGame::Calculate
    s32 GetStep() const {
        return mStep;
    }
    bool IsRetry() const {
        return mRetry;
    }
    void Start();

private:
    s32 mStep;  // at 0x44 [M]
    s32 mTimer; // at 0x48 [M]
    s32 mWinner; // at 0x4C -1 tie, else the player with more goals
    bool mRetry; // at 0x50 result menu choice 0 (play again)
};

//! @}

#endif
