#include <nw4r/ef/drawstrategy/ef_drawstripestrategy.h>
#include <nw4r/ef/drawstrategy/ef_drawstrategyimpl.h>
#include <nw4r/ef/ef_drawinfo.h>
#include <nw4r/ef/ef_particle.h>
#include <nw4r/ef/ef_particlemanager.h>
#include <nw4r/ef/ef_res_emitter.h>
#include <nw4r/math.h>

#include <revolution/GX.h>

namespace nw4r {
namespace ef {

DrawStripeStrategy::DrawStripeStrategy() {}

inline math::MTX34
DrawStripeStrategy::CalcAxisMtx(AheadContextStripe* pContext, int flags,
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

inline math::MTX34 DrawStripeStrategy::CalcRotMtx(Particle* pParticle,
                              f32 pivot) {
    math::VEC3 rot;
    pParticle->Draw_GetRotate(&rot);

    f32 sx = pParticle->Draw_GetSizeX();
    f32 c = sx * math::CosRad(rot.y);
    f32 s = sx * math::SinRad(rot.y);

    return math::MTX34(c, 0.0f, -s, pivot - c * pivot, 0.0f, 1.0f, 0.0f,
                        0.0f, s, 0.0f, c, -s * pivot);
}

inline void DrawStripeStrategy::DrawParticle(AheadContextStripe* pContext, int flags,
                                      Particle* pParticle,
                                      CalcAheadFuncStripe pCalcAheadFunc,
                                      const math::VEC3& rPos,
                                      const math::VEC3& rP0,
                                      const math::VEC3& rP1, f32 pivot,
                                      f32 texCoord) {

    if (pParticle == NULL) {
        pParticle =
            GetYoungestDrawParticle(pContext->mCommon.mParticleManager);
    }

    math::VEC3 axisY;
    pCalcAheadFunc(&axisY, pContext, pParticle);

    math::MTX34 mtx;
    math::MTX34 axisMtx =
        CalcAxisMtx(pContext, flags, pParticle, axisY, rPos);

    math::MTX34 rotMtx = CalcRotMtx(pParticle, pivot);

    math::MTX34Mult(&mtx, &axisMtx, &rotMtx);

    math::VEC3 p0;
    math::VEC3Transform(&p0, &mtx, &rP0);

    math::VEC3 p1;
    math::VEC3Transform(&p1, &mtx, &rP1);

    GXPosition3f32(p0.x, p0.y, p0.z);
    if (flags & 1) {
        GXTexCoord2f32(1.0f, texCoord);
    }

    GXPosition3f32(p1.x, p1.y, p1.z);
    if (flags & 1) {
        GXTexCoord2f32(0.0f, texCoord);
    }
}

void DrawStripeStrategy::DrawStripe(AheadContextStripe* pContext, int flags,
                                    const math::VEC3& rP0,
                                    const math::VEC3& rP1) {

    ParticleManager* pManager = pContext->mCommon.mParticleManager;

    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    CalcAheadFuncStripe pCalcAheadFunc =
        reinterpret_cast<CalcAheadFuncStripe>(GetCalcAheadFunc(pManager));

    int drawOrder = rSetting.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER;

    GetFirstDrawParticleFunc pGetFirstFunc =
        GetGetFirstDrawParticleFunc(drawOrder);

    GetNextDrawParticleFunc pGetNextFunc = GetGetNextDrawParticleFunc(drawOrder);

    int numParticle = GetNumDrawParticle(pManager);
    f32 pivot = 0.01f * rSetting.pivotX;

    bool connectRing = (rSetting.typeOption2 & 7) == 1;
    bool connectEmitter = (rSetting.typeOption2 & 7) == 2;

    if (connectRing || connectEmitter) {
        numParticle++;
    }

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, numParticle * 2);

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

    int index;
    int step;
    switch (rSetting.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER) {
    case DRAWORDER_YOUNGERS_FIRST: {
        index = 0;
        step = 1;
        break;
    }

    case DRAWORDER_ELDERS_FIRST:
    default: {
        index = numParticle - 1;
        step = -1;
        break;
    }
    }

    if (connectEmitter && drawOrder == 0) {
        DrawParticle(pContext, flags, NULL, pCalcAheadFunc,
                     pContext->mCommon.mEmitterCenter, rP0, rP1, pivot,
                     texStep * index);
        index += step;
    }

    Particle* pParticle = pGetFirstFunc(pManager);
    while (pParticle != NULL) {
        DrawParticle(pContext, flags, pParticle, pCalcAheadFunc,
                     pParticle->mParameter.mPosition, rP0, rP1, pivot,
                     texStep * index);

        pParticle = pGetNextFunc(pManager, pParticle);
        index += step;
    }

    if (connectRing) {
        pParticle = pGetFirstFunc(pManager);

        DrawParticle(pContext, flags, pParticle, pCalcAheadFunc,
                     pParticle->mParameter.mPosition, rP0, rP1, pivot,
                     texStep * index);
    }

    if (connectEmitter && drawOrder != 0) {
        DrawParticle(pContext, flags, NULL, pCalcAheadFunc,
                     pContext->mCommon.mEmitterCenter, rP0, rP1, pivot,
                     texStep * index);
    }

    GXEnd();
}

void DrawStripeStrategy::CalcVertexTube(VertexTube* pVertex,
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

void DrawStripeStrategy::DrawTubeSegment(const VertexTube& rV0,
                                         const VertexTube& rV1, int flags,
                                         int numVertex,
                                         const Trigonometric* pTrigonometric) {

    f32 texStep = 1.0f / numVertex;

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, numVertex * 2 + 2);

    if (flags & 4) {
        for (int i = 0; i <= numVertex; pTrigonometric++, i++) {
            f32 sin = pTrigonometric->mSin;
            f32 cos = pTrigonometric->mCos;

            GXPosition(rV0.mX * cos +
                       rV0.mZ * sin + rV0.mCenter);

            if (flags & 1) {
                GXTexCoord2f32(texStep * i, rV0.mTexCoord);
            }

            GXPosition(rV1.mX * cos +
                       rV1.mZ * sin + rV1.mCenter);

            if (flags & 1) {
                GXTexCoord2f32(texStep * i, rV1.mTexCoord);
            }
        }
    } else {
        for (int i = 0; i <= numVertex; pTrigonometric++, i++) {
            f32 sin = pTrigonometric->mSin;
            f32 cos = pTrigonometric->mCos;

            GXPosition(rV1.mX * cos +
                       rV1.mZ * sin + rV1.mCenter);

            if (flags & 1) {
                GXTexCoord2f32(texStep * i, rV1.mTexCoord);
            }

            GXPosition(rV0.mX * cos +
                       rV0.mZ * sin + rV0.mCenter);

            if (flags & 1) {
                GXTexCoord2f32(texStep * i, rV0.mTexCoord);
            }
        }
    }

    GXEnd();
}

void DrawStripeStrategy::DrawStripeTube_ConnectNone(
    AheadContextStripe* pContext, int flags) {

    ParticleManager* pManager = pContext->mCommon.mParticleManager;

    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    CalcAheadFuncStripe pCalcAheadFunc =
        reinterpret_cast<CalcAheadFuncStripe>(GetCalcAheadFunc(pManager));

    int drawOrder = rSetting.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER;

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

    Particle* pParticle = pGetFirstFunc(pManager);

    VertexTube vertex;
    CalcVertexTube(&vertex, pContext, flags, pParticle, pCalcAheadFunc, pivotX,
                   pivotY, texStep * index);
    index += step;

    pParticle = pGetNextFunc(pManager, pParticle);
    while (pParticle != NULL) {

        VertexTube prev = vertex;
        CalcVertexTube(&vertex, pContext, flags, pParticle, pCalcAheadFunc,
                       pivotX, pivotY, texStep * index);

        DrawTubeSegment(prev, vertex, flags, rSetting.typeOption0,
                        pContext->mTrigonometric);

        pParticle = pGetNextFunc(pManager, pParticle);
        index += step;
    }
}

void DrawStripeStrategy::DrawStripeTube_ConnectRing(
    AheadContextStripe* pContext, int flags) {

    ParticleManager* pManager = pContext->mCommon.mParticleManager;

    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    CalcAheadFuncStripe pCalcAheadFunc =
        reinterpret_cast<CalcAheadFuncStripe>(GetCalcAheadFunc(pManager));

    int drawOrder = rSetting.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER;

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

    Particle* pParticle = pGetFirstFunc(pManager);

    VertexTube vertex;
    VertexTube prev;
    VertexTube first;

    CalcVertexTube(&first, pContext, flags, pParticle, pCalcAheadFunc, pivotX,
                   pivotY, texStep * index);
    index += step;

    vertex = first;

    pParticle = pGetNextFunc(pManager, pParticle);
    while (pParticle != NULL) {

        prev = vertex;
        CalcVertexTube(&vertex, pContext, flags, pParticle, pCalcAheadFunc,
                       pivotX, pivotY, texStep * index);

        DrawTubeSegment(prev, vertex, flags, rSetting.typeOption0,
                        pContext->mTrigonometric);

        pParticle = pGetNextFunc(pManager, pParticle);
        index += step;
    }

    first.mTexCoord = texStep * index;
    DrawTubeSegment(vertex, first, flags, rSetting.typeOption0,
                    pContext->mTrigonometric);
}

void DrawStripeStrategy::DrawStripeTube_ConnectEmitter(
    AheadContextStripe* pContext, int flags) {

    ParticleManager* pManager = pContext->mCommon.mParticleManager;

    const EmitterDrawSetting& rSetting =
        *pManager->mResource->GetEmitterDrawSetting();

    CalcAheadFuncStripe pCalcAheadFunc =
        reinterpret_cast<CalcAheadFuncStripe>(GetCalcAheadFunc(pManager));

    int drawOrder = rSetting.mFlags & EmitterDrawSetting::FLAG_DRAW_ORDER;

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

    Particle* pParticle = pGetFirstFunc(pManager);

    VertexTube vertex;
    VertexTube prev;

    CalcVertexTube(&prev, pContext, flags, pParticle, pCalcAheadFunc, pivotX,
                   pivotY, texStep * index);
    index += step;

    vertex = prev;
    vertex.mTexCoord = texStep * index;

    Particle* pYoungest = GetYoungestDrawParticle(pManager);
    prev.mCenter += pContext->mCommon.mEmitterCenter -
                    pYoungest->mParameter.mPosition;
    index += step;

    DrawTubeSegment(prev, vertex, flags, rSetting.typeOption0,
                    pContext->mTrigonometric);

    pParticle = pGetNextFunc(pManager, pParticle);
    while (pParticle != NULL) {

        prev = vertex;
        CalcVertexTube(&vertex, pContext, flags, pParticle, pCalcAheadFunc,
                       pivotX, pivotY, texStep * index);

        DrawTubeSegment(prev, vertex, flags, rSetting.typeOption0,
                        pContext->mTrigonometric);

        pParticle = pGetNextFunc(pManager, pParticle);
        index += step;
    }
}

void DrawStripeStrategy::DrawStripeTube(AheadContextStripe* pContext,
                                        int flags) {

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
        DrawStripeTube_ConnectNone(pContext, flags);
        GXSetCullMode(GX_CULL_BACK);
        DrawStripeTube_ConnectNone(pContext, flags);
        break;
    }

