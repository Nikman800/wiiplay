#ifndef RP_PARTY_BOM_GAME_MANAGER_H
#define RP_PARTY_BOM_GAME_MANAGER_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

//! @addtogroup rp_bom
//! @{

// Forward declarations
class RPBomBgm;
class RPBomQuestionParam;
class RPBomScore;

/**
 * @brief Pose Mii game flow (singleton RPBomGameManager::sInstance)
 * @details vtable 8035EE14. Size 0x104, verified (Bom_GameManager_createInstance). Create
 * builds the score (RPBomScore::sInstance), the question parameters (RPBomQuestionParam::sInstance)
 * and the BGM control. Calculate runs mStateFuncs[mState] (copied from
 * Bom_gameManagerStateFuncs) and hides the off-screen pointer guide while mHideLeadCursor:
 * 0 intro, 1 wait ready, 2 show theme, 3 play, 4 miss, 5 wait bubbles, 6 level end, 7 level
 * up, 8 next level theme, 9 game over, 10 all clear, 11 result, 12 end (result menu choice).
 * A level (mLevel, 0-15) runs for its time (level data, seconds); waves of bubbles are spawned
 * every question's interval. Points move the question (difficulty) forward
 * (TryAdvanceQuestion); a miss moves it to the question's miss target and ends the game after
 * mMissMax misses.
 * @note Name is a project convention (ledger Bom_GameManager).
 */
class RPBomGameManager : public RPSceneObj {
public:
    static RPBomGameManager* sInstance; // 8043CA98

    typedef void (RPBomGameManager::*StateFunc)();

public:
    RPBomGameManager();
    virtual ~RPBomGameManager(); // at 0x8

    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10 loads the question parameters
    virtual void Calculate(); // at 0x18
    /**
     * @brief Destroys the score and question parameter singletons [H]
     */
    virtual void Exit(); // at 0x1C

    void Reset(s32 playerNum);
    void Start();
    void StartGame();
    void PlaySpecialJingle();
    void SetBubbleResult(s32 player);
    void SetForcedMiss();
    void SetPlayerMiss(s32 player);
    BOOL TryAdvanceQuestion();

    void StateIntro();
    void StateWaitReady();
    void StateShowTheme();
    void StatePlay();
    void StateMiss();
    void StateWaitBubbles();
    void StateLevelEnd();
    void StateLevelUp();
    void StateNextLevelTheme();
    void StateGameOver();
    void StateAllClear();
    void StateResult();
    void StateEnd();

    /**
     * @brief Not in the vtable: called by RPBomScene's UserDraw after RPBomScreenFx (empty)
     * and after the layouts (an empty RPTextWriter pass) [H]
     */
    void DrawAfterScreenFx();
    void DrawAfterLayouts();

    /**
     * @brief RPBomBgm update with the bubbles in the hit zone and the background level [H]
     */
    void UpdateBgm();

    static void CreateInstance();
    static void DestroyInstance();

public:
    s32 mPlayerNum;                // at 0x4 1 or 2 (RPBomScene::mPlayerNum)
    s32 mTimer;                    // at 0x8 state timer (theme 80 frames, end 180)
    s32 mPlayFrame;                // at 0xC
    s32 mLevelTimer;               // at 0x10 frames of spawning left (level seconds * 60)
    s32 mWaitTimer;                // at 0x14 miss / bubble wait (level data)
    s32 mMissTimer;                // at 0x18 frames between front-bubble misses (25, 5, 60)
    bool mHideLeadCursor;          // at 0x1C [M] passed to RPLeadCursor (off-screen guide)
    bool mSpawning;                // at 0x1D waves are spawned until mLevelTimer ends
    s32 mWaveNum;                  // at 0x20 waves left in the current burst
    s32 mWaveTimer;                // at 0x24 frames since the last wave
    s32 mMissCount;                // at 0x28
    s32 mMissMax;                  // at 0x2C parameter file header 0x2C
    s32 mBubbleResult;             // at 0x30 player who hit a special bubble, -1
    s32 mMissPlayer;               // at 0x34 player who missed (two players), -1
    bool mMissPending;             // at 0x38
    bool mBubbleResultPending;     // at 0x39
    bool unk3A;                    // at 0x3A [L] cleared only
    bool mForcedMiss;              // at 0x3B score capped at 9999: the miss ends the game
    bool mMissAllRequest;          // at 0x3C
    bool mNoMiss[2];               // at 0x3D per player, cleared on a miss (win animation)
    s32 mQuestion;                 // at 0x40 current question (difficulty) index
    s32 mQuestionMax;              // at 0x44 RPBomQuestionParam::mQuestionNum
    s32 mLevel;                    // at 0x48 0-15
    s32 mLevelMax;                 // at 0x4C 15
    bool mRetry;                   // at 0x50 [M] result menu choice 0 (scene 0x18)
    bool mQuit;                    // at 0x51 [M] result menu choice 1 (scene 0x1B)
    s32 mMode;                     // at 0x54 0 single player, 1 two players
    s32 mState;                    // at 0x58
    RPBomScore* mpScore;           // at 0x5C
    RPBomQuestionParam* mpParam;   // at 0x60
    RPBomBgm* mpBgm;               // at 0x64
    StateFunc mStateFuncs[13];     // at 0x68

    /**
     * @brief Number of levels; read from memory (the definition follows most of the code) [H]
     */
    static s32 GetLevelNum() {
        return sLevelNum;
    }

    static const s32 sLevelNum; // 80441700 (16 levels)
};

//! @}

#endif
