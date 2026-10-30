#ifndef RP_PARTY_TNK_TANK_H
#define RP_PARTY_TNK_TANK_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpModelCallback.h>
#include <Pack/RPParty/RPTnkScene/RPTnkActor.h>
#include <Pack/RPParty/RPTnkScene/RPTnkSound.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPSysEffect;
class RPTnkAiController;
class RPTnkPlayerController;

/**
 * @brief A tank (player or enemy)
 * @details vtable 8035B490 (IRPGrpModelCallback part at 0xB0, vtable pointer 8035B554). Size
 * 0x20C, verified (RPTnkTankManager::CreateTanks). States: 0 idle, 1 player control, 2 enemy
 * control, 3 player stop, 4 enemy stop. The body model follows the actor's rotation, the
 * turret (mTurretRot) is aimed separately; EditWorldMtx builds both from the joint matrices.
 * Models tnk_tank_1p, _2p, _e_1.._e_9 (G3D/tnk_tank.brres), chosen by mModelIdx.
 */
class RPTnkTank : public RPTnkActor, public IRPGrpModelCallback {
public:
    typedef void (RPTnkTank::*StateFunc)();

    /**
     * @brief Nearest hit of one kind this step (written by RPTnkTankManager::Record*Hit) [M]
     */
    struct HitInfo {
        s32 mId;    // at 0x0 [L]
        f32 mDist;  // at 0x4 1000 when nothing was hit
    };

public:
    RPTnkTank();
    virtual ~RPTnkTank(); // at 0x8

    /**
     * @brief Creates the controllers, the stealth effect and the sound actor [H]
     */
    virtual void Create();       // at 0xC
    virtual void Load();         // at 0x10 loads the 11 tank models
    virtual void Reset();        // at 0x14
    virtual void Calculate();    // at 0x18 state, sound positions
    virtual void CalcModelMtx(); // at 0x24 draws the model of mModelIdx

    virtual void OnSpawn();                  // at 0x64 hit circles, scale, turret
    virtual void ApplySpawnParam(u32 param); // at 0x68 stores bits 0-3
    /**
     * @brief Remembers the previous position, then pos += velocity [H]
     */
    virtual void ApplyVelocity();    // at 0x6C
    virtual void ClearForcesStep();  // at 0x70 also clears mShellHit / mMineHit
    /**
     * @brief Separates from other tanks, then RPTnkField::CollideActor (radius 16) [H]
     */
    virtual void CollideWithField(); // at 0x74
    /**
     * @brief IntegrateStep, then adds the tread travel to mTreadDistance [H]
     */
    virtual void IntegrateStep();    // at 0x78
    virtual void Kill();             // at 0x80 also stops the tread sound
    virtual BOOL TestHit(RPTnkActor* pOther, f32* pDist);    // at 0x84 mines
    virtual BOOL TestHitAlt(RPTnkActor* pOther, f32* pDist); // at 0x88 shells
    virtual void OnHitWall(f32 depth, const nw4r::math::VEC3* pNormal, s32 wall,
                           const nw4r::math::VEC3& rVel); // at 0x8C

    /**
     * @brief IRPGrpModelCallback::EditWorldMtx override: body/turret matrices, copies the
     * body and "cannon" joint matrices
     * @details Appends a primary slot (at 0xE0) and a thunk in the callback part (slot 52).
     */
    virtual void EditWorldMtx(RPGrpModel* pModel); // at 0xE0
    /**
     * @brief Push-out from another tank [M]
     */
    virtual void AccumulatePushOut(f32 depth, const nw4r::math::VEC3* pNormal,
                                   const nw4r::math::VEC3& rVel); // at 0xE4

    void Activate();
    void SpawnPlayer(const nw4r::math::VEC3& rPos, s32 param, BOOL show);
    void SpawnEnemy(const nw4r::math::VEC3& rPos, s32 type, s32 idx, BOOL show);
    void Explode(s32 hitId);
    void PushApartFrom(RPTnkTank* pOther);
    void InitOrientation();
    void PlayLayMineSe();
    void PlayFireSeByType();
    void HoldMineFuseSe();
    void PlayShotEffect();
    void SetSePlayerId();
    void SetSeVariant();
    void SpawnBombPieceEffect();
    void DrawTreadMark();
    void KeepNearestMineHit(const HitInfo& rHit);
    void KeepNearestShellHit(const HitInfo& rHit);

    void StateIdle();
    void StatePlayerControl();
    void StateEnemyControl();
    void StatePlayerStop();
    void StateEnemyStop();

    f32 GetBodyHeight() const { return mBodyHeight; }
    RPTnkAiController* GetAiController() const { return mpAiCtrl; }
    void SetIsEnemy(bool enemy) { mIsEnemy = enemy; }
    s32 GetEnemyType() const { return mEnemyType; }
    s32 GetAlive() const { return mAlive; }
    const nw4r::math::VEC3& GetScale() const { return mScale; }

private:
    void LoadTankModels(RPGrpModelResManager* manager, RPGrpHandle file, const char** names);
    void SetExGuideTarget();

    f32 mBodyHeight;                       // at 0xB4 model Y offset (17.6)
    RPAxes mTurretRot;                     // at 0xB8 turret basis (z: aim direction)
public:
    f32 mSpeed;                            // at 0xDC |velocity|
private:
    f32 mMaxSpeed;                         // at 0xE0
public:
    s32 mAlive;                            // at 0xE4 [M] 1 after spawn, 0 when destroyed
private:
    s32 mModelIdx;                         // at 0xE8 also the tread-mark texture
    s32 mState;                            // at 0xEC
    HitInfo mShellHit;                     // at 0xF0
    HitInfo mMineHit;                      // at 0xF8
    bool mIsEnemy;                         // at 0x100
    bool mStealth;                         // at 0x101 invisible enemy (effect on spawn)
    s32 mEnemyType;                        // at 0x104
    u32 mSpawnParam;                       // at 0x108
    nw4r::math::MTX34 mBodyMtx;            // at 0x10C
    nw4r::math::MTX34 mCannonMtx;          // at 0x13C muzzle position for shots
    nw4r::math::VEC3 mTreadPos[2];         // at 0x16C left/right tread sample points
    nw4r::math::VEC3 mPrevPos;             // at 0x184
    f32 mTreadDistance;                    // at 0x190 drives the tread marks
    RPTnkPlayerController* mpPlayerCtrl;   // at 0x194
    RPTnkAiController* mpAiCtrl;           // at 0x198
    RPGrpModel* mpModels[11];              // at 0x19C
    RPSysEffect* mpStealthEffect;          // at 0x1C8 "Tnk_stealth_change"
    RPTnkTankSound* mpTankSound;           // at 0x1CC
    StateFunc mStateFuncs[5];              // at 0x1D0

public:
    static const HitInfo sNoHit;
};

//! @}

#endif
