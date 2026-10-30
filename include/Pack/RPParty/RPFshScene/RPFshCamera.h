#ifndef RP_PARTY_FSH_CAMERA_H
#define RP_PARTY_FSH_CAMERA_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPInterpCamera.h>

#include <egg/math.h>

//! @addtogroup rp_fsh
//! @{

/**
 * @brief Fishing camera: default, catch and result views and the intro move
 * @details vtable 80358DD0 (at 0x174). Size 0x204, verified (Fsh_CameraMgr_create). The views
 * are static EGG::Vector3f pairs (eye, target) built by a static initialiser: default
 * (0, 210, 430) -> (0, 10, 0), catch (0, 280, 430) -> (0, 280, 0), result (0, 280, 430) ->
 * (0, 280, 0), and the intro start (default eye + (0, 150, 130)). The Move* functions start an
 * RPInterpCamera move with ease type 1 over 20 (catch), 40 (default) or 120 (result) frames.
 */
class RPFshCamera : public RPInterpCamera {
public:
    RPFshCamera();
    virtual ~RPFshCamera(); // at 0x8

    virtual void Reset();     // at 0x10
    virtual void Calculate(); // at 0x14

    /**
     * @brief Jumps to the default view [H]
     */
    void SetDefault();
    void MoveToDefault();
    void MoveToCatchView();
    void MoveToResultView();
    /**
     * @brief Starts the 180-frame intro move once [H]
     */
    void StartIntroMove();
    void CalcIntroMove();
    /**
     * @brief False while the camera sits exactly on the catch or the result view [H]
     */
    bool IsOffPreset() const;

    bool IsIntroMoving() const {
        return mIsIntroMoving;
    }

private:
    bool mIsIntroMoving; // at 0x1FD set by StartIntroMove, cleared by Reset/SetDefault [M]
    s32 mIntroFrame;     // at 0x200 [M]

    static EGG::Vector3f sDefaultEye;     // 8043A5A4
    static EGG::Vector3f sDefaultTarget;  // 8043A5BC
    static EGG::Vector3f sCatchEye;       // 8043A5D4
    static EGG::Vector3f sCatchTarget;    // 8043A5EC
    static EGG::Vector3f sResultEye;      // 8043A604
    static EGG::Vector3f sResultTarget;   // 8043A61C
    static EGG::Vector3f sIntroEye;       // 8043A634 default eye + (0, 150, 130)
    static EGG::Vector3f sIntroTarget;    // 8043A64C default target
    static EGG::Vector3f sIntroEndEye;    // 8043A664 default eye
    static EGG::Vector3f sIntroEndTarget; // 8043A67C default target

    static f32 sFovy;          // 8043B590 37.5
    static s32 sCatchFrames;   // 8043B594 20
    static s32 sDefaultFrames; // 8043B598 40
    static s32 sResultFrames;  // 8043B59C 120
    static s32 sIntroFrames;   // 8043B5A0 180
};

/**
 * @brief Holder of the Fishing camera (singleton Fsh_cameraMgr)
 * @details vtable 80358D98 (destructor only). Size 0x8 [H] (Fsh_CameraMgr_create allocates 0x8).
 * Calculate/Reset run the camera's virtual and copy its view matrix to the 3D sound listener.
 */
class RPFshCameraMgr {
public:
    RPFshCameraMgr() {
        mpCamera = new RPFshCamera();
    }
    virtual ~RPFshCameraMgr(); // at 0x8

    void Reset();
    void Calculate();

    RPFshCamera* GetCamera() const {
        return mpCamera;
    }

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshCameraMgr* GetInstance() {
        return sInstance;
    }

private:
    RPFshCamera* mpCamera; // at 0x4

    static RPFshCameraMgr* sInstance; // 8043C750

};

//! @}

#endif
