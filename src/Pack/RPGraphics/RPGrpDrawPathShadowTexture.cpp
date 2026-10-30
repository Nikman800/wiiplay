#include <Pack/RPGraphics.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPGraphics/RPGrpDrawPathShadowTexture.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

#include <egg/gfxe.h>

/**
 * @brief Constructor
 *
 * @param pRenderer Scene renderer
 */
RPGrpDrawPathShadowTexture::RPGrpDrawPathShadowTexture(
    EGG::ScnRenderer* pRenderer)
    : RPGrpDrawPath(pRenderer) {

    mpShadowTextureManager = pRenderer->getShadowTextureManager();
}

/**
 * @brief Inserts a new shadow texture
 *
 * @param idx Texture index
 * @param pTexture Shadow texture
 */
void RPGrpDrawPathShadowTexture::SetTexture(u16 idx,
                                            RPGrpShadowTexture* pTexture) {
    mpShadowTextureManager->Set(idx, pTexture);
}

/**
 * @brief Appends a new shadow texture
 *
 * @param pTexture Shadow texture
 */
void RPGrpDrawPathShadowTexture::SetTexture(RPGrpShadowTexture* pTexture) {
    mpShadowTextureManager->Append(pTexture);
}
