#ifndef RP_PARTY_BIL_GAME_MGR_H
#define RP_PARTY_BIL_GAME_MGR_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPBilScene/RPBilObject.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <nw4r/snd/snd_SoundHandle.h>

//! @addtogroup rp_bil
//! @{

/**
 * @brief Billiards game flow (singleton RPBilGameMgr::sInstance)
 * @details vtable 8035DF88 (destructor only). Size 0x8, verified (Bil_GameMgr_create). The
 * constructor copies the player count to sPlayerNum, builds the 6-state machine and creates
 * every Billiards singleton (cameras, table, players, balls, number tag, break lottery,
 * pocket-camera window, next-ball marker, HUD, message window, result, score popup, hint,
 * stroke guide, music) and the party common objects (game 5). Load / Reset / Exit go through
 * every object; Calculate runs the state machine and the music and copies the fade colour.
 */
class RPBilGameMgr {
public:
    static RPBilGameMgr* sInstance; // 8043CA00

    enum EState {
        STATE_INTRO,     //!< break lottery chooses who breaks
        STATE_START,
        STATE_PLAY,
        STATE_SCORE,     //!< balls were pocketed
        STATE_FOUL,
        STATE_GAME_OVER,

        STATE_NUM
    };

public:
    RPBilGameMgr();
    virtual ~RPBilGameMgr(); // at 0x8

    void Load();
    void Reset();
    void Calculate();
    void Exit();
    /**
     * @brief Forwards to RPBilPlayerMgr::SetCursorsVisible (scene pause callback) [H]
     */
    void SetCursorsVisible(bool visible);

    static void CreateInstance();
    static void DestroyInstance();

private:
    void IntroEnter();
    void IntroCalc();
    void StartEnter();
    void StartCalc();
    void PlayEnter();
    void PlayCalc();
    void ScoreEnter();
    void ScoreCalc();
    void FoulEnter();
    void FoulCalc();
    void GameOverEnter();
    void GameOverCalc();

private:
    RPStateMachine<RPBilGameMgr>* mpStateMachine; // at 0x4

public:
    /**
     * @brief Player count, copied from the game config at construction (8043CA04)
     */
    static s32 sPlayerNum;

    static s32 sPlayTimeout[2];
};

class RPBilBallNumberTag;
class RPBilBreakLottery;
class RPBilHint;
class RPBilStrokeGuide;
class RPBilMsgWindow;
class RPBilResult;
class RPBilScorePopup;
class RPBilHint;
class RPBilStrokeGuide;

//! Ledger Bil_GameSM (size 0x28)
typedef RPStateMachine<RPBilGameMgr> RPBilGameSeq;

/**
 * @brief Billiards BGM state: requested and applied music state, sequence variables
 * @details No vtable; allocated with 0x14 bytes by Bil_Music_create (constructed inline) and
 * reset by Start, which starts BGM 0x4E (tempo 0.83 on some regions) and mutes tracks
 * 0x86 / 0x358. ApplyState sets the track mutes for the state. Name is a project convention.
 */
class RPBilMusicState {
public:
    RPBilMusicState() {
        unk10 = false;
        unk11 = false;
        mPrevSeqVar = -1;
        mSeqVar = -1;
        mState = 6;
        mNextState = 6;
    }

    void ApplyState();
    void SetState(s32 state);
    void Calculate();
    void Start();

private:
    s32 mState;       // at 0x0 [M]
    s32 mNextState;   // at 0x4 [M] set by SetState
    nw4r::snd::SoundHandle mSubHandle; // at 0x8 [M] state jingle started by Calculate (RPBilMusicState::sTrackTable)
    s16 mPrevSeqVar;  // at 0xC [L]
    s16 mSeqVar;      // at 0xE [L]
    bool unk10;       // at 0x10 [L]
    bool unk11;       // at 0x11 [L]

public:
    static const u32 sTrackTable[6][4];
};

/**
 * @brief Holder of the Billiards music state (singleton RPBilMusic::sInstance)
 * @details vtable 8035E528 (destructor only). Size 0x8 [H] (Bil_Music_create allocates 0x8).
 */
class RPBilMusic {
public:
    static RPBilMusic* sInstance; // 8043CA48

    RPBilMusic();
    virtual ~RPBilMusic(); // at 0x8

    RPBilMusicState* GetState() const {
        return mpState;
    }

    static void CreateInstance();
    static void DestroyInstance();

private:
    RPBilMusicState* mpState; // at 0x4
};

//! @}

#endif
