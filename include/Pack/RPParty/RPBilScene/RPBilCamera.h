#ifndef RP_PARTY_BIL_CAMERA_H
#define RP_PARTY_BIL_CAMERA_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPBilScene/RPBilObject.h>
#include <Pack/RPParty/RPPartyCommon/RPInterpCamera.h>

#include <egg/math.h>

//! @addtogroup rp_bil
//! @{

/**
 * @brief Billiards main camera: default and overview views, aiming view, orbiting while the
 * player rotates the view
 * @details vtable 8035DD48 (at 0x174). Size 0x238, verified (Bil_CameraMgr_create). The
 * constructor builds the camera's 0x60-byte sound actor (camera tick sounds while orbiting).
 * Reset jumps to the default view (sDefaultPos / sDefaultTarget / sDefaultUp, fovy 40). The
 * Move* functions start an RPInterpCamera move (60 frames to the aim view, 60 to the default
 * view, 120 / 20 to a look-at target).
 */
class RPBilCamera : public RPInterpCamera {
public:
    RPBilCamera();
    virtual ~RPBilCamera(); // at 0x8

    virtual void Reset();     // at 0x10
    virtual void Calculate(); // at 0x14

    /**
     * @brief Sets pCamera's position, target and up to the default view looking along rDir's
     * side of the table (distance RPBilCamera::sBaseDist, drop RPBilCamera::sDefaultDrop) [M]
     * @details Ledger Bil_Camera_computeDefaultView; also used by RPBilAuxCamera::Reset and on a
     * stack RPGrpCamera, so it does not use this.
     */
    static void ComputeDefaultView(RPGrpCamera* pCamera, const EGG::Vector3f& rDir);
    /**
     * @brief Moves to the view over the table from sOverviewOffset's side [M]
     */
    void MoveToOverview(s32 frames);
    void MoveToDefaultView();
    /**
     * @brief Orbits around the aim target; plays the camera tick sound by angle [M]
     */
    void OrbitPitch(f32 delta);
    void OrbitYaw(f32 delta);
    /**
     * @brief Moves behind rFrom looking at rTarget (60 frames, ease 1) and stores rFrom as the
     * aim target [H]
     */
    void MoveToAimAt(const EGG::Vector3f& rFrom, const EGG::Vector3f& rTarget);
    /**
     * @brief Moves to RPBilCamera::sLookAtDist along rDir from rTarget, looking at rTarget [M]
     */
    void MoveToLookAt(const EGG::Vector3f& rTarget, const EGG::Vector3f& rDir);
    /**
     * @brief Stores the current position, target and up and sets mHasSavedView [H]
     */
    void SaveView();
    /**
     * @brief RPBilCamera::sBaseDist (does not use this; called without one) [M]
     */
    static f32 GetBaseDistance();
    /**
     * @brief Zoomed-in distance, by TV mode (does not use this; called without one) [M]
     */
    static f32 GetZoomDistance();

private:
    EGG::Vector3f mAimTarget;        // at 0x200 set by MoveToAimAt, zero after Reset
    bool mHasSavedView;              // at 0x20C
    EGG::Vector3f mSavedPos;         // at 0x210
    EGG::Vector3f mSavedTarget;      // at 0x21C
    EGG::Vector3f mSavedUp;          // at 0x228
    RPBilSoundActor* mpSoundActor;   // at 0x234

    static EGG::Vector3f sDefaultPos;    // 8043A95C (-300, 100, 300)
    static EGG::Vector3f sDefaultTarget; // 8043A974 (100, 0, 0)
    static EGG::Vector3f sDefaultUp;     // 8043A98C (0, 1, 0)
    static EGG::Vector3f sOverviewOffset; // 8043A9A4 (0, 20, 0) [L]

public:
    static f32 sFovy;
    static s32 sLookAtFrames;
    static f32 sLookAtDist;
    static s32 sAimFrames;
    static f32 sAimHeight;
    static f32 sPitchMin;
    static f32 sPitchMax;
    static f32 sZoomNearAlt;
    static f32 sZoomNear;
    static f32 sZoomFar;
    static f32 sOverviewDist;
    static f32 sOverviewDir;
    static s32 sDefaultFrames;
    static f32 sBaseDist;
    static f32 sDefaultDrop;
};

/**
 * @brief Second Billiards camera (3D view 1); only resets to its default view
 * @details vtable 8035E348 (at 0x174). Size 0x200, verified: adds no members to
 * RPInterpCamera.
 */
class RPBilAuxCamera : public RPInterpCamera {
public:
    RPBilAuxCamera();
    virtual ~RPBilAuxCamera(); // at 0x8

    virtual void Reset();     // at 0x10
    virtual void Calculate(); // at 0x14
};

/**
 * @brief Camera looking into a pocket for the sub-window replay (two made by RPBilCameraMgr,
 * 3D views 2 and 3)
 * @details vtable 8035E360 (at 0x174). Size 0x204, verified (Bil_CameraMgr_create).
 */
class RPBilPocketCam : public RPInterpCamera {
public:
    explicit RPBilPocketCam(s32 index);
    virtual ~RPBilPocketCam(); // at 0x8

    virtual void Reset();     // at 0x10
    virtual void Calculate(); // at 0x14

    /**
     * @brief Places the camera over the given pocket of RPBilTable, at a random angle [M]
     */
    void LookAtPocket(s32 pocket);

private:
    s32 mIndex; // at 0x200

public:
    static f32 sPocketCamParamA;
    static f32 sPocketCamDist;
};

/**
 * @brief Holder of the Billiards cameras (singleton RPBilCameraMgr::sInstance)
 * @details vtable 8035DD38 (destructor only). Size 0x14 [H] (Bil_CameraMgr_create allocates
 * 0x14). Calculate runs every camera's Calculate and copies the main camera's view matrix to
 * the 3D sound listener; Reset does the same with Reset.
 */
class RPBilCameraMgr {
public:
    static RPBilCameraMgr* sInstance; // 8043C9C0

    static const int POCKET_CAM_NUM = 2;

public:
    RPBilCameraMgr();
    virtual ~RPBilCameraMgr(); // at 0x8

    void Reset();
    void Calculate();

    RPBilCamera* GetCamera() const {
        return mpCamera;
    }
    RPBilAuxCamera* GetAuxCamera() const {
        return mpAuxCamera;
    }
    RPBilPocketCam* GetPocketCam(s32 i) const {
        return mpPocketCams[i];
    }

    static void CreateInstance();
    static void DestroyInstance();

private:
    RPBilCamera* mpCamera;                         // at 0x4
    RPBilAuxCamera* mpAuxCamera;                   // at 0x8
    RPBilPocketCam* mpPocketCams[POCKET_CAM_NUM];  // at 0xC
};

//! @}

#endif
