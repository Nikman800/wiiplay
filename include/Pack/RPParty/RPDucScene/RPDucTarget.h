#ifndef RP_PARTY_DUC_TARGET_H
#define RP_PARTY_DUC_TARGET_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPDucScene/RPDucSound.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_duc
//! @{

// Forward declarations
class RPDucAvatar;
class RPDucTargetMgr;

/**
 * @brief Base of every Shooting Range target (clay, UFO, balloon, round target, duck, can,
 * Mii avatar)
 * @details vtable 8035A0F0 (instantiated by the constructor only; every allocated target uses
 * a derived vtable). Size 0xFC [H]: the constructor initialises members up to 0xF8 and the
 * derived classes start theirs at 0xFC (Clay/Can/CircleTarget/Avatar members, or the
 * IRPGrpModelCallback part of Balloon/Duck/Ufo).
 * Physics per sub-step (RPDucScene runs three): ApplyVelocity (pos += mVelocity), the force
 * hooks, Calculate, then the age counter. ApplyForces adds mPosForces[0..1] to the position and
 * mVelForces[0..1] to the velocity; ClearForces zeroes them.
 * mAge is -1 while the target is free, 0.. while active and -999 once removed;
 * RPDucTargetMgr::Spawn picks a target with a negative age.
 * @note Name is a project convention (ledger Duc_Target).
 */
class RPDucTarget : public RPModelObj {
    friend class RPDucTargetMgr;
    friend class RPDucAvatarMgr;
    friend class RPDucShot;

public:
    RPDucTarget();
    virtual ~RPDucTarget(); // at 0x8

    /**
     * @brief Creates mpSndActor [H]
     */
    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18

    virtual void ClearForces();     // at 0x58
    virtual void ApplyForces();     // at 0x5C
    virtual void ResetParams();     // at 0x60
    /**
     * @brief Starts the target's flight/appearance; the argument selects a pattern [M]
     */
    virtual void Launch(u32 pattern); // at 0x64
    virtual void SpawnHitEffect();    // at 0x68
    virtual void PlaySpawnSound();    // at 0x6C
    virtual void PlayHitSound(s32 player); // at 0x70

    // Returns the plain base struct: RPDucAvatarMgr::IsLowest copies it word by word, then into
    // a VEC3 through VEC3(const _VEC3&) (a VEC3 or EGG::Vector3f return copies differently)
    nw4r::math::_VEC3 GetPos() const {
        return mPos;
    }
    /**
     * @brief Projects mPos to the screen and spawns an RPDucHud point popup [H]
     */
    virtual void ShowScorePopup(s32 points, u32 arg1, s32 player); // at 0x74
    virtual void ApplyVelocity();                                  // at 0x78
    virtual void ClearForcesHook();                                // at 0x7C
    virtual void VF_0x80();                                        // at 0x80
    virtual void ApplyForcesHook();                                // at 0x84
    virtual void DebugDrawSphere();                                // at 0x88
    virtual void DebugDrawBox();                                   // at 0x8C
    /**
     * @brief resetState, copy rPos to mPos, ResetParams, Launch(arg), mAge = 0,
     * PlaySpawnSound, SetUnk40(1) [H]
     */
    virtual void Activate(const nw4r::math::VEC3& rPos, u32 arg); // at 0x90
    /**
     * @brief mAge = -999, decrements the manager's active count, SetUnk40(0) [H]
     */
    virtual void Remove();                  // at 0x94
    virtual void VF_0x98();                 // at 0x98
    virtual void VF_0x9C(RPDucAvatar* pAvatar); // at 0x9C (RPDucUfo: SetTargetAvatar)
    /**
     * @brief NULL here; RPDucAvatar returns itself [M]
     */
    virtual RPDucAvatar* AsAvatar() { // at 0xA0
        return NULL;
    }
    virtual bool CanShotPassThrough() { // at 0xA4
        return false;
    }

    /**
     * @brief Back to the free state: transform, velocity, forces and hit records reset,
     * mAge = -1, SetUnk40(0) [H]
     */
    void ResetState();

    /**
     * @brief Clears the per-player hit records (inline in ResetState and
     * RPDucTargetMgr::ClearHitRecords)
     */
    void ClearHitOrders() {
        s16 i = 0;
        do {
            mHitOrder[i] = -1;
        } while (++i < 4);
    }
    void ClearHitCounts() {
        s16 i = 0;
        do {
            mHitCount[i] = 0;
        } while (++i < 4);
    }
    void ClearHitPositions() {
        s16 i = 0;
        do {
            mHitPos[i] = RP_vecZero;
        } while (++i < 4);
    }
    void ClearHitRecord() {
        s16 i = 0;
        do {
            mHitOrder[i] = -1;
        } while (++i < 4);
        s16 j = 0;
        do {
            mHitCount[j] = 0;
        } while (++j < 4);
        s16 k = 0;
        do {
            mHitPos[k] = RP_vecZero;
        } while (++k < 4);
    }

    /**
     * @brief Player whose shot hit first this step (lowest mHitOrder), awarding/clearing the
     * other players' streaks [M]
     */
    s32 GetHittingPlayer();
    f32 GetDistanceToCamera();

protected:
    nw4r::math::VEC3 mVelocity;        // at 0x48
    nw4r::math::VEC3 mPosForces[2];    // at 0x54 added to mPos by ApplyForces
    nw4r::math::VEC3 mVelForces[2];    // at 0x6C added to mVelocity by ApplyForces
    nw4r::math::VEC3 unk84;            // at 0x84 [L]
    f32 mHitRadius;                    // at 0x90 [M] from the parameter file
    nw4r::math::VEC3 mHitOffset;       // at 0x94 [M] sphere centre relative to mPos
    f32 mHitScale;                     // at 0xA0 [M] debug sphere scale
    bool unkA4;                        // at 0xA4 [L] set by Activate
    s16 mHitOrder[4];                  // at 0xA6 per player, -1 = not hit [M]
    s16 mHitCount[4];                  // at 0xAE per player [L]
    nw4r::math::VEC3 mHitPos[4];       // at 0xB8 per player [M]
    s32 unkE8;                         // at 0xE8 [L] launch pattern
    s32 mAge;                          // at 0xEC -1 free, -999 removed
    s32 mIndex;                        // at 0xF0 index in the manager's pool
    RPDucTargetMgr* mpMgr;             // at 0xF4
    RPDucSndActor* mpSndActor;         // at 0xF8 0x74-byte sound actor (vtable 80359B00)
};

//! @}

#endif
