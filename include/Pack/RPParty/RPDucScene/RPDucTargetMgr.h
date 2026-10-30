#ifndef RP_PARTY_DUC_TARGET_MGR_H
#define RP_PARTY_DUC_TARGET_MGR_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_duc
//! @{

// Forward declarations
class RPDucAvatar;
class RPDucTarget;

/**
 * @brief Pool of targets of one type, driven per sub-step by RPDucTargetRegistry
 * @details vtable 8035A090 (abstract: AllocTarget and DestroyInstance are empty slots). Size
 * 0x1C, verified (the six simple managers allocate 0x1C with its constructor). The pool is a
 * ring: Spawn scans from mppNext for a target with a negative age, activates it and advances.
 * Each "...All" virtual calls the target virtual of the same name on every active target.
 * Target types (index in the registry tables 8035A198/8035A1B4): 0 clay, 1 UFO, 2 balloon,
 * 3 round target, 4 duck, 5 can, 6 avatar.
 * @note Name is a project convention (ledger Duc_TargetMgr).
 */
class RPDucTargetMgr : public RPSceneObj {
    friend class RPDucTarget;
    friend class RPDucShot;

public:
    RPDucTargetMgr();
    virtual ~RPDucTargetMgr(); // at 0x8

    virtual void Load();  // at 0x10
    virtual void Reset(); // at 0x14

    /**
     * @brief Allocates and constructs one target of the manager's type
     */
    virtual RPDucTarget* AllocTarget() = 0; // at 0x24
    /**
     * @brief Allocates the pool and creates every target (Create, mIndex, mpMgr) [H]
     */
    virtual void CreatePool(s32 num);      // at 0x28
    virtual void DestroyInstance() = 0;    // at 0x2C
    virtual void ClearHitRecords();        // at 0x30
    virtual void ApplyVelocityAll();       // at 0x34
    virtual void ClearForcesHookAll();     // at 0x38
    virtual void VF_0x80All();             // at 0x3C
    virtual void ApplyForcesHookAll();     // at 0x40
    virtual void CalculateAll();           // at 0x44
    virtual void IncAgeAll();              // at 0x48
    virtual void CalcModelMtxAll();        // at 0x4C
    virtual void DebugDrawAll();           // at 0x50
    virtual RPDucTarget* Spawn(const nw4r::math::VEC3& rPos, u32 arg); // at 0x54
    virtual void RemoveAll();              // at 0x58

    s32 GetType() const { return mType; }
    void SetType(s32 type) { mType = type; }
    s32 GetActiveNum() const { return mActiveNum; }

protected:
    s32 mType;                // at 0x4 set by RPDucTargetRegistry::AddManager
    s32 mPoolNum;             // at 0x8
    s32 mActiveNum;           // at 0xC
    RPDucTarget** mppPool;    // at 0x10
    RPDucTarget** mppPoolEnd; // at 0x14 last entry
    RPDucTarget** mppNext;    // at 0x18
};

/**
 * @brief Clay pigeon pool (singleton Duc_clayMgrInstance)
 * @details vtable 80359DA0. Size 0x1C, verified.
 */
class RPDucClayMgr : public RPDucTargetMgr {
public:
    virtual ~RPDucClayMgr();                // at 0x8
    virtual void Load();                    // at 0x10
    virtual RPDucTarget* AllocTarget();     // at 0x24
    virtual void DestroyInstance();         // at 0x2C

    /**
     * @brief Allocates the singleton and its pool; returns it for RPDucTargetRegistry::AddManager
     */
    static RPDucTargetMgr* CreateInstance(s32 num);
    /**
     * @brief sInstance->Spawn, the registry's spawn table entry (defined last in the file)
     */
    static RPDucTarget* SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg);

    static RPDucClayMgr* sInstance;
};

/**
 * @brief UFO pool (singleton Duc_ufoMgrInstance)
 * @details vtable 8035A1F8. Size 0x1C, verified.
 */
