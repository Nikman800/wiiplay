#ifndef RP_PARTY_PNP_GAME_H
#define RP_PARTY_PNP_GAME_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>

//! @addtogroup rp_pnp
//! @{

// Forward declarations
class RPPnpField;
class RPPnpTable;
class RPPnpNetShadow;
class RPPnpBall;
class RPPnpPlayer;
class RPPnpIntroPhase;
class RPPnpReadyPhase;
class RPPnpServePhase;
class RPPnpPointPhase;
class RPPnpResultPhase;
class RPPnpBgmCtrl;
class RPGrpShadowTexture;

/**
 * @brief Table Tennis game manager (singleton Pnp_gameInstance)
 * @details vtable 80359958. Size 0x54, verified (Pnp_createGame). Owns the court objects, the two
 * players and the phases; the audience (Pnp_audienceInstance), HUD (Pnp_hudInstance) and level
 * table (Pnp_levelTableInstance) are separate singletons created by the constructor.
 * Game states: 0 intro, 1 ready, 2 serve, 3 rally, 4 point, 5 result.
 * Single player is the rally game (mRallyCount from RPPnpBall's hit count); with two players it is
 * a points game (RPPnpPointPhase).
 */
class RPPnpGame {
public:
    enum State {
        STATE_INTRO,
        STATE_READY,
        STATE_SERVE,
        STATE_RALLY,
        STATE_POINT,
        STATE_RESULT,
    };

public:
    RPPnpGame();
    virtual ~RPPnpGame();     // at 0x8
    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Draw();      // at 0x1C
    virtual void Exit();      // at 0x20

    /**
     * @brief Ball integration, player input, floor/table/racket collisions (only while the ball
     * is in play), then every object's Calculate and the HUD
     */
    void StepPhysics(BOOL ballInPlay);
    /**
     * @brief In the point state, waits for the point phase and starts the next serve or the
     * result
     */
    void CheckPointEnd();
    /**
     * @brief 0 <-> 1, anything else -> 2 (no side)
     */
    s32 GetOpponentSide(s32 side) const;

    /**
     * @brief Allocates the singleton (Pnp_createGame)
     */
    static void CreateInstance();

    static RPPnpGame* GetInstance() {
        return sInstance;
    }
    BOOL IsMultiPlayer() const {
        return mIsMultiPlayer;
    }
    s32 GetState() const {
        return mState;
    }
    // Wii Play: RPPnpBall::TossServe starts the rally
    void SetState(s32 state) {
        mState = state;
    }
    RPSysMessage* GetMessage() const {
        return mpMessage;
    }
    RPPnpBgmCtrl* GetBgmCtrl() const {
        return mpBgmCtrl;
    }
    // Wii Play: spectators only walk in while set [L]
    u8 IsUnk18() const {
        return unk18;
    }
    // Wii Play: RPPnpPlayer::Calculate, when the first serve is taken
    void SetUnk18(u8 value) {
        unk18 = value;
    }
    RPGrpShadowTexture* GetShadowTexture() const {
        return mpShadowTexture;
    }
    bool IsCourtSwapped() const {
        return mCourtSwapped;
    }
    s32 GetRallyCount() const {
        return mRallyCount;
    }
    RPPnpTable* GetTable() const {
        return mpTable;
    }
    RPPnpNetShadow* GetNetShadow() const {
        return mpNetShadow;
    }
    RPPnpBall* GetBall() const {
        return mpBall;
    }
    RPPnpPlayer* GetPlayer(s32 side) const {
        return mpPlayers[side];
    }
    RPGrpCamera* GetCamera() const {
        return mpCamera;
    }
    RPGrpScreen* GetScreen() const {
        return mpScreen;
    }
    RPPnpPointPhase* GetPointPhase() const {
        return mpPointPhase;
    }

private:
    BOOL mIsMultiPlayer;             // at 0x4 player count != 1: points game
    s32 mState;                      // at 0x8
    RPSysMessage* mpMessage;         // at 0xC party_message.bmg
    RPPnpBgmCtrl* mpBgmCtrl;         // at 0x10
    s32 mRallyCount;                 // at 0x14 single player: rallies of the last point
    u8 unk18;                        // at 0x18 [L] cleared in Reset
    bool mCourtSwapped;              // at 0x19 two players: random start side
    RPPnpField* mpField;             // at 0x1C
    RPPnpTable* mpTable;             // at 0x20
    RPPnpNetShadow* mpNetShadow;     // at 0x24
    RPPnpBall* mpBall;               // at 0x28
    RPPnpPlayer* mpPlayers[2];       // at 0x2C
    RPGrpShadowTexture* mpShadowTexture; // at 0x34 0x1C0 x 0x80 "dummy_shadow" on the table
    RPPnpIntroPhase* mpIntroPhase;   // at 0x38
    RPPnpReadyPhase* mpReadyPhase;   // at 0x3C
    RPPnpServePhase* mpServePhase;   // at 0x40
    RPPnpPointPhase* mpPointPhase;   // at 0x44
    RPPnpResultPhase* mpResultPhase; // at 0x48
    RPGrpCamera* mpCamera;           // at 0x4C
    RPGrpScreen* mpScreen;           // at 0x50 EGG::Screen constructor + RPGrpScreen vtable

    static RPPnpGame* sInstance; // 8043C81C (Pnp_gameInstance)

public:
    /**
     * @brief Single-player rally limit: 100, or 999 in the endless mode (8043B728)
     */
    static s32 sRallyLimit;
    /**
     * @brief Rally limit mode: 0 from the saved medal, 1 always 100, 2 always 999 (8043C818)
     */
    static s32 sRallyLimitMode;
    static f32 sCameraOffsetNear; // 8043B72C (Pnp_cameraOffsetNear) -15, serve camera target z
    static f32 sCameraOffsetFar;  // 8043B730 (Pnp_cameraOffsetFar) 15
};

//! @}

#endif
