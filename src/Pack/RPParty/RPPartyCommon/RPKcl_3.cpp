// Shadow-volume drawing (unit 0x801B84F0-0x801B85B0).
#include <Pack/RPParty/RPPartyCommon/RPKcl.h>

#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

#include <egg/gfxe/eggDrawGX.h>

#include <revolution/GX.h>

#include <math.h>

extern "C" {
void RP_makeMtxFromRotPos(nw4r::math::MTX34* pMtx, const nw4r::math::VEC3* pPos,
                          const nw4r::math::MTX33* pRot);
void RP_drawShadowVolume(const nw4r::math::MTX34* pMtx);
}


extern "C" void RP_drawShadowVolume(const nw4r::math::MTX34* pMtx) {
    EGG::DrawGX::BeginDrawShadowVolume(0);
    EGG::DrawGX::SetVtxState(static_cast<EGG::DrawGX::VtxType>(3));
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, static_cast<GXLogicOp>(5));
    GXSetCullMode(GX_CULL_BACK);

    EGG::DrawGX::DrawCylinderY(EGG::DrawGX::CIRCLE_32, *pMtx, EGG::DrawGX::RED);

    GXSetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, static_cast<GXLogicOp>(5));
    GXSetCullMode(GX_CULL_FRONT);
    EGG::DrawGX::DrawCylinderY(EGG::DrawGX::CIRCLE_32, *pMtx, EGG::DrawGX::RED);
}
