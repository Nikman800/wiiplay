#ifndef RP_PARTY_TNK_MINE_H
#define RP_PARTY_TNK_MINE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPTnkScene/RPTnkActor.h>
#include <Pack/RPParty/RPTnkScene/RPTnkActorPool.h>
#include <Pack/RPParty/RPTnkScene/RPTnkSound.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPTnkAiMove;

// Wii Play: the mine sound needs the empty intermediate sound base. Keep this
// corrected actor here because the shared RPTnkSound.h declaration is used by
// other units and is outside this unit's permitted edit scope.
class RPTnkMineSoundActor : public TRPSndGameObject<0> {
public:
    RPTnkMineSoundActor()
        : TRPSndGameObject<0>(RPSndAudioMgr::instance()->getPlayer(),
                              *static_cast<nw4r::snd::Sound3DManager*>(NULL)) {}
    virtual ~RPTnkMineSoundActor();
};

/**
 * @brief Mine laid by a tank
 * @details vtable 8035BA54. Size 0x104, verified (RPTnkMinePool::CreateObject). States: 0 move
 * to target, 1 armed (explodes after 480 frames, 20 frames after a tank comes near, or at
 * once when shot), 2 explode (blast radius grows from 8 to 80 over 6 frames, Kill after 40).
 */
class RPTnkMine : public RPTnkActor {
public:
    typedef void (RPTnkMine::*StateFunc)();

public:
    RPTnkMine() DECOMP_DONT_INLINE;
    virtual ~RPTnkMine(); // at 0x8

    virtual void Create();       // at 0xC also creates mpMineSound
    virtual void Load();         // at 0x10
    virtual void Calculate();    // at 0x18
    virtual void CalcModelMtx(); // at 0x24

    virtual void OnSpawn();                  // at 0x64
    virtual void ApplySpawnParam(u32 param); // at 0x68 owner slot (bits 0-3)
    virtual void ApplyVelocity();            // at 0x6C
    virtual void ClearForcesStep();          // at 0x70
    virtual void CollideWithField();         // at 0x74
    virtual BOOL TestHit(RPTnkActor* pOther, f32* pDist);    // at 0x84
    virtual BOOL TestHitAlt(RPTnkActor* pOther, f32* pDist); // at 0x88
    /**
     * @brief Accumulates the wall push-out [M]
     */
    virtual void OnHitWall(f32 depth, const nw4r::math::VEC3* pNormal, s32 wall,
                           const nw4r::math::VEC3& rVel); // at 0x8C
    virtual s32 GetOwnerSlot() const;                       // at 0x98
    virtual void SetTargetPos(const nw4r::math::VEC3& rPos); // at 0xA4
    virtual BOOL IsInBlastRadius(const RPTnkActor* pOther) const; // at 0xA8
    virtual BOOL IsInTriggerRange(const RPTnkActor* pOther) const; // at 0xAC
    /**
     * @brief Effect "Tnk_mine_bomb", explosion sound, state 2 [H]
     */
    virtual void Explode();                           // at 0xB4
    virtual BOOL IsExploding() const;                 // at 0xB8
    virtual void SetOwnerTank(RPTnkTank* pTank);      // at 0xBC
    virtual RPTnkTank* GetOwnerTank() const;          // at 0xC0

    void StateMoveToTarget();
    void StateArmed();
    void StateExplode();

private:
    static f32 GetActorHitRadius(const RPTnkActor* pActor) {
        return pActor->*(&RPTnkMine::mHitRadius);
    }

    nw4r::math::VEC3 mTargetPos; // at 0xB0
    s32 mOwnerSlot;              // at 0xBC
    s32 mTimer;                  // at 0xC0
    f32 unkC4;                   // at 0xC4 [L] reset to 9
    f32 mBlastRadius;            // at 0xC8 9, then 8-80 while exploding
    bool mIsEnemy;               // at 0xCC
    bool mHitFlags[6];           // at 0xCD [M] tank near, shell hit, explosion hit, ...
    bool mFuseLit;               // at 0xD3
    s32 mState;                  // at 0xD4
    RPTnkMineSoundActor* mpMineSound; // at 0xD8
    RPTnkTank* mpOwnerTank;      // at 0xDC
    StateFunc mStateFuncs[3];    // at 0xE0
};

/**
 * @brief Pool of the 16 mines (singleton Tnk_minePoolInstance)
 * @details vtable 8035B9D8. Size 0x40, verified (Tnk_MinePool_createInstance). Owned by
 * RPTnkWeaponManager.
 */
class RPTnkMinePool : public RPTnkActorPool {
public:
    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkMinePool(); // at 0x8

    virtual RPTnkActor* CreateObject(); // at 0x24
    virtual void Destroy();             // at 0x2C

    void CountByOwner();
    s32 GetMineNum(s32 slot) const { return mMineNum[slot]; }
    BOOL IsNearAnyMine(RPTnkActor* pActor);
    BOOL IsInExplosion(RPTnkActor* pActor);
    BOOL IsInOtherExplosion(RPTnkActor* pActor) DECOMP_DONT_INLINE;
    BOOL CheckShellHit(RPTnkActor* pShell);
    void AddTankAvoidance(RPTnkAiMove* pAiMove);

    static RPTnkMinePool* sInstance; // Tnk_minePoolInstance

private:
    s32 GetActiveMineNum() const { return mActiveNum; }

    s32 mMineNum[10]; // at 0x18 active mines per owner slot
};

//! @}

#endif
