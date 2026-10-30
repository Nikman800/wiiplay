#ifndef RP_PARTY_WLY_PHASE_H
#define RP_PARTY_WLY_PHASE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

//! @addtogroup rp_wly
//! @{

/**
 * @brief Game-flow phases owned by RPWlyGameMgr
 * @details Each derives directly from RPSimpleModel and overrides only Calculate; the rest is
 * called directly. Sizes verified (Wly_GameMgr_ctor): Intro/Target/Clear 0x50, Result 0x54.
 */
class RPWlyIntroPhase : public RPSimpleModel { // vtable 8035CA88
public:
    virtual ~RPWlyIntroPhase(); // at 0x8
    virtual void Calculate();   // at 0x14

    void Create();
    void Init();
    void Reset();
    void Draw();
    void Destroy();

public:
    s32 mStep;  // at 0x44
private:
    s32 mTimer; // at 0x48
    u32 unk4C;  // at 0x4C [L] never accessed
};

/**
 * @brief Question phase: plays the stage BGM, level / timer / question layouts, and closes
 * the wipe when the question goes out
 */
class RPWlyTargetPhase : public RPSimpleModel { // vtable 8035CAA8
public:
    virtual ~RPWlyTargetPhase(); // at 0x8
    virtual void Calculate();    // at 0x14

    void Create();
    void Init();
    void Reset();
    void Draw();
    void Destroy();
    void Start();

public:
    s32 mStep;  // at 0x44
private:
    s32 mTimer; // at 0x48
    u32 unk4C;  // at 0x4C [L] never accessed
};

/**
 * @brief Stage clear: Wtd_clear effect, then the clear sequence (UpdateSequence)
 */
class RPWlyClearPhase : public RPSimpleModel { // vtable 8035C8B8
public:
    virtual ~RPWlyClearPhase(); // at 0x8
    virtual void Calculate();   // at 0x14

    void Create();
    void Init();
    void Reset();
    void Draw();
    void Destroy();
    void Start();
    void UpdateSequence();

public:
    s32 mStep;  // at 0x44
private:
    s32 mTimer; // at 0x48
    u32 unk4C;  // at 0x4C [L] never accessed
};

/**
 * @brief Time-up / result: plays the result jingle (0x78) and tallies the scores
 */
class RPWlyResultPhase : public RPSimpleModel { // vtable 8035CA6C
public:
    virtual ~RPWlyResultPhase(); // at 0x8
    virtual void Calculate();    // at 0x14

    void Create();
    void Init();
    void Reset();
    void Draw();
    void Destroy();
    void Start();
    void CalcTally();

public:
    s32 mStep;     // at 0x44
private:
    s32 mTimer;    // at 0x48
    s32 mTallyIdx; // at 0x4C [M] -1 after Reset
public:
    bool unk50;    // at 0x50 [L] true after Reset
};

//! @}

#endif
