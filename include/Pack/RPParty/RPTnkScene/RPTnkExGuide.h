#ifndef RP_PARTY_TNK_EX_GUIDE_H
#define RP_PARTY_TNK_EX_GUIDE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPTnkTank;

/**
 * @brief Control hints ("ex" messages) shown on the first stage until the player moves and
 * aims (singleton RPTnkExGuide::sInstance)
 * @details vtable 8035C320. Size 0x6C, verified (Tnk_ExGuide_createInstance). Four states
 * (the state table is copied from 8035C2F0); Calculate also projects the target tank to the
 * screen for the message balloon.
 */
class RPTnkExGuide : public RPSceneObj {
public:
    static RPTnkExGuide* sInstance; // 8043C948

    typedef void (RPTnkExGuide::*StateFunc)();

public:
    RPTnkExGuide() : mpTarget(NULL) {
        mStateFuncs[0] = &RPTnkExGuide::StateCheckMoved;
        mStateFuncs[1] = &RPTnkExGuide::StateShowHint;
        mStateFuncs[2] = &RPTnkExGuide::StateShowHintUntilNear;
        mStateFuncs[3] = &RPTnkExGuide::StateCheckCursorNear;
    }

    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkExGuide(); // at 0x8

    virtual void Create();    // at 0xC
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Exit();      // at 0x1C

    void Start();
    // Wii Play: RPTnkTank::SpawnEnemy (first stage)
    void SetScreenPos(nw4r::math::VEC2 pos) { mScreenPos = pos; }
    void SetTarget(RPTnkTank* pTank) { mpTarget = pTank; }

    void StateCheckCursorNear();
    void StateShowHintUntilNear();
    void StateShowHint();
    void StateCheckMoved();

private:
    nw4r::math::VEC3 mStartPos[2]; // at 0x4 player tanks at stage start
    nw4r::math::VEC2 mScreenPos;   // at 0x1C
    RPTnkTank* mpTarget;           // at 0x24
    bool mDone;                    // at 0x28
    s32 mState;                    // at 0x2C
    f32 mMovedDistance[2];         // at 0x30 distance from the start position
    s32 mIdleTimer;                // at 0x38 frames without movement
    StateFunc mStateFuncs[4];      // at 0x3C order [L]
};

//! @}

#endif
