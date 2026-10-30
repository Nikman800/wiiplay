#ifndef RP_PARTY_DUC_SPAWNER_H
#define RP_PARTY_DUC_SPAWNER_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>
#include <nw4r/math.h>

class RPDucTarget;

//! @addtogroup rp_duc
//! @{

/**
 * @brief Runs the spawn pattern of the current round (singleton Duc_spawnerInstance)
 * @details vtable 8035A6D4. Size 0x11C, verified. Twenty pattern functions (setup0-9, then
 * balloons B/A, type 3 (round targets), clay, clay pairs, UFOs B/A, ducks B/A, cans), called
 * every frame while mActive until mRemaining reaches 0. A delayed special-duck wave
 * (mDelayedSpawn) can be scheduled by setup2 or TriggerBonusWave.
 * @note Name is a project convention (ledger Duc_Spawner).
 */
class RPDucSpawner : public RPSceneObj {
public:
    typedef void (RPDucSpawner::*PatternFunc)();

public:
    RPDucSpawner();
    virtual ~RPDucSpawner(); // at 0x8

    virtual void Create();    // at 0xC
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18

    static void CreateInstance();
    static void DestroyInstance();

    bool IsActive() const { return mActive; }

    static RPDucSpawner* sInstance;

    void StartPattern(s32 pattern, s32 count);
    void AddCount(s32 count);
    void TriggerBonusWave();
    void UpdateDelayedSpawn();
    void SpawnSpecialDucks(s32 num);

private:
    inline RPDucTarget* SpawnTarget(s32 type, const nw4r::math::VEC3& pos, u32 arg);
    void Setup0();
    void Setup1();
    void Setup2();
    void Setup3();
    void Setup4();
    void Setup5();
    void Setup6();
    void Setup7();
    void Setup8();
    void Setup9();
    void PatternBalloonsB();
    void PatternBalloonsA();
    void PatternCircleTargets();
    void PatternClay();
    void PatternClayPairs();
    void PatternUfosB();
    void PatternUfosA();
    void PatternDucksB();
    void PatternDucksA();
    void PatternCans();

    bool mActive;                   // at 0x4
    s32 mPattern;                   // at 0x8
    s32 mCount;                     // at 0xC
    s32 mRemaining;                 // at 0x10
    s32 mTimer;                     // at 0x14 [M] frames to the next spawn
    s32 mStep;                      // at 0x18 [M]
    s32 unk1C;                      // at 0x1C [L] 600-frame timer
    bool unk20;                     // at 0x20 [L]
    bool mDelayedSpawn;             // at 0x21
    bool mBonusWaveDone;            // at 0x22
    s32 mDelayedSpawnNum;           // at 0x24
    s32 mDelayedSpawnTimer;         // at 0x28
    PatternFunc mPatternFuncs[20];  // at 0x2C
};

//! @}

#endif
