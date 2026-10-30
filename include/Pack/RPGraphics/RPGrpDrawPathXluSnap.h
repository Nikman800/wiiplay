#ifndef RP_GRAPHICS_DRAW_PATH_XLU_SNAP_H
#define RP_GRAPHICS_DRAW_PATH_XLU_SNAP_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/RPGrpDrawPath.h>

#include <egg/gfxe.h>

//! @addtogroup rp_graphics
//! @{

class RPGrpDrawPathXluSnap : public RPGrpDrawPath {
public:
    // Wii Play: declared (not implicit), so the weak copy comes out in declaration order
    virtual ~RPGrpDrawPathXluSnap() {}
    RPGrpDrawPathXluSnap(EGG::ScnRenderer* pRenderer)
        : RPGrpDrawPath(pRenderer, pRenderer->getDrawPathBase(
                                       EGG::ScnRenderer::cDrawPath_XluSnap)) {

        mpXluSnap = static_cast<EGG::DrawPathXluSnap*>(
            pRenderer->getDrawPathBase(EGG::ScnRenderer::cDrawPath_XluSnap));
    }

    // Wii Play: RPFshScene, RPFshBonusFish, RPFshFish
    EGG::DrawPathXluSnap* GetXluSnap() const {
        return mpXluSnap;
    }

private:
    EGG::DrawPathXluSnap* mpXluSnap; // at 0xC
};

//! @}

#endif
