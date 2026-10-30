#ifndef RP_PARTY_TNK_ACTOR_H
#define RP_PARTY_TNK_ACTOR_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>
#include <Pack/RPParty/RPTnkScene/RPTnkSound.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPTnkActorPool;
class RPTnkTank;

/**
 * @brief Base of every Tanks! world object (tank, shell, mine, block, check mark, AI probe)
 * @details vtable 8035B290 (ledger Tnk_Actor; the ledger's Tnk_Cls_8035B290_vf34..46 are this
 * class's default slots 0x90-0xC0). Size 0xB0 [H]: the constructor initialises members up to
 * 0xAC, Create stores the sound actor at 0xAC, and RPTnkTank's IRPGrpModelCallback base sits
 * at 0xB0. Objects live in an RPTnkActorPool; mAge < 0 means the slot is free (-1 after
 * ResetState, -999 after Kill), Spawn sets it to 0 and the pool's TickAge counts it up.
 * Physics per step: velocity is applied, accumulators cleared, collisions push into the
 * accumulators, IntegrateStep adds them to position and velocity.
 */
class RPTnkActor : public RPModelObj {
    friend class RPTnkActorPool;
    friend class RPTnkWeaponManager;
    friend class RPTnkTankManager; // IncrementTimers

public:
    RPTnkActor();
    virtual ~RPTnkActor(); // at 0x8

    /**
     * @brief Creates the sound actor (mpSound) [H]
     */
    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    /**
     * @brief Calls ResetState [H]
     */
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18

    /**
     * @brief Zeroes position and velocity, unit scale, identity rotation, hit radius 1,
     * mAge -1, then SetUnk40(0) (hidden) [H]
     */
    virtual void ResetState(); // at 0x58
    /**
     * @brief Zeroes mPosAccum and mVelAccum [H]
     */
    virtual void ClearForces(); // at 0x5C
    /**
     * @brief pos += mPosAccum[0] + mPosAccum[1], vel += mVelAccum[0] + mVelAccum[1] [H]
     */
    virtual void Integrate(); // at 0x60
    /**
     * @brief Called by Spawn after the position is set (RPTnkTank: InitParams) [M]
     */
    virtual void OnSpawn(); // at 0x64
    /**
     * @brief Called by Spawn with the packed spawn parameter (shell/mine: owner slot, type) [M]
     */
    virtual void ApplySpawnParam(u32 param); // at 0x68
    /**
     * @brief pos += mVelocity [H]
     */
    virtual void ApplyVelocity(); // at 0x6C
    /**
     * @brief Start of the collision step: ClearForces (overrides also clear hit flags) [H]
     */
    virtual void ClearForcesStep(); // at 0x70
    /**
     * @brief Collision step (field walls, blocks, other objects) [M]
     */
    virtual void CollideWithField(); // at 0x74
    /**
     * @brief End of the collision step: Integrate [H]
     */
    virtual void IntegrateStep(); // at 0x78
    /**
     * @brief ResetState, place at rPos, OnSpawn, ApplySpawnParam(param), mAge 0, SetUnk40(1) [H]
     */
    virtual void Spawn(const nw4r::math::VEC3& rPos, u32 param); // at 0x7C
    /**
     * @brief mAge -999, decrements the pool's active count, SetUnk40(0) [H]
     */
    virtual void Kill(); // at 0x80
    /**
     * @brief Circle test in XZ against another actor using mHitOffset and mHitRadius; writes
     * the distance [H]
     */
    virtual BOOL TestHit(RPTnkActor* pOther, f32* pDist); // at 0x84
    /**
     * @brief Same test using this actor's mHitOffsetAlt [H]
     */
    virtual BOOL TestHitAlt(RPTnkActor* pOther, f32* pDist); // at 0x88
    /**
     * @brief Called by RPTnkField::CollideActor for each wall the hit circle crosses [L]
     */
    virtual void OnHitWall(f32 depth, const nw4r::math::VEC3* pNormal, s32 wall,
                           const nw4r::math::VEC3& rVel); // at 0x8C
    // Wii Play: 0x90, 0x94, 0xA4, 0xA8, 0xAC and 0xBC are defined in RPTnkActorPool.cpp (normal
    // functions there), the other empty slots are inline (weak copies after RPTnkActor.cpp)
    virtual void VF_0x90(); // at 0x90 never overridden
    /**
     * @brief Pushes another actor out of this one (RPTnkBlock) [M]
     */
    virtual BOOL CollideObject(RPTnkActor* pOther, s32 mode); // at 0x94
    /**
     * @brief Player slot of the owner: 0/1 players, 2+ enemies (shell, mine) [M]
     */
    virtual s32 GetOwnerSlot() const { return 0; } // at 0x98
    virtual s32 GetKind() const { return 0; }      // at 0x9C RPTnkBlock
    virtual s32 GetHeight() const { return 0; }    // at 0xA0 RPTnkBlock
    virtual void SetTargetPos(const nw4r::math::VEC3& rPos); // at 0xA4 RPTnkMine
    virtual BOOL IsInBlastRadius(const RPTnkActor* pOther) const; // at 0xA8
    virtual BOOL IsInTriggerRange(const RPTnkActor* pOther) const; // at 0xAC
    virtual void StopEffects() {}                   // at 0xB0 RPTnkShell
    virtual void Explode() {}                       // at 0xB4 RPTnkMine
    virtual BOOL IsExploding() const { return FALSE; } // at 0xB8 RPTnkMine
    virtual void SetOwnerTank(RPTnkTank* pTank);  // at 0xBC
    virtual RPTnkTank* GetOwnerTank() const { return NULL; } // at 0xC0

