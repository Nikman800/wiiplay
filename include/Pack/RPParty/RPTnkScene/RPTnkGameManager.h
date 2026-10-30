#ifndef RP_PARTY_TNK_GAME_MANAGER_H
#define RP_PARTY_TNK_GAME_MANAGER_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPTnkBgmMix;
class RPTnkEmptyTask;
class RPTnkExGuide;
class RPTnkGameStats;

/**
 * @brief Tanks! game flow: stages, lives, results (singleton Tnk_gameManagerInstance)
 * @details vtable 8035B98C. Size 0xE4, verified (Tnk_GameManager_createInstance). States
 * (mState): 0 idle, 1 wait (life count shown), 2 stage start, 3 countdown, 4 play, 5 miss
 * wait, 6 stage clear wait, 7 next stage, 8 complete wait, 9 failed wait, 10 result wait,
 * 11 result menu, 12 end menu. The scene reads mFreeze (no gameplay step), mRetry and mQuit.
 */
class RPTnkGameManager : public RPSceneObj {
public:
    typedef void (RPTnkGameManager::*StateFunc)();

    enum EMode { MODE_1P, MODE_2P };

public:
    static void CreateInstance();
    static void DestroyInstance();

    RPTnkGameManager();
    virtual ~RPTnkGameManager(); // at 0x8

    /**
     * @brief Creates the game stats, the guide, the empty task and the BGM mixer [H]
     */
    virtual void Create();    // at 0xC
    /**
     * @brief Loads param/TnkGameParam.bin into mpParam [H]
     */
    virtual void Load();      // at 0x10
    virtual void Calculate(); // at 0x18
    virtual void Exit();      // at 0x1C

    void ResetForMode(s32 playerNum);
    void Start();
    void SetupFirstStage();
    void SetStageReady(bool ready);
    s32 GetModeIdx() const { return mModeIdx; }
    // The enum-typed read keeps the mode out of r0 (RPTnkFade::Start, RPTnkHud::PlayIn)
    EMode GetMode() const { return static_cast<EMode>(mModeIdx); }
    s32 GetStage() const { return mStage; }
    s32 GetState() const { return mState; }
    s32 GetLastStage() const { return mLastStage; }
    void* GetParam() const { return mpParam; }
    bool IsFreeze() const { return mFreeze; }
    bool IsRetry() const { return mRetry; }
    bool IsQuit() const { return mQuit; }

    static RPTnkGameManager* sInstance; // Tnk_gameManagerInstance

    void StartBgm();
    void StartStageBgm();
    BOOL CheckStageClear();
    void PickStageVariant();
    void PickStageVariant(s32 stage);

    static const s32 STAGE_NUM;
    static s32 GetStageNum() { return STAGE_NUM; }

    void StateIdle();
    void StateWaitLifeNo();
    void StateStageStart();
    void StateCountdown();
    void StatePlay();
    void StateMissWait();
    void StateStageClearWait();
    void StateNextStage();
    void StateCompleteWait();
    void StateFailedWait();
    void StateResultWait();
    void StateResultMenu();
    void StateEndMenu();

private:
    s32 mPlayerNum;             // at 0x4 1 or 2
    s32 mStage;                 // at 0x8 0-based
    s32 mLastStage;             // at 0xC the fade shows "final mission" on this stage
    s32 unk10;                  // at 0x10 [L] stage variant
    s32 unk14;                  // at 0x14 [L] reset to -1
    s32 mTimer;                 // at 0x18
    s32 mRetryCount;            // at 0x1C
    bool unk20;                 // at 0x20 [L]
    bool mFreeze;               // at 0x21 the scene skips the physics step
    bool mStageReady;           // at 0x22
    bool mRetry;                // at 0x23 scene changes to 0x17
    bool mQuit;                 // at 0x24 scene changes to 0x1B
    s32 mModeIdx;               // at 0x28 0 one player, 1 two players
    s32 mState;                 // at 0x2C
    s32 unk30;                  // at 0x30 [L]
    void* mpParam;              // at 0x34 TnkGameParam.bin
    RPTnkGameStats* mpStats;    // at 0x38
    RPTnkExGuide* mpExGuide;    // at 0x3C
    RPTnkEmptyTask* mpEmptyTask; // at 0x40
    RPTnkBgmMix* mpBgmMix;      // at 0x44
    StateFunc mStateFuncs[13];  // at 0x48
};

/**
 * @brief Scores, lives and destroyed enemy types (singleton Tnk_gameStatsInstance)
 * @details vtable 8035C218 (no overrides but the destructor). Size 0x3A8, verified
 * (Tnk_GameStats_createInstance).
 */
class RPTnkGameStats : public RPSceneObj {
public:
    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkGameStats(); // at 0x8

    void Setup(s16 playerNum);
    void ClearCounts();
    void AddKill(s32 player, s32 type, s32 num);
    void AddKilledType(s32 type);
    /**
     * @brief Adds to a player's lives (clamped to 0-99); true when none are left [H]
     */
    BOOL AddLives(s32 player, s32 num);
    void SetAllLives(s32 lives);
    /**
     * @brief 0 or 1, -1 on a draw [H]
     */
    s32 GetWinner() const;

    s32 GetKills(s32 player) const { return mKills[player]; }
    s32 GetLives(s32 player) const { return mLives[player]; }
    s32 GetKillsByType(s32 player, s32 type) const { return mKillsByType[player][type]; }
    s32 GetTypeOrder(s32 idx) const { return mTypeOrder[idx]; }
    s32 GetTypeNum() const { return mTypeNum; }

    static RPTnkGameStats* sInstance; // Tnk_gameStatsInstance

private:
    // Clear helpers shared by Setup and ClearCounts [H]
    void ClearPlayerCounts() {
        s32 i = 0;
        do {
            unk8[i] = 0;
        } while (++i < mPlayerNum);
    }
    void ClearKills() {
        s32 i = 0;
        do {
            mKills[i] = 0;
            s32 j = 0;
            do {
                mKillsByType[i][j] = 0;
            } while (++j < 11);
        } while (++i < mPlayerNum);
    }
    void ClearTypes() {
        s32 i = 0;
        do {
            mTypeSeen[i] = false;
            mTypeOrder[i] = -1;
        } while (++i < 11);
        mTypeNum = 0;
    }
    void ClearRecords() {
        s32 i = 0;
        do {
            unk128[i] = 0;
            unk228[i] = 0;
            unk1A8[i] = 0;
            unk2A8[i][0] = 0.0f;
            unk2A8[i][1] = 0.0f;
        } while (++i < 32);
        unk124 = 0;
    }
    void SetLives(s32 value) {
        s32 i = 0;
        do {
            mLives[i] = value;
        } while (++i < mPlayerNum);
    }

    s16 mPlayerNum;              // at 0x4
    s32 unk8[4];                 // at 0x8 [L]
    s32 mKills[4];               // at 0x18
    s32 mKillsByType[4][11];     // at 0x28
    bool mTypeSeen[11];          // at 0xD8
    s32 mTypeOrder[11];          // at 0xE4 types in order of appearance, -1 terminated
    s32 mTypeNum;                // at 0x110
    s32 mLives[4];               // at 0x114
    s32 unk124;                  // at 0x124 [L]
    s32 unk128[32];              // at 0x128 [L] per stage, only cleared
    s32 unk1A8[32];              // at 0x1A8 [L]
    s32 unk228[32];              // at 0x228 [L]
    f32 unk2A8[32][2];           // at 0x2A8 [L] cleared to 0.0f
};

//! @}

#endif
