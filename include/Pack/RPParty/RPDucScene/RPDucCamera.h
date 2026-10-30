#ifndef RP_PARTY_DUC_CAMERA_H
#define RP_PARTY_DUC_CAMERA_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLookAtCam.h>

#include <nw4r/math.h>

//! @addtogroup rp_duc
//! @{

// Forward declarations
class RPPolyline;

/**
 * @brief Shooting Range camera: intro fly-in along a Bezier path, then a fixed view
 * (singleton Duc_cameraInstance)
 * @details vtable 8035A3F8 (at 0x174). Size 0x234, verified. Two modes/states: 0 intro, 1 idle.
 * The frustum edge points at 0x1E8.. are the left/right/top/bottom screen edges one unit in
 * front of the camera; the GetScreen*AtDepth helpers intersect them with a z plane.
 * @note Name is a project convention (ledger Duc_Camera).
 */
class RPDucCamera : public RPLookAtCam {
public:
    typedef void (RPDucCamera::*StateFunc)();

public:
    RPDucCamera() : mpIntroPath(NULL), mIntroT(0.0f), unk1E4(0), mMode(1) {
        mStateFuncs[0] = &RPDucCamera::StateIntro;
        mStateFuncs[1] = &RPDucCamera::StateIdle;
    }
    virtual ~RPDucCamera(); // at 0x8

    /**
     * @brief RPLookAtCam::Reset, then creates the 5-point intro path [H]
     */
    virtual void Reset();     // at 0x10
    virtual void Calculate(); // at 0x18

    static void CreateInstance();
    static void DestroyInstance();

    static RPDucCamera* sInstance;

    void SetMode(s32 mode);
    s32 GetMode() const {
        return mMode;
    }
    void CalcFrustumEdges();
    void ProjectToScreen(nw4r::math::VEC2* pOut, const nw4r::math::VEC3& rPos) const;
    f32 GetScreenLeftAtDepth(f32 z) const;
    f32 GetScreenRightAtDepth(f32 z) const;
    f32 GetScreenTopAtDepth(f32 z) const;

    void StateIntro();
    void StateIdle();

private:
    RPPolyline* mpIntroPath;       // at 0x1D8
    f32 mIntroT;                   // at 0x1DC path parameter
    f32 mIntroSpeed;               // at 0x1E0 reset to 0.001667
    u32 unk1E4;                    // at 0x1E4 [L]
    nw4r::math::VEC3 mEdgeLeft;    // at 0x1E8
    nw4r::math::VEC3 mEdgeRight;   // at 0x1F4
    nw4r::math::VEC3 mEdgeTop;     // at 0x200
    nw4r::math::VEC3 mEdgeBottom;  // at 0x20C
    s32 mMode;                     // at 0x218 0 intro, 1 idle
    StateFunc mStateFuncs[2];      // at 0x21C
};

//! @}

#endif
