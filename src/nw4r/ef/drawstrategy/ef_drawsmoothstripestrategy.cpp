#include <nw4r/ef/drawstrategy/ef_drawstrategyimpl.h>
#include <nw4r/ef/ef_drawinfo.h>
#include <nw4r/ef/ef_particle.h>
#include <nw4r/ef/ef_particlemanager.h>
#include <nw4r/ef/ef_res_emitter.h>
#include <nw4r/math.h>

#include <revolution/GX.h>

namespace nw4r {
namespace ef {

// TODO(header): this declaration belongs in
// include/nw4r/ef/drawstrategy/ef_drawsmoothstripestrategy.h. The class layout
// and vtable are unchanged; the header version declares
// GetGetFirstDrawParticleFunc/GetGetNextDrawParticleFunc overrides that the
// binary's vtable does not have, and lacks the private members below.
class DrawSmoothStripeStrategy : public DrawStrategyImpl {
public:
    struct Trigonometric {
        f32 mCos; // at 0x0
        f32 mSin; // at 0x4
    };

    struct Vertex {
        math::VEC3 mVertex0; // at 0x0
        math::VEC3 mVertex1; // at 0xC
        f32 mTexCoord;       // at 0x18

        Vertex();

        Vertex(const math::VEC3& rVertex0, const math::VEC3& rVertex1,
               f32 texCoord)
            : mVertex0(rVertex0), mVertex1(rVertex1), mTexCoord(texCoord) {}
    };

    struct VertexTube {
        math::VEC3 mCenter; // at 0x0
        math::VEC3 mX;      // at 0xC
        math::VEC3 mZ;      // at 0x18
        f32 mTexCoord;      // at 0x24

        VertexTube() {}

        VertexTube(const math::VEC3& rCenter, const math::VEC3& rX,
                   const math::VEC3& rZ, f32 texCoord)
            : mCenter(rCenter), mX(rX), mZ(rZ), mTexCoord(texCoord) {}
    };

    struct AheadContextStripe : public AheadContext {
        math::VEC3 mEmitterAxisX;      // at 0xBC
        math::VEC3 mScreenAxisZ;       // at 0xC8
        Trigonometric* mTrigonometric; // at 0xD4

