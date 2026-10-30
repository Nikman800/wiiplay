#ifndef RP_PARTY_TNK_WEAPON_MANAGER_H
#define RP_PARTY_TNK_WEAPON_MANAGER_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPTnkMinePool;
class RPTnkShellPool;
class RPTnkTank;
class RPTnkActor;

/**
 * @brief Owns the shell and mine pools (singleton Tnk_weaponManagerInstance)
 * @details vtable 8035B8C8. Size 0xC, verified (Tnk_WeaponManager_createInstance). The step
 * methods forward to the pools' slot of the same meaning.
 */
class RPTnkWeaponManager : public RPSceneObj {
public:
    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkWeaponManager(); // at 0x8

    /**
     * @brief Creates the shell pool (32) and the mine pool (16) [H]
     */
    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18 empty
    virtual void Exit();      // at 0x1C destroys both pools

    /**
     * @brief Spawns a shell at rPos (inside the field and not in a block) moving along rDir
     * at speed [H]
     */
    void FireShell(f32 speed, RPTnkTank* pTank, nw4r::math::VEC3* pPos,
                   nw4r::math::VEC3* pDir, s32 slot, s32 bounces, s32 smoke, s32 unk);
    /**
     * @brief Spawns a mine at rPos moving towards rTarget [H]
     */
    void LayMine(f32 speed, RPTnkTank* pTank, nw4r::math::VEC3* pPos,
                 const nw4r::math::VEC3* pTarget, u32 param);
    void DeactivateAll();
    void ApplyVelocity();
    void ClearHitFlags();
    void CheckCollisions();
    void IntegrateAll();
    void CalcAll();
    void TickAge();
    void CalcModelMtx();

    RPTnkShellPool* GetShellPool() const { return mpShellPool; }
    RPTnkMinePool* GetMinePool() const { return mpMinePool; }

    static RPTnkWeaponManager* sInstance; // Tnk_weaponManagerInstance

private:
    static inline void SetVelocity(RPTnkActor* pActor, nw4r::math::VEC3 velocity);
    static inline void SetDirection(RPTnkActor* pActor, nw4r::math::VEC3 direction);

    RPTnkShellPool* mpShellPool; // at 0x4
    RPTnkMinePool* mpMinePool;   // at 0x8
};

//! @}

#endif
