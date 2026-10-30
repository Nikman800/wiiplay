#ifndef RP_PARTY_TNK_CAMERA_H
#define RP_PARTY_TNK_CAMERA_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLookAtCam.h>

#include <nw4r/math.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPPolyline;

/**
 * @brief Tanks! field camera: intro fly-in along a 5-point Bezier path, then fixed top view
 * (singleton Tnk_cameraInstance)
 * @details vtable 8035B178 (at 0x174). Size 0x204, verified (Tnk_Camera_createInstance).
 * Modes/states: 0 intro move, 1 fixed. The intro path points are Tnk_cameraIntroStartPos and
 * Tnk_cameraIntroPath; the fixed view looks at Tnk_cameraFixedTarget from 2000 units at 29.47
 * degrees.
 */
class RPTnkCamera : public RPLookAtCam {
public:
    typedef void (RPTnkCamera::*StateFunc)();

public:
    RPTnkCamera() : mpIntroPath(NULL), mIntroT(0.0f), unk1E4(0), mMode(1) {
        mStateFuncs[0] = &RPTnkCamera::StateIntroMove;
        mStateFuncs[1] = &RPTnkCamera::StateFixed;
    }

    static void CreateInstance();
    static void DestroyInstance();

    static RPTnkCamera* sInstance;

    virtual ~RPTnkCamera(); // at 0x8

    /**
     * @brief RPLookAtCam::Reset, then creates the intro path [H]
     */
    virtual void Reset();      // at 0x10
    virtual void InitView();   // at 0x14
    /**
     * @brief Runs the state, then UpdateView [H]
     */
    virtual void Calculate();  // at 0x18
    virtual void UpdateView(); // at 0x1C

    void SetMode(s32 mode);
    /**
     * @brief Camera-space position of pPos divided by view 1's screen scale (2D layout
     * coordinates) [H]
     */
    void ProjectToScreen(nw4r::math::VEC2* pOut, const nw4r::math::VEC3* pPos) const;

    void StateIntroMove();
    void StateFixed();

private:
    RPPolyline* mpIntroPath;   // at 0x1D8
    f32 mIntroT;               // at 0x1DC
    f32 mIntroSpeed;           // at 0x1E0 0.006667, slowed after t 0.9
    u32 unk1E4;                // at 0x1E4 [L]
    s32 mMode;                 // at 0x1E8
    StateFunc mStateFuncs[2];  // at 0x1EC
};

//! @}

#endif
