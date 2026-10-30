#ifndef RP_PARTY_COMMON_DOLL_H
#define RP_PARTY_COMMON_DOLL_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPDollBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Clickable Mii doll of a player on the game-select screen
 * @details vtable 80358490. Size 0x1E8, verified (Party_DollMgr_create). 18-state reaction
 * machine (table 8031DF10): 0 idle, 1-2 result loops (win/lose faces), 3 result loop
 * (neutral), 4-5 one-shot animations, 6 doze, 7 sleep, 8 wake up, 9-14 table reactions, 15 look
 * at the other doll, 16 pair reaction, 17 pair animation. Reset picks the start state from the
 * scene info's last result (RP_sceneInfoInstance + 0x14/+0x18). The doll dozes after the
 * pointer has been away for a while (mIdleTimer/mDozeTimer) when mDozeAllowed is set.
 * Overrides EditJointWorldMtx (primary slot 0xAC; ledger Party_Doll_adjustBillboardMatrix) to
 * turn the chest joint toward the camera.
 * @note Wii Play-only framework class (ledger Party_Doll); the name is a project convention.
 */
struct DollLookSpeedData;
struct DollReaction;

class RPDoll : public RPDollBase {
    friend class RPDollMgr;

public:
    RPDoll(s32 playerNum, s32 index);
    virtual ~RPDoll(); // at 0x8

    virtual void Load();      // at 0x94 adds the chest joint callback and 20 body animations
    virtual void Reset();     // at 0x98
    virtual void Calculate(); // at 0x9C

    /**
     * @brief IRPGrpModelCallback override (chest joint billboard) [M]
     */
    virtual void EditJointWorldMtx(RPGrpModel* pModel, u16 joint,
                                   nw4r::math::MTX34* pMtx); // at 0xAC

    void SetPointerActive(bool active);
    void LookAtPointer(const nw4r::math::VEC2& rPos);
    void SetScreenPos(const nw4r::math::VEC2& rPos);
    bool RequestReaction(s32 reaction);

private:
    void EnterIdle();
    void CalcIdle();
    void EnterFace1();
    void CalcAnim1Loop();
    void EnterFace3();
    void CalcAnim2Loop();
    void EnterResultLoop();
    void CalcResultLoop();
    void EnterAnim5();
    void CalcAnim5();
    void EnterAnim6();
    void CalcAnim6();
    void EnterDoze();
    void CalcDoze();
    void EnterSleep();
    void CalcSleep();
    void EnterWakeUp();
    void CalcWakeUp();
    void EnterTableReaction();
    void CalcTableReaction();
    void EnterLookAtOther();
    void CalcLookAtOther();
    void EnterPairReact();
    void CalcPairReact();
    void EnterPairAnim();
    void CalcPairAnim();

private:
    RPStateMachine<RPDoll>* mpStateMachine; // at 0x1CC 18 states
    s32 mStateEndTime;                      // at 0x1D0 state timer value that ends the state
    s32 mPlayerNum;                         // at 0x1D4
    bool mPointerActive;                    // at 0x1D8
    s32 mIdleTimer;                         // at 0x1DC frames without the pointer
    s32 mDozeTimer;                         // at 0x1E0
    bool mIdleExitAllowed;                  // at 0x1E4
    bool mDozeAllowed;                      // at 0x1E5
    u16 mChestJointId;                      // at 0x1E6

public:
    static f32 sAngles[2][2]; // [TV mode][doll], degrees
    static f32 sScale[2];     // [TV mode]
    static f32 sLookDistance;
    static f32 sBillboardFactor;
    static EGG::Vector2f sOffsets1P[2];    // [TV mode]
    static EGG::Vector2f sOffsets2P[2][2]; // [TV mode][doll]
    static DollLookSpeedData sLookSpeed;
    static const DollReaction sTableReactions[6];
};

//! Ledger Party_DollSeq (size 0x28)
typedef RPStateMachine<RPDoll> RPDollSeq;

/**
 * @brief Holds one RPDoll per player and fades them (singleton sInstance)
 * @details vtable 80358540. Size 0x18 (Party_DollMgr_create allocates 0x18 and constructs it
 * inline). The destructor is the last slot. Calculate steps mAlpha by mAlphaStep (0..255) and
 * writes it to the scene's light map, then calculates every doll.
 * @note Wii Play-only framework class (ledger Party_DollMgr); the name is a project convention.
 */
class RPDollMgr {
public:
    static void CreateInstance(s32 playerNum);
    static void DestroyInstance();
    /**
     * @brief Model resources the dolls need, 12 (ledger Party_getConst12b; defined alone in RPDoll_3.cpp) [L]
     */
    static s32 GetResourceNum();
    static RPDollMgr* GetInstance() {
        return sInstance;
    }
    s32 GetAlpha() const {
        return mAlpha;
    }

    virtual void Load();      // at 0x8
    virtual void Reset();     // at 0xC
    virtual void Calculate(); // at 0x10
    virtual ~RPDollMgr();     // at 0x14

    s32 HitTestDoll(const nw4r::math::VEC2& rPos, s32 player) const;
    void SetPointerActive(bool active);
    void LookAt(const nw4r::math::VEC2& rPos, s32 player);
    void ClearLook(s32 player);
    const nw4r::math::VEC3& GetDollPos(s32 player) const;
    void SetDollScreenPos(const nw4r::math::VEC2& rPos, s32 player);
    void SetDozeAllowed(bool allowed);
    void SetIdleExitAllowed(bool allowed);
    bool AreAllAsleep() const;
    bool RequestReaction(s32 reaction, s32 player);
    RPSysKokeshi* GetAvatar(s32 player) const;

private:
    explicit RPDollMgr(s32 playerNum);

private:
    s32 mPlayerNum;                      // at 0x4
    s32 mAlpha;                          // at 0x8 255 after Reset
public:
    s32 mAlphaStep;                      // at 0xC
private:
    RPDoll* mpDolls[RP_PARTY_MAX_PLAYERS]; // at 0x10

    static RPDollMgr* sInstance; // 8043C6F8
};

//! @}

#endif
