#include <Pack/RPGraphics.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPGraphics/RPGrpShadowTexture.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

#include <cstdio>

/**
 * @brief Replaces a texture on the specified model
 *
 * @param pModel Target model
 * @param pName Texture name (format string)
 * @param ... Format string arguments
 */
void RPGrpShadowTexture::Replace(RPGrpModel* pModel, const char* pName, ...) {
    // Wii Play: a larger buffer
    char buffer[260];

    std::va_list list;
    va_start(list, pName);

    // @bug Unsafe string operations
#if defined(BUG_FIX)
    std::vsnprintf(buffer, sizeof(buffer), pName, list);
#else
    std::vsprintf(buffer, pName, list);
#endif

    EGG::ShadowTexture::Replace(pModel->GetModelEx(), buffer);

    va_end(list);
}

/**
 * @brief Adds a new model to the shadow texture
 *
 * @param pModel Model
 * @param density Model density
 */
void RPGrpShadowTexture::AddModel(RPGrpModel* pModel, f32 density) {
    AddModelEx(pModel->GetModelEx(), density);
}

/**
 * @brief Adds a new model with materials to the shadow texture
 *
 * @param pModel Model
 * @param density Model density
 */
// Wii Play: no out-of-line copy in the binary (inlined; unused copy dead-stripped)
inline void RPGrpShadowTexture::AddModelUseMaterial(RPGrpModel* pModel, f32 density) {
    AddModelEx(pModel->GetModelEx(), density);
    m_mdlList[m_mdlNum - 1].useMat = true;
}