class RPDucUfoMgr : public RPDucTargetMgr {
public:
    virtual ~RPDucUfoMgr();                 // at 0x8
    virtual void Load();                    // at 0x10
    virtual RPDucTarget* AllocTarget();     // at 0x24
    virtual void DestroyInstance();         // at 0x2C

    /**
     * @brief Allocates the singleton and its pool; returns it for RPDucTargetRegistry::AddManager
     */
    static RPDucTargetMgr* CreateInstance(s32 num);
    /**
     * @brief sInstance->Spawn, the registry's spawn table entry (defined last in the file)
     */
    static RPDucTarget* SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg);

    static RPDucUfoMgr* sInstance;
};

/**
 * @brief Balloon pool (singleton Duc_balloonMgrInstance)
 * @details vtable 80359C60. Size 0x1C, verified.
 */
class RPDucBalloonMgr : public RPDucTargetMgr {
public:
    virtual ~RPDucBalloonMgr();             // at 0x8
    virtual void Load();                    // at 0x10
    virtual RPDucTarget* AllocTarget();     // at 0x24
    virtual void DestroyInstance();         // at 0x2C

    /**
     * @brief Allocates the singleton and its pool; returns it for RPDucTargetRegistry::AddManager
     */
    static RPDucTargetMgr* CreateInstance(s32 num);
    /**
     * @brief sInstance->Spawn, the registry's spawn table entry (defined last in the file)
     */
    static RPDucTarget* SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg);

    static RPDucBalloonMgr* sInstance;
};

/**
 * @brief Round target pool with the 32-slot placement grid (singleton
 * Duc_circleTargetMgrInstance)
 * @details vtable 8035ADD8. Size 0x3C, verified.
 */
class RPDucCircleTargetMgr : public RPDucTargetMgr {
public:
    virtual ~RPDucCircleTargetMgr();        // at 0x8
    virtual void Load();                    // at 0x10
    /**
     * @brief Also clears the grid slots [H]
     */
    virtual void Reset();                   // at 0x14
    virtual RPDucTarget* AllocTarget();     // at 0x24
    virtual void DestroyInstance();         // at 0x2C

    /**
     * @brief Allocates the singleton and its pool; returns it for RPDucTargetRegistry::AddManager
     */
    static RPDucTargetMgr* CreateInstance(s32 num);
    /**
     * @brief sInstance->Spawn, the registry's spawn table entry (defined last in the file)
     */
    static RPDucTarget* SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg);

    static RPDucCircleTargetMgr* sInstance;

    bool IsGridSlotUsed(s32 slot) const;
    void SetGridSlotUsed(s32 slot, bool used);
    void ClearGridSlots();

private:
    bool mGridSlotUsed[32]; // at 0x1C
};

/**
 * @brief Duck pool (singleton Duc_duckMgrInstance)
 * @details vtable 80359ED8. Size 0x1C, verified.
 */
class RPDucDuckMgr : public RPDucTargetMgr {
public:
    virtual ~RPDucDuckMgr();                // at 0x8
    virtual void Load();                    // at 0x10
    virtual RPDucTarget* AllocTarget();     // at 0x24
    virtual void DestroyInstance();         // at 0x2C

    /**
     * @brief Allocates the singleton and its pool; returns it for RPDucTargetRegistry::AddManager
     */
    static RPDucTargetMgr* CreateInstance(s32 num);
    /**
     * @brief sInstance->Spawn, the registry's spawn table entry (defined last in the file)
     */
    static RPDucTarget* SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg);

    static RPDucDuckMgr* sInstance;
};

/**
 * @brief Can pool (singleton Duc_canMgrInstance)
 * @details vtable 8035AB88. Size 0x1C, verified.
 */
class RPDucCanMgr : public RPDucTargetMgr {
public:
    virtual ~RPDucCanMgr();                 // at 0x8
    virtual void Load();                    // at 0x10
    virtual RPDucTarget* AllocTarget();     // at 0x24
    virtual void DestroyInstance();         // at 0x2C

