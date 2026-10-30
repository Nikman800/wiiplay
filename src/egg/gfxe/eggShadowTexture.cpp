#include <egg/gfxe/eggShadowTexture.h>

#include <egg/gfxe/eggDrawGX.h>
#include <egg/gfxe/eggG3DUtility.h>
#include <egg/gfxe/eggModelEx.h>
#include <egg/gfxe/eggScreen.h>
#include <egg/gfxe/eggStateGX.h>
#include <egg/prim/eggAssert.h>

#include <nw4r/g3d.h>
#include <nw4r/math.h>

#include <revolution/GX.h>
#include <revolution/OS.h>

namespace EGG {

// Wii Play: lbl_8043BBD0, never written by the game
u16 ShadowTexture::sFlags;

namespace {

const GXColor DEFAULT_CLEAR_COLOR = {0, 0, 0, 0};

const int CAPTURE_FORMATS[2][2] = {
    {0x28, 0x20},
    {0x28, 0x20},
};

const DrawGX::Blend BLEND_MODES[] = {
    DrawGX::BLEND_MUL,
    DrawGX::BLEND_SUB,
    DrawGX::BLEND_NORMAL,
};

} // namespace

ShadowTexture::ShadowTexture(u16 size, u16 format, u16 mdlMax)
    : ModelSnapshot(size, size, GX_TF_I8, mdlMax),
      SHORT_0x70(0x30),
      BYTE_0x72(0xFF),
      FLOAT_0x74(1.05f),
      VEC_0x78(0.0f, -1.0f, -1.0f),
      mEffectMtxNum(0) {
#line 66
    EGG_ASSERT_MSG(static_cast<GXTexFmt>(format) == GX_TF_I8 || static_cast<GXTexFmt>(format) == GX_TF_I4, "Can't use this format for shadow texture.");

    PSMTXIdentity(mLightMtx);
    PSMTXIdentity(mProjectionMtx);

    POST_EFF_0xE4 = new PostEffectBlur();
    POST_EFF_0xE4->configure();
    POST_EFF_0xE4->setNumBlurData(2);
    for (u32 i = 0; i < 2; i++) {
        POST_EFF_0xE4->getBlurData(i)->BYTE_0x0 = 8;
    }

    POST_EFF_0xE8 = new PostEffectBlurSimple();
    POST_EFF_0xE8->configure();
    POST_EFF_0xE8->setUnk2C(0);
    POST_EFF_0xE8->setRadius(1.0f);

    mBlurParam[0].radius = 0.5f;
    mBlurParam[1].radius = 2.0f;
    mBlurParam[1].alpha = 200;
    mBlurParam[0].alpha = 200;

    ResetEffectMtx();
}

void ShadowTexture::InitList() {
    ModelSnapshot::InitList();
    setVFilterBlur();
}

void ShadowTexture::Replace(ModelEx* pModelEx, const char* pName) {
#line 137
    EGG_ASSERT_MSG(pModelEx, "pMdl");

    GXTexObj texObj;
    getTexObj(&texObj);

    TextureReplaceResult results[0xFF];
    u16 resultNum =
        pModelEx->replaceTexture(pName, texObj, false, results, 0xFF, true);

#line 145
    EGG_ASSERT_MSG(mEffectMtxNum < EFFECT_MTX_MAX, "m_effectMtxNum < EFFECT_MTX_MAX");
#line 146
    EGG_ASSERT_MSG(resultNum < 0x80, "Too much texture.");
#line 147
    EGG_ASSERT_MSG(resultNum != 0, "no such a texture.");

    for (u16 i = 0; i < resultNum; i++) {
        EffectMtxSet& rSet = mEffectMtxSet[mEffectMtxNum++];
        rSet.texMapID = results[i].texCoordID;
        rSet.texSrt = pModelEx->getResTexSrt(results[i].materialID);
        G3DUtility::setManualMapMode(rSet.texSrt, rSet.texMapID);
    }
}

void ShadowTexture::ResetEffectMtx() {
    mEffectMtxNum = 0;
    for (int i = 0; i < EFFECT_MTX_MAX; i++) {
        mEffectMtxSet[i].texMapID = 0;
    }
}

void ShadowTexture::Calc() {
    if (!(SHORT_0x70 & 0x10)) {
        return;
    }

    for (u8 i = 0; i < 2; i++) {
        PostEffectBlur::BlurData* pData = POST_EFF_0xE4->getBlurData(i);
        pData->mRadius = mBlurParam[i].radius;
        u8 alpha = mBlurParam[i].alpha;
        pData->mColor.r = alpha;
        pData->mColor.g = alpha;
        pData->mColor.b = alpha;
        pData->mColor.a = alpha;
    }

    GatherModel(FLOAT_0x74);
    CalcShadow();
    CalcProjectionMatrix();

    setClearColor(DEFAULT_CLEAR_COLOR);
    enableColorUpdate();
    enableAlphaUpdate();
    enableZBufferUpdate();

    SHORT_0x70 &= ~0x08;
}

void ShadowTexture::CalcShadow() {
    nw4r::math::MTX34* pMtx = &mLightMtx;

    if (m_radius > 0.0f) {
        f32 texScale = !(SHORT_0x70 & 0x04) ? 1.0f : 2.0f;

        f32 len = PSVECMag(VEC_0x78);
#line 210
        EGG_ASSERT_MSG(len > 0.0f, "len > 0.f");

        f32 inv = -1.0f / len;
        nw4r::math::VEC3 dir(VEC_0x78.x * inv, VEC_0x78.y * inv,
                             VEC_0x78.z * inv);

        f32 dist = 2.0f * m_radius;
        nw4r::math::VEC3 eye(dir.x * dist + m_center.x, dir.y * dist + m_center.y,
                             dir.z * dist + m_center.z);

        nw4r::math::VEC3 up(0.0f, 1.0f, 0.0f);
        if (nw4r::math::FAbs(dir.y) > 0.9999f) {
            up.y = 0.0f;
            up.z = dir.y > 0.0f ? -1.0f : 1.0f;
        }

        nw4r::math::VEC3 right;
        PSVECCrossProduct(up, dir, right);
        PSVECNormalize(right, right);
        PSVECCrossProduct(dir, right, up);

        f32 aspect = 1.0f;
        f32 scale = texScale * getWidth() / (2.0f * m_radius * aspect);

        (*pMtx)[0][0] = right.x * scale;
        (*pMtx)[0][1] = right.y * scale;
        (*pMtx)[0][2] = right.z * scale;
        (*pMtx)[0][3] =
            scale * -(eye.x * right.x + eye.y * right.y + eye.z * right.z);
        (*pMtx)[1][0] = up.x * scale;
        (*pMtx)[1][1] = up.y * scale;
        (*pMtx)[1][2] = up.z * scale;
        (*pMtx)[1][3] = scale * -(eye.x * up.x + eye.y * up.y + eye.z * up.z);
        (*pMtx)[2][0] = dir.x;
        (*pMtx)[2][1] = dir.y;
        (*pMtx)[2][2] = dir.z;
        (*pMtx)[2][3] = -(eye.x * dir.x + eye.y * dir.y + eye.z * dir.z);
    } else {
        PSMTXIdentity(*pMtx);
    }
}

void ShadowTexture::CalcProjectionMatrix() {
    f32 scale = !(SHORT_0x70 & 0x04) ? 0.5f : 1.0f;

    nw4r::math::MTX34 projMtx;
    f32 w = scale * getWidth();
    f32 h = scale * getHeight();
    C_MTXLightOrtho(projMtx, w, -w, -h, h, 0.5f, -0.5f, 0.5f, 0.5f);
    PSMTXConcat(projMtx, mLightMtx, mProjectionMtx);

    for (int i = 0; i < mEffectMtxNum; i++) {
#line 274
        EGG_ASSERT_MSG(mEffectMtxSet[i].texSrt.IsValid(), "m_effectMtxSet[i].m_texSrt.IsValid()");
        mEffectMtxSet[i].texSrt.SetEffectMtx(mEffectMtxSet[i].texMapID,
                                             &mProjectionMtx);
    }
}

void ShadowTexture::DrawShape() {
    nw4r::math::MTX34 lightMtx = mLightMtx;
    nw4r::math::MTX34* viewMtx[256];

    DrawGX::ResetMaterial(DrawGX::COLORCHAN_COLOR);
    DrawGX::SetBlendMode(DrawGX::BLEND_NONE);

    GXColor color = {BYTE_0x72, 0, 0, BYTE_0x72};
    GXSetChanMatColor(GX_COLOR0A0, color);

    u8* pStart = (u8*)G3DUtility::getTempMemCurrent();
    int countCalc = 0;
    int countDraw = 0;

    for (int i = 0; i < m_mdlNum; i++) {
        ModelEx* pModel = m_mdlList[i].pModel;
        int num = pModel->getNumViewMtx();

        if (num > 0) {
            viewMtx[i] = G3DUtility::allocArray<nw4r::math::MTX34>(num, 32);
            pModel->calcView(lightMtx, viewMtx[i]);
            countCalc += num;
        } else {
            viewMtx[i] = NULL;
            if (pModel->getScnObj() != NULL) {
                pModel->getScnObj()->G3dProc(
                    nw4r::g3d::G3dObj::G3DPROC_CALC_VIEW, 0, &lightMtx);
            }
        }
    }

    u32 size = (u8*)G3DUtility::getTempMemCurrent() - pStart;
    if (size != 0) {
        DCStoreRange(pStart, size);
    }

    for (int i = 0; i < m_mdlNum; i++) {
        if (!m_mdlList[i].useMat) {
            ModelEx* pModel = m_mdlList[i].pModel;
            pModel->drawShapeDirectly(ModelEx::cDrawFlag_IgnoreMaterial |
                                          ModelEx::cDrawFlag_ForceLightOff,
                                      true, true, viewMtx[i]);
            countDraw += pModel->getNumViewMtx();
        }
    }

    for (int i = 0; i < m_mdlNum; i++) {
        if (m_mdlList[i].useMat) {
            ModelEx* pModel = m_mdlList[i].pModel;
            pModel->drawShapeDirectly(ModelEx::cDrawFlag_ForceLightOff, true,
                                      true, viewMtx[i]);
            countDraw += pModel->getNumViewMtx();
        }
    }

#line 375
    EGG_ASSERT_MSG(countCalc == countDraw, "count_calc == count_draw");
}

void ShadowTexture::Draw(Screen* pScreen, bool alpha, bool clear,
                         bool setColor) {
    bool enable = (SHORT_0x70 & 0x10) && !(0x08 & SHORT_0x70);
    if (!enable) {
        return;
    }

    bool isAlpha = (alpha & sFlags) >> 1 & 1;
    u16 width = getWidth();
    u16 height = getHeight();
    if (SHORT_0x70 & 0x04) {
        width *= 2;
        height *= 2;
    }

    f32 farZ = 4.0f * m_radius;
    s16 posY = pScreen->GetPosition().y;
    s16 posX = pScreen->GetPosition().x;
    Screen screen(posX, posY, width, height, pScreen->GetParent(),
                  Frustum::CANVAS_CC);
    screen.SetVpSizeAbsolute();
    screen.SetProjectionAbsolute();
    screen.SetNearZ(0.0f);
    screen.SetProjectionType(Frustum::PROJ_ORTHO);
    screen.SetFarZ(0.02f + farZ);
    screen.SetProjectionGX();

    if (clear) {
        const Screen::DataEfb & rEfb = pScreen->GetDataEfb();
#line 417
        EGG_ASSERT_MSG(rEfb.vp.width >= width, "Width range over.");
#line 418
        EGG_ASSERT_MSG(rEfb.vp.height >= height, "Height range over.");

        nw4r::math::MTX34 mtx;
        screen.CalcMatrixForDrawQuad(&mtx, -width / 2.0f, height / 2.0f,
                                       width, height);
        DrawGX::ClearEfb(mtx, true, true, false, DEFAULT_CLEAR_COLOR, false);
    }

    if (isAlpha) {
        StateGX::GXSetColorUpdate_(false);
        StateGX::GXSetAlphaUpdate_(true);
    } else {
        StateGX::GXSetColorUpdate_(true);
        StateGX::GXSetAlphaUpdate_(false);
    }

    DrawShape();

    const Screen::DataEfb& rEfb = pScreen->GetDataEfb();
    u16 x = rEfb.vp.x;
    u16 y = rEfb.vp.y;

    if ((SHORT_0x70 & 0x02) || (0x04 & SHORT_0x70)) {
        StateGX::GXSetColorUpdate_(true);
        StateGX::GXSetAlphaUpdate_(false);

        CapTexture* pCapTexture = POST_EFF_0xE4->getCapTexture();
        pCapTexture->setVFilterBlur();

        bool zbuffer = (0x20 & SHORT_0x70) != 0;
        if (!zbuffer) {
            pCapTexture->enableColorUpdate();
            pCapTexture->enableAlphaUpdate();
        } else {
            pCapTexture->enableZBufferUpdate();
        }

        if (zbuffer) {
            pCapTexture->disableAlphaUpdate();
            pCapTexture->disableColorUpdate();
        } else {
            pCapTexture->setClearColor(DEFAULT_CLEAR_COLOR);
        }

        GXTexFmt format = isAlpha ? GX_CTF_A8 : GX_CTF_R8;
        pCapTexture->capture(x, y, 0x04 & SHORT_0x70, format);
        pCapTexture->setLoadMap(GX_TEXMAP0);

        Screen effectScreen(screen);
        effectScreen.SetNearZ(0.0f);
        effectScreen.SetFarZ(1.0f);
        effectScreen.SetCanvasMode(Frustum::CANVAS_LU);
        effectScreen.SetProjectionGX();

        if (zbuffer) {
            bool blend = !isAlpha;
            u8 intensity = mBlurParam[0].alpha;
            // The mode and intensity are locals (computed before the stores) for
            // the original's register order
            PostEffectBase::BlendMode mode = blend ? PostEffectBase::cBlendMode_Normal
                                                   : PostEffectBase::cBlendMode_None;
            POST_EFF_0xE8->setIntensity(intensity);
            POST_EFF_0xE8->setRadius(mBlurParam[0].radius);
            POST_EFF_0xE8->setUnk30(blend);
            POST_EFF_0xE8->setBlendMode(mode);
            POST_EFF_0xE8->drawTexture();

            if (0 != mBlurParam[1].alpha) {
                pCapTexture->capture(x, y, false, 0x28);
                pCapTexture->setLoadMap(GX_TEXMAP1);

                POST_EFF_0xE8->setIntensity(mBlurParam[1].alpha);
                POST_EFF_0xE8->setRadius(mBlurParam[1].radius);
                POST_EFF_0xE8->setUnk30(true);
                POST_EFF_0xE8->setBlendMode(PostEffectBase::cBlendMode_Normal);
                POST_EFF_0xE8->drawTexture();
            }
        } else {
            POST_EFF_0xE4->drawTexture();
        }
    }

    if (setColor) {
        setClearColor(StateGX::getEfbClearColor());
    }

    disablePixModeSync();
    capture(x, y, false,
            CAPTURE_FORMATS[isAlpha ? 0 : 1][getFormat() == GX_TF_I8 ? 0 : 1]);

    SHORT_0x70 |= 0x08;
}

void ShadowTexture::BeginDrawQuad(Blend blend) {
    SetMaterial(blend);
    DrawGX::SetVtxState(DrawGX::VTXTYPE_QUAD_SHADOW);
}

void ShadowTexture::DrawQuad(const nw4r::math::MTX34& rMtx,
                             const GXColor& rColor) const {
    SetUpTextureAndColor(rMtx, rColor);

    DrawGX::DrawDLWorld(DrawGX::DL_QUAD_TEXTURE, rMtx);
}

void ShadowTexture::SetUpTextureAndColor(const nw4r::math::MTX34& rMtx,
                                         const GXColor& rColor) const {
    const_cast<ShadowTexture*>(this)->load(GX_TEXMAP0);
    GXSetTevKColor(GX_KCOLOR0, rColor);

    nw4r::math::MTX34 texMtx;
    PSMTXConcat(mProjectionMtx, rMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX_3x4);
}

void ShadowTexture::SetMaterial(Blend blend) {
    DrawGX::SetMat_ColorChannel(DrawGX::COLORCHAN_COLOR);

    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0,
                      GX_FALSE, GX_PTIDENTITY);

    DrawGX::SetMat_Ind();

    GXSetNumTevStages(1);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE,
                          GX_CH_ALPHA);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);

    switch (blend) {
    case BLEND_MUL:
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ONE, GX_CC_KONST, GX_CC_TEXC,
                        GX_CC_ZERO);
        break;
    case BLEND_COLOR:
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                        GX_CC_KONST);
        break;
    case BLEND_SUB:
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC,
                        GX_CC_ZERO);
        break;
    }

    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                    GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                    GX_TEVPREV);

    if (sFlags & 1) {
        GXSetAlphaCompare(GX_GEQUAL, 0, GX_AOP_AND, GX_ALWAYS, 0);
    } else {
        GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    }

    DrawGX::SetZMode(DrawGX::ZMODE_XLU);
    DrawGX::SetBlendMode(BLEND_MODES[blend]);
}

} // namespace EGG
