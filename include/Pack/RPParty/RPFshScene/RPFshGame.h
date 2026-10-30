#ifndef RP_PARTY_FSH_GAME_H
#define RP_PARTY_FSH_GAME_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshObject.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

//! @addtogroup rp_fsh
//! @{

/**
 * @brief Fishing game manager (singleton Fsh_gameInstance, ledger Fsh_Scene)
 * @details vtable 803590E0 (destructor only). Size 0x8, verified (Fsh_Scene_new). The
 * constructor stores the player count in sPlayerNum, builds the one-state machine and creates
 * every Fishing object singleton (start layout, timer, camera, stage, players, fish, result,
 * score, yank prompt, point popup, aim arrow, bonus fish, both messages) and the party common
 * objects. The only state (0, play) runs the per-frame calculation of every object and changes
 * the scene once the result layout reaches its last state.
 * @note Name is a project convention (RPFshScene is the RPSysScene).
 */
class RPFshGame {
public:
    enum EState {
        STATE_PLAY,

        STATE_NUM
    };

public:
    RPFshGame();
    virtual ~RPFshGame(); // at 0x8

    /**
     * @brief Create() of every object, then the party common resources (from
     * RPFshScene::LoadResource) [H]
     */
    void CreateObjects();
    /**
     * @brief Resets the state machine to STATE_PLAY and Reset() of every object [H]
     */
    void ResetObjects();
    /**
     * @brief Runs the state machine and copies the fog colour to the renderer [M]
     */
    void Calculate();
    /**
     * @brief Exit() of every object, then deletes every singleton [H]
     */
    void DestroyObjects();

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshGame* GetInstance() {
        return sInstance;
    }

private:
    void PlayEnter();
    void PlayCalc();

private:
    RPStateMachine<RPFshGame>* mpStateMachine; // at 0x4

    /**
     * @brief Player count, copied from the game config at construction (8043C7BC)
     */
    static s32 sPlayerNum;

public:
    static s32 GetPlayerNum() {
        return sPlayerNum;
    }

private:
    // The state functions are a local table in the constructor (80320480, .rodata)

    static RPFshGame* sInstance; // 8043C7B8
};

//! @}

#endif
