#ifndef RP_PARTY_DUC_SCENE_H
#define RP_PARTY_DUC_SCENE_H
#include <Pack/types_pack.h>

// Wii Play: RPDucScene instantiates TRPSndObject<4> first (see RPDucSndObjectInstantiate)
#define RP_DUC_SOUND_INSTANTIATE_SND_OBJECT
#include <Pack/RPParty/RPDucScene/RPDucSound.h>
#undef RP_DUC_SOUND_INSTANTIATE_SND_OBJECT
#include <Pack/RPSystem.h>

//! @addtogroup rp_duc
//! @{

// Forward declarations
class RPDucCamera;
class RPDucField;
class RPDucGame;
class RPDucHud;
class RPDucShotMgr;
class RPDucTargetRegistry;
class RPCommon;
class RPGrpScreen;
class RPMiiIconSet;
class RPResSlots;
namespace EGG {
class Disposer;
}

/**
 * @brief Party Pack Shooting Range scene
 * @details vtable 80359A38. Size 0x80, verified against Wii Play's scene creator. Creates and
 * drives the Duc singletons; each frame runs three physics sub-steps over every target
 * (RPDucTargetRegistry), then the camera, shots, field and game.
 */
class RPDucScene : public RPSysScene {
public:
    RPDucScene();
    virtual ~RPDucScene(); // at 0x8

    /**
     * @brief Hides or restores the shot cursors [H]
     */
    virtual void pauseCallBack(bool enter); // at 0x40
    virtual void Configure();               // at 0x44
    virtual void LoadResource();            // at 0x48
    virtual void Reset();                   // at 0x4C
    virtual void Calculate();               // at 0x50
    virtual void Exit();                    // at 0x54

    /**
     * @brief IRPGrpDrawObject::UserDraw override: draws the HUD layouts on the layout pass
     * @details Overriding a virtual of the secondary base appends a slot to the primary
     * vtable (at 0x5C) and a this-adjusting thunk in the IRPGrpDrawObject part (slot 11).
     */
    virtual void UserDraw(); // at 0x5C

    void CalcSubsteps();
    void CreateCameras();
    void LoadLight();

private:
    static const f32 sLightFrame;

    RPResSlots* mpResSlots;              // at 0x44
    RPMiiIconSet* mpMiiIconSet;          // at 0x48
    RPDucField* mpField;                 // at 0x4C
    RPDucTargetRegistry* mpTargetRegistry; // at 0x50
    RPDucShotMgr* mpShotMgr;             // at 0x54
    RPGrpScreen* mpScreen3D;             // at 0x58 [M] 0x68-byte screen (vtable 8034F32C)
    RPGrpScreen* mpScreen2D;             // at 0x5C [M]
    RPDucCamera* mpCamera;               // at 0x60
    RPDucHud* mpHud;                     // at 0x64
    RPDucGame* mpGame;                   // at 0x68
    RPDucSceneSound* mpSound;            // at 0x6C
    RPCommon* mpPartyCommon;             // at 0x70 party common UI object (8043C6D8)
    s32 mPlayMode;                       // at 0x74 [M] 1-3 from the player count
    u32 mSubstep;                        // at 0x78 0..2 within the frame
    u32 mSubstepTotal;                   // at 0x7C
};

//! @}

#endif
