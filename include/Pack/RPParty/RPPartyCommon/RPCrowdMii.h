#ifndef RP_PARTY_COMMON_CROWD_MII_H
#define RP_PARTY_COMMON_CROWD_MII_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPDollBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Background Mii walking across the game-select screen
 * @details vtable 80358608. Size 0x1E0, verified (Party_CrowdMgr_create). Eight states (table
 * 8031E370): 0 hidden, 1 idle, 2-3 walk, 4 run away, 5 swim, 6 flail, 7 look (animations
 * cmn_waitR, runawayR, swimR, batabataR ...). Start puts it at a random screen edge and walks
 * it across; the walk speed eases from mSpeed toward mTargetSpeed.
 * @note Wii Play-only framework class (ledger Party_CrowdMii); the name is a project
 * convention.
 */
class RPCrowdMii : public RPDollBase {
public:
    explicit RPCrowdMii(s32 index);
    virtual ~RPCrowdMii(); // at 0x8

    virtual void Load();      // at 0x94
    virtual void Reset();     // at 0x98
    virtual void Calculate(); // at 0x9C

    bool Start(s32 kind);
    bool IsHidden() const;

private:
    void PickMoveState();
    void EnterIdle();
    void CalcIdle();
    void EnterMove1();
    void CalcMove1();
    void EnterMove2();
    void CalcMove2();
    void EnterRunaway();
    void CalcRunaway();
    void EnterSwim();
    void CalcSwim();
    void EnterFlail();
    void CalcFlail();
    void EnterLook();
    void CalcLook();

private:
    RPStateMachine<RPCrowdMii>* mpStateMachine; // at 0x1CC 8 states
    f32 mScreenAspect;                          // at 0x1D0 screen width ratio [M]
    s32 mStateEndTime;                          // at 0x1D4
    f32 mSpeed;                                 // at 0x1D8
    f32 mTargetSpeed;                           // at 0x1DC

public:
    static f32 sScale;
    static f32 sBaseY;
    static f32 sBaseZ;
    static f32 sEdgeX;
    static f32 sSpacingZ;
    static f32 sMove1Speed;
    static f32 sMove2Speed;
    static f32 sRunawaySpeed;
    static f32 sSwimSpeed;
    static f32 sLookDistance;
    static f32 sLookAngle;
};

//! Ledger Party_CrowdMiiSeq (size 0x28)
typedef RPStateMachine<RPCrowdMii> RPCrowdMiiSeq;

/**
 * @brief Holds the ten crowd Miis and spawns them at random (singleton sInstance)
 * @details vtable 803586B8. Size 0x2C (Party_CrowdMgr_create allocates 0x2C and constructs it
 * inline). While fewer than five are walking, starts a hidden one with a small chance per frame
 * (always when none is walking).
 * @note Wii Play-only framework class (ledger Party_CrowdMgr); the name is a project
 * convention.
 */
class RPCrowdMgr {
public:
    static const int MII_NUM = 10;

public:
    static void CreateInstance();
    static void DestroyInstance();
    /**
     * @brief Model resources the crowd Miis need, 60 (ledger Party_getConst60; defined alone in RPCrowdMii_3.cpp) [L]
     */
    static s32 GetResourceNum();
    static RPCrowdMgr* GetInstance() {
        return sInstance;
    }

    virtual ~RPCrowdMgr();    // at 0x8
    virtual void Load();      // at 0xC
    virtual void Reset();     // at 0x10
    virtual void Calculate(); // at 0x14

private:
    RPCrowdMgr();
    s32 GetWalkingNum() const;

private:
    RPCrowdMii* mpMiis[MII_NUM]; // at 0x4

    static RPCrowdMgr* sInstance; // 8043C708
};

//! @}

#endif
