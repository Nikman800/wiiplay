#ifndef RP_PARTY_COMMON_LOOK_AT_CAM_H
#define RP_PARTY_COMMON_LOOK_AT_CAM_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/RPGrpCamera.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief RPGrpCamera driven by a position, target and up vector
 * @details vtable 80357228 (at 0x174, RPGrpCamera's vtable pointer). Size 0x1D8 [H]: members
 * end at 0x1D8; the Tnk/Bom cameras (0x204) and the Duc camera (0x234) derive from it.
 * InitView/UpdateView rebuild the basis (dir = target - pos, normalised; side = up x dir;
 * up = dir x side) and copy pos/target/up into RPGrpCamera's mPosition/mTarget/mUp.
 * @note Wii Play-only framework class; the name is a project convention.
 */
class RPLookAtCam : public RPGrpCamera {
public:
    RPLookAtCam();
    virtual ~RPLookAtCam(); // at 0x8

    /**
     * @brief Resets pos/target to zero, up to +Y, directions to +Z, unk1D0 to 1 [H]
     */
    virtual void Reset(); // at 0x10
    /**
     * @brief Rebuilds the basis and copies it into the camera [H]
     */
    virtual void InitView(); // at 0x14
    /**
     * @brief Polls the core controller manager, then calls UpdateView [M]
     */
    virtual void Calculate(); // at 0x18
    /**
     * @brief Same code as InitView; the Duc camera overrides it [H]
     */
    virtual void UpdateView(); // at 0x1C

    // Wii Play: read by Shooting Range (RPDucTarget::GetDistanceToCamera)
    nw4r::math::VEC3 GetLookPos() const {
        return mLookPos;
    }
    // Wii Play: read by Shooting Range (RPDucShot::UpdateAim)
    nw4r::math::VEC3 GetDir() const {
        return mDir;
    }
    nw4r::math::VEC3 GetSide() const {
        return mSide;
    }
    // Wii Play: the setters take the vector by value (RPDucCamera::SetMode)
    void SetLookPos(nw4r::math::VEC3 pos) {
        mLookPos = pos;
    }
    void SetLookTarget(nw4r::math::VEC3 target) {
        mLookTarget = target;
    }
    void SetLookUp(nw4r::math::VEC3 up) {
        mLookUp = up;
    }

protected:
    nw4r::math::VEC3 mLookPos;    // at 0x178
    nw4r::math::VEC3 mLookUp;     // at 0x184
    nw4r::math::VEC3 mLookTarget; // at 0x190
    nw4r::math::VEC3 mLookVec;    // at 0x19C target - pos
    f32 mLookDist;                // at 0x1A8 |mLookVec|
    nw4r::math::VEC3 mDir;        // at 0x1AC normalised mLookVec
    nw4r::math::VEC3 mSide;       // at 0x1B8
    nw4r::math::VEC3 mUpOrtho;    // at 0x1C4
    f32 unk1D0;                   // at 0x1D0 [L] reset to 1.0
    f32 unk1D4;                   // at 0x1D4 [L] reset to 0.0f
};

//! @}

#endif
