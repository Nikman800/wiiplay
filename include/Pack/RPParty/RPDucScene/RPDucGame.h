#ifndef RP_PARTY_DUC_GAME_H
#define RP_PARTY_DUC_GAME_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPDucScene/RPDucSound.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

//! @addtogroup rp_duc
//! @{

// Forward declarations
class RPDucAmmoMgr;
class RPDucScoreMgr;
class RPDucSpawner;

/**
 * @brief Per-player scores, hits and streak bonuses (singleton Duc_scoreMgrInstance)
 * @details vtable 8035AD88. Size 0x88, verified. In the cooperative mode (RPDucGame::mMode 2)
 * everything is added to player 0. Scores are clamped to 0-999; a streak of five or more
 * awards streak - 4 bonus points.
 * @note Name is a project convention (ledger Duc_ScoreMgr).
 */
class RPDucScoreMgr : public RPSceneObj {
    friend class RPDucBalloon;

public:
    virtual ~RPDucScoreMgr(); // at 0x8

    static void CreateInstance();
    static void DestroyInstance();

    static RPDucScoreMgr* sInstance;

    void Setup(s16 playerNum);
    void ClearScores();
    void ClearHits();
    void ClearStreaks();
    void ClearBonuses();
    void ClearLastColors();
    void ClearCircleHits();
    void AddScore(s32 player, s32 points);
    void AddHits(s32 player, s32 num);
    void AddStreak(s32 player, s32 num);
    void AddCircleHits(s32 player, s32 num);
    bool AwardStreakBonus(s32 player);
    void ResetStreak(s32 player);
    bool HasStreakBonus(s32 player) const;
    void SetLastColor(s32 player, s32 color);
    /**
     * @brief 0 or 1 for the player ahead, -1 on a tie [H]
     */
    s32 GetLeader() const;

    s32 GetScore(s32 player) const { return mScores[player]; }
    s32 GetLastColor(s32 player) const { return mLastColors[player]; }
    s32 GetStreak(s32 player) const { return mStreaks[player]; }
    s32 GetCircleHits(s32 player) const { return mCircleHits[player]; }

private:
    s32 mScores[4];      // at 0x4
    s32 mHits[4];        // at 0x14
    s32 mStreaks[4];     // at 0x24
    s32 mMaxStreaks[4];  // at 0x34
    s32 mBonus[4];       // at 0x44
    s32 mBonusTimer[4];  // at 0x54 180 frames
    s32 mLastColors[4];  // at 0x64 -1 = none [M]
    s32 mCircleHits[4];  // at 0x74 [M] round targets hit, cleared by the spawner
    s16 mPlayerNum;      // at 0x84
};

/**
 * @brief Per-player ammunition and shot counters (singleton Duc_ammoMgrInstance)
 * @details vtable 8035ADB0. Size 0x24, verified. TryFire returns 0 (not in play),
 * 1 (empty) or 2 (fired).
 * @note Name is a project convention (ledger Duc_AmmoMgr).
 */
class RPDucAmmoMgr : public RPSceneObj {
    friend class RPDucGame;
public:
    virtual ~RPDucAmmoMgr(); // at 0x8

    static void CreateInstance();
    static void DestroyInstance();

    static RPDucAmmoMgr* sInstance;

    void Setup(s16 playerNum);
    s32 TryFire(s32 player);
    void Refill();
    void ClearShotCounts();

private:
    s16 mAmmo[4];        // at 0x4
    s16 mShots[4];       // at 0xC this round
    s16 mTotalShots[4];  // at 0x14
    s16 mPlayerNum;      // at 0x1C
    s16 mMaxAmmo;        // at 0x1E 99, then the round's value
    bool mLimitedAmmo;   // at 0x20
};

/**
 * @brief Shooting Range game flow (singleton Duc_gameInstance)
 * @details vtable 8035AACC. Size 0xA8, verified. States: 0 none, 1 intro, 2 round start,
 * 3 round play, 4 round end, 5 wait for score, 6 show results, 7 end. Modes (mMode): 0 one
 * player, 1 two-player versus, 2 two-player cooperative [M].
 * Round parameters (SetRoundParams, rounds 1-7): spawn pattern, ammo, count, unused, waves.
 * @note Name is a project convention (ledger Duc_Game).
 */
