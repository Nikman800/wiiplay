#ifndef RP_PARTY_TNK_TASK_H
#define RP_PARTY_TNK_TASK_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

//! @addtogroup rp_tnk
//! @{

/**
 * @name Small Tanks! task objects
 */
//! @{

/**
 * @brief Fixed-rate step counter (singleton Tnk_stepTimerInstance)
 * @details vtable 8035C040. Size 0x14, verified (Tnk_StepTimer_createInstance). Each frame
 * mAccum += mRate; mSteps = whole part. The scene redraws the tread-mark capture texture
 * mSteps times; mClear requests a cleared capture.
 */
class RPTnkStepTimer : public RPSceneObj {
public:
    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkStepTimer(); // at 0x8

    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18

    void Start(f32 rate);
    s32 GetSteps() const { return mSteps; }
    bool IsClear() const { return mClear; }
    void SetClear(bool clear) { mClear = clear; }

    static RPTnkStepTimer* sInstance; // Tnk_stepTimerInstance

private:
    f32 mRate;   // at 0x4
    f32 mAccum;  // at 0x8 fractional part
    s32 mSteps;  // at 0xC
    bool mClear; // at 0x10 set by Start/Reset, cleared by the scene's draw
};

/**
 * @brief Task with every slot empty (singleton Tnk_emptyTaskInstance)
 * @details vtable 8035C068. Size 0x4, verified (Tnk_EmptyTask_createInstance). Owned by
 * RPTnkGameManager at 0x40.
 */
class RPTnkEmptyTask : public RPSceneObj {
public:
    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkEmptyTask(); // at 0x8

    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    virtual void Exit();      // at 0x1C

    static RPTnkEmptyTask* sInstance; // Tnk_emptyTaskInstance
};

/**
 * @brief Tread-mark textures, one per tank model (singleton Tnk_footmarkTexInstance)
 * @details vtable 8035C410. Size 0x30, verified (Tnk_FootmarkTex_createInstance).
 * tnk_footmark/tnk_mark_1p.bti, _2p, _e_1.._e_9; indexed by RPTnkTank's model index.
 */
class RPTnkFootmarkTex : public RPSceneObj {
public:
    static void CreateInstance();
    static void DestroyInstance();

    RPTnkFootmarkTex() {
        s32 i = 0;
        do {
            mpTextures[i] = NULL;
        } while (++i < 11);
    }
    virtual ~RPTnkFootmarkTex(); // at 0x8

    virtual void Create();   // at 0xC
    virtual void Load();     // at 0x10

    void* GetTexture(s32 idx) const { return mpTextures[idx]; }

    static RPTnkFootmarkTex* sInstance; // Tnk_footmarkTexInstance

private:
    void* mpTextures[11]; // at 0x4 ResTIMG data from the archive
};

//! @}

//! @}

#endif
