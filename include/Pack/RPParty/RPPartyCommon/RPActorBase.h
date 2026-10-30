#ifndef RP_PARTY_COMMON_ACTOR_BASE_H
#define RP_PARTY_COMMON_ACTOR_BASE_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpDrawObject.h>
#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPKernel/RP_DEBUG_STUB_1.h>

#include <egg/math.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPGrpModel;

/**
 * @brief Drawable object with a position, two more vectors, an orthonormal basis, a uniform
 * scale and one model
 * @details vtable 80357268. Size 0x60 [H]: the constructor initialises every member up to
 * 0x5C; RPPhysSphere (0x108), Bil_NextBallMarker (0x7C) and Bil_Table (0xB8) derive from it.
 * The basis vectors are the model matrix columns: column 0 = mAxisX, 1 = mAxisY, 2 = mAxisZ.
 * vtable layout (CodeWarrior): RP_DEBUG_STUB_1 part (dtor) at 0x8, IRPSysHostIOSocket part at
 * 0xC (thunk -4), IRPGrpDrawObject part at 0x1C (thunk -8), new virtuals from 0x34.
 * @note Wii Play-only framework class; the name is a project convention. The ledger's
 * RP_ModelActor2 (applyTransform, slot 0x74) is this class's CalcModelMtx.
 */
class RPActorBase : public RP_DEBUG_STUB_1,
                    public IRPSysHostIOSocket,
                    public IRPGrpDrawObject {
public:
    RPActorBase();
    // Wii Play: inline (the Fishing actors' destructors inline it; RP_ActorBase_dtor 801B8F40
    // is the weak copy)
#ifndef RP_ACTOR_BASE_DEFINE_VF
    virtual ~RPActorBase() {} // at 0x8
#endif

#ifdef RP_ACTOR_BASE_DEFINE_VF
    // RPActorBase.cpp emits the vtable: these are inline there (weak copies after the
    // destructor, in declaration order). Other units keep the plain declarations so that their
    // objects stay as they are.
    virtual const nw4r::math::VEC3& GetPos() const { return mPos; }          // at 0x34
    virtual void SetPos(const nw4r::math::VEC3& rPos) { mPos = rPos; }      // at 0x38
    virtual const nw4r::math::VEC3& GetVec1C() const { return mVec1C; }     // at 0x3C
    virtual void SetVec1C(const nw4r::math::VEC3& rVec) { mVec1C = rVec; }  // at 0x40
    virtual const nw4r::math::VEC3& GetVec28() const { return mVec28; }     // at 0x44
    virtual void SetVec28(const nw4r::math::VEC3& rVec) { mVec28 = rVec; }  // at 0x48
    virtual const nw4r::math::VEC3& GetAxisZ() const { return mAxisZ; }     // at 0x4C
    virtual void SetAxisZ(const nw4r::math::VEC3& rVec) { static_cast<nw4r::math::VEC3&>(mAxisZ) = rVec; }  // at 0x50
    virtual const nw4r::math::VEC3& GetAxisY() const { return mAxisY; }     // at 0x54
    virtual void SetAxisY(const nw4r::math::VEC3& rVec) { static_cast<nw4r::math::VEC3&>(mAxisY) = rVec; }  // at 0x58
    virtual const nw4r::math::VEC3& GetAxisX() const { return mAxisX; }     // at 0x5C
    virtual void SetAxisX(const nw4r::math::VEC3& rVec) { static_cast<nw4r::math::VEC3&>(mAxisX) = rVec; }  // at 0x60
    virtual f32 GetScale() const { return mScale; }                         // at 0x64
    virtual void SetScale(f32 scale) { mScale = scale; }                    // at 0x68
    virtual RPGrpModel* GetModel() const { return mpModel; }                // at 0x6C
    virtual void SetModel(RPGrpModel* pModel) { mpModel = pModel; }         // at 0x70
#else
    virtual const nw4r::math::VEC3& GetPos() const;      // at 0x34
    virtual void SetPos(const nw4r::math::VEC3& rPos);   // at 0x38
    virtual const nw4r::math::VEC3& GetVec1C() const;    // at 0x3C
    virtual void SetVec1C(const nw4r::math::VEC3& rVec); // at 0x40
    virtual const nw4r::math::VEC3& GetVec28() const;    // at 0x44
    virtual void SetVec28(const nw4r::math::VEC3& rVec); // at 0x48
    virtual const nw4r::math::VEC3& GetAxisZ() const;    // at 0x4C
    virtual void SetAxisZ(const nw4r::math::VEC3& rVec); // at 0x50
    virtual const nw4r::math::VEC3& GetAxisY() const;    // at 0x54
    virtual void SetAxisY(const nw4r::math::VEC3& rVec); // at 0x58
    virtual const nw4r::math::VEC3& GetAxisX() const;    // at 0x5C
    virtual void SetAxisX(const nw4r::math::VEC3& rVec); // at 0x60
    virtual f32 GetScale() const;                        // at 0x64
    virtual void SetScale(f32 scale);                    // at 0x68
    virtual RPGrpModel* GetModel() const;                // at 0x6C
    virtual void SetModel(RPGrpModel* pModel);           // at 0x70
#endif

    /**
     * @brief Model matrix from the axes and mPos, uniform scale max(mScale, 0.001), then
     * UpdateFrame/Calc/Entry on the model [H]
     */
    virtual void CalcModelMtx(); // at 0x74

    /**
     * @brief Axes back to X = (1, 0, 0), Y = (0, 1, 0), Z = (0, 0, 1) (ledger
     * RP_ActorBase_resetAxes; RPDollBase's reset) [H]
     */
    void ResetAxes();

protected:
    nw4r::math::VEC3 mPos;   // at 0x10
    nw4r::math::VEC3 mVec1C; // at 0x1C velocity in RPPhysSphere [M]
    nw4r::math::VEC3 mVec28; // at 0x28 [L]
    // Wii Play: EGG vectors (RPFshResultFish normalises and crosses them in place)
    EGG::Vector3f mAxisZ; // at 0x34 initially (0, 0, 1)
    EGG::Vector3f mAxisY; // at 0x40 initially (0, 1, 0)
    EGG::Vector3f mAxisX; // at 0x4C initially (1, 0, 0)
    f32 mScale;              // at 0x58 initially 1.0
public:
    RPGrpModel* mpModel;     // at 0x5C
};

/**
 * @brief rMtx's 3x3 part times rVec (ledger RP_mtx33MultVec, 801B9254) [M]
 */
EGG::Vector3f RPMtx33MultVec(const EGG::Matrix34f& rMtx, const EGG::Vector3f& rVec);

/**
 * @brief Interpolates a basis from (rFromX, rFromY, rFromZ) to (rToX, rToY, rToZ) at t (ledger
 * Fsh_slerpBasis, 801B90B0) [L]
 */
void RPSlerpBasis(f32 t, const EGG::Vector3f* pFromX, const EGG::Vector3f* pFromY,
                  const EGG::Vector3f* pFromZ, const EGG::Vector3f& rToX,
                  const EGG::Vector3f& rToY, const EGG::Vector3f& rToZ, EGG::Vector3f& rOutX,
                  EGG::Vector3f& rOutY, EGG::Vector3f& rOutZ);

/**
 * @brief Draws a shadow-volume box of half width size * k and depth height at rPos (ledger
 * RP_drawShadowVolumeQuad, 801B92B8; RPFshRod's hook shadow) [L]
 */
void RPDrawShadowVolumeQuad(const nw4r::math::VEC3& rPos, f32 size, f32 height);

//! @}

#endif