    /**
     * @brief Allocates the singleton and its pool; returns it for RPDucTargetRegistry::AddManager
     */
    static RPDucTargetMgr* CreateInstance(s32 num);
    /**
     * @brief sInstance->Spawn, the registry's spawn table entry (defined last in the file)
     */
    static RPDucTarget* SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg);

    static RPDucCanMgr* sInstance;
};

/**
 * @brief Mii avatar pool (singleton Duc_avatarMgrInstance)
 * @details vtable 8035AF28. Size 0x1C, verified. Uses the base Load.
 */
class RPDucAvatarMgr : public RPDucTargetMgr {
public:
    virtual ~RPDucAvatarMgr();              // at 0x8
    virtual void Reset();                   // at 0x14
    virtual RPDucTarget* AllocTarget();     // at 0x24
    virtual void DestroyInstance();         // at 0x2C

    /**
     * @brief Allocates the singleton and its pool; returns it for RPDucTargetRegistry::AddManager
     */
    static RPDucTargetMgr* CreateInstance(s32 num);
    /**
     * @brief sInstance->Spawn, the registry's spawn table entry (defined last in the file)
     */
    static RPDucTarget* SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg);

    static RPDucAvatarMgr* sInstance;

    /**
     * @brief Random active avatar that is not being carried (unk158 clear), NULL if none
     */
    RPDucAvatar* PickFree();
    /**
     * @brief PickFree, else any active avatar at random
     */
    RPDucAvatar* PickAny();
    /**
     * @brief Calculates the position in the line-up of the two sides for pAvatar [M]
     */
    nw4r::math::VEC3 CalcLinePosition(const RPDucAvatar* pAvatar);
    /**
     * @brief Whether no other active avatar has a smaller x than pAvatar [M]
     */
    bool IsLowest(const RPDucAvatar* pAvatar);
    /**
     * @brief Whether every active avatar is in state 10 [M]
     */
    bool AreAllGone();

    static bool sFinalRoundFlag; // 8043C8D0, cleared by Reset
};

/**
 * @brief Holds the seven target managers and forwards the per-step calls to all of them
 * (singleton Duc_targetRegistryInstance)
 * @details vtable 8035A1D0. Size 0x24, verified.
 * @note Name is a project convention (ledger Duc_TargetRegistry).
 */
class RPDucTargetRegistry : public RPSceneObj {
    friend class RPDucShot;

public:
    virtual ~RPDucTargetRegistry(); // at 0x8
    virtual void Load();            // at 0x10
    virtual void Reset();           // at 0x14

    static void CreateInstance();
    static void DestroyInstance();

    RPDucTargetRegistry() {
        u32 i = 0;
        do {
            mpMgrs[i] = NULL;
        } while (++i < 7);
        mMgrNum = 0;
    }

    /**
     * @brief Creates a manager of the type through the table 8035A1B4 with a pool of num [H]
     */
    void AddManager(s32 type, s32 num);
    void DestroyManagers();
    void ClearHitRecords();
    void ApplyVelocityAll();
    void ClearForcesHookAll();
    void VF_0x80All();
    void ApplyForcesHookAll();
    void CalculateAll();
    void IncAgeAll();
    void CalcModelMtxAll();
    /**
     * @brief Spawns a target of the type through the table 8035A198
     */
    RPDucTarget* Spawn(s32 type, const nw4r::math::VEC3& rPos, u32 arg);
    void RemoveAll();
    s32 CountActive() const;
    /**
     * @brief Active targets excluding ducks (4) and avatars (6) [H]
     */
    s32 CountActiveExclDucksAvatars() const;

    static RPDucTargetRegistry* sInstance;

private:
    RPDucTargetMgr* mpMgrs[7]; // at 0x4
    u32 mMgrNum;               // at 0x20
};

//! @}

#endif
