#include <egg/gfxe.h>

#include <revolution/GX.h>


#include <egg/gfxe/eggPostEffectMaskDOF.h>

namespace EGG {

namespace {

inline u8 scaleAlpha(u8 alpha, int n, int d) {
    return (f32)n * alpha / d;
}

} // namespace

DrawPathDOF::DrawPathDOF()
    : mFlags(0x111),
      unk80(1),
      mFocus(30.0f),
      mFocusRange(20.0f),
      unk8C(20.0f),
      mBlurRadius(1.6f),
      mBlurType(0),
      mpBlurSimple(NULL),
      mpBlur(NULL),
      mpDOF(NULL),
      mpMask(NULL) {
    mpDOF = new PostEffectMaskDOF();
#line 66
    EGG_ASSERT(mpDOF);
    mpDOF->configure();

    mpBlur = new PostEffectBlur();
#line 70
    EGG_ASSERT(mpBlur);
    mpBlur->configure();
    mpBlur->setNumBlurData(1);
    mpBlur->getBlurData(0)->mRadius = 2.0f;

    mpBlurSimple = new PostEffectBlurSimple();
#line 76
    EGG_ASSERT(mpBlurSimple);
    mpBlurSimple->configure();
    mpBlurSimple->setBlendMode(PostEffectBase::cBlendMode_None);
    mpBlurSimple->setUnk2C(0);
    mpBlurSimple->setIntensity(200);
    mpBlurSimple->setUnk30(0);

    mpMask = new PostEffectMask();
#line 84
    EGG_ASSERT(mpMask);
    mpMask->configure();
    mpMask->setBlendMode(PostEffectBase::cBlendMode_None);
    mpMask->setIntensity(0.0f);
    mpMask->setUnk20(0);

    mpTexBuffer = NULL;
    mAlpha[0] = 255;
    mAlpha[1] = 255;
}

DrawPathDOF::~DrawPathDOF() {
#line 111
    EGG_ASSERT(mpDOF);
    delete mpDOF;
    mpDOF = NULL;

#line 115
    EGG_ASSERT(mpBlur);
    delete mpBlur;
    mpBlur = NULL;

#line 119
    EGG_ASSERT(mpBlurSimple);
    delete mpBlurSimple;
    mpBlurSimple = NULL;

#line 123
    EGG_ASSERT(mpMask);
    delete mpMask;
    mpMask = NULL;
}

namespace {

const u8 BLUR_SAMPLES[] = {4, 2};
inline f32 calcDepth(const Screen& rScreen, f32 z) {
    z = z >= -0.000001f ? -0.000001f : z;

    f32 n = rScreen.GetNearZ();
    f32 f = rScreen.GetFarZ();
    f32 clip = (z * -n) / (f - n) - (f * n) / (f - n);
    f32 d = 1.0f + clip / -z;
    u32 depth = 16777215.0f * d;
    return depth / 16777215.0f;
}

} // namespace

void DrawPathDOF::internalCalc() {
    const Screen& rScreen = IDrawGX::getScreen();

#line 135
    EGG_ASSERT(mFocusRange > 0.f);

    f32 center = calcDepth(rScreen, -mFocus);
    f32 start = calcDepth(rScreen, -(mFocus - mFocusRange / 2));
    f32 end = calcDepth(rScreen, -(mFocus + mFocusRange / 2));

    if (end - start <= 0.0f) {
        end = start + 0.000001f;
    }

    mpDOF->setStart(start);
    mpDOF->setCenter(center);
    mpDOF->setEnd(end);

    mpBlur->getBlurData(0)->BYTE_0x0 = BLUR_SAMPLES[mBlurType];
    mpBlur->getBlurData(0)->mRadius = mBlurRadius;

    u8 intensity = 0.5f + 255.0f / BLUR_SAMPLES[mBlurType];
    GXColor color = {intensity, intensity, intensity, 200};
    mpBlur->getBlurData(0)->mColor = color;

    mpBlurSimple->setRadius(mBlurRadius);
}

void DrawPathDOF::internalDraw(u16 step) {
    switch (step) {
    case cStep_0: {
        if (!isGlbFlag1()) {
            pushWorkBuffer(cWorkBuffer_0);
        }
        break;
    }

    case cStep_1: {
        break;
    }

    case cStep_2: {
        break;
    }
    }

    if (unk80 == 3) {
        internalDrawLite(step);
    } else {
        internalDrawStandard(step);
    }

    switch (step) {
    case cStep_0: {
        break;
    }

    case cStep_1: {
        popWorkBuffer(false);
        releaseEfb(cBufferType_3);
        break;
    }

    case cStep_2: {
        if (mpTexBuffer != NULL) {
            mpTexBuffer->free();
            mpTexBuffer = NULL;
        }
        break;
    }
    }
}

void DrawPathDOF::internalDrawStandard(u16 step) {
    const Screen& rScreen = IDrawGX::getScreen();
    const Screen::DataEfb& rEfb = rScreen.GetDataEfb();

    u16 efbX = rEfb.vp.x;
    u16 efbY = rEfb.vp.y;
    bool alpha = mFlags & 1;

    switch (step) {
    case cStep_0: {
        StateGX::GXSetColorUpdate_(true);
        StateGX::GXSetAlphaUpdate_(true);
        StateGX::GXSetDither_(true);

        u16 texHalfW = rEfb.vp.width / 2;
        u16 texHalfH = rEfb.vp.height / 2;
        u16 drawHalfW = rScreen.GetWidth() / 2;
        u16 drawHalfH = rScreen.GetHeight() / 2;

        TextureBuffer* pEfb = captureEfb(cBufferType_3, true);
        pEfb->setLoadMap(GX_TEXMAP0);

        PostEffectBase::setProjection(rScreen);

        TextureBuffer* pDepth = NULL;

        switch (unk80) {
        case 0: {
            mpDOF->setMode(PostEffectMaskDOF::cMode_0);
            pDepth = TextureBuffer::alloc(rEfb.vp.width, rEfb.vp.height, GX_TF_IA8);
            pDepth->setMinFilter(GX_NEAR);
            pDepth->setMagFilter(GX_NEAR);
            pDepth->capture(efbX, efbY, false, GX_TF_Z16);
            pDepth->setLoadMap(GX_TEXMAP0);
            break;
        }

        case 1: {
#line 284
            EGG_ASSERT(StateGX::getDefaultPixelFormat() == GX_PF_RGBA6_Z24);
            mpDOF->setMode(PostEffectMaskDOF::cMode_3);

            TextureBuffer* pZ = TextureBuffer::alloc(rEfb.vp.width, rEfb.vp.height, GX_TF_RGBA8);
            pZ->setMinFilter(GX_NEAR);
            pZ->setMagFilter(GX_NEAR);
            pZ->capture(efbX, efbY, false, GX_TF_Z24X8);
            pZ->load(GX_TEXMAP0);

            nw4r::math::MTX34 mtx;
            PSMTXScale(mtx, drawHalfW, drawHalfH, 1.0f);
            GXColor drawColor = {0, 0, 0, 0};

            StateGX::setPixelFormatGX(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

            GXColor fogColor = {0x00, 0x00, 0x3F, 0xFF};
            GXSetFog(GX_FOG_PERSP_LIN, mFocus - mFocusRange / 2, mFocus + mFocusRange / 2,
                     rScreen.GetNearZ(), rScreen.GetFarZ(), fogColor);
            GXSetFogRangeAdj(GX_FALSE, 0, NULL);

            DrawGX::SetMat_ColorChannel(DrawGX::COLORCHAN_COLOR);
            DrawGX::SetMat_TexGen(DrawGX::TEXGEN_ENABLE);
            DrawGX::SetMat_Ind();

            GXColor tevColor = {0, 0, 0, 0};
            GXSetTevColor(GX_TEVREG0, tevColor);
            GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE,
                                  GX_CH_ALPHA);
            GXSetNumTevStages(1);
            GXSetTevDirect(GX_TEVSTAGE0);
            GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
            GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                            GX_TEVPREV);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
            GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                            GX_TEVPREV);
            GXSetZTexture(GX_ZT_REPLACE, GX_TF_Z24X8, 0);
            GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
            GXSetZCompLoc(GX_FALSE);
            GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
            GXSetBlendMode(GX_BM_LOGIC, GX_BL_ONE, GX_BL_ONE, GX_LO_OR);
            DrawGX::SetVtxState(DrawGX::VTX_TYPE_SCREEN_TEXTURE_LU);

            DrawGX::DrawScreenTexture(mtx, drawColor);

            GXColor fogOff = {0x00, 0x00, 0x00, 0xFF};
            GXSetFog(GX_FOG_NONE, 0.0f, 1.0f, 0.0f, 1.0f, fogOff);

            StateGX::setPixelFormatGX(StateGX::getDefaultPixelFormat(),
                                      StateGX::getDefaultZFmt16());
            pZ->free();

            pDepth = TextureBuffer::alloc(texHalfW, texHalfH, GX_TF_I8);
            pDepth->capture(efbX, efbY, false, GX_CTF_A8);
            pDepth->setLoadMap(GX_TEXMAP1);
            break;
        }

        case 2: {
            break;
        }

        default: {
            break;
        }
        }

        const f32* pVp = shiftWorkSpaceViewportGX();
        u16 x = pVp[0];
        u16 y = pVp[1];

        mpDOF->setCapTexture(pDepth);
        mpDOF->draw(drawHalfW, drawHalfH);

        TextureBuffer* pMaskBuf = TextureBuffer::alloc(texHalfW, texHalfH, GX_TF_I8);
        pMaskBuf->setVFilterBlur();
        pMaskBuf->capture(x, y, false, GX_CTF_R8);
        pMaskBuf->setLoadMap(GX_TEXMAP0);

        {
            StateGX::AutoColorUpdate colorLock(true);
            StateGX::AutoAlphaUpdate alphaLock(false);
            StateGX::AutoDither ditherLock(false);

            PostEffectSimple simple;
            simple.configure();
            simple.setSimpleBlendMode(0);
            simple.setBlendMode(PostEffectBase::cBlendMode_None);
            simple.setCapTexture(pEfb);
            simple.draw(drawHalfW, drawHalfH);
        }

        TextureBuffer* pBlurBuf = TextureBuffer::alloc(texHalfW, texHalfH, GX_TF_RGBA8);
        pBlurBuf->setVFilterBlur();
        pBlurBuf->capture(x, y);
        pBlurBuf->setLoadMap(GX_TEXMAP0);

        mpTexBuffer = TextureBuffer::alloc(texHalfW, texHalfH, GX_TF_RGBA8);

        {
            StateGX::AutoAlphaUpdate alphaLock(alpha);

            mpBlur->setCapTexture(pBlurBuf);
            mpBlur->draw(drawHalfW, drawHalfH);

            {
                StateGX::AutoColorUpdate colorLock(false);
                StateGX::AutoAlphaUpdate alphaLock2(true);

                PostEffectSimple simple;
                simple.configure();
                simple.setSimpleBlendMode(1);
                simple.setBlendMode(PostEffectBase::cBlendMode_Add);
                simple.setCapTexture(pMaskBuf);
                simple.draw(drawHalfW, drawHalfH);
            }
        }

        mpTexBuffer->capture(x, y);
        mpTexBuffer->setLoadMap(GX_TEXMAP0);

        pBlurBuf->free();
        pMaskBuf->free();
        pDepth->free();
        break;
    }

