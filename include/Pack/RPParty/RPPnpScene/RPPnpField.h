#ifndef RP_PARTY_PNP_FIELD_H
#define RP_PARTY_PNP_FIELD_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

//! @addtogroup rp_pnp
//! @{

/**
 * @name Table Tennis court models
 * @details Allocated with 0x44 bytes (= sizeof(RPSimpleModel)) in RPPnpGame's constructor and
 * given their own vtable: they add no members. Unlike Laser Hockey's objects, Create/Load/Reset/
 * Exit are not virtual here (each vtable ends after RPSimpleModel's slots); RPPnpGame calls them
 * directly. Sizes 0x44, verified.
 */
//! @{

/**
 * @brief Room floor; detects the ball touching the floor
 * @details vtable 803595C0.
 */
class RPPnpField : public RPSimpleModel {
public:
    virtual ~RPPnpField();    // at 0x8
    virtual void Calculate(); // at 0x14

    void Create();
    void Load();
    void Reset();
    void Exit();

    /**
     * @brief Ball below the floor height: reports the bounce to RPPnpBall::OnEvent [M]; also
     * bounces it off the room walls and the barriers around the three tables
     */
    bool CheckBallFloorHit();

    static f32 sFloorBounce;   // 8043B6A8 (Pnp_floorHitParamA) 0.5
    static f32 sBarrierBounce; // 8043B6AC (Pnp_floorHitParamB) 0.5
    static f32 sWallBounce;    // 8043B6B0 (Pnp_floorHitParamC) 0.5
};

/**
 * @brief The table; bounces the ball off its top and the net
 * @details vtable 80359810.
 */
class RPPnpTable : public RPSimpleModel {
public:
    virtual ~RPPnpTable();    // at 0x8
    virtual void Calculate(); // at 0x14

    void Create();
    void Load();
    void Reset();
    void Exit();

    bool HandleBallCollision();
    f32 GetBallRestHeight() const;

    RPGrpModel* GetModel() const {
        return mpModel;
    }
    f32 GetHeight() const {
        return mScale.y * sHeightUnit;
    }
    f32 GetNetHeight() const {
        return mScale.y * sNetHeightUnit;
    }
    f32 GetScaleZ() const {
        return mScale.z;
    }
    f32 GetLength() const {
        return mScale.z * sLengthUnit;
    }
    f32 GetWidth() const {
        return mScale.x * sWidthUnit;
    }

    /**
     * @brief Table length unit (34), scaled by the model's z scale [L name]
     * @note The first object of the table unit's .sdata2 (8043F950; dtk put it in the Ball unit's)
     */
    static const f32 sLengthUnit;
    /**
     * @brief Table width unit (27), scaled by the model's x scale [L name]
     */
    static const f32 sWidthUnit;
    static const f32 sNetHeightUnit; // 2.08, net height above the table top [L name]
    static const f32 sHeightUnit;    // 9.12, table top height [L name]
    static f32 sRestitution;               // 8043B6C0 (Pnp_tableRestitution) 0.9
    static f32 sFriction;                  // 8043B6C4 (Pnp_tableFriction) 0.2
    static nw4r::math::VEC3 sBounceScale;  // 8043A700 (Pnp_tableBounceScale) bounce effect, hidden ball
};

/**
 * @brief Animated shadow of the net
 * @details vtable 80359A18.
 */
class RPPnpNetShadow : public RPSimpleModel {
public:
    virtual ~RPPnpNetShadow(); // at 0x8
    virtual void Calculate();  // at 0x14

    void Create();
    void Load();
    void Reset();
    void Exit();

    void RestartAnim(f32 frame);
};

//! @}

//! @}

#endif
