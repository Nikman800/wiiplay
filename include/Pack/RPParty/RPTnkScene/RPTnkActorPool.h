#ifndef RP_PARTY_TNK_ACTOR_POOL_H
#define RP_PARTY_TNK_ACTOR_POOL_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPTnkActor;

/**
 * @brief Fixed-size pool of RPTnkActor objects
 * @details vtable 8035B238 (ledger Tnk_ActorPool; abstract: CreateObject and Destroy are pure,
 * every pool in the binary uses a derived vtable). Size 0x18 [H] (RPTnkBlockPool and
 * RPTnkCheckMarkPool allocate 0x18 with this constructor and add nothing). Init allocates the
 * pointer table, creates every object through CreateObject and calls its Create.
 * The ...All methods call the actor slot of the same name on every active actor.
 */
class RPTnkActorPool : public RPSceneObj {
public:
    RPTnkActorPool();
    virtual ~RPTnkActorPool(); // at 0x8

    /**
     * @brief Load on every actor [H]
     */
    virtual void Load(); // at 0x10
    /**
     * @brief Reset on every actor, active count 0 [H]
     */
    virtual void Reset(); // at 0x14

    virtual RPTnkActor* CreateObject() = 0; // at 0x24
    /**
     * @brief Allocates num objects; sets their mIndex and mpPool [H]
     */
    virtual void Init(s32 num); // at 0x28
    /**
     * @brief Deletes the pool singleton (each derived pool's DestroyInstance) [M]
     */
    virtual void Destroy() = 0; // at 0x2C

    virtual void ApplyVelocityAll();    // at 0x30
    virtual void ClearForcesAll();      // at 0x34
    virtual void CollideAllWithField(); // at 0x38
    virtual void IntegrateAll();        // at 0x3C
    virtual void CalcAll();             // at 0x40
    /**
     * @brief Increments mAge of every active actor [H]
     */
    virtual void TickAge(); // at 0x44
    /**
     * @brief CalcModelMtx on every active actor [H]
     */
    virtual void CalcModelMtxAll(); // at 0x48
    /**
     * @brief Spawns the next free actor (round robin from mppNext); NULL when full [H]
     */
    virtual RPTnkActor* Spawn(const nw4r::math::VEC3& rPos, u32 param); // at 0x4C
    virtual void KillAll(); // at 0x50

    void DecActiveNum() { mActiveNum--; }

protected:
    s32 mCapacity;          // at 0x4
    s32 mActiveNum;         // at 0x8
    RPTnkActor** mppBegin;  // at 0xC
    RPTnkActor** mppLast;   // at 0x10 last entry (inclusive)
    RPTnkActor** mppNext;   // at 0x14 next spawn candidate
};

//! @}

#endif
