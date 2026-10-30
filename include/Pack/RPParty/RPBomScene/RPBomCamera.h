#ifndef RP_PARTY_BOM_CAMERA_H
#define RP_PARTY_BOM_CAMERA_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLookAtCam.h>

//! @addtogroup rp_bom
//! @{

// Forward declarations
class RPPolyline;

/**
 * @brief Pose Mii camera: looks straight down at the play field, with an optional zoom-in
 * along a 5-point path (singleton Bom_cameraInstance)
 * @details vtable 8035E7B8 (at 0x174). Size 0x204, verified (Bom_Camera_createInstance). Modes
 * (mMode, PTMFs copied from Bom_cameraStateZoomIn / Bom_cameraStateIdle): 0 zoom in along the
 * path Bom_cameraPathStart..Bom_cameraPathPoints, 1 idle (position 5000 units straight above,
 * up -Z, target Bom_cameraIdleTarget). The scene sets mode 1 on every Reset.
 * @note Name is a project convention (ledger Bom_Camera).
 */
class RPBomCamera : public RPLookAtCam {
public:
    typedef void (RPBomCamera::*StateFunc)();

    RPBomCamera()
        : mpPath(NULL), mPathT(0.0f), unk1E4(0), mMode(1) {
        mStateFuncs[0] = &RPBomCamera::StateZoomIn;
        mStateFuncs[1] = &RPBomCamera::StateIdle;
    }

public:
    virtual ~RPBomCamera(); // at 0x8

    /**
     * @brief RPLookAtCam::Reset, then creates the 5-point path (ledger create) [H]
     */
    virtual void Reset(); // at 0x10
    /**
     * @brief Same code as RPLookAtCam::InitView (ledger reset) [H]
     */
    virtual void InitView(); // at 0x14
    /**
     * @brief Polls controller 0, runs mStateFuncs[mMode], then UpdateView [H]
     */
    virtual void Calculate(); // at 0x18
    /**
     * @brief Same code as RPLookAtCam::UpdateView (ledger updateMatrix) [H]
     */
    virtual void UpdateView(); // at 0x1C

    void SetMode(s32 mode);
    void StateZoomIn();
    void StateIdle();

    static RPBomCamera* sInstance;

    static void CreateInstance();
    static void DestroyInstance();

private:
    RPPolyline* mpPath;           // at 0x1D8 5 points
    f32 mPathT;                   // at 0x1DC 0..1
    f32 mPathSpeed;               // at 0x1E0 0.001667, x0.94 per frame after 0.9
    u32 unk1E4;                   // at 0x1E4 [L] written 0 only
    s32 mMode;                    // at 0x1E8 0 zoom in, 1 idle
    StateFunc mStateFuncs[2];     // at 0x1EC
};

//! @}

#endif