        AheadContextStripe(const math::MTX34& rViewMtx,
                           ParticleManager* pManager)
            : AheadContext(rViewMtx, pManager) {

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
    DrawSmoothStripeStrategy();

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

    int GetDrawOrder(const EmitterDrawSetting& rSetting) const {
        return rSetting.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER;
    }

    int GetStripeTexmapType(const EmitterDrawSetting& rSetting) const {
        return rSetting.typeOption2 & 0xC0;
    }

    void CalcVertex(Vertex* pVertex, AheadContextStripe* pContext, int flags,
                    Particle* pParticle, CalcAheadFuncStripe pCalcAheadFunc,
                    const math::VEC3& rPos, const math::VEC3& rP0,
                    const math::VEC3& rP1, f32 pivot, f32 texCoord);

    void CalcVertexTube(VertexTube* pVertex, AheadContextStripe* pContext,
                        int flags, Particle* pParticle,
                        CalcAheadFuncStripe pCalcAheadFunc, f32 pivotX,
                        f32 pivotY, f32 texCoord);

    void DrawSection_BSpline(int numDiv, const Vertex& rV0, const Vertex& rV1,
                             const Vertex& rV2, int flags);

    void DrawSectionEnd_BSpline(const Vertex& rV0, const Vertex& rV1,
                                const Vertex& rV2, int flags);

    void DrawSectionTube_BSpline(int numDiv, int numVertex,
                                 const VertexTube& rV0, const VertexTube& rV1,
                                 const VertexTube& rV2, int flags,
                                 const Trigonometric* pTrigonometric);

    void DrawSmoothStripe_BSpline_ConnectNone(AheadContextStripe* pContext,
                                              int numDiv, int flags,
                                              const math::VEC3& rP0,
                                              const math::VEC3& rP1);
    void DrawSmoothStripe_BSpline_ConnectRing(AheadContextStripe* pContext,
                                              int numDiv, int flags,
                                              const math::VEC3& rP0,
                                              const math::VEC3& rP1);
    void DrawSmoothStripe_BSpline_ConnectEmitter(AheadContextStripe* pContext,
                                                 int numDiv, int flags,
                                                 const math::VEC3& rP0,
                                                 const math::VEC3& rP1);

    void DrawSmoothStripe_BSpline(AheadContextStripe* pContext, int numDiv,
                                  int flags, const math::VEC3& rP0,
                                  const math::VEC3& rP1) {
        switch (pContext->mCommon.mParticleManager->mResource
                    ->GetEmitterDrawSetting()
                    ->typeOption2 &
                7) {
        case 0:
        default: {
            DrawSmoothStripe_BSpline_ConnectNone(pContext, numDiv, flags, rP0,
                                                 rP1);
            break;
        }

        case 1: {
            DrawSmoothStripe_BSpline_ConnectRing(pContext, numDiv, flags, rP0,
                                                 rP1);
            break;
        }

        case 2: {
            DrawSmoothStripe_BSpline_ConnectEmitter(pContext, numDiv, flags,
                                                    rP0, rP1);
            break;
        }
        }
    }

    void DrawSmoothStripeTube_BSpline_ConnectNone(AheadContextStripe* pContext,
                                                  int numDiv, int flags);
    void DrawSmoothStripeTube_BSpline_ConnectRing(AheadContextStripe* pContext,
                                                  int numDiv, int flags);
    void
    DrawSmoothStripeTube_BSpline_ConnectEmitter(AheadContextStripe* pContext,
                                                int numDiv, int flags);
    void DrawSmoothStripeTube(AheadContextStripe* pContext, int numDiv,
                              int flags);

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

static inline void CalcBSplineWeight(f32* pWeight, f32 t, f32 t2) {
    pWeight[0] = 0.5f + (0.5f * t2 - t);
    f32 weight1 = -1.0f * t2;
    pWeight[1] = 0.5f + (weight1 + t);
    pWeight[2] = 0.5f * t2;
}

DrawSmoothStripeStrategy::DrawSmoothStripeStrategy() {}

inline math::MTX34
DrawSmoothStripeStrategy::CalcAxisMtx(AheadContextStripe* pContext, int flags,
                                Particle* pParticle, const math::VEC3& rAxisY,
                                const math::VEC3& rPos) {
    math::VEC3 axisX;
    if (!(flags & 2)) {
        math::VEC3Cross(&axisX, &rAxisY, &pParticle->mPrevAxis);
    } else {
        math::VEC3Cross(&axisX, &rAxisY, &pContext->mScreenAxisZ);
    }

    if (!Normalize(&axisX)) {
        axisX = pContext->mEmitterAxisX;
    }

    math::VEC3 axisZ;
    math::VEC3Cross(&axisZ, &axisX, &rAxisY);
    Normalize(&axisZ);

    pParticle->mPrevAxis = axisZ;

    return math::MTX34(axisX.x, rAxisY.x, axisZ.x, rPos.x, axisX.y, rAxisY.y,
                        axisZ.y, rPos.y, axisX.z, rAxisY.z, axisZ.z, rPos.z);
}

inline math::MTX34 DrawSmoothStripeStrategy::CalcRotMtx(Particle* pParticle,
                              f32 pivot) {
    math::VEC3 rot;
    pParticle->Draw_GetRotate(&rot);

    f32 sx = pParticle->Draw_GetSizeX();
    f32 c = sx * math::CosRad(rot.y);
    f32 s = sx * math::SinRad(rot.y);

    return math::MTX34(c, 0.0f, -s, pivot - c * pivot, 0.0f, 1.0f, 0.0f,
                        0.0f, s, 0.0f, c, -s * pivot);
}

void DrawSmoothStripeStrategy::CalcVertex(Vertex* pVertex,
                                          AheadContextStripe* pContext,
                                          int flags, Particle* pParticle,
                                          CalcAheadFuncStripe pCalcAheadFunc,
                                          const math::VEC3& rPos,
                                          const math::VEC3& rP0,
                                          const math::VEC3& rP1, f32 pivot,
                                          f32 texCoord) {

    math::VEC3 axisY;
    pCalcAheadFunc(&axisY, pContext, pParticle);

    math::MTX34 mtx;
    math::MTX34 axisMtx = CalcAxisMtx(pContext, flags, pParticle, axisY, rPos);

    math::MTX34 rotMtx = CalcRotMtx(pParticle, pivot);

    math::MTX34Mult(&mtx, &axisMtx, &rotMtx);

    math::VEC3Transform(&pVertex->mVertex0, &mtx, &rP0);
    math::VEC3Transform(&pVertex->mVertex1, &mtx, &rP1);

    pVertex->mTexCoord = texCoord;
}

void DrawSmoothStripeStrategy::CalcVertexTube(VertexTube* pVertex,
                                        AheadContextStripe* pContext,
                                        int flags, Particle* pParticle,
                                        CalcAheadFuncStripe pCalcAheadFunc,
                                        f32 pivotX, f32 pivotY, f32 texCoord) {

    math::VEC3 axisY;
    math::VEC3 rot;

    pCalcAheadFunc(&axisY, pContext, pParticle);

    math::MTX34 axisMtx = CalcAxisMtx(pContext, flags, pParticle, axisY,
                                      pParticle->mParameter.mPosition);

    pParticle->Draw_GetRotate(&rot);

    f32 sx = pParticle->Draw_GetSizeX();
    f32 sy = pParticle->Draw_GetSizeY();

    f32 s, c;
    math::SinCosRad(&s, &c, rot.y);

    math::MTX34 rotMtx(c * sx, 0.0f, -s * sy,
                       pivotX - pivotX * (c * sx) + pivotY * (s * sy), 0.0f,
                       1.0f, 0.0f, 0.0f, s * sx, 0.0f, c * sy,
                       pivotY - pivotX * (s * sx) - pivotY * (c * sy));

    math::MTX34 mtx;
    math::MTX34Mult(&mtx, &axisMtx, &rotMtx);

    pVertex->mCenter = math::VEC3(mtx._03, mtx._13, mtx._23);

    math::VEC3TransformNormal(&pVertex->mX, &mtx, &GetXUnitVec());
    math::VEC3TransformNormal(&pVertex->mZ, &mtx, &GetZUnitVec());

    pVertex->mTexCoord = texCoord;
}

static inline void
CalcBSpline(DrawSmoothStripeStrategy::Vertex* pVertex, f32* pWeight,
            const DrawSmoothStripeStrategy::Vertex& rV0,
            const DrawSmoothStripeStrategy::Vertex& rV1,
            const DrawSmoothStripeStrategy::Vertex& rV2, f32 t, f32 t2) {
    CalcBSplineWeight(pWeight, t, t2);

    pVertex->mVertex0 = rV0.mVertex0 * pWeight[0] +
                        rV1.mVertex0 * pWeight[1] +
                        rV2.mVertex0 * pWeight[2];
    pVertex->mVertex1 = rV0.mVertex1 * pWeight[0] +
                        rV1.mVertex1 * pWeight[1] +
                        rV2.mVertex1 * pWeight[2];
    pVertex->mTexCoord = rV0.mTexCoord * pWeight[0] +
                         rV1.mTexCoord * pWeight[1] +
                         rV2.mTexCoord * pWeight[2];
}

static inline void DrawBSplineVertex(const math::VEC3& rVertex0,
                                     const math::VEC3& rVertex1, f32 texCoord,
                                     int flags) {
    GXPosition3f32(rVertex0.x, rVertex0.y, rVertex0.z);
    if (flags & 1) {
        GXTexCoord2f32(1.0f, texCoord);
    }

    GXPosition3f32(rVertex1.x, rVertex1.y, rVertex1.z);
    if (flags & 1) {
        GXTexCoord2f32(0.0f, texCoord);
    }
}

void DrawSmoothStripeStrategy::DrawSection_BSpline(int numDiv,
                                                   const Vertex& rV0,
                                                   const Vertex& rV1,
                                                   const Vertex& rV2,
                                                   int flags) {
    f32 step = 1.0f / numDiv;

    for (int i = 0; i < numDiv; i++) {
        Vertex vertex;
        f32 weight[3];

        f32 t = i * step;
        CalcBSpline(&vertex, weight, rV0, rV1, rV2, t, t * t);
        DrawBSplineVertex(vertex.mVertex0, vertex.mVertex1,
                          vertex.mTexCoord, flags);
    }
}

void DrawSmoothStripeStrategy::DrawSectionEnd_BSpline(const Vertex& rV0,
                                                      const Vertex& rV1,
                                                      const Vertex& rV2,
                                                      int flags) {
    Vertex vertex;
    f32 weight[3];

    CalcBSpline(&vertex, weight, rV0, rV1, rV2, 1.0f, 1.0f);
    DrawBSplineVertex(vertex.mVertex0, vertex.mVertex1,
                      vertex.mTexCoord, flags);
}

static inline math::VEC3 CalcBSplineWeight(f32 t) {
    f32 t2 = t * t;

    math::VEC3 weight;
    weight.x = 0.5f + (0.5f * t2 - t);
    f32 weight1 = -1.0f * t2;
    weight.y = 0.5f + (weight1 + t);
    weight.z = 0.5f * t2;
    return weight;
}

static inline DrawSmoothStripeStrategy::VertexTube
CalcBSplineTube(const math::VEC3& rWeight,
                const DrawSmoothStripeStrategy::VertexTube& rV0,
                const DrawSmoothStripeStrategy::VertexTube& rV1,
                const DrawSmoothStripeStrategy::VertexTube& rV2) {
    DrawSmoothStripeStrategy::VertexTube vertex;

    vertex.mCenter = rV0.mCenter * rWeight.x + rV1.mCenter * rWeight.y +
                     rV2.mCenter * rWeight.z;
    vertex.mX = rV0.mX * rWeight.x + rV1.mX * rWeight.y + rV2.mX * rWeight.z;
    vertex.mZ = rV0.mZ * rWeight.x + rV1.mZ * rWeight.y + rV2.mZ * rWeight.z;
    vertex.mTexCoord = rV0.mTexCoord * rWeight.x + rV1.mTexCoord * rWeight.y +
                       rV2.mTexCoord * rWeight.z;
    return vertex;
}

void DrawSmoothStripeStrategy::DrawSectionTube_BSpline(
    int numDiv, int numVertex, const VertexTube& rV0, const VertexTube& rV1,
    const VertexTube& rV2, int flags, const Trigonometric* pTrigonometric) {

    f32 step = 1.0f / numDiv;
    f32 texStep = 1.0f / numVertex;

    VertexTube prev;
    math::VEC3 weight = CalcBSplineWeight(0.0f);
    VertexTube vertex = CalcBSplineTube(weight, rV0, rV1, rV2);

    for (int i = 1; i <= numDiv; i++) {
        prev = vertex;
        f32 t = i * step;
        weight = CalcBSplineWeight(t);
        vertex = CalcBSplineTube(weight, rV0, rV1, rV2);

        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, numVertex * 2 + 2);

        if (flags & 4) {
            for (int j = 0; j <= numVertex; j++) {
                f32 sin = pTrigonometric[j].mSin;
                f32 cos = pTrigonometric[j].mCos;

                GXPosition(prev.mX * cos + prev.mZ * sin + prev.mCenter);
                if (flags & 1) {
                    GXTexCoord2f32(texStep * j, prev.mTexCoord);
                }

                GXPosition(vertex.mX * cos + vertex.mZ * sin + vertex.mCenter);
                if (flags & 1) {
                    GXTexCoord2f32(texStep * j, vertex.mTexCoord);
                }
            }
        } else {
            for (int j = 0; j <= numVertex; j++) {
                f32 sin = pTrigonometric[j].mSin;
                f32 cos = pTrigonometric[j].mCos;

                GXPosition(vertex.mX * cos + vertex.mZ * sin + vertex.mCenter);
                if (flags & 1) {
                    GXTexCoord2f32(texStep * j, vertex.mTexCoord);
                }

                GXPosition(prev.mX * cos + prev.mZ * sin + prev.mCenter);
                if (flags & 1) {
                    GXTexCoord2f32(texStep * j, prev.mTexCoord);
                }
            }
        }

        GXEnd();
    }
}

void DrawSmoothStripeStrategy::DrawSmoothStripe_BSpline_ConnectNone(
    AheadContextStripe* pContext, int numDiv, int flags, const math::VEC3& rP0,
    const math::VEC3& rP1) {

    ParticleManager* pManager = pContext->mCommon.mParticleManager;

    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    CalcAheadFuncStripe pCalcAheadFunc =
        reinterpret_cast<CalcAheadFuncStripe>(GetCalcAheadFunc(pManager));

    int drawOrder = GetDrawOrder(rSetting);

    GetFirstDrawParticleFunc pGetFirstFunc =
        GetGetFirstDrawParticleFunc(drawOrder);

    GetNextDrawParticleFunc pGetNextFunc = GetGetNextDrawParticleFunc(drawOrder);

    int numParticle = GetNumDrawParticle(pManager);
    f32 pivot = 0.01f * rSetting.pivotX;

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (numParticle * numDiv + 1) * 2);

