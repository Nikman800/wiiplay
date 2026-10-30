#ifndef RP_GRAPHICS_DRAW_PATH_DOF_H
#define RP_GRAPHICS_DRAW_PATH_DOF_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/RPGrpDrawPath.h>

#include <egg/gfxe.h>

//! @addtogroup rp_graphics
//! @{

class RPGrpDrawPathDOF : public RPGrpDrawPath {
public:
    // Wii Play: declared (not implicit), so the weak copy comes out in declaration order
    virtual ~RPGrpDrawPathDOF() {}
    RPGrpDrawPathDOF(EGG::ScnRenderer* pRenderer)
        : RPGrpDrawPath(pRenderer, pRenderer->getDrawPathBase(
                                       EGG::ScnRenderer::cDrawPath_DOF)) {

        mpDOF = static_cast<EGG::DrawPathDOF*>(
            pRenderer->getDrawPathBase(EGG::ScnRenderer::cDrawPath_DOF));

        // Wii Play: confirmed by RPGrpDrawPath::Construct<RPGrpDrawPathDOF>
        mpDOF->unk80 = 0;
    }

private:
    EGG::DrawPathDOF* mpDOF; // at 0xC
};

//! @}

#endif
