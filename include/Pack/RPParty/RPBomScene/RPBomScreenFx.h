#ifndef RP_PARTY_BOM_SCREEN_FX_H
#define RP_PARTY_BOM_SCREEN_FX_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

//! @addtogroup rp_bom
//! @{

// Forward declarations
class RPGrpCaptureTexture;
class RPBomScreenFx;

/**
 * @brief Full-screen effect passes drawn by RPBomScene::UserDraw (singleton
 * RPBomScreenFx::sInstance)
 * @details vtable 8035EF68. Size 0x84, verified (CreateInstance inlines the constructor, which
 * fills the three state tables). Per state mState (0-2) Calculate, CaptureState (followed by a
 * capture into mpCapture) and DrawState run mCalcFuncs / mCaptureFuncs / mDrawFuncs: capture
 * state 0 draws the full-screen mask, capture state 1 the player spotlights, draw state 1 the
 * captured screen; the others are empty. The game always stays in state 0.
 * @note The original's .text has the constructor defined last (stripped, out-of-line copy):
 * that is why its member-function-pointer constants appear twice in .data.
 * @note Name is a project convention (ledger Bom_ScreenFx).
 */
class RPBomScreenFx : public RPSceneObj {
public:
    static RPBomScreenFx* sInstance; // 8043CAB8

    typedef void (RPBomScreenFx::*StateFunc)();

public:
    virtual ~RPBomScreenFx(); // at 0x8

    /**
     * @brief Creates the capture texture of view 0's screen [H]
     */
    virtual void Create();    // at 0xC
    virtual void Load();      // at 0x10 empty
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18

    void SetAlpha(f32 alpha);
    void CaptureState();
    void DrawState();

    void StateNop0();
    void StateNop1();
    void StateNop2();
    void StateNop3();
    void StateNop4();
    void StateNop5();
    void DrawCapturedScreen();
    void DrawFullMask();
    void DrawPlayerSpotlights();

    static void CreateInstance();
    static void DestroyInstance();

    RPBomScreenFx();

private:
    s32 mState;                       // at 0x4
    f32 mAlpha;                       // at 0x8 1.0 after Reset, 0.92 from the scene
    RPGrpCaptureTexture* mpCapture;   // at 0xC 0x10 bytes
    f32 unk10;                        // at 0x10 [M] 10.0 after Reset, spotlight radius / 2
    f32 mScreenDiag;                  // at 0x14 screen diagonal (spotlight radius) [M]
    StateFunc mCalcFuncs[3];          // at 0x18 run by Calculate
    StateFunc mCaptureFuncs[3];       // at 0x3C run by CaptureState
    StateFunc mDrawFuncs[3];          // at 0x60 run by DrawState
};

//! @}

#endif