    f32 texStep;
    switch (GetStripeTexmapType(rSetting)) {
    case 0:
    default: {
        texStep = 1.0f / (numParticle - 1);
        break;
    }

    case 0x40: {
        texStep = 1.0f;
        break;
    }
    }

    int index = 0;
    int step = 1;
    if (drawOrder != 0) {
        index = numParticle - 1;
        step = -1;
    }

    Vertex vertex[3];
    int prev2;
    int prev = 0;
    int cur = 0;

    Particle* pParticle = pGetFirstFunc(pManager);

    CalcVertex(&vertex[0], pContext, flags, pParticle, pCalcAheadFunc,
               pParticle->mParameter.mPosition, rP0, rP1, pivot,
               texStep * index);
    index += step;

    pParticle = pGetNextFunc(pManager, pParticle);
    while (pParticle != NULL) {
        prev2 = prev;
        prev = cur;
        cur = (cur + 1) % 3;

        CalcVertex(&vertex[cur], pContext, flags, pParticle, pCalcAheadFunc,
                   pParticle->mParameter.mPosition, rP0, rP1, pivot,
                   texStep * index);

        DrawSection_BSpline(numDiv, vertex[prev2], vertex[prev], vertex[cur],
                            flags);

        pParticle = pGetNextFunc(pManager, pParticle);
        index += step;
    }