    case 1: {
        GXSetCullMode(GX_CULL_FRONT);
        DrawStripeTube_ConnectRing(pContext, flags);
        GXSetCullMode(GX_CULL_BACK);
        DrawStripeTube_ConnectRing(pContext, flags);
        break;
    }

    case 2: {
        GXSetCullMode(GX_CULL_FRONT);
        DrawStripeTube_ConnectEmitter(pContext, flags);
        GXSetCullMode(GX_CULL_BACK);
        DrawStripeTube_ConnectEmitter(pContext, flags);
        break;
    }
    }
}

void DrawStripeStrategy::Draw(const DrawInfo& rInfo,
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

    switch (rSetting.typeOption) {
    default: {
        DrawStripe(&context, flags, math::VEC3(1.0f, 0.0f, 0.0f),
                   math::VEC3(-1.0f, 0.0f, 0.0f));

        if (rSetting.typeOption == EmitterDrawSetting::ASSIST_ST_CROSS) {
            DrawStripe(&context, flags, math::VEC3(0.0f, 0.0f, -1.0f),
                       math::VEC3(0.0f, 0.0f, 1.0f));
        }
        break;
    }

    case EmitterDrawSetting::ASSIST_ST_TUBE: {
        DrawStripeTube(&context, flags);
        break;
    }
    }
}

math::VEC3
DrawStripeStrategy::GetInitialPrevAxis(const EmitterDrawSetting& rSetting,
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
DrawStripeStrategy::GetCalcAheadFunc(ParticleManager* pManager) {
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

void DrawStripeStrategy::CalcAhead_Particle_Stripe(math::VEC3* pAxisY,
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

void DrawStripeStrategy::CalcAhead_ParticleBoth_Stripe(
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

void DrawStripeStrategy::CalcAhead_ParticleBoth_Ring(
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

void DrawStripeStrategy::CalcAhead_ParticleBoth_Origin(
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
