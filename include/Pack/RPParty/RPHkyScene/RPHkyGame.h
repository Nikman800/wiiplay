#ifndef RP_PARTY_HKY_GAME_H
#define RP_PARTY_HKY_GAME_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>

//! @addtogroup rp_hky
//! @{

// Forward declarations
class RPHkyPuck;
class RPHkyField;
class RPHkyPaddle;
class RPHkyPaddleRound;
class RPHkyIntroPhase;
class RPHkyGoalPhase;
class RPHkyTimeUpPhase;
class RPHkyWaitPhase;
class RPHkyResultPhase;
class RPHkyBgmCtrl;

/**
 * @brief Laser Hockey game manager (singleton Hky_gameInstance)
 * @details vtable 80359300. Size 0x50, verified. Owns every world object and phase and runs
 * the game state machine: 0 intro, 1 play, 2 goal, 3 time-up, 4 wait, 5 result.
 * Create/Load/Reset/Calculate/Exit forward to the world objects through their RPHkyObject
 * slots and call the HUD (Hky_hudInstance) and the phases directly. Exit deletes the singleton.
 */
class RPHkyGame {
public:
    RPHkyGame();
    virtual ~RPHkyGame();      // at 0x8
    virtual void Create();     // at 0xC
    virtual void Load();       // at 0x10
    virtual void Reset();      // at 0x14
    virtual void Calculate();  // at 0x18
    virtual void Draw();       // at 0x1C
    virtual void Exit();       // at 0x20

    /**
     * @brief Paddle control, then mSubSteps rounds of velocity / collision / movement, then
     * goal check and the objects' Calculate; the puck only moves while playing
     */
    void StepPhysics(BOOL playing);

    /**
     * @brief Allocates the singleton (Hky_createGame)
     */
    static void CreateInstance();

    // Wii Play: used by RPHkyScene
    RPHkyBgmCtrl* GetBgmCtrl() const {
        return mpBgmCtrl;
    }
    // Wii Play: used by RPHkyPuck::CheckGoal
    void SetState(s32 state) {
        mState = state;
    }
    RPHkyGoalPhase* GetGoalPhase() const {
        return mpGoalPhase;
    }

    static RPHkyGame* GetInstance() {
        return sInstance;
    }
    RPSysMessage* GetMessage() const {
        return mpMessage;
    }
    BOOL IsMultiPlayer() const {
        return mIsMultiPlayer;
    }
    s32 GetState() const {
        return mState;
    }
    RPHkyPuck* GetPuck() const {
        return mpPuck;
    }
    RPHkyPaddle* GetPaddle(s32 player) const {
        return mpPaddles[player];
    }
    // Wii Play: used by RPHkyPaddle
    s32 GetSubSteps() const {
        return mSubSteps;
    }
    s32 GetTimer() const {
        return mTimer;
    }
    RPHkyField* GetField() const {
        return mpField;
    }
    RPHkyPaddleRound* GetPaddleRound(s32 player) const {
        return mpPaddleRounds[player];
    }
    RPGrpCamera* GetCamera() const {
        return mpCamera;
    }

private:
    RPSysMessage* mpMessage;            // at 0x4 party_message.bmg, used by the HUD texts
    RPHkyBgmCtrl* mpBgmCtrl;            // at 0x8
    s32 mSubSteps;                      // at 0xC physics sub-steps per frame (16)
    s32 mTimer;                         // at 0x10 frames left (7200 = 2 min); -1 when time is up
    BOOL mIsMultiPlayer;                // at 0x14 player count != 1: no time limit, first to 7 [M]
    s32 mState;                         // at 0x18
    RPHkyPuck* mpPuck;                  // at 0x1C
    RPHkyField* mpField;                // at 0x20
    RPHkyPaddle* mpPaddles[2];          // at 0x24
    RPHkyPaddleRound* mpPaddleRounds[2]; // at 0x2C
    RPHkyIntroPhase* mpIntroPhase;      // at 0x34
    RPHkyGoalPhase* mpGoalPhase;        // at 0x38
    RPHkyTimeUpPhase* mpTimeUpPhase;    // at 0x3C
    RPHkyWaitPhase* mpWaitPhase;        // at 0x40
    RPHkyResultPhase* mpResultPhase;    // at 0x44
    RPGrpCamera* mpCamera;              // at 0x48 (allocated 0x178 = sizeof(RPGrpCamera))
    RPGrpScreen* mpScreen;              // at 0x4C EGG::Screen constructor + RPGrpScreen vtable

    static RPHkyGame* sInstance; // 8043C7E0 (Hky_gameInstance)
};

//! @}

#endif