    DrawSection_BSpline(numDiv, vertex[prev], vertex[cur], vertex[cur], flags);
    DrawSectionEnd_BSpline(vertex[prev], vertex[cur], vertex[cur], flags);

    GXEnd();
}

inline DrawSmoothStripeStrategy::Vertex::Vertex() {}

void DrawSmoothStripeStrategy::DrawSmoothStripe_BSpline_ConnectRing(
    AheadContextStripe* pContext, int numDiv, int flags, const math::VEC3& rP0,
    const math::VEC3& rP1) {

    ParticleManager* pManager = pContext->mCommon.mParticleManager;

    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    CalcAheadFuncStripe pCalcAheadFunc =
        reinterpret_cast<CalcAheadFuncStripe>(GetCalcAheadFunc(pManager));

    int drawOrder = GetDrawOrder(rSetting);

    GetFirstDrawParticleFunc pGetFirstFunc =
        GetGetFirstDrawParticleFunc(drawOrder);

    GetNextDrawParticleFunc pGetNextFunc = GetGetNextDrawParticleFunc(drawOrder);

    int numParticle = GetNumDrawParticle(pManager);
    f32 pivot = 0.01f * rSetting.pivotX;

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (numParticle * numDiv + 1) * 2);

    f32 texStep;
    switch (GetStripeTexmapType(rSetting)) {
    case 0:
    default: {
        texStep = 0.5f / numParticle;
        break;
    }

    case 0x40: {
        texStep = 0.5f;
        break;
    }
    }

    int index = -1;
    int step = 2;
    if (drawOrder != 0) {
        index = numParticle * 2 + 1;
        step = -2;
    }

    Vertex vertex[5];
    math::VEC3 unused0;
    math::VEC3 unused1;

    Particle* pParticle = pGetFirstFunc(pManager);

    CalcVertex(&vertex[3], pContext, flags, pParticle, pCalcAheadFunc,
               pParticle->mParameter.mPosition, rP0, rP1, pivot,
               texStep * index);

    pParticle = pGetNextFunc(pManager, pParticle);
    index += step;

    CalcVertex(&vertex[4], pContext, flags, pParticle, pCalcAheadFunc,
               pParticle->mParameter.mPosition, rP0, rP1, pivot,
               texStep * index);

    pParticle = pGetNextFunc(pManager, pParticle);
    index += step;

    int prev2;
    int prev = 3;
    int cur = 4;

    while (pParticle != NULL) {
        prev2 = prev;
        prev = cur;
        cur = (cur + 1) % 3;

        CalcVertex(&vertex[cur], pContext, flags, pParticle, pCalcAheadFunc,
                   pParticle->mParameter.mPosition, rP0, rP1, pivot,
                   texStep * index);

        DrawSection_BSpline(numDiv, vertex[prev2], vertex[prev], vertex[cur],
                            flags);

        pParticle = pGetNextFunc(pManager, pParticle);
        index += step;
    }

    vertex[3].mTexCoord = texStep * index;
    DrawSection_BSpline(numDiv, vertex[prev], vertex[cur], vertex[3], flags);

    index += step;
    vertex[4].mTexCoord = texStep * index;
    DrawSection_BSpline(numDiv, vertex[cur], vertex[3], vertex[4], flags);

    DrawSectionEnd_BSpline(vertex[cur], vertex[3], vertex[4], flags);

    GXEnd();
}

void DrawSmoothStripeStrategy::DrawSmoothStripe_BSpline_ConnectEmitter(
    AheadContextStripe* pContext, int numDiv, int flags, const math::VEC3& rP0,
    const math::VEC3& rP1) {

    ParticleManager* pManager = pContext->mCommon.mParticleManager;

    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    CalcAheadFuncStripe pCalcAheadFunc =
        reinterpret_cast<CalcAheadFuncStripe>(GetCalcAheadFunc(pManager));

    int drawOrder = GetDrawOrder(rSetting);

    GetFirstDrawParticleFunc pGetFirstFunc =
        GetGetFirstDrawParticleFunc(drawOrder);

    GetNextDrawParticleFunc pGetNextFunc = GetGetNextDrawParticleFunc(drawOrder);

    f32 pivot = 0.01f * rSetting.pivotX;

    Particle* pYoungest =
        GetYoungestDrawParticle(pContext->mCommon.mParticleManager);

    math::VEC3 offset =
        pContext->mCommon.mEmitterCenter - pYoungest->mParameter.mPosition;

    int numParticle = GetNumDrawParticle(pManager);

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0,
            ((numParticle + 1) * numDiv + 1) * 2);

    f32 texStep;
    switch (GetStripeTexmapType(rSetting)) {
    case 0:
    default: {
        texStep = 1.0f / numParticle;
        break;
    }

    case 0x40: {
        texStep = 1.0f;
        break;
    }
    }

    int index = 0;
    int step = 1;
    if (drawOrder != 0) {
        index = numParticle;
        step = -1;
    }

    Vertex vertex[3];
    int prev2;
    int prev = 0;
    int cur = 0;
    math::VEC3 unused0;
    math::VEC3 unused1;

    Particle* pParticle = pGetFirstFunc(pManager);

    CalcVertex(&vertex[0], pContext, flags, pParticle, pCalcAheadFunc,
               pParticle->mParameter.mPosition, rP0, rP1, pivot,
               texStep * index);
    index += step;