    s32 GetAge() const { return mAge; }
    // A POD copy: RPTnkShellPool::DetectIncomingShells reads the position through a
    // returned temporary (the extra 12-byte stack copy) [M]
    nw4r::math::_VEC3 GetPosCopy() const { return mPos; }
    const nw4r::math::VEC3& GetPos() const { return mPos; }
    nw4r::math::VEC3 GetHitCenter() const {
        nw4r::math::VEC3 center;
        center.x = mPos.x + mHitOffset.x;
        center.y = mPos.y + mHitOffset.y;
        center.z = mPos.z + mHitOffset.z;
        return center;
    }
    nw4r::math::VEC3 GetHitCenterAlt() const {
        nw4r::math::VEC3 center;
        center.x = mPos.x + mHitOffsetAlt.x;
        center.y = mPos.y + mHitOffsetAlt.y;
        center.z = mPos.z + mHitOffsetAlt.z;
        return center;
    }
    bool IsActive() const { return mAge >= 0; }
    f32 GetHitRadius() const { return mHitRadius; }
    const nw4r::math::VEC3& GetHitOffsetAlt() const { return mHitOffsetAlt; }
    s32 GetIndex() const { return mIndex; }
    void SetIndex(s32 idx) { mIndex = idx; }

protected:
    nw4r::math::VEC3 mVelocity;       // at 0x48
    nw4r::math::VEC3 mPosAccum[2];    // at 0x54 cleared by ClearForces
    nw4r::math::VEC3 mVelAccum[2];    // at 0x6C cleared by ClearForces
    nw4r::math::VEC3 mHitOffset;      // at 0x84 hit circle centre relative to mPos
    f32 mHitRadius;                   // at 0x90
    nw4r::math::VEC3 mHitOffsetAlt;   // at 0x94 second hit circle (TestHitAlt)
    s32 mAge;                         // at 0xA0 frames since spawn; < 0 when inactive
    s32 mIndex;                       // at 0xA4 index in the pool / tank list
    RPTnkActorPool* mpPool;           // at 0xA8
    RPTnkActorSound* mpSound;         // at 0xAC
};

//! @}

#endif
