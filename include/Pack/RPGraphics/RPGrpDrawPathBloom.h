#ifndef RP_GRAPHICS_DRAW_PATH_BLOOM_H
#define RP_GRAPHICS_DRAW_PATH_BLOOM_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/RPGrpDrawPath.h>

#include <egg/gfxe.h>

//! @addtogroup rp_graphics
//! @{

class RPGrpDrawPathBloom : public RPGrpDrawPath {
public:
    // Wii Play: declared (not implicit), so the weak copy comes out in declaration order
    virtual ~RPGrpDrawPathBloom() {}
    RPGrpDrawPathBloom(EGG::ScnRenderer* pRenderer)
        : RPGrpDrawPath(pRenderer, pRenderer->getDrawPathBase(
                                       EGG::ScnRenderer::cDrawPath_Bloom)) {

        mpBloom = static_cast<EGG::DrawPathBloom*>(
            pRenderer->getDrawPathBase(EGG::ScnRenderer::cDrawPath_Bloom));
    }

    // Wii Play: used by RPHkyGame::Load
    EGG::DrawPathBloom* GetBloom() const {
        return mpBloom;
    }

private:
    char unkC[0x10 - 0xC];
    EGG::DrawPathBloom* mpBloom; // at 0x10
    // Wii Play: Construct allocates 0x34; the rest is not written by the constructor [M]
    char unk14[0x34 - 0x14];
};

//! @}

#endif
