#ifndef RP_GRAPHICS_DRAW_PATH_SV_H
#define RP_GRAPHICS_DRAW_PATH_SV_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/RPGrpDrawPath.h>

#include <egg/gfxe.h>

//! @addtogroup rp_graphics
//! @{

class RPGrpDrawPathSV : public RPGrpDrawPath {
public:
    // Wii Play: declared (not implicit), so the weak copy comes out in declaration order
    virtual ~RPGrpDrawPathSV() {}
    RPGrpDrawPathSV(EGG::ScnRenderer* pRenderer)
        : RPGrpDrawPath(pRenderer, pRenderer->getDrawPathBase(
                                       EGG::ScnRenderer::cDrawPath_SV)) {

        mpSV = static_cast<EGG::DrawPathShadowVolume*>(
            pRenderer->getDrawPathBase(EGG::ScnRenderer::cDrawPath_SV));
    }

    // Wii Play: used by RPHkyGame
    EGG::DrawPathShadowVolume* GetShadowVolume() const {
        return mpSV;
    }

private:
    EGG::DrawPathShadowVolume* mpSV; // at 0xC
};

//! @}

#endif
