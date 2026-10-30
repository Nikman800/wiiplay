#ifndef RP_PARTY_BOM_OBJ_POOL_H
#define RP_PARTY_BOM_OBJ_POOL_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_bom
//! @{

// Forward declarations
class RPBomObjPool;
class RPBomSoundActorBase;

/**
 * @brief Spawn description passed to RPBomPoolObj::Spawn (built by RPBomQuestionParam and
 * the game manager's intro bubbles)
 * @details Size 0x20 (copied as eight words).
 */
struct RPBomSpawnInfo {
    f32 mAngle;                 // at 0x0 initial y rotation (degrees)
    nw4r::math::VEC3 mVel;      // at 0x4 (0, 0, speed)
    f32 mRotSpeed;              // at 0x10 degrees per frame
    f32 mScale;                 // at 0x14
    f32 mOrbitRadius;           // at 0x18 sideways sway amplitude
    f32 mOrbitStep;             // at 0x1C sway angle per frame (degrees)
};

/**
 * @brief Pooled model object with simple physics (base of RPBomPanel)
 * @details vtable 8035EFB8 (only referenced by the constructor, inlined into RPBomPanel's).
 * Size 0xA4 [H] (RPBomPanel's IRPGrpModelCallback part starts at 0xA4). mAge is -1 while the
 * object is free, -999 right after Deactivate and counts frames while active
 * (RPBomObjPool::IncrementAges). Each pass the owner pool calls Move, PreCalc (clears the
 * forces), CalcHit, PostCalc (integrates them), Calculate and CalcModelMtx on the active
 * objects.
 * @note Name is a project convention (ledger Bom_PoolObj).
 */
class RPBomPoolObj : public RPModelObj {
public:
    RPBomPoolObj();
    virtual ~RPBomPoolObj(); // at 0x8

    /**
     * @brief Creates the object's sound actor (RPBomSoundActorBase) [H]
     */
    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10 empty
    virtual void Reset();     // at 0x14 ResetState
    virtual void Calculate(); // at 0x18 empty

    /**
     * @brief Zero position and velocities, unit scale and rotation, mAge -1 [H]
     */
    virtual void ResetState(); // at 0x58
    virtual void ClearForces(); // at 0x5C
    /**
     * @brief pos += mPosDelta + mPosDelta2; vel += mVelDelta + mVelDelta2 [H]
     */
    virtual void Integrate();   // at 0x60
    virtual void InitFields();  // at 0x64 empty
    virtual void Setup(u32 flags); // at 0x68 empty
    /**
     * @brief pos += vel [H]
     */
    virtual void Move();        // at 0x6C
    virtual void PreCalc();     // at 0x70 ClearForces
    virtual void CalcHit();     // at 0x74 empty
    virtual void PostCalc();    // at 0x78 Integrate
    /**
     * @brief ResetState, set the position, InitFields, Setup(flags), mAge 0, SetUnk40(1) [H]
     */
    virtual void Activate(const nw4r::math::VEC3& rPos, u32 flags); // at 0x7C
    /**
     * @brief mAge -999, one less active object in the pool, SetUnk40(0) [H]
     */
    virtual void Deactivate(); // at 0x80
    virtual void VF_0x84();    // at 0x84 empty
    virtual void Spawn(const nw4r::math::VEC3& rPos, u32 flags,
                       RPBomSpawnInfo info); // at 0x88 empty
    virtual s32 GetColorType(); // at 0x8C 0
    virtual s32 GetPoseIndex(); // at 0x90 0
    virtual void VF_0x94();     // at 0x94 empty
    virtual bool IsFalling();   // at 0x98 false
    virtual void Miss();        // at 0x9C empty

public:
    nw4r::math::VEC3 mVel;        // at 0x48
    nw4r::math::VEC3 mPosDelta;   // at 0x54 cleared each pass
    nw4r::math::VEC3 mPosDelta2;  // at 0x60 cleared each pass
    nw4r::math::VEC3 mVelDelta;   // at 0x6C cleared each pass
    nw4r::math::VEC3 mVelDelta2;  // at 0x78 cleared each pass
    nw4r::math::VEC3 unk84;       // at 0x84 [L] zeroed only
    f32 unk90;                    // at 0x90 [L] 1.0 after ResetState
    s32 mAge;                     // at 0x94
    s32 mIndex;                   // at 0x98 slot in the pool
    RPBomObjPool* mpPool;         // at 0x9C
    RPBomSoundActorBase* mpSound; // at 0xA0
};

/**
 * @brief Abstract fixed-size pool of RPBomPoolObj (base of RPBomPanelPool)
 * @details vtable 8035F058 (CreateItem and Destroy are pure). Size 0x18 [H] (RPBomPanelPool
 * adds nothing). CreateItems allocates the pointer array and calls CreateItem and Create for
 * each slot; Spawn takes the next free object (round robin from mpNext) and activates it. The
 * *Items loops call the matching RPBomPoolObj virtual on every active object.
 * @note Name is a project convention (ledger Bom_ObjPool).
 */
class RPBomObjPool : public RPSceneObj {
public:
    RPBomObjPool();
    virtual ~RPBomObjPool(); // at 0x8

    virtual void Load();  // at 0x10 Load of every object
    virtual void Reset(); // at 0x14 Reset of every object, nothing active

    virtual RPBomPoolObj* CreateItem() = 0; // at 0x24
    virtual void CreateItems(s32 num);      // at 0x28
    virtual void Destroy() = 0;             // at 0x2C
    virtual void MoveItems();               // at 0x30
    virtual void PreCalcItems();            // at 0x34
    virtual void CalcHitItems();            // at 0x38
    virtual void PostCalcItems();           // at 0x3C
    virtual void CalcItems();               // at 0x40
    virtual void IncrementAges();           // at 0x44
    virtual void CalcModelMtxItems();       // at 0x48
    virtual RPBomPoolObj* Spawn(const nw4r::math::VEC3& rPos, u32 flags); // at 0x4C
    virtual void DeactivateAll();           // at 0x50

public:
    s32 mCapacity;             // at 0x4
    s32 mActiveNum;            // at 0x8
    RPBomPoolObj** mppBegin;   // at 0xC
    RPBomPoolObj** mppLast;    // at 0x10 last slot (inclusive)
    RPBomPoolObj** mppNext;    // at 0x14 next slot to try
};

//! @}

#endif
