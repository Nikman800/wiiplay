// RPSlerpBasis, RP_mtx33MultVec and RPDrawShadowVolumeQuad. Unit 0x801B90B0-0x801B93B0
// (dtk/game_units.tsv): split from RPActorBase.cpp (they follow its thunks and have their own
// 8-byte aligned .sdata2, 8043EAD0).
#include <Pack/RPParty/RPPartyCommon/RPActorBase.h>
#include <Pack/RPGraphics/RPGrpModel.h>
#include <nw4r/ut.h>


// CodeWarrior emits deferred functions in reverse definition order.
void RPDrawShadowVolumeQuad(const nw4r::math::VEC3& rPos, f32 size, f32 height) {
    EGG::Matrix34f mtx;
    PSMTXScale(mtx, -size * 2.0f, -height, size * 2.0f);
    mtx._03 = rPos.x;
    mtx._13 = rPos.y;
    mtx._23 = rPos.z;
    EGG::DrawGX::BeginDrawShadowVolume(0);
    EGG::DrawGX::SetVtxState(EGG::DrawGX::VTXTYPE_CONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP);
    GXSetCullMode(GX_CULL_BACK);
    // One argument copy (a word copy at 0xC) refreshed before each call, below the byte-copied
    // colour at 0x8 (DrawCornY takes the colour by reference)
    nw4r::ut::Color arg;
    GXColor color = EGG::DrawGX::WHITE;
    arg = color;
    EGG::DrawGX::DrawCornY(EGG::DrawGX::CIRCLE_32, mtx, arg);
    GXSetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP);
    GXSetCullMode(GX_CULL_FRONT);
    arg = color;
    EGG::DrawGX::DrawCornY(EGG::DrawGX::CIRCLE_32, mtx, arg);
}

extern "C" EGG::Vector3f RP_mtx33MultVec(const EGG::Matrix34f& rMtx, const EGG::Vector3f& rVec) {
    f32 x = rMtx._00 * rVec.x + rMtx._01 * rVec.y + rMtx._02 * rVec.z;
    f32 y = rMtx._10 * rVec.x + rMtx._11 * rVec.y + rMtx._12 * rVec.z;
    f32 z = rMtx._20 * rVec.x + rMtx._21 * rVec.y + rMtx._22 * rVec.z;
    return EGG::Vector3f(x, y, z);
}

// Rotates the basis (rFromX, rFromY, rFromZ) towards (rToX, rToY, rToZ) by t (quaternion slerp)
// and writes it to (rOutX, rOutY, rOutZ); the callers pass the same vectors as from and out.
// The pointer/reference constness is what the code shows: loads from all six inputs come before
// any stack store (const pointers and const references), and each output store stays after its
// load (writable references may alias).
void RPSlerpBasis(f32 t, const EGG::Vector3f* pFromX, const EGG::Vector3f* pFromY,
                  const EGG::Vector3f* pFromZ, const EGG::Vector3f& rToX,
                  const EGG::Vector3f& rToY, const EGG::Vector3f& rToZ, EGG::Vector3f& rOutX,
                  EGG::Vector3f& rOutY, EGG::Vector3f& rOutZ) {
    EGG::Matrix34f from;
    from.setBase(0, *pFromX);
    from.setBase(1, *pFromY);
    from.setBase(2, *pFromZ);
    EGG::Matrix34f to;
    to.setBase(0, rToX);
    to.setBase(1, rToY);
    to.setBase(2, rToZ);
    EGG::Quatf qTo;
    to.toQuat(qTo);
    EGG::Quatf qFrom;
    from.toQuat(qFrom);
    EGG::Quatf q;
    qFrom.slerpTo(qTo, t, q);
    EGG::Matrix34f mtx;
    mtx.fromQuat(q);
    mtx.getBase(0, rOutX);
    mtx.getBase(1, rOutY);
    mtx.getBase(2, rOutZ);
}
