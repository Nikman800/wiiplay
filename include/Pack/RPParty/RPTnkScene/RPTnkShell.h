#ifndef RP_PARTY_TNK_SHELL_H
#define RP_PARTY_TNK_SHELL_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPTnkScene/RPTnkActor.h>
#include <Pack/RPParty/RPTnkScene/RPTnkActorPool.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPSysEffect;
class RPTnkAiMove;

/**
 * @brief Tank shell
 * @details vtable 8035B804. Size 0xEC, verified (RPTnkShellPool::CreateObject). Spawn parameter
 * (RPTnkWeaponManager::FireShell): owner slot in bits 0-3, bounces in bits 4-7, smoke effect
 * in bits 8-10, bits 12-15 [L]. Hit radius 6 for collisions.
 */
class RPTnkShell : public RPTnkActor {
public:
    typedef void (RPTnkShell::*StateFunc)();

public:
    RPTnkShell();
    virtual ~RPTnkShell(); // at 0x8

    /**
     * @brief Creates the four smoke effects ("Tnk_shellsmoke01".."03") [H]
     */
    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10 G3D/shell.brres "shell"
    virtual void Calculate(); // at 0x18 state, sound position

    virtual void OnSpawn();                  // at 0x64 scale, rotation, flags, 1 bounce
    virtual void ApplySpawnParam(u32 param); // at 0x68
    virtual void ApplyVelocity();            // at 0x6C
    virtual void ClearForcesStep();          // at 0x70 also clears the hit flags
    /**
     * @brief Field walls, tanks (RPTnkTankManager::FindShellHit), mines and other shells [H]
     */
    virtual void CollideWithField(); // at 0x74
    virtual void Kill();             // at 0x80 also stops the smoke effects
    virtual BOOL TestHit(RPTnkActor* pOther, f32* pDist); // at 0x84
    /**
     * @brief Reflects the velocity off a wall and counts the bounce [M]
     */
    virtual void OnHitWall(f32 depth, const nw4r::math::VEC3* pNormal, s32 wall,
                           const nw4r::math::VEC3& rVel); // at 0x8C
    virtual s32 GetOwnerSlot() const;                    // at 0x98
    virtual void StopEffects();                          // at 0xB0
    virtual void SetOwnerTank(RPTnkTank* pTank);         // at 0xBC
    virtual RPTnkTank* GetOwnerTank() const;             // at 0xC0

    void PlayHitSound();
    /**
     * @brief Only state: applies the hit flags (explode, bounce effects, rotation) [M]
     */
    void StateResolveHits();

private:
    s32 mBouncesLeft;          // at 0xB0 reset to 1
    s32 mOwnerSlot;            // at 0xB4
    u32 mSmokeType;            // at 0xB8 index into mpSmokeEffects
    s32 unkBC;                 // at 0xBC [L] spawn parameter bits 12-15
    bool mIsEnemy;             // at 0xC0 owner slot > 1
    bool mHitTank;             // at 0xC1
    bool mHitFlags[4];         // at 0xC2 [L] mine, shell, wall, ...
    s32 mState;                // at 0xC8
    RPSysEffect* mpSmokeEffects[4]; // at 0xCC
    RPTnkTank* mpOwnerTank;    // at 0xDC
    StateFunc mStateFuncs[1];  // at 0xE0
};

/**
 * @brief Pool of the 32 shells (singleton Tnk_shellPoolInstance)
 * @details vtable 8035B7A0. Size 0x40, verified (Tnk_ShellPool_createInstance). Owned by
 * RPTnkWeaponManager.
 */
class RPTnkShellPool : public RPTnkActorPool {
public:
    static void CreateInstance();
    static void DestroyInstance() {
        if (sInstance != NULL) {
            sInstance->Destroy();
        }
    }

    virtual ~RPTnkShellPool(); // at 0x8

    virtual RPTnkActor* CreateObject(); // at 0x24
    virtual void Destroy();             // at 0x2C

    /**
     * @brief Recounts mShellNum per owner slot [H]
     */
    void CountShellsPerOwner();
    s32 GetShellNum(s32 slot) const { return mShellNum[slot]; }
    void StopAllEffects();
    BOOL IsObjectHit(RPTnkActor* pActor, BOOL ignoreOwnShells);
    BOOL CollideShellWithShells(RPTnkShell* pShell);
    void DetectIncomingShells(RPTnkAiMove* pAiMove);

    static RPTnkShellPool* sInstance; // Tnk_shellPoolInstance

private:
    s32 mShellNum[10]; // at 0x18 active shells per owner slot
};

//! @}

#endif
