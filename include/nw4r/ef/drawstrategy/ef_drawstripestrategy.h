#ifndef NW4R_EF_DRAW_STRIPE_STRATEGY_H
#define NW4R_EF_DRAW_STRIPE_STRATEGY_H
#include <nw4r/types_nw4r.h>

#include <nw4r/ef/drawstrategy/ef_drawstrategyimpl.h>

namespace nw4r {
namespace ef {

class DrawStripeStrategy : public DrawStrategyImpl {
public:
    struct Trigonometric {
        f32 mSin; // at 0x0
        f32 mCos; // at 0x4
    };

    struct VertexTube {
        math::VEC3 mCenter; // at 0x0
        math::VEC3 mX;      // at 0xC
        math::VEC3 mZ;      // at 0x18
        f32 mTexCoord;      // at 0x24
    };

    struct AheadContextStripe : public AheadContext {
        math::VEC3 mEmitterAxisX;      // at 0xBC
        math::VEC3 mScreenAxisZ;       // at 0xC8
        Trigonometric* mTrigonometric; // at 0xD4

        AheadContextStripe(const math::MTX34& rViewMtx,
                           ParticleManager* pManager)
            : AheadContext(rViewMtx, pManager), mTrigonometric(NULL) {

            math::VEC3 axisX(mCommon.mEmitterMtx._00, mCommon.mEmitterMtx._10,
                             mCommon.mEmitterMtx._20);

            math::VEC3TransformNormal(&mEmitterAxisX,
                                      &mCommon.mParticleManagerMtxInv, &axisX);
        }
    };

    typedef void (*CalcAheadFuncStripe)(math::VEC3* pAxisY,
                                        AheadContextStripe* pContext,
                                        Particle* pParticle);

public:
    DrawStripeStrategy();

    virtual void Draw(const DrawInfo& rInfo,
                      ParticleManager* pManager); // at 0xC

    virtual CalcAheadFunc
    GetCalcAheadFunc(ParticleManager* pManager); // at 0x18

private:
    void InitGraphics(const DrawInfo& rInfo, ParticleManager* pManager) {
        const EmitterDrawSetting& rSetting =
            *pManager->mResource->GetEmitterDrawSetting();

        InitTexture(rSetting);
        InitTev(rSetting, rInfo);
        InitColor(pManager, rSetting, rInfo);

        GXEnableTexOffsets(GX_TEXCOORD0, TRUE, TRUE);

        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

        if (mNumTexmap > 0) {
            GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        }

        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

        GXSetCurrentMtx(GX_PNMTX0);
    }

    void DrawStripe(AheadContextStripe* pContext, int flags,
                    const math::VEC3& rP0, const math::VEC3& rP1);

    int GetStripeTexmapType(const EmitterDrawSetting& rSetting) const {
        return rSetting.typeOption2 & 0xC0;
    }

    void DrawParticle(AheadContextStripe* pContext, int flags,
                      Particle* pParticle, CalcAheadFuncStripe pCalcAheadFunc,
                      const math::VEC3& rPos, const math::VEC3& rP0,
                      const math::VEC3& rP1, f32 pivot, f32 texCoord);

    void CalcVertexTube(VertexTube* pVertex, AheadContextStripe* pContext,
                        int flags, Particle* pParticle,
                        CalcAheadFuncStripe pCalcAheadFunc, f32 pivotX,
                        f32 pivotY, f32 texCoord);

    void DrawTubeSegment(const VertexTube& rV0, const VertexTube& rV1,
                         int flags, int numVertex,
                         const Trigonometric* pTrigonometric);

    void DrawStripeTube_ConnectNone(AheadContextStripe* pContext, int flags);
    void DrawStripeTube_ConnectRing(AheadContextStripe* pContext, int flags);
    void DrawStripeTube_ConnectEmitter(AheadContextStripe* pContext,
                                       int flags);
    void DrawStripeTube(AheadContextStripe* pContext, int flags);

    math::VEC3 GetInitialPrevAxis(const EmitterDrawSetting& rSetting,
                                  const AheadContextStripe& rContext);

    static math::MTX34 CalcAxisMtx(AheadContextStripe* pContext, int flags,
                                   Particle* pParticle,
                                   const math::VEC3& rAxisY,
                                   const math::VEC3& rPos);

    static math::MTX34 CalcRotMtx(Particle* pParticle, f32 pivot);

    static void CalcAhead_Particle_Stripe(math::VEC3* pAxisY,
                                          AheadContextStripe* pContext,
                                          Particle* pParticle);
    static void CalcAhead_ParticleBoth_Stripe(math::VEC3* pAxisY,
                                              AheadContextStripe* pContext,
                                              Particle* pParticle);
    static void CalcAhead_ParticleBoth_Ring(math::VEC3* pAxisY,
                                            AheadContextStripe* pContext,
                                            Particle* pParticle);
    static void CalcAhead_ParticleBoth_Origin(math::VEC3* pAxisY,
                                              AheadContextStripe* pContext,
                                              Particle* pParticle);
};

} // namespace ef
} // namespace nw4r

#endif
