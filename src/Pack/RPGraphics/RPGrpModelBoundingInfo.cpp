#include <Pack/RPGraphics.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPGraphics/RPGrpModelBoundingInfo.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

/**
 * @brief Constructor
 *
 * @param flags Bound type flags
 * @param pModel Owner model
 */
RPGrpModelBoundingInfo::RPGrpModelBoundingInfo(u32 flags, RPGrpModel* pModel)
    : EGG::ModelBoundingInfo(flags, pModel->GetModelEx()) {}
