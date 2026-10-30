#ifndef RP_PARTY_BOM_SCENE_H
#define RP_PARTY_BOM_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPBomScene/RPBomSound.h>
#include <Pack/RPSystem.h>

//! @addtogroup rp_bom
//! @{

// Forward declarations
class RPBomBorder;
class RPBomCamera;
class RPBomCursorManager;
class RPBomGameManager;
class RPBomLayoutMgr;
class RPBomPanelCtrl;
class RPBomPlayerManager;
class RPBomScreenFx;
class RPBomStartBg;
class RPCommon;
class RPGrpScreen;
class RPResSlots;


/**
 * @brief Party Pack Pose Mii scene
 * @details vtable 8035E6B8. Size 0x8C, verified against Wii Play's scene creator. Configure
 * creates the resource slots (100), six model scenes, RPCommon (game 3), the cameras and
 * screens and every Bom singleton; LoadResource loads the scene light, bloom and light map,
 * the objects' resources, basic_bomb.breff/breft and rp_Bom_sound.brsar. Calculate runs one
 * object pass (CalcObjects) and the model updates, then changes scene when the result menu
 * asks for it (RPBomGameManager::mRetry / mQuit).
 */
class RPBomScene : public RPSysScene {
public:
    RPBomScene();
    virtual ~RPBomScene(); // at 0x8

    /**
     * @brief Empty [H]
     */
    virtual void pauseCallBack(bool enter); // at 0x40
    virtual void Configure();               // at 0x44
    virtual void LoadResource();            // at 0x48
    virtual void Reset();                   // at 0x4C
    virtual void Calculate();               // at 0x50
    virtual void Exit();                    // at 0x54

    /**
     * @brief IRPGrpDrawObject::UserDraw override: screen capture / spotlight passes of
     * RPBomScreenFx on view 0 and 2, the layouts on view 3 (draw step 3)
     * @details Overriding a virtual of the secondary base appends a slot to the primary
     * vtable (at 0x5C) and a this-adjusting thunk in the IRPGrpDrawObject part (slot 11).
     */
    virtual void UserDraw(); // at 0x5C

    /**
     * @brief One object pass: camera, cursors, player states, panel move / pre / hit / post,
     * player moves, panels, start background, border, screen effects, game manager, panel
     * ages [H]
     */
    void CalcObjects();
    void CreateCameras();
    void LoadLightMap();

    static const f32 sOne;  // 1.0 (blur offset, controller and 3D sound parameters)
    static const f32 sZero; // 0.0 (scene light frame and position, screen origin)

private:

    RPResSlots* mpResSlots;            // at 0x44 RPResSlots::sInstance
    RPBomStartBg* mpStartBg;           // at 0x48 RPBomStartBg::sInstance
    RPBomBorder* mpBorder;             // at 0x4C RPBomBorder::sInstance
    RPBomScreenFx* mpScreenFx;         // at 0x50 RPBomScreenFx::sInstance
    RPBomPlayerManager* mpPlayerMgr;   // at 0x54 RPBomPlayerManager::sInstance
    RPBomPanelCtrl* mpPanelCtrl;       // at 0x58 RPBomPanelCtrl::sInstance
    RPBomCursorManager* mpCursorMgr;   // at 0x5C RPBomCursorManager::sInstance
    RPBomLayoutMgr* mpLayoutMgr;       // at 0x60 RPBomLayoutMgr::sInstance
    RPBomGameManager* mpGameMgr;       // at 0x64 RPBomGameManager::sInstance
    RPBomSoundHolder* mpSound;         // at 0x68
    RPCommon* mpCommon;                // at 0x6C RPCommon::sInstance (8043C6D8)
    RPGrpScreen* mpScreen3D;           // at 0x70 0x68-byte screen (vtable 8034F32C), view 0
    RPGrpScreen* mpScreen3D2;          // at 0x74 views 1, 2 and 5
    RPGrpScreen* mpScreen2D;           // at 0x78 views 3 and 4
    RPBomCamera* mpCamera;             // at 0x7C Bom_cameraInstance
    s32 mPlayerNum;                    // at 0x80 1 or 2 (from the party scene info)
    u32 mObjPassRun;                   // at 0x84 [M] set by CalcObjects, cleared by Reset
    u32 mObjPassNum;                   // at 0x88 [M] CalcObjects calls since Reset
};

//! @}

#endif
