#ifndef RP_PARTY_COW_FIELD_OBJ_H
#define RP_PARTY_COW_FIELD_OBJ_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPCowScene/RPCowSound.h>
#include <Pack/RPParty/RPPartyCommon/RPCollisionShape.h>
#include <Pack/RPParty/RPPartyCommon/RPActor.h>
#include <Pack/RPParty/RPPartyCommon/RPColliderList.h>

#include <egg/math.h>

//! @addtogroup rp_cow
//! @{

// Forward declarations
class RPCollisionShape;
class RPCowKakasiGroup;
class RPCowModelBatch;
class RPGrpModel;
class RPGrpModelMaterial;
class RPPathObject;

/**
 * @brief Colliders of the course objects (mask 0x20 etc.), tested by the cows
 */
extern RPColliderList Cow_objectColliders; // defined in RPCowGame.cpp

/**
 * @brief Unit Y vector (0, 1, 0) in a Wii Play framework unit's .bss (8043A304, placeholder name)
 */
extern const EGG::Vector3f RP_eggVecEy;

/**
 * @brief Abstract base of the course objects placed from the object table (trees, hurdles,
 * scarecrows, scarecrow kings, clouds, knit cows)
 * @details No vtable of its own: the constructor is inlined into every derived constructor
 * and the derived vtables (8035D320, 8035D388, 8035D3F0, 8035D6C0, 8035DB08, 8035DB70) share
 * VF_0x3C/VF_0x40/OnActivate/Calculate and override IsEnabled. Size 0x150 [H] (Tree, Hurdle,
 * Cloud and KnitCowEx add nothing and are allocated with 0x150).
 * Objects are knocked away by the cow through a state machine: mpState points at one of the
 * twelve member state objects (drop, rise, sink, idle, hop, king jump 3/2/1, fall spin, knock
 * slide, knock fly, knock hurdle). Each object is drawn either
 * with its own model or through an RPCowModelBatch per level of detail.
 * @note Name is a project convention (ledger Cow_FieldObj).
 */
class RPCowFieldObj : public RPActor {
public:
    /**
     * @brief Constructs the twelve member states (their vtable stores are all it does) [H]
     */
    RPCowFieldObj() {}

    /**
     * @brief Ledger create: allocates mpSound [H]
     */
    virtual void VF_0x3C(); // at 0x3C
    /**
     * @brief Ledger destroy: deletes mpSound [H]
     */
    virtual void VF_0x40(); // at 0x40
    /**
     * @brief Ledger calc: runs the state, chooses the level of detail per screen and adds
     * the matrix to the model batches [M]
     */
    virtual void OnActivate(); // at 0x50
    /**
     * @brief Ledger entryDraw [M]
     */
    virtual void Calculate(); // at 0x54

    /**
     * @brief IRPActorEnable::IsEnabled override (ledger onCollide); every derived object
     * reacts to the cow (the other actor, passed by the collider list) here [M]
     */
    virtual bool IsEnabled(IRPActorPriority* pOther) = 0; // at 0x60

    /**
     * @brief Hides the one-player model on the screen being drawn (Lyt_getCurrentViewNo) when
     * mDrawMode of that screen is 4 [H]
     */
    void SetVisibleForScreen();
    /**
     * @brief Rebuilds the collision shape and the matrices from mpPathObj [H]
     */
    void ResetTransform();
    void SetSoundMode();
    void KnockHurdle();
    void KnockFly();
    void KnockSlide();
    void StartSink();
    void SpawnAppear();
    void SpawnDrop();
    // Ledger Cow_FieldObj_spawnAppear / Cow_FieldObj_spawnDrop (RPCowKakasiGroup::SpawnKakasi) [M]
    void SpawnAppear(const EGG::Vector3f& rPos, const EGG::Vector3f& rRot);
    void SpawnDrop(const EGG::Vector3f& rPos, s32 drop);
    EGG::Vector3f GetForwardDir();

