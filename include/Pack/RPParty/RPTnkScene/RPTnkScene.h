#ifndef RP_PARTY_TNK_SCENE_H
#define RP_PARTY_TNK_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPTnkScene/RPTnkSound.h>
#include <Pack/RPSystem.h>

//! @addtogroup rp_tnk
//! @{

// Forward declarations
class RPCommon;
class RPGrpCamera;
class RPGrpScreen;
class RPMiiIconSet;
class RPResSlots;
class RPTnkCamera;
class RPTnkCursorManager;
class RPTnkField;
class RPTnkGameManager;
class RPTnkLayoutManager;
class RPTnkTankManager;
class RPTnkWeaponManager;

/**
 * @brief Party Pack Tanks! scene
 * @details vtable 8035B100. Size 0x8C, verified against Wii Play's scene creator. Creates the
 * Tnk singletons in Configure and steps them in Calculate: velocities, hit-info clear,
 * collisions, tread distances and hit resolution for tanks, shells, mines and blocks, unless
 * RPTnkGameManager freezes the game.
 */
class RPTnkScene : public RPSysScene {
public:
    RPTnkScene();
    virtual ~RPTnkScene(); // at 0x8

    /**
     * @brief Hides the cursors when the pause menu opens [H]
     */
    virtual void pauseCallBack(bool enter); // at 0x40
    virtual void Configure();               // at 0x44
    virtual void LoadResource();            // at 0x48
    virtual void Reset();                   // at 0x4C
    virtual void Calculate();               // at 0x50
    virtual void Exit();                    // at 0x54

    /**
     * @brief IRPGrpDrawObject::UserDraw override: tread-mark capture, floor quad, layouts
     * @details Overriding a virtual of the secondary base appends a slot to the primary
     * vtable (at 0x5C) and a this-adjusting thunk in the IRPGrpDrawObject part (slot 11).
     */
    virtual void UserDraw(); // at 0x5C

    void CreateCamerasAndScreens();
    void UpdateGameplay();

private:
    RPResSlots* mpResSlots;                // at 0x44
    RPMiiIconSet* mpMiiIconSet;            // at 0x48
    RPTnkField* mpField;                   // at 0x4C
    RPTnkTankManager* mpTankManager;       // at 0x50
    RPTnkWeaponManager* mpWeaponManager;   // at 0x54
    RPTnkCursorManager* mpCursorManager;   // at 0x58
    RPTnkLayoutManager* mpLayoutManager;   // at 0x5C
    RPTnkGameManager* mpGameManager;       // at 0x60
    RPCommon* mpCommon;                    // at 0x64 RPCommon::sInstance
    RPGrpScreen* mpScreen3D;               // at 0x68 [M] 0x68-byte screen (vtable 8034F32C)
    RPGrpScreen* mpScreenField;            // at 0x6C [M]
    RPGrpScreen* mpScreen2D;               // at 0x70 [M]
    RPGrpCamera* mpCamera;                 // at 0x74 view 0 camera
    RPTnkCamera* mpFieldCamera;            // at 0x78 view 1 camera
    RPTnkSceneSound* mpSound;              // at 0x7C
    s32 mPlayerNum;                        // at 0x80 1 or 2
    BOOL mStepped;                         // at 0x84 physics ran this frame
    u32 mStepCount;                        // at 0x88 physics steps since Reset
};

//! @}

#endif
