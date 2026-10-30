#ifndef RP_PARTY_COMMON_MENU_BG_MODEL_H
#define RP_PARTY_COMMON_MENU_BG_MODEL_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/RP_DEBUG_STUB_1.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPGrpCamera;
class RPGrpModel;

/**
 * @brief Menu background model (G3D/menu_BG_model.brres) with its own camera and 3D view
 * @details vtable 80357A08 (destructor only). Size 0x28, verified (Menu_MenuScene_configure).
 * The constructor creates an RPGrpCamera and a 30-degree-fov EGG::Screen, registers a 3D view
 * and sets the light manager's ambient colour to (150, 150, 150). Reset plays the animations
 * from frame mStartFrame.
 * @note Wii Play-only framework class (ledger Menu_MenuBgModel); the name is a project convention.
 */
class RPMenuBgModel : public RP_DEBUG_STUB_1 {
public:
    RPMenuBgModel(u8 viewNo, u8 sceneNo, u8 startFrame);
    virtual ~RPMenuBgModel(); // at 0x8

    /**
     * @param pArchive Archive to load from (scene common carc when NULL)
     */
    void Load(void* pArchive);
    void Reset();
    void Calculate();

private:
    u8 mViewNo;               // at 0x4
    u8 mSceneNo;              // at 0x5 model scene / light manager index
    u8 mStartFrame;           // at 0x6
    RPGrpModel* mpModel;      // at 0x8
    RPGrpCamera* mpCamera;    // at 0xC
    nw4r::math::VEC3 mPos;    // at 0x10
    nw4r::math::VEC3 mScale;  // at 0x1C
};

/**
 * @brief Wii Play logo model (G3D/logo_model.brres) on the menu, with its own camera and 3D view
 * @details vtable 80358010 (destructor only). Size 0x28, verified (Menu_MenuScene_configure).
 * mPos is set from a screen position through the view (SetPosFromScreen); scale 13.
 * @note Wii Play-only framework class (ledger Menu_LogoModel); the name is a project convention.
 */
class RPLogoModel : public RP_DEBUG_STUB_1 {
public:
    RPLogoModel(u8 viewNo, u8 sceneNo);
    virtual ~RPLogoModel(); // at 0x8

    void Load(void* pArchive);
    void Reset();
    void Calculate();
    void SetPosFromScreen(const nw4r::math::VEC2& rScreenPos);

private:
    u8 mViewNo;               // at 0x4
    u8 mSceneNo;              // at 0x5
    RPGrpModel* mpModel;      // at 0x8
    RPGrpCamera* mpCamera;    // at 0xC
    nw4r::math::VEC3 mPos;    // at 0x10
    nw4r::math::VEC3 mScale;  // at 0x1C
};

//! @}

#endif