    /**
     * @brief A state of the machine: a 4-byte member object (vtable pointer only) whose only
     * virtual runs the state on the owner (called as mpState->Calc(this)) [H]
     * @details The vtables of the twelve state classes are .data 8035D0E0..8035D170
     * ({0, 0, Calc} each, emitted in reverse declaration order); the inlined constructor
     * stores them into the members at 0x120.
     */
    class State {
    public:
        virtual void Calc(RPCowFieldObj* pOwner) = 0; // at 0x8
    };
#define RP_COW_FIELD_OBJ_STATE(name)                                                         \
    class name : public State {                                                              \
    public:                                                                                  \
        virtual void Calc(RPCowFieldObj* pOwner);                                            \
    };
    RP_COW_FIELD_OBJ_STATE(StateDrop)
    RP_COW_FIELD_OBJ_STATE(StateRise)
    RP_COW_FIELD_OBJ_STATE(StateSink)
    RP_COW_FIELD_OBJ_STATE(StateIdle)
    RP_COW_FIELD_OBJ_STATE(StateHop)
    RP_COW_FIELD_OBJ_STATE(StateKingJump3)
    RP_COW_FIELD_OBJ_STATE(StateKingJump2)
    RP_COW_FIELD_OBJ_STATE(StateKingJump1)
    RP_COW_FIELD_OBJ_STATE(StateFallSpin)
    RP_COW_FIELD_OBJ_STATE(StateKnockSlide)
    RP_COW_FIELD_OBJ_STATE(StateKnockFly)
    RP_COW_FIELD_OBJ_STATE(StateKnockHurdle)
#undef RP_COW_FIELD_OBJ_STATE

private:
    // Named consts after the code (loaded, not folded; RPCowFieldObj.cpp) [L]
    static const f32 sModelScale;    // 1: RPActor::CalcModel
    static const f32 sProbeRadius;   // 1: ground probe of the hopping scarecrow
    static const f32 sProbeDist;     // 5: probe distance ahead
    static const f32 sStopY;         // -20: spawn depth, stops the actor below it
    static const f32 sSinkSpeed;     // 1: downward speed when sinking / dropping
    static const f32 sKingJumpSpeed; // 2: upward speed of the king's last jump
    static const f32 sLodNear1P;     // 500
    static const f32 sLodMid1P;      // 1000
    static const f32 sLodFar1P;      // 5000
    static const f32 sLodNear2P;     // 500
    static const f32 sLodMid2P;      // 1000
    static const f32 sLodFar2P;      // 5000
    static const f32 sGroundY;       // 0 (.sbss2)
    static const f32 sFadeInY;       // 0 (.sbss2): shadow fades in above it
    static const f32 sFadeOutY;      // 0 (.sbss2): shadow fades out below it

protected:
    friend class RPCowKakasiGroup;

    RPPathObject* mpPathObj;       // at 0x24 placement from the object table
    RPGrpModel* mpModel;           // at 0x28 one-player only
    RPGrpModel* mpShadowModel;     // at 0x2C cow_maru_shadow / cow_hurdle_shadow
    RPGrpModelMaterial* mpShadowMaterial; // at 0x30 [M] cow_maru_kage material of mpShadowModel
    /**
     * @brief Model batches of the current level of detail (OnActivate) [M]
     */
    RPCowModelBatch* mpBatch;       // at 0x34
    RPCowModelBatch* mpShadowBatch; // at 0x38
    /**
     * @brief Model or model batch to draw with, per level of detail (two sets of three) [M]
     */
    void* mpLodModels[6];          // at 0x3C
    void* mpShadowLodModels[6];    // at 0x54 [M]
    RPCollisionShape* mpCollision; // at 0x6C
    RPCowObjSound* mpSound;        // at 0x70
    EGG::Matrix34f mMtx;           // at 0x74
    EGG::Matrix34f mBaseMtx;       // at 0xA4 placement matrix
    EGG::Vector3f unkD4;           // at 0xD4 [L] set by the derived OnStart
    EGG::Vector3f unkE0;           // at 0xE0 [L] knock velocity
    EGG::Vector3f unkEC;           // at 0xEC [L] y: knock spin speed (set by IsEnabled)
    f32 unkF8;                     // at 0xF8 [L]
    f32 unkFC;                     // at 0xFC [L]
    s32 mScore;                    // at 0x100 [M] points given to the cow on a hit
    s32 unk104;                    // at 0x104 [L] frame counter of the jump states
    s32 mDrawMode[2];              // at 0x108 [M] per screen: 2 in view, 4 hidden (OnActivate)
    s32 unk110;                    // at 0x110 [L] 1 on reset
    s32 unk114;                    // at 0x114 [L]
    State* mpState;                // at 0x118 one of the states below (NULL when idle)
    State* mpNextState;            // at 0x11C [M] state after the drop / rise (NULL: idle)
    StateDrop mStateDrop;          // at 0x120
    StateRise mStateRise;          // at 0x124
    StateSink mStateSink;          // at 0x128
    StateIdle mStateIdle;          // at 0x12C
    StateHop mStateHop;            // at 0x130
    StateKingJump3 mStateKingJump3; // at 0x134
    StateKingJump2 mStateKingJump2; // at 0x138
    StateKingJump1 mStateKingJump1; // at 0x13C
    StateFallSpin mStateFallSpin;  // at 0x140
    StateKnockSlide mStateKnockSlide; // at 0x144
    StateKnockFly mStateKnockFly;  // at 0x148
    StateKnockHurdle mStateKnockHurdle; // at 0x14C
};