    if (drawOrder == 0) {
        cur = 1;
        vertex[1] = vertex[0];
        vertex[1].mTexCoord = texStep * index;
        vertex[0].mVertex0 += offset;
        vertex[0].mVertex1 += offset;
        index += step;

        DrawSection_BSpline(numDiv, vertex[0], vertex[0], vertex[1], flags);
    }

    pParticle = pGetNextFunc(pManager, pParticle);
    while (pParticle != NULL) {
        prev2 = prev;
        prev = cur;
        cur = (cur + 1) % 3;

        CalcVertex(&vertex[cur], pContext, flags, pParticle, pCalcAheadFunc,
                   pParticle->mParameter.mPosition, rP0, rP1, pivot,
                   texStep * index);

        DrawSection_BSpline(numDiv, vertex[prev2], vertex[prev], vertex[cur],
                            flags);

        pParticle = pGetNextFunc(pManager, pParticle);
        index += step;
    }

    if (drawOrder != 0) {
        prev2 = prev;
        prev = cur;
        cur = (cur + 1) % 3;

        vertex[cur] = Vertex(vertex[prev].mVertex0 + offset,
                             vertex[prev].mVertex1 + offset, texStep * index);

        DrawSection_BSpline(numDiv, vertex[prev2], vertex[prev], vertex[cur],
                            flags);
    }

    DrawSection_BSpline(numDiv, vertex[prev], vertex[cur], vertex[cur], flags);
    DrawSectionEnd_BSpline(vertex[prev], vertex[cur], vertex[cur], flags);

