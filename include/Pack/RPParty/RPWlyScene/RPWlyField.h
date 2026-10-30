#ifndef RP_PARTY_WLY_FIELD_H
#define RP_PARTY_WLY_FIELD_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

#include <nw4r/math.h>

//! @addtogroup rp_wly
//! @{

// Forward declarations
class RPGrpProjectionObject;
class RPSysEffect;
class RPWlyEarth;  // RPWlyField_2.h
class RPWlyGround; // RPWlyField_3.h

/**
 * @brief Find Mii background (one of wtd_field01-09 by field type and spotlight flag)
 * @details vtable 8035CB18. Size 0x7C, verified (Wly_GameMgr_ctor). Init creates the ground
 * (type 1) or earth and shooting star (type 2) and, on spotlight stages, one projection
 * object per player whose position follows the cursor (or the found avatar during
 * clear/result). Create also sets RPWlySoundActor::sFieldIndex (BGM / voice table index 0-9).
 * Create/Init/Reset/Destroy are called directly by RPWlyGameMgr.
 */
class RPWlyField : public RPSimpleModel {
public:
    virtual ~RPWlyField();    // at 0x8
    virtual void Calculate(); // at 0x14

    void Create();
    void Init();
    void Reset();
    void Destroy();

private:
    RPWlyGround* mpGround;                   // at 0x44 field type 1
    RPWlyEarth* mpEarth;                     // at 0x48 field type 2
    RPSysEffect* mpShootingStar;             // at 0x4C Wtd_shootingstar01, field type 2
    RPGrpProjectionObject* mpSpotlights[2];  // at 0x50 spotlight stages (0x1E8 bytes each)
    nw4r::math::VEC3 mSpotPos;               // at 0x58 level data 0x2C; spotlight screen scale
                                             // while the spotlights follow the cursors
    nw4r::math::VEC3 mSpotScale;             // at 0x64 [M] (2, 2, 1) etc. by field type
    f32 mSpotScaleRate;                      // at 0x70 grows by 0.5 per frame on result (<= 1000)
    f32 mPlaneY;                             // at 0x74 [M] level data 0x38, cursor plane normal
                                             // (0, 1, mPlaneY)
    f32 mPlaneZ;                             // at 0x78 [M] level data 0x3C, cursor plane point
                                             // (0, mPlaneZ, 0)
};

//! @}

#endif
