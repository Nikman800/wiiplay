#ifndef RP_PARTY_FSH_PLAYER_MGR_H
#define RP_PARTY_FSH_PLAYER_MGR_H
#include <Pack/types_pack.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPFshPlayer;

/**
 * @brief Owner of the two players (singleton Fsh_playerMgr)
 * @details vtable 80358BE0. Size 0x1C, verified (Fsh_PlayerMgr_create allocates 0x1C and
 * always builds two RPFshPlayers; only mPlayerNum of them are calculated).
 */
class RPFshPlayerMgr {
public:
    enum {
        PLAYER_MAX = 2
    };

public:
    RPFshPlayerMgr();
    virtual ~RPFshPlayerMgr(); // at 0x8

    /**
     * @brief Calculates the players' rods (after the fish, from RPFshGame) [M]
     */
    virtual void CalculateRods(); // at 0xC

    void CreateAll();
    void InitPlayers();
    void Reset();
    void Calculate();
    void ExitAll();
    /**
     * @brief The catching player goes to LAND, the others to RETURN [H]
     */
    void SetCatchingPlayer(s32 player);

    // Wii Play: RPFshPlayer::FightCalc stores the player before SetCatchingPlayer
    void SetCatchingIdx(s32 player) {
        mCatchingPlayer = player;
    }
    s32 GetCatchingPlayer() const {
        return mCatchingPlayer;
    }
    s32 GetPlayerNum() const {
        return mPlayerNum;
    }
    RPFshPlayer* GetPlayer(s32 idx) const {
        return mpPlayers[idx];
    }

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshPlayerMgr* GetInstance() {
        return sInstance;
    }

private:
    s32 mPlayerNum;                      // at 0x4 RPFshGame::sPlayerNum
    s32 mCatchingPlayer;                 // at 0x8 [M] -1 after Reset
    bool mResultRequested;               // at 0xC [M] starts the result layout
    RPFshPlayer* mpPlayers[PLAYER_MAX];  // at 0x10
    bool mIsMsgOpen;                     // at 0x18 [M] RPFshMsg01 (pull warning) shown

    static f32 sMsgOpenDist;  // 8043B558 2.7
    static f32 sMsgCloseDist; // 8043B55C 2.5

    static RPFshPlayerMgr* sInstance; // 8043C740
};

//! @}

#endif