    GXEnd();
}

void DrawSmoothStripeStrategy::DrawSmoothStripeTube_BSpline_ConnectNone(
    AheadContextStripe* pContext, int numDiv, int flags) {

    ParticleManager* pManager = pContext->mCommon.mParticleManager;

    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    CalcAheadFuncStripe pCalcAheadFunc =
        reinterpret_cast<CalcAheadFuncStripe>(GetCalcAheadFunc(pManager));

    int drawOrder = GetDrawOrder(rSetting);

    GetFirstDrawParticleFunc pGetFirstFunc =
        GetGetFirstDrawParticleFunc(drawOrder);

    GetNextDrawParticleFunc pGetNextFunc = GetGetNextDrawParticleFunc(drawOrder);

    f32 pivotX = 0.01f * rSetting.pivotX;
    f32 pivotY = 0.01f * rSetting.pivotY;

    int numParticle = GetNumDrawParticle(pManager);

    f32 texStep;
    switch (GetStripeTexmapType(rSetting)) {
    case 0:
    default: {
        texStep = 1.0f / (numParticle - 1);
        break;
    }

    case 0x40: {
        texStep = 1.0f;
        break;
    }
    }

    int index = 0;
    int step = 1;
    if (drawOrder != 0) {
        index = numParticle - 1;
        step = -1;
    }

    VertexTube vertex[3];
    int prev2;
    int prev = 0;
    int cur = 0;

    Particle* pParticle = pGetFirstFunc(pManager);

    CalcVertexTube(&vertex[0], pContext, flags, pParticle, pCalcAheadFunc,
                   pivotX, pivotY, texStep * index);
    index += step;

    pParticle = pGetNextFunc(pManager, pParticle);
    while (pParticle != NULL) {
        prev2 = prev;
        prev = cur;
        cur = (cur + 1) % 3;

        CalcVertexTube(&vertex[cur], pContext, flags, pParticle,
                       pCalcAheadFunc, pivotX, pivotY, texStep * index);

        DrawSectionTube_BSpline(numDiv, rSetting.typeOption0, vertex[prev2],
                                vertex[prev], vertex[cur], flags,
                                pContext->mTrigonometric);

        pParticle = pGetNextFunc(pManager, pParticle);
        index += step;
    }

    DrawSectionTube_BSpline(numDiv, rSetting.typeOption0, vertex[prev],
                            vertex[cur], vertex[cur], flags,
                            pContext->mTrigonometric);
}

void DrawSmoothStripeStrategy::DrawSmoothStripeTube_BSpline_ConnectRing(
    AheadContextStripe* pContext, int numDiv, int flags) {

    ParticleManager* pManager = pContext->mCommon.mParticleManager;

    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    CalcAheadFuncStripe pCalcAheadFunc =
        reinterpret_cast<CalcAheadFuncStripe>(GetCalcAheadFunc(pManager));

    int drawOrder = GetDrawOrder(rSetting);

    GetFirstDrawParticleFunc pGetFirstFunc =
        GetGetFirstDrawParticleFunc(drawOrder);

    GetNextDrawParticleFunc pGetNextFunc = GetGetNextDrawParticleFunc(drawOrder);

    f32 pivotX = 0.01f * rSetting.pivotX;
    f32 pivotY = 0.01f * rSetting.pivotY;

    int numParticle = GetNumDrawParticle(pManager);

    f32 texStep;
    switch (GetStripeTexmapType(rSetting)) {
    case 0:
    default: {
        texStep = 0.5f / numParticle;
        break;
    }

    case 0x40: {
        texStep = 0.5f;
        break;
    }
    }

    int index = -1;
    int step = 2;
    if (drawOrder != 0) {
        index = numParticle * 2 + 1;
        step = -2;
    }

    VertexTube vertex[5];

    Particle* pParticle = pGetFirstFunc(pManager);

    CalcVertexTube(&vertex[3], pContext, flags, pParticle, pCalcAheadFunc,
                   pivotX, pivotY, texStep * index);

    pParticle = pGetNextFunc(pManager, pParticle);
    index += step;

    CalcVertexTube(&vertex[4], pContext, flags, pParticle, pCalcAheadFunc,
                   pivotX, pivotY, texStep * index);

    pParticle = pGetNextFunc(pManager, pParticle);
    index += step;

    int prev2;
    int prev = 3;
    int cur = 4;

    while (pParticle != NULL) {
        prev2 = prev;
        prev = cur;
        cur = (cur + 1) % 3;

        CalcVertexTube(&vertex[cur], pContext, flags, pParticle,
                       pCalcAheadFunc, pivotX, pivotY, texStep * index);

        DrawSectionTube_BSpline(numDiv, rSetting.typeOption0, vertex[prev2],
                                vertex[prev], vertex[cur], flags,
                                pContext->mTrigonometric);

        pParticle = pGetNextFunc(pManager, pParticle);
        index += step;
    }

    vertex[3].mTexCoord = texStep * index;
    DrawSectionTube_BSpline(numDiv, rSetting.typeOption0, vertex[prev],
                            vertex[cur], vertex[3], flags,
                            pContext->mTrigonometric);

    index += step;
    vertex[4].mTexCoord = texStep * index;
    DrawSectionTube_BSpline(numDiv, rSetting.typeOption0, vertex[cur],
                            vertex[3], vertex[4], flags,
                            pContext->mTrigonometric);
}

void DrawSmoothStripeStrategy::DrawSmoothStripeTube_BSpline_ConnectEmitter(
    AheadContextStripe* pContext, int numDiv, int flags) {

    ParticleManager* pManager = pContext->mCommon.mParticleManager;

    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    CalcAheadFuncStripe pCalcAheadFunc =
        reinterpret_cast<CalcAheadFuncStripe>(GetCalcAheadFunc(pManager));

    int drawOrder = GetDrawOrder(rSetting);

    GetFirstDrawParticleFunc pGetFirstFunc =
        GetGetFirstDrawParticleFunc(drawOrder);

    GetNextDrawParticleFunc pGetNextFunc = GetGetNextDrawParticleFunc(drawOrder);

    f32 pivotX = 0.01f * rSetting.pivotX;
    f32 pivotY = 0.01f * rSetting.pivotY;

    math::VEC3 offset =
        pContext->mCommon.mEmitterCenter -
        GetYoungestDrawParticle(pContext->mCommon.mParticleManager)
            ->mParameter.mPosition;

    int numParticle = GetNumDrawParticle(pManager);

    f32 texStep;
    switch (GetStripeTexmapType(rSetting)) {
    case 0:
    default: {
        texStep = 1.0f / numParticle;
        break;
    }

    case 0x40: {
        texStep = 1.0f;
        break;
    }
    }

    int index = 0;
    int step = 1;
    if (drawOrder != 0) {
        index = numParticle;
        step = -1;
    }

    VertexTube vertex[3];
    int prev2;
    int prev = 0;
    int cur = 0;

    Particle* pParticle = pGetFirstFunc(pManager);

    CalcVertexTube(&vertex[0], pContext, flags, pParticle, pCalcAheadFunc,
                   pivotX, pivotY, texStep * index);
    index += step;

    if (drawOrder == 0) {
        cur = 1;
        vertex[1] = VertexTube(vertex[0].mCenter, vertex[0].mX, vertex[0].mZ,
                               texStep * index);
        vertex[0].mCenter += offset;
        index += step;

        DrawSectionTube_BSpline(numDiv, rSetting.typeOption0, vertex[0],
                                vertex[0], vertex[1], flags,
                                pContext->mTrigonometric);
    }

    pParticle = pGetNextFunc(pManager, pParticle);
    while (pParticle != NULL) {
        prev2 = prev;
        prev = cur;
        cur = (cur + 1) % 3;

        CalcVertexTube(&vertex[cur], pContext, flags, pParticle,
                       pCalcAheadFunc, pivotX, pivotY, texStep * index);

        DrawSectionTube_BSpline(numDiv, rSetting.typeOption0, vertex[prev2],
                                vertex[prev], vertex[cur], flags,
                                pContext->mTrigonometric);

        pParticle = pGetNextFunc(pManager, pParticle);
        index += step;
    }

    if (drawOrder != 0) {
        prev2 = prev;
        prev = cur;
        cur = (cur + 1) % 3;

        vertex[cur] = VertexTube(vertex[prev].mCenter + offset,
                                 vertex[prev].mX, vertex[prev].mZ,
                                 texStep * index);

        DrawSectionTube_BSpline(numDiv, rSetting.typeOption0, vertex[prev2],
                                vertex[prev], vertex[cur], flags,
                                pContext->mTrigonometric);
    }

    DrawSectionTube_BSpline(numDiv, rSetting.typeOption0, vertex[prev],
                            vertex[cur], vertex[cur], flags,
                            pContext->mTrigonometric);
}

void DrawSmoothStripeStrategy::DrawSmoothStripeTube(
    AheadContextStripe* pContext, int numDiv, int flags) {

    const EmitterDrawSetting& rSetting =
        *pContext->mCommon.mParticleManager->mResource->GetEmitterDrawSetting();

    int numVertex = rSetting.typeOption0;
    if (numVertex < 3) {
        return;
    }

    Trigonometric* pIt;
    Trigonometric* pTrigonometric = static_cast<Trigonometric*>(
        __alloca(sizeof(Trigonometric) * (numVertex + 1)));

    if (pTrigonometric == NULL) {
        return;
    }

    f32 step = 360.0f / numVertex;

    pTrigonometric[0].mCos = 1.0f;
    pTrigonometric[0].mSin = 0.0f;

    pIt = &pTrigonometric[1];
    for (int i = 1; i < numVertex; pIt++, i++) {
        math::SinCosDeg(&pIt->mSin, &pIt->mCos, step * i);
    }

    pTrigonometric[numVertex].mCos = 1.0f;
    pTrigonometric[numVertex].mSin = 0.0f;

    pContext->mTrigonometric = pTrigonometric;

    switch (rSetting.typeOption2 & 7) {
    case 0:
    default: {
        GXSetCullMode(GX_CULL_FRONT);
        DrawSmoothStripeTube_BSpline_ConnectNone(pContext, numDiv, flags);
        GXSetCullMode(GX_CULL_BACK);
        DrawSmoothStripeTube_BSpline_ConnectNone(pContext, numDiv, flags);
        break;
    }

    case 1: {
        GXSetCullMode(GX_CULL_FRONT);
        DrawSmoothStripeTube_BSpline_ConnectRing(pContext, numDiv, flags);
        GXSetCullMode(GX_CULL_BACK);
        DrawSmoothStripeTube_BSpline_ConnectRing(pContext, numDiv, flags);
        break;
    }

    case 2: {
        GXSetCullMode(GX_CULL_FRONT);
        DrawSmoothStripeTube_BSpline_ConnectEmitter(pContext, numDiv, flags);
        GXSetCullMode(GX_CULL_BACK);
        DrawSmoothStripeTube_BSpline_ConnectEmitter(pContext, numDiv, flags);
        break;
    }
    }
}

void DrawSmoothStripeStrategy::Draw(const DrawInfo& rInfo,
                              ParticleManager* pManager) {

    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    int numParticle = GetNumDrawParticle(pManager);

    if (numParticle == 0) {
        return;
    }

    AheadContextStripe context(*rInfo.GetViewMtx(), pManager);

    math::VEC3 initAxis = GetInitialPrevAxis(rSetting, context);

    for (Particle* pParticle = GetYoungestParticle(pManager);
         pParticle != NULL; pParticle = GetElderParticle(pManager, pParticle)) {

        if (pParticle->mPrevAxis.x <= 1.0f) {
            break;
        }

        pParticle->mPrevAxis = initAxis;
    }

    int drawOrder = rSetting.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER;

    int flags =
        (rSetting.typeOption == EmitterDrawSetting::ASSIST_ST_BILLBOARD ? 2
                                                                         : 0) |
        (drawOrder ? 4 : 0);

    int connect = rSetting.typeOption2 & 7;

    if ((connect == 1 && numParticle < 3) ||
        (connect == 0 && numParticle < 2)) {

        CalcAheadFuncStripe pCalcAheadFunc =
            reinterpret_cast<CalcAheadFuncStripe>(GetCalcAheadFunc(pManager));

        GetFirstDrawParticleFunc pGetFirstFunc =
            GetGetFirstDrawParticleFunc(drawOrder);

        GetNextDrawParticleFunc pGetNextFunc =
            GetGetNextDrawParticleFunc(drawOrder);

        for (Particle* pParticle = pGetFirstFunc(pManager); pParticle != NULL;
             pParticle = pGetNextFunc(pManager, pParticle)) {

            math::VEC3 axisY;
            pCalcAheadFunc(&axisY, &context, pParticle);

            CalcAxisMtx(&context, flags, pParticle, axisY,
                        pParticle->mParameter.mPosition);
        }

        return;
    }

    InitGraphics(rInfo, pManager);

    flags |= mNumTexmap > 0 ? 1 : 0;

    math::MTX34 posMtx;
    math::MTX34Mult(&posMtx, rInfo.GetViewMtx(),
                    &context.mCommon.mParticleManagerMtx);
    GXLoadPosMtxImm(posMtx, GX_PNMTX0);

    if (flags & 2) {
        math::MTX34 invMtx;

        if (math::MTX34Inv(&invMtx, &posMtx)) {
            context.mScreenAxisZ =
                math::VEC3(invMtx._02, invMtx._12, invMtx._22);
        } else {
            context.mScreenAxisZ = math::VEC3(0.0f, 0.0f, 1.0f);
        }
    }

    SetupGP(GetGetFirstDrawParticleFunc(rSetting.mFlags &
                                        EmitterDrawSetting::FLAG_DRAW_ORDER)(
                pManager),
            rSetting, rInfo, true, false);

    int numDiv = rSetting.typeOption1;
    if (numDiv == 0) {
        numDiv = 1;
    }

    static const math::VEC3 sStripeP0(1.0f, 0.0f, 0.0f);
    static const math::VEC3 sStripeP1(-1.0f, 0.0f, 0.0f);
    static const math::VEC3 sCrossP0(0.0f, 0.0f, 1.0f);
    static const math::VEC3 sCrossP1(0.0f, 0.0f, -1.0f);

    switch (rSetting.typeOption) {
    default: {
        DrawSmoothStripe_BSpline(&context, numDiv, flags, sStripeP0,
                                 sStripeP1);

        if (rSetting.typeOption == EmitterDrawSetting::ASSIST_ST_CROSS) {
            DrawSmoothStripe_BSpline(&context, numDiv, flags, sCrossP0,
                                     sCrossP1);
        }
        break;
    }

    case EmitterDrawSetting::ASSIST_ST_TUBE: {
        DrawSmoothStripeTube(&context, numDiv, flags);
        break;
    }
    }
}

math::VEC3
DrawSmoothStripeStrategy::GetInitialPrevAxis(const EmitterDrawSetting& rSetting,
                                       const AheadContextStripe& rContext) {
    math::VEC3 axis;

    switch (rSetting.typeOption2 & 0x38) {
    case 0x08: {
        math::VEC3TransformNormal(&axis, &rContext.mCommon.mEmitterMtx,
                                  &GetXUnitVec());
        math::VEC3TransformNormal(&axis, &rContext.mCommon.mParticleManagerMtxInv,
                                  &axis);
        break;
    }

    case 0x00:
    default: {
        axis = rContext.mCommon.mEmitterAxisY;
        return axis;
    }

    case 0x10: {
        math::VEC3TransformNormal(&axis, &rContext.mCommon.mEmitterMtx,
                                  &GetZUnitVec());
        math::VEC3TransformNormal(&axis, &rContext.mCommon.mParticleManagerMtxInv,
                                  &axis);
        break;
    }

    case 0x18: {
        math::VEC3 one(1.0f, 1.0f, 1.0f);
        math::VEC3TransformNormal(&axis, &rContext.mCommon.mEmitterMtx, &one);
        math::VEC3TransformNormal(&axis, &rContext.mCommon.mParticleManagerMtxInv,
                                  &axis);
        break;
    }
    }

    if (!Normalize(&axis)) {
        axis = rContext.mCommon.mEmitterAxisY;
    }

    return axis;
}

DrawStrategyImpl::CalcAheadFunc
DrawSmoothStripeStrategy::GetCalcAheadFunc(ParticleManager* pManager) {
    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    switch (rSetting.typeDir) {
    case EmitterDrawSetting::AHEAD_BB_SPEED: {
        return CalcAhead_Speed;
    }

    case EmitterDrawSetting::AHEAD_BB_EMITTER_CENTER: {
        return CalcAhead_EmitterCenter;
    }

    case EmitterDrawSetting::AHEAD_BB_EMITTER_DESIGN: {
        return CalcAhead_EmitterDesign;
    }

    case EmitterDrawSetting::AHEAD_BB_PARTICLE: {
        return reinterpret_cast<CalcAheadFunc>(CalcAhead_Particle_Stripe);
    }

    case EmitterDrawSetting::AHEAD_CMN_NODESIGN: {
        return CalcAhead_NoDesign;
    }

    case 6: {
        switch (rSetting.typeOption2 & 7) {
        case 1: {
            return reinterpret_cast<CalcAheadFunc>(CalcAhead_ParticleBoth_Ring);
        }

        case 2: {
            return reinterpret_cast<CalcAheadFunc>(
                CalcAhead_ParticleBoth_Origin);
        }

        default: {
            return reinterpret_cast<CalcAheadFunc>(
                CalcAhead_ParticleBoth_Stripe);
        }
        }
    }

    default: {
        return CalcAhead_Speed;
    }
    }
}

void DrawSmoothStripeStrategy::CalcAhead_Particle_Stripe(math::VEC3* pAxisY,
                                                   AheadContextStripe* pContext,
                                                   Particle* pParticle) {

    Particle* pYounger =
        GetYoungerDrawParticle(pContext->mCommon.mParticleManager, pParticle);

    if (pYounger != NULL) {
        math::VEC3Sub(pAxisY, &pParticle->mParameter.mPosition,
                      &pYounger->mParameter.mPosition);
    } else {
        math::VEC3Sub(pAxisY, &pParticle->mParameter.mPosition,
                      &pContext->mCommon.mEmitterCenter);
    }

    if (!Normalize(pAxisY)) {
        *pAxisY = pContext->mCommon.mEmitterAxisY;
    }
}

void DrawSmoothStripeStrategy::CalcAhead_ParticleBoth_Stripe(
    math::VEC3* pAxisY, AheadContextStripe* pContext, Particle* pParticle) {

    Particle* pElder =
        GetElderDrawParticle(pContext->mCommon.mParticleManager, pParticle);

    Particle* pYounger =
        GetYoungerDrawParticle(pContext->mCommon.mParticleManager, pParticle);

    math::VEC3 elderDir(0.0f, 0.0f, 0.0f);

    if (pElder != NULL) {
        math::VEC3Sub(&elderDir, &pElder->mParameter.mPosition,
                      &pParticle->mParameter.mPosition);

        if (!Normalize(&elderDir)) {
            elderDir = math::VEC3(0.0f, 0.0f, 0.0f);
        }
    }

    math::VEC3 youngerDir(0.0f, 0.0f, 0.0f);

    if (pYounger != NULL) {
        math::VEC3Sub(&youngerDir, &pYounger->mParameter.mPosition,
                      &pParticle->mParameter.mPosition);

        if (!Normalize(&youngerDir)) {
            youngerDir = math::VEC3(0.0f, 0.0f, 0.0f);
        }
    }

    math::VEC3Sub(pAxisY, &elderDir, &youngerDir);

    if (!Normalize(pAxisY)) {
        *pAxisY = pContext->mCommon.mEmitterAxisY;
    }
}

void DrawSmoothStripeStrategy::CalcAhead_ParticleBoth_Ring(
    math::VEC3* pAxisY, AheadContextStripe* pContext, Particle* pParticle) {

    Particle* pElder =
        GetElderDrawParticle(pContext->mCommon.mParticleManager, pParticle);

    if (pElder == NULL) {
        pElder = GetYoungestDrawParticle(pContext->mCommon.mParticleManager);
    }

    Particle* pYounger =
        GetYoungerDrawParticle(pContext->mCommon.mParticleManager, pParticle);

    if (pYounger == NULL) {
        pYounger = GetOldestDrawParticle(pContext->mCommon.mParticleManager);
    }

    math::VEC3 elderDir;
    math::VEC3Sub(&elderDir, &pElder->mParameter.mPosition,
                  &pParticle->mParameter.mPosition);

    if (!Normalize(&elderDir)) {
        elderDir = math::VEC3(0.0f, 0.0f, 0.0f);
    }

    math::VEC3 youngerDir;
    math::VEC3Sub(&youngerDir, &pYounger->mParameter.mPosition,
                  &pParticle->mParameter.mPosition);

    if (!Normalize(&youngerDir)) {
        youngerDir = math::VEC3(0.0f, 0.0f, 0.0f);
    }

    math::VEC3Sub(pAxisY, &elderDir, &youngerDir);

    if (!Normalize(pAxisY)) {
        *pAxisY = pContext->mCommon.mEmitterAxisY;
    }
}

void DrawSmoothStripeStrategy::CalcAhead_ParticleBoth_Origin(
    math::VEC3* pAxisY, AheadContextStripe* pContext, Particle* pParticle) {

    Particle* pElder =
        GetElderDrawParticle(pContext->mCommon.mParticleManager, pParticle);

    Particle* pYounger =
        GetYoungerDrawParticle(pContext->mCommon.mParticleManager, pParticle);

    math::VEC3 elderDir(0.0f, 0.0f, 0.0f);

    if (pElder != NULL) {
        math::VEC3Sub(&elderDir, &pElder->mParameter.mPosition,
                      &pParticle->mParameter.mPosition);
        Normalize(&elderDir);
    }

    math::VEC3 youngerDir;

    if (pYounger != NULL) {
        math::VEC3Sub(&youngerDir, &pYounger->mParameter.mPosition,
                      &pParticle->mParameter.mPosition);
    } else {
        math::VEC3Sub(&youngerDir, &pContext->mCommon.mEmitterCenter,
                      &pParticle->mParameter.mPosition);
    }

    Normalize(&youngerDir);

    math::VEC3Sub(pAxisY, &elderDir, &youngerDir);

    if (!Normalize(pAxisY)) {
        *pAxisY = pContext->mCommon.mEmitterAxisY;
    }
}

} // namespace ef
} // namespace nw4r
