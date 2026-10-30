#ifndef RP_PARTY_PNP_PHASE_H
#define RP_PARTY_PNP_PHASE_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

#include <nw4r/snd.h>

//! @addtogroup rp_pnp
//! @{

/**
 * @brief Game-flow phases owned by RPPnpGame (one per game state)
 * @details Each derives directly from RPSimpleModel and overrides only Calculate (which does
 * nothing unless RPPnpGame is in the phase's state); Create/Load/Reset/Draw/Exit are called
 * directly. Sizes verified: Intro/Ready 0x4C, Serve 0x54, Point 0x5C, Result 0x60.
 */
class RPPnpIntroPhase : public RPSimpleModel { // state 0, vtable 803599D8
public:
    virtual ~RPPnpIntroPhase(); // at 0x8
    virtual void Calculate();   // at 0x14

    void Create();
    void Load();
    void Reset();
    void Draw();
    void Exit();

    /**
     * @brief Intro camera fly-in
     */
    void CalcCamera();

    s32 GetStep() const {
        return mStep;
    }

private:
    s32 mStep;  // at 0x44 2 = done
    s32 mTimer; // at 0x48
};

class RPPnpReadyPhase : public RPSimpleModel { // state 1, vtable 80359938
public:
    virtual ~RPPnpReadyPhase(); // at 0x8
    virtual void Calculate();   // at 0x14

    void Create();
    void Load();
    void Reset();
    void Draw();
    void Exit();

    // Wii Play: the rally BGM handle, defined in the ready phase's file (Pnp_bgmHandle)
    /**
     * @brief Restarts the rally BGM: sound 13, or 14 with two players (RPPnpGame::Reset)
     */
    static void StartBgm(BOOL multiPlayer);
    /**
     * @brief Fades the rally BGM out over 60 frames; false when none plays
     */
    static bool FadeOutBgm();

    s32 GetStep() const {
        return mStep;
    }

private:
    static nw4r::snd::SoundHandle sBgmHandle; // 8043C810

private:
    s32 mStep;  // at 0x44 5 = done (after 90 or 40 frames)
    s32 mTimer; // at 0x48
};

class RPPnpServePhase : public RPSimpleModel { // state 2, vtable 803598A8
public:
    virtual ~RPPnpServePhase(); // at 0x8
    virtual void Calculate();   // at 0x14

    void Create();
    void Load();
    void Reset();
    void Draw();
    void Exit();

    /**
     * @brief Shows the server message and swaps courts / server when due
     */
    void Start();

    static s32 sServeWaitFrames; // 8043B720 (Pnp_serveWaitFrames) 80

private:
    void CalcCourtChange();
    void CalcWaitServe();

private:
    s32 mStep;           // at 0x44 1 court change, 2 wait for the serve
    s32 mTimer;          // at 0x48
    s32 mServeInput;     // at 0x4C [M] this frame's serve input state (3 = none)
    s32 mPrevServeInput; // at 0x50 [M]
};

class RPPnpPointPhase : public RPSimpleModel { // state 4, vtable 80359850
public:
    virtual ~RPPnpPointPhase(); // at 0x8
    virtual void Calculate();   // at 0x14

    void Create();
    void Load();
    void Reset();
    void Draw();
    void Exit();

    void Start();
    /**
     * @brief Two-player game: scores a point for a side (capped at 99) and plays the score switch
     */
    void AddPoint(s32 side);
    bool IsGameOver() const;
    s32 GetServeTurn() const;
    bool IsServeChangePoint() const;
    bool IsCourtChangePoint() const;
    bool IsServeTurnOdd() const;
    s32 GetServerSide() const;

    s32 GetScore(s32 side) const {
        return mScores[side];
    }
    s32 GetStep() const {
        return mStep;
    }

    static s32 sServeChangePoints; // 8043B710 (Pnp_serveChangePoints) 2
    static s32 sServeTurnPoints;   // 8043B714 (Pnp_serveTurnPoints) 4

private:
    s32 mWinPoints;  // at 0x44 11
    s32 mStep;       // at 0x48 3 = done
    s32 mTimer;      // at 0x4C
    s32 mScores[2];  // at 0x50
    s32 mLastScorer; // at 0x58 2 = none
};

class RPPnpResultPhase : public RPSimpleModel { // state 5, vtable 8035988C
public:
    virtual ~RPPnpResultPhase(); // at 0x8
    virtual void Calculate();    // at 0x14

    void Create();
    void Load();
    void Reset();
    void Draw();
    void Exit();

    void Start();

    s32 GetStep() const {
        return mStep;
    }
    bool IsRetry() const {
        return mRetry;
    }

    static s32 sResultFrames;    // 8043B718 (Pnp_resultFrames) 180, camera pull-back
    static f32 sCameraDistance;  // 8043B71C (Pnp_resultCameraValue) -1350

private:
    s32 mStep;                 // at 0x44 6 = done
    s32 mTimer;                // at 0x48
    s32 mWinner;               // at 0x4C -1 none / draw
    bool mRetry;               // at 0x50 [M] picks the next scene (0x13 retry, 0x1B quit)
    f32 mStartCameraValue;     // at 0x54 [L] camera +0xE4 when the phase started
    RPSysEffect* mpEffects[2]; // at 0x58 "Pnp_fubuki" confetti
};

//! @}

#endif
