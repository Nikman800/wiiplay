#ifndef RP_GRAPHICS_DRAW_PATH_LIGHT_MAP_H
#define RP_GRAPHICS_DRAW_PATH_LIGHT_MAP_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/RPGrpDrawPath.h>

#include <egg/gfxe.h>

//! @addtogroup rp_graphics
//! @{

// Forward declarations
class RPGrpModel;

/**
 * @brief Light map draw path
 */
class RPGrpDrawPathLightMap : public RPGrpDrawPath {
public:
    RPGrpDrawPathLightMap(EGG::ScnRenderer* pRenderer);

    void ReplaceModelTexture(u16 index, RPGrpModel* pModel);

    // Wii Play: vtable 80350160 overrides On/Off/IsEnable; they toggle bit 0x10 of the light
    // texture manager's flags (+0x4) instead of the draw path base. Size 0x10, verified.
    virtual void On();             // at 0xC
    virtual void Off();            // at 0x10
    virtual bool IsEnable() const; // at 0x14

    EGG::LightTextureManager* GetLightTextureManager() const {
        return mpLightTextureManager;
    }

private:
    EGG::LightTextureManager* mpLightTextureManager; // at 0xC
};

//! @}

#endif
