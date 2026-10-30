#ifndef RP_PARTY_TNK_BLOCK_H
#define RP_PARTY_TNK_BLOCK_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPTnkScene/RPTnkActor.h>
#include <Pack/RPParty/RPTnkScene/RPTnkActorPool.h>
#include <Pack/RPParty/RPTnkScene/RPTnkField.h>

//! @addtogroup rp_tnk
//! @{

/**
 * @brief Wall block of a stage (one map cell)
 * @details vtable 8035B684. Size 0x180, verified (RPTnkBlockPool::CreateObject). Kinds:
 * 0 breakable (cork, destroyed by mine explosions), 1 solid, 2 hole (no collision) [M].
 * Spawn parameter: kind in bits 0-1, stack height - 1 in bits 4-6 (RPTnkStageMap::SpawnBlocks
 * passes (cell - 100) * 16 or (cell - 200) * 16 | 1). Models tnk_block_1/7/8 and
 * tnk_block_shadow from G3D/tnk_block.brres.
 */
class RPTnkBlock : public RPTnkActor {
public:
    typedef void (RPTnkBlock::*StateFunc)();

public:
    RPTnkBlock();
    virtual ~RPTnkBlock(); // at 0x8

    virtual void Create();       // at 0xC
    virtual void Load();         // at 0x10 loads the three block models and the shadow
    virtual void Calculate();    // at 0x18 state, shadow, sound position
    virtual void CalcModelMtx(); // at 0x24 draws the block stack and its shadow

    virtual void OnSpawn();                  // at 0x64 scale from the cell size
    virtual void ApplySpawnParam(u32 param); // at 0x68 kind, height, neighbours, edges
    virtual void ApplyVelocity();            // at 0x6C
    /**
     * @brief ClearForces, then looks up the four neighbour blocks [H]
     */
    virtual void ClearForcesStep(); // at 0x70
    /**
     * @brief Breakable blocks: flag a break when inside a mine explosion [H]
     */
    virtual void CollideWithField(); // at 0x74
    /**
     * @brief Pushes an actor out of the block's edges (RPTnkField test against mEdges) [M]
     */
    virtual BOOL CollideObject(RPTnkActor* pOther, s32 mode); // at 0x94
    virtual s32 GetKind() const;   // at 0x9C
    virtual s32 GetHeight() const; // at 0xA0

    void SetupEdges();
    void UpdateShadow();
    void SpawnBreakEffect();
    /**
     * @brief Only state: breaks the block (effect "Tnk_blockbreak01", Kill) when flagged [H]
     */
    void StateBreakIfFlagged();

private:
    static f32 GetActorHitRadius(const RPTnkActor* actor) {
        return actor->*(&RPTnkBlock::mHitRadius);
    }

    // POD coordinates reproduce the original whole-wall copies without changing layout.
    struct Wall {
        nw4r::math::_VEC3 mPos;
        nw4r::math::_VEC3 mNormal;
        f32 mLength;
        u32 unk1C;
    };

    s32 mKind;                   // at 0xB0
    s32 mState;                  // at 0xB4
    Wall mEdges[4];              // at 0xB8
    s32 mpNeighbors[4];          // at 0x138 -z, +x, +z, -x lookup results (0, 1, 2)
    bool mBreakRequested;        // at 0x148
    s32 mHeight;                 // at 0x14C stacked block count 1-8
    bool mShadowFlags[8];        // at 0x150
    RPGrpModel* mpModels[3];     // at 0x158 tnk_block_1, _7, _8 [M]
    RPGrpModel* mpShadowModel;   // at 0x164
    f32 mShadowAnm[3];           // at 0x168 [L] reset to 0, 0, 1
    StateFunc mStateFuncs[1];    // at 0x174
};

/**
 * @brief Pool of the 256 stage blocks (singleton Tnk_blockPoolInstance)
 * @details vtable 8035B748. Size 0x18, verified. Created and owned by RPTnkField.
 */
class RPTnkBlockPool : public RPTnkActorPool {
public:
    static void CreateInstance();
    static void DestroyInstance();

    virtual ~RPTnkBlockPool(); // at 0x8

    virtual RPTnkActor* CreateObject(); // at 0x24
    virtual void Destroy();             // at 0x2C

    /**
     * @brief 0 no block at the cell centre rPos, 1 kind-0 block, 2 other block [H]
     */
    s32 GetBlockAt(const nw4r::math::VEC3& rPos) const;
    /**
     * @brief Whether an active kind-0 block lies within half a cell plus radius of rPos (XZ) [H]
     */
    BOOL IsKind0BlockNear(f32 radius, const nw4r::math::VEC3& rPos);
    // Non-const: a const this schedules the mActiveNum load into the prologue.
    BOOL IsPointInsideBlock(const nw4r::math::VEC3& rPos);
    void KillAllBlocks();
    BOOL CollideObject(RPTnkActor* pActor, s32 mode);

    static RPTnkBlockPool* sInstance; // Tnk_blockPoolInstance
};

//! @}

#endif