/**
 * @brief Tree (cow_tree.brres, resource type 0x70004)
 * @details vtable 8035D388. Size 0x150, verified (Cow_Game_load).
 * @note Name is a project convention (ledger Cow_Tree).
 */
class RPCowTree : public RPCowFieldObj {
public:
    RPCowTree();
    virtual ~RPCowTree(); // at 0x38

    virtual void LoadResource(); // at 0x44 ledger init
    virtual void OnStart();      // at 0x48 ledger reset
    virtual void OnStop();       // at 0x4C ledger exit
    virtual bool IsEnabled(IRPActorPriority* pOther); // at 0x60 ledger onCollide

private:
    // Knock parameters, named consts after the code (loaded, not folded) [L]
    static const f32 sRadius;
    static const f32 sOfsX;
    static const f32 sOfsY;
    static const f32 sOfsZ;
    static const f32 sSlideSide;
    static const f32 sSlideHeight;
    static const f32 sSlideSpeedMin;
    static const f32 sSlideSpeedMax;
    static const f32 sFlyHeightMin;
    static const f32 sFlySpeedMin;
    static const f32 sFlySpeedMax;
};

/**
 * @brief Hurdle (cow_hurdle.brres)
 * @details vtable 8035D3F0. Size 0x150, verified (Cow_Game_load).
 * @note Name is a project convention (ledger Cow_Hurdle).
 */
class RPCowHurdle : public RPCowFieldObj {
public:
    RPCowHurdle();
    virtual ~RPCowHurdle(); // at 0x38

    virtual void LoadResource();    // at 0x44 ledger init
    virtual void OnStart();         // at 0x48 ledger reset
    virtual void OnStop();          // at 0x4C ledger exit
    virtual bool IsEnabled(IRPActorPriority* pOther); // at 0x60 ledger onCollide

private:
    // Knock parameters, named consts after the code (loaded, not folded) [L]
    static const f32 sRadius;
    static const f32 sOfsX;
    static const f32 sOfsY;
    static const f32 sOfsZ;
    static const f32 sSlideSide;
    static const f32 sSlideHeight;
    static const f32 sSlideSpeedMin;
    static const f32 sSlideSpeedMax;
    static const f32 sFlyHeightMin;
    static const f32 sFlyHeightMax;
    static const f32 sFlySpeedMin;
    static const f32 sFlySpeedMax;
    static const f32 sHurdleSpeed;
    static const f32 sHurdleHeight;
};

/**
 * @brief Scarecrow (cow_kakasi.brres), spawned in groups by RPCowKakasiGroup
 * @details vtable 8035D320. Size 0x154, verified (array stride in the RPCowGame constructor,
 * 32 scarecrows).
 * @note Name is a project convention (ledger Cow_Kakasi).
 */
class RPCowKakasi : public RPCowFieldObj {
public:
    RPCowKakasi();
    virtual ~RPCowKakasi(); // at 0x38

    virtual void LoadResource();    // at 0x44 ledger init
    virtual void OnStart();         // at 0x48 ledger reset
    virtual void OnStop();          // at 0x4C ledger exit
    virtual bool IsEnabled(IRPActorPriority* pOther); // at 0x60 ledger onCollide

private:
    // Knock parameters, named consts after the code (loaded, not folded) [L]
    static const f32 sRadius;
    static const f32 sOfsX;
    static const f32 sOfsY;
    static const f32 sSlideSide;
    static const f32 sSlideHeight;
    static const f32 sSlideSpeedMin;
    static const f32 sSlideSpeedMax;
    static const f32 sFlySide;
    static const f32 sFlyHeightMin;
    static const f32 sFlyHeightMax;
    static const f32 sFlySpeedMin;
    static const f32 sFlySpeedMax;

    friend class RPCowKakasiGroup;

    RPCowKakasiGroup* mGroup; // at 0x150 owning group (NULL when placed alone)
};