    case cStep_2: {
        StateGX::AutoColorUpdate colorLock(true);
        StateGX::AutoAlphaUpdate alphaLock(false);
        StateGX::AutoDither ditherLock(false);

        PostEffectBase::setProjection(rScreen);

        PostEffectSimple simple;
        simple.configure();
        simple.setSimpleBlendMode(1);
        simple.setBlendMode(PostEffectBase::cBlendMode_Normal);
        simple.setCapTexture(mpTexBuffer);
        simple.draw(rScreen.GetWidth(), rScreen.GetHeight());
        break;
    }
    }
}

// NONMATCHING (96.6%): float register colouring of the PSMTXScale arguments in the step-2
// loop (1/tan, size.y and the width/height conversions; the original converts the height
// first and multiplies half * cot)
void DrawPathDOF::internalDrawLite(u16 step) {
    const Screen& rScreen = IDrawGX::getScreen();
    const Screen::DataEfb& rEfb = rScreen.GetDataEfb();

    u16 drawHalfW = rScreen.GetWidth() / 2;
    u16 drawHalfH = rScreen.GetHeight() / 2;

    switch (step) {
    case cStep_0: {
        TextureBuffer* pEfb = captureEfb(cBufferType_3, false);
        pEfb->setLoadMap(GX_TEXMAP0);

        u16 w = rEfb.vp.width / 2;
        u16 h = rEfb.vp.height / 2;
        mpTexBuffer = TextureBuffer::alloc(w, h, GX_TF_RGBA8);
        mpBlur->setCapTexture(pEfb);

        if (mFlags & 0x10) {
            StateGX::setPixelFormatGX(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
        }

        StateGX::AutoColorUpdate colorLock(true);
        StateGX::AutoAlphaUpdate alphaLock(false);
        StateGX::AutoDither ditherLock(false);

        PostEffectBase::setProjection(rScreen);
        const f32* pVp = shiftWorkSpaceViewportGX();

        mpBlur->draw(drawHalfW, drawHalfH);
        mpTexBuffer->capture(pVp[0], pVp[1]);

        StateGX::setPixelFormatGX(StateGX::getDefaultPixelFormat(),
                                  StateGX::getDefaultZFmt16());
        break;
    }

    case cStep_2: {
        StateGX::AutoColorUpdate colorLock(true);
        StateGX::AutoAlphaUpdate alphaLock(false);
        StateGX::AutoDither ditherLock(true);

        Screen screen(IDrawGX::getScreen());
        screen.SetFlag80();
        screen.SetProjectionGX();

        DrawGX::BeginDrawQuad(DrawGX::COLORCHAN_COLOR, DrawGX::ZMODE_XLU,
                              DrawGX::BLEND_NORMAL, true, true);
        mpTexBuffer->load(GX_TEXMAP0);

        int num = 1;
        for (int i = 0; i < 2; i++) {
            f32 offsets[] = {-unk8C, mFocusRange};
            u8 a = scaleAlpha(mAlpha[i], num, num);
            f32 t = 0.0f;
            f32 z = mFocus + offsets[i] / 2 + t * offsets[i] / 2;

            if (a == 0) {
                continue;
            }

            if ((i == 0 && z >= screen.GetNearZ()) || (i == 1 && z <= screen.GetFarZ())) {
                if (i == 0) {
                    GXSetZMode(GX_TRUE, GX_GEQUAL, GX_FALSE);
                } else {
                    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
                }

                GXColor color = {0xFF, 0xFF, 0xFF, a};
                f32 w = screen.GetWidth();
                f32 h = screen.GetHeight();

                nw4r::math::MTX34 mtx;
                PSMTXScale(mtx, w * nw4r::math::FAbs(z) / screen.GetScreenDistance(),
                           h * nw4r::math::FAbs(z) / screen.GetScreenDistance(), 1.0f);
                PSMTXTransApply(mtx, mtx, 0.0f, 0.0f, -z);

                GXSetZCompLoc((mFlags >> 8) & 1);
                DrawGX::DrawDL(DrawGX::DL_QUAD_TEXTURE, mtx, color);
            }
        }
        break;
    }
    }
}

u16 DrawPathDOF::getNumStep() const {
    return cStep_Max;
}

} // namespace EGG

