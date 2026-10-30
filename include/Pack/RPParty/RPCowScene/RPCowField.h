#ifndef RP_PARTY_COW_FIELD_H
#define RP_PARTY_COW_FIELD_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPActor.h>

#include <egg/math.h>

//! @addtogroup rp_cow
//! @{

// Forward declarations
class RPCollisionShape;
class RPGrpModel;
class RPPathObject;

/**
 * @brief The course model (cow_field*.brres)
 * @details vtable 8035D080. Size 0x2C, verified (RPCowGame constructor).
 * @note Name is a project convention (ledger Cow_Field).
 */
class RPCowField : public RPActor {
public:
    RPCowField();
    virtual ~RPCowField(); // at 0x38

    /**
     * @brief Ledger init: collision shape and the field model [H]
     */
    virtual void LoadResource(); // at 0x44
    virtual void OnStart();      // at 0x48 ledger reset
    virtual void OnStop();       // at 0x4C ledger exit (empty)
    virtual void OnActivate();   // at 0x50 ledger calc
    virtual void Calculate();    // at 0x54 ledger entryDraw

private:
    // Named consts after the code (RPCowFieldObj.cpp) [L]
    static const f32 sModelScale; // 1

    RPGrpModel* mpModel;           // at 0x24
    RPCollisionShape* mpCollision; // at 0x28
};

/**
 * @brief Goal gate (resource type 0x70002): detects a player crossing its plane
 * @details vtable 8035D2B8. Size 0x30, verified (Cow_Game_load). Its collision shape is
 * linked into Cow_objectColliders with mask 0x20.
 * @note Name is a project convention (ledger Cow_Goal).
 */
class RPCowGoal : public RPActor {
public:
    RPCowGoal();
    virtual ~RPCowGoal(); // at 0x38

    virtual void LoadResource(); // at 0x44 ledger init
    virtual void OnStart();      // at 0x48 ledger reset
    virtual void OnStop();       // at 0x4C ledger exit
    /**
     * @brief Ledger calc: sets the shape's sizes (300, 90000) [H]
     */
    virtual void OnActivate(); // at 0x50
    virtual void Calculate();  // at 0x54 ledger entryDraw (empty)

    /**
     * @brief IRPActorEnable::IsEnabled override (ledger onCollide, thunk in the IRPActorEnable
     * part at 0x20). The collider list passes the other actor's IRPActorPriority part (the
     * player): returns true when the player's previous and current positions lie on opposite
     * sides of the goal plane [M].
     */
    virtual bool IsEnabled(IRPActorPriority* pOther); // at 0x60

    EGG::Vector3f GetPos() const;
    /**
     * @brief Draws the goal's box as a shadow volume (520 x 120 x 35 around the shape) [M]
     */
    void DrawDebugBox();

private:
    static const f32 sPathScale; // 1: RPPathObject scale
    static const f32 sRadius;    // 300
    static const f32 sBoxOfsY;   // 60
    static const f32 sBoxOfsZ;   // -53
    static const f32 sBoxScaleX; // 520
    static const f32 sBoxScaleY; // 120
    static const f32 sBoxScaleZ; // 35

    RPPathObject* mpPathObj;       // at 0x24
    u32 unk28;                     // at 0x28 [L] never accessed
    RPCollisionShape* mpCollision; // at 0x2C
};

//! @}

#endif
