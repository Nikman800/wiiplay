#ifndef RP_PARTY_BIL_POCKET_CAM_WINDOW_H
#define RP_PARTY_BIL_POCKET_CAM_WINDOW_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpDrawObject.h>
#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPKernel/RP_DEBUG_STUB_1.h>

#include <egg/math.h>
#include <nw4r/lyt.h>

//! @addtogroup rp_bil
//! @{

// Forward declarations
namespace EGG {
class Archive;
class Vector3f;
} // namespace EGG
class RPSysLayout;
class RPSysLytResAccessor;

/**
 * @brief Two picture-in-picture windows showing a pocket camera after a ball drops (singleton
 * RPBilPocketCamWindow::sInstance)
 * @details vtable 8035E378. Size 0x30, verified (Bil_PocketCamWindow_create). Same bases as
 * RPBilScreen, but its own virtuals are ordered Reset, Load, Calculate, Exit (RPBilGameMgr
 * calls Load through 0x38 and Reset through 0x34); the Exit slot holds the Calculate
 * function. Uses billiard_03.brlyt (W_subWindow_00) as the frame. ShowPocket starts a window
 * for 120 frames with RPBilPocketCam::LookAtPocket; each window's timer drops by one per frame.
 */
class RPBilPocketCamWindow : public RP_DEBUG_STUB_1,
                             public IRPSysHostIOSocket,
                             public IRPGrpDrawObject {
public:
    static RPBilPocketCamWindow* sInstance; // 8043CA20

    static const int WINDOW_NUM = 2;

public:
    RPBilPocketCamWindow();
    virtual ~RPBilPocketCamWindow(); // at 0x8

    virtual void Reset();                     // at 0x34
    virtual void Load(EGG::Archive* pArchive); // at 0x38 billiard.arc
    virtual void Calculate();                 // at 0x3C
    virtual void Exit();                      // at 0x40 [M] empty (0x802CBBC8, the ledger
                                              // counts it in Bil_PocketCamWindow_calc)

    virtual void UserDraw(); // at 0x44

    /**
     * @brief Caps the running timers at 60 frames (end of the shot) [M]
     */
    void ClampTimers();
    /**
     * @brief Opens a window on the pocket (or extends its timer); false if both are busy [M]
     */
    bool ShowPocket(s32 pocket);
    /**
     * @brief The pocket a ball at rPos moving along rVel is about to enter, or -1 (does not use
     * this; ledger Bil_findEnteredPocket, called by RPBilBall) [M]
     */
    s32 FindEnteredPocket(const EGG::Vector3f& rPos, const EGG::Vector3f& rVel);

    /**
     * @brief Size of one sub window: the TV mode's screen size divided by RPBilPocketCamWindow::sSubWindowDivisor
     * (ledger Bil_getSubWindowWidth / Bil_getSubWindowHeight) [M]
     */
    static u16 GetSubWindowWidth();
    static u16 GetSubWindowHeight();

    static void CreateInstance();
    static void DestroyInstance();

private:
    s32 mPockets[WINDOW_NUM];             // at 0x10 -1 when the window is free
    s32 mTimers[WINDOW_NUM];              // at 0x18
    bool mDrawn[WINDOW_NUM];              // at 0x20 [L] cleared by Calculate
    RPSysLayout* mpLayout;                // at 0x24 billiard_03
    nw4r::lyt::Pane* mpWindowPane;        // at 0x28 W_subWindow_00
    RPSysLytResAccessor* mpResAccessor;   // at 0x2C

public:
    static EGG::Vector2f sSubWindowPos[RPBilPocketCamWindow::WINDOW_NUM];
    static s32 sPocketWindowFrames;
    static s32 sPocketWindowClamp;
    static s32 sPocketWindowStep;
    static f32 sPocketEnterParamA;
    static f32 sPocketEnterParamB;
    static s32 sSubWindowDivisor;
    static s32 sPocketCamParam;
    static f32 sPocketAngle60;
    static f32 sPocketAngle80;
    static f32 sPocketAngle120;
};

//! @}

#endif
