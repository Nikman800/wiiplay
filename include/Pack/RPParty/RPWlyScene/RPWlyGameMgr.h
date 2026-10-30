#ifndef RP_PARTY_WLY_GAME_MGR_H
#define RP_PARTY_WLY_GAME_MGR_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPWlyScene/RPWlySound.h>

//! @addtogroup rp_wly
//! @{

// Forward declarations
class RPGrpCamera;
class RPGrpCaptureTexture;
class RPGrpScreen;
class RPSysMessage;
class RPWlyClearPhase;
class RPWlyCursor;
class RPWlyField;
class RPWlyField06;
class RPWlyIntroPhase;
class RPWlyResultPhase;
class RPWlyTargetPhase;
class RPWlyTimerSe;
class RPWlyWipe;

/**
 * @brief Find Mii game manager (singleton RPWlyGameMgr::sInstance)
 * @details vtable 8035C870. Size 0x70, verified (Wly_GameMgr_createGlobal). The constructor
 * creates the model scenes, RPCommon, the BGM sound object, the wipe, field and cursors, the
 * level table (RPWlyLevelTable::sInstance), the avatar manager (RPWlyAvatarMgr::sInstance), the layout
 * set (RPWlyLayoutSet::sInstance), the four phases, the cameras and screens and the views (3D 0-2
 * and 4, 2D 3, 5, 6). Game states (mState): 0 intro, 1 wipe in, 2 question / search, 3 clear,
 * 4 result [M].
 * Everything but mpBgm/mpTimerSe/mpMessage/the cameras is driven through direct calls.
 */
class RPWlyGameMgr {
public:
    RPWlyGameMgr();
    virtual ~RPWlyGameMgr(); // at 0x8

    /**
     * @brief Seeds the random generator, sets the player count and timer (900 frames) and
     * initialises every object (called by RPWlyScene::Configure) [H]
     */
    virtual void Init(); // at 0xC
    /**
     * @brief Loads party_message.bmg, the scene light (RPScene or WNTScene for spotlight
     * stages) and light map, then creates every object (called by RPWlyScene::LoadResource) [H]
     */
    virtual void Create();    // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Draw();      // at 0x1C
    virtual void Destroy();   // at 0x20

    void CalcClearPhase();
    /**
     * @brief A cursor stayed on avatar idx: scores it or plays the wrong reaction [M]
     */
    void OnAvatarPicked(s32 player, s32 idx);
    bool OnPickResolved(s32 idx, BOOL found);
    /**
     * @brief Single-player stage of question type A (no avatar voices) [L]
     */
    bool Is1pTypeAStage() const;

    static void CreateInstance(); // Wly_GameMgr_createGlobal

public:
    RPWlySoundObj* mpBgm;               // at 0x4 BGM sound object (0x68 bytes), ducked on picks
    RPSysMessage* mpMessage;            // at 0x8 party_message.bmg, used by every layout
    RPWlyTimerSe* mpTimerSe;            // at 0xC
    s32 mTimer;                         // at 0x10 frames left (900; RPWlyGameConfig::mTimer)
    s32 mStageNo;                       // at 0x14 1-based stage number (99: endless) [M]
    s32 mCurPlayer;                     // at 0x18 [L] player whose pick is being resolved
    s32 mMode;                          // at 0x1C 0 single player, else two players [M]
    s32 mState;                         // at 0x20
    s32 mScores[2];                     // at 0x24 [L] copied from RPWlyGameConfig 0x4/0x8
    s32 mPenaltyTimers[2];              // at 0x2C frames left before the player may pick again
    RPWlyWipe* mpWipe;                  // at 0x34
    RPWlyField* mpField;                // at 0x38
    s32 mPlayerNum;                     // at 0x3C 1 or 2
    RPWlyCursor* mpCursors[2];          // at 0x40
    RPWlyField06* mpField06;            // at 0x48 spotlight stages only
    RPGrpCaptureTexture* mpCapTexture;  // at 0x4C field type 1 only (0x10 bytes)
    RPWlyIntroPhase* mpIntroPhase;      // at 0x50
    RPWlyTargetPhase* mpTargetPhase;    // at 0x54
    RPWlyClearPhase* mpClearPhase;      // at 0x58
    RPWlyResultPhase* mpResultPhase;    // at 0x5C
    RPGrpCamera* mpCamera;              // at 0x60 views 0 and 1
    RPGrpCamera* mpCamera2;             // at 0x64 view 2
    RPGrpCamera* mpIconCamera;          // at 0x68 view 4 (small screen at the center) [M]
    RPGrpScreen* mpScreen;              // at 0x6C 0x68-byte screen (vtable 8034F32C)

public:
    static RPWlyGameMgr* sInstance; // 8043C970
    static BOOL sIsMultiPlayer;
    static u16 sTimeUpLock; // nonzero holds the timer at 0
    static f32 sDpdDistParam0; // setDistParam arguments
    static f32 sDpdDistParam1;
};

//! @}

#endif
