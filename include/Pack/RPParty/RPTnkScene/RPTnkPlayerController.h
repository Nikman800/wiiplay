#ifndef RP_PARTY_TNK_PLAYER_CONTROLLER_H
#define RP_PARTY_TNK_PLAYER_CONTROLLER_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPTnkTank;
struct RPAxes;

/**
 * @brief Player input for a tank: d-pad movement, pointer aiming, B fires, A lays a mine
 * @details vtable 8035B650. Size 0x5C, verified (RPTnkTank::Create). Reset copies the player
 * tank parameters from the game parameter file (TnkGameParam.bin) [M].
 */
class RPTnkPlayerController : public RPSceneObj {
public:
    explicit RPTnkPlayerController(RPTnkTank* pTank);
    virtual ~RPTnkPlayerController(); // at 0x8

    virtual void Reset(); // at 0x14

    f32 GetMaxSpeed() const;
    BOOL GetMoveInput(nw4r::math::VEC2* pOut, s32 player);
    void CalcMoveVelocity(RPAxes* pRot, nw4r::math::VEC3* pVel, s32 player);
    void TurnBodyTowards(RPAxes* pRot, s32* pSign,
                         const nw4r::math::VEC3& rDir);
    void TurnBodyStep(RPAxes* pRot, const nw4r::math::VEC3& rDir);
    void AimAtCursor(RPAxes* pTurret, const nw4r::math::VEC3& rPos, s32 player);
    void HandleFireInput(f32 muzzleDist, const RPAxes* pTurret,
                         const nw4r::math::VEC3& rPos, s32 player);

private:
    f32 mParams[4];        // at 0x4 [L] movement parameters
    f32 mMaxSpeed;         // at 0x14
    f32 unk18;             // at 0x18 [L]
    f32 mTurnSpeed;        // at 0x1C [M]
    s32 mFireCooldown;     // at 0x20 frames between shots
    s32 mMaxShells;        // at 0x24 shells on the field at once
    s32 mShellBounces;     // at 0x28 [M] passed to FireShell
    f32 mShellSpeed;       // at 0x2C
    s32 mMineCooldownInit; // at 0x30 [M]
    s32 unk34;             // at 0x34 [L]
    s32 mMaxMines;         // at 0x38
    s32 unk3C;             // at 0x3C [L]
    f32 unk40;             // at 0x40 [L]
    s32 unk44;             // at 0x44 [L] reset to 1
    s32 mShellTimer;       // at 0x48
    s32 mMineTimer;        // at 0x4C
    s32 unk50;             // at 0x50 [L]
    bool mMineHeld;        // at 0x54 [M]
    bool mFired;           // at 0x55
    bool mLaidMine;        // at 0x56
    RPTnkTank* mpTank;     // at 0x58
};

//! @}

#endif