class RPDucGame : public RPSceneObj {
    friend class RPDucSpawner; // PatternClay's volatile read of mMode (NONMATCHING-DEBT)

public:
    typedef void (RPDucGame::*StateFunc)();

public:
    RPDucGame() {
        // Keep the original's two sets of member-function pointer literals.
        // CodeWarrior retains these literals even though the locals are unused.
        StateFunc none = &RPDucGame::StateNone;
        StateFunc intro = &RPDucGame::StateIntro;
        StateFunc start = &RPDucGame::StateRoundStart;
        StateFunc play = &RPDucGame::StateRoundPlay;
        StateFunc end = &RPDucGame::StateRoundEnd;
        StateFunc wait = &RPDucGame::StateWaitScore;
        StateFunc results = &RPDucGame::StateShowResults;
        StateFunc finish = &RPDucGame::StateEnd;
        mStateFuncs[0] = &RPDucGame::StateNone;
        mStateFuncs[1] = &RPDucGame::StateIntro;
        mStateFuncs[2] = &RPDucGame::StateRoundStart;
        mStateFuncs[3] = &RPDucGame::StateRoundPlay;
        mStateFuncs[4] = &RPDucGame::StateRoundEnd;
        mStateFuncs[5] = &RPDucGame::StateWaitScore;
        mStateFuncs[6] = &RPDucGame::StateShowResults;
        mStateFuncs[7] = &RPDucGame::StateEnd;
    }

    virtual ~RPDucGame(); // at 0x8

    /**
     * @brief Creates the spawner, score and ammo managers and mpSndActor [H]
     */
    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10 empty
    virtual void Calculate(); // at 0x18
    /**
     * @brief Destroys the spawner, score and ammo managers [H]
     */
    virtual void Exit();      // at 0x1C
    virtual void Draw();      // at 0x20

    static void CreateInstance();
    static void DestroyInstance();

    static RPDucGame* sInstance;

    void Setup(s32 playMode);
    /**
     * @brief Called from RPDucScene::Reset (not the RPSceneObj::Reset slot) [H]
     */
    void ResetGame();
    void Start();
    void SetRoundParams(s32 round);
    s32 GetRoundNum() const;
    s32 GetState() const { return mState; }
    void PlayStreakSound();
    void PlayStreakBonusSound();
    void PlayRemoteAlertSound(s32 type);
    s32 GetMode() const { return mMode; }
    bool IsP2Joined() const { return mP2Joined; }
    bool IsQuit() const { return mQuit; }
    bool IsRetry() const { return mRetry; }
    s32 GetWaves() const { return mWaves; }
    s32 GetSpawnPattern() const { return mSpawnPattern; }
    s32 GetCurrentRound() const;
    void SpawnAvatars();

    void StateNone();
    void StateIntro();
    void StateRoundStart();
    void StateRoundPlay();
    void StateRoundEnd();
    void StateWaitScore();
    void StateShowResults();
    void StateEnd();

private:
    bool AwardStreakBonuses() {
        bool bonus = false;
        s32 player = 0;
        do {
            if (mpScoreMgr->AwardStreakBonus(player)) bonus = true;
            mpScoreMgr->ResetStreak(player);
        } while (++player < 2);
        return bonus;
    }

    s32 mPlayerNum;            // at 0x4 [M] 1 or 2
    s32 mTimer;                // at 0x8
    bool mInPlay;              // at 0xC shooting allowed; disables the lead cursor
    s32 mRound;                // at 0x10 1-based
    s32 mRoundNum;             // at 0x14
    s32 mMode;                 // at 0x18
    s32 mState;                // at 0x1C
    bool mP2Joined;            // at 0x20 [M] second remote pressed in during the cooperative mode
    bool mQuit;                // at 0x21 result menu choice 0
    bool mRetry;               // at 0x22 result menu choice 1
    RPDucSpawner* mpSpawner;   // at 0x24
    s32 mWaves;                // at 0x28 waves left in the round
    s32 mSpawnPattern;         // at 0x2C
    s32 mRoundAmmo;            // at 0x30 copied to RPDucAmmoMgr::mMaxAmmo
    s32 mSpawnCount;           // at 0x34
    s32 unk38;                 // at 0x38 [L] never read
    RPDucScoreMgr* mpScoreMgr; // at 0x3C
    RPDucAmmoMgr* mpAmmoMgr;   // at 0x40
    RPDucSndActor* mpSndActor; // at 0x44
    StateFunc mStateFuncs[8];  // at 0x48
};

//! @}

#endif