/**
 * @brief Scarecrow king (cow_kakasi_king), jumps when hit
 * @details vtable 8035D6C0. Size 0x178, verified (array stride in the RPCowGame constructor,
 * 8 kings).
 * @note Name is a project convention (ledger Cow_KakasiKing).
 */
class RPCowKakasiKing : public RPCowFieldObj {
public:
    RPCowKakasiKing();
    virtual ~RPCowKakasiKing(); // at 0x38

    virtual void LoadResource();    // at 0x44 ledger init
    virtual void OnStart();         // at 0x48 ledger reset
    virtual void OnStop();          // at 0x4C ledger exit
    virtual bool IsEnabled(IRPActorPriority* pOther); // at 0x60 ledger onCollide

private:
    // Knock parameters, named consts after the code (loaded, not folded) [L]
    static const f32 sRadius;
    static const f32 sOfsX;
    static const f32 sOfsY;
    static const f32 sSlideSide;
    static const f32 sSlideHeight;
    static const f32 sSlideSpeedMin;
    static const f32 sSlideSpeedMax;
    static const f32 sFlySide;
    static const f32 sFlyHeightMin;
    static const f32 sFlyHeightMax;
    static const f32 sFlySpeedMin;
    static const f32 sFlySpeedMax;
    static const f32 sPopupOfsY;
    static const f32 sPopupScale;

    friend class RPCowKakasiGroup;

    RPCowKakasiGroup* mGroup; // at 0x150 owning group (NULL when placed alone)
    u32 mHitPlayer;           // at 0x154 [M] player that hit the king (GetUnk1A)
    s32 unk158;               // at 0x158 [M] points given on a hit
    s32 unk15C;               // at 0x15C [L] 30 on a hit
    EGG::Vector3f unk160;     // at 0x160 [M] screen position of the hit (popup)
    f32 unk16C;               // at 0x16C [L]
    f32 unk170;               // at 0x170 [L]
    f32 unk174;               // at 0x174 [L]
};

/**
 * @brief Cloud (cow_cloud.brres, cloudA/cloudB)
 * @details vtable 8035DB08. Size 0x150, verified (Cow_Game_load).
 * @note Name is a project convention (ledger Cow_Cloud).
 */
class RPCowCloud : public RPCowFieldObj {
public:
    RPCowCloud();
    virtual ~RPCowCloud(); // at 0x38

    virtual void LoadResource(); // at 0x44 ledger init
    virtual void OnStart();      // at 0x48 ledger reset
    virtual void OnStop();       // at 0x4C ledger exit
    virtual bool IsEnabled(IRPActorPriority* pOther); // at 0x60 ledger onCollide

private:
    // Knock parameters, named consts after the code (loaded, not folded) [L]
    static const f32 sRadius;
    static const f32 sOfsX;
    static const f32 sOfsY;
    static const f32 sOfsZ;
    static const f32 sSlideSide;
    static const f32 sSlideHeight;
    static const f32 sSlideSpeedMin;
    static const f32 sSlideSpeedMax;
    static const f32 sFlyHeightMin;
    static const f32 sFlySpeedMin;
    static const f32 sFlySpeedMax;
};

/**
 * @brief Knitted cow extra (cow_knit.brres exA/exB, waits and eats)
 * @details vtable 8035DB70. Size 0x150, verified (Cow_Game_load).
 * @note Name is a project convention (ledger Cow_KnitCowEx).
 */
class RPCowKnitCowEx : public RPCowFieldObj {
public:
    RPCowKnitCowEx();
    virtual ~RPCowKnitCowEx(); // at 0x38

    virtual void LoadResource(); // at 0x44 ledger init
    virtual void OnStart();      // at 0x48 ledger reset
    virtual void OnStop();       // at 0x4C ledger exit
    virtual bool IsEnabled(IRPActorPriority* pOther); // at 0x60 ledger onCollide

private:
    // Knock parameters, named consts after the code (loaded, not folded) [L]
    static const f32 sRadius;
    static const f32 sOfsX;
    static const f32 sOfsY;
    static const f32 sOfsZ;
    static const f32 sSlideSide;
    static const f32 sSlideHeight;
    static const f32 sSlideSpeedMin;
    static const f32 sSlideSpeedMax;
    static const f32 sFlyHeightMin;
    static const f32 sFlySpeedMin;
    static const f32 sFlySpeedMax;
};

//! @}

#endif
