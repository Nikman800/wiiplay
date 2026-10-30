#include <egg/gfxe.h>

#include <nw4r/g3d/g3d_state.h>
#include <revolution/GX.h>
#include <revolution/MTX.h>

namespace EGG {

DrawPathShadowVolume::DrawPathShadowVolume()
    : mSVFlags(0),
      mClearAlpha(0),
      mpFull(NULL),
      mpFogZ(NULL),
      mShadowColor((GXColor){0, 0, 0, 255}),
      mBlendType(2),
      mDrawFlag(0),
      mFogIndex(-1) {}

DrawPathShadowVolume::~DrawPathShadowVolume() {}

void DrawPathShadowVolume::internalCalc() {}

void DrawPathShadowVolume::internalDraw(u16 step) {
    static const DrawGX::Blend cBlendTable[] = {DrawGX::BLEND_MUL_INV,
                                                DrawGX::BLEND_SUB,
                                                DrawGX::BLEND_NORMAL};

    const Screen& rScreen = IDrawGX::getScreen();
    const Screen::DataEfb& rEfb = rScreen.GetDataEfb();
    bool half;
    bool isAlpha = StateGX::getDefaultPixelFormat() == GX_PF_RGBA6_Z24;

    switch (step) {
    case cStep_0: {
        mDrawFlag = IDrawGX::getDrawFlag();
        if (isAlpha && !(mSVFlags & 2)) {
            IDrawGX::disableAlphaUpdate();
        }
        break;
    }

    case cStep_1: {
        if (isAlpha) {
            if (mSVFlags & 2) {
                mpFull = TextureBuffer::alloc(rEfb.vp.width, rEfb.vp.height,
                                              GX_TF_I8);
                mpFull->enableAlphaUpdate();
                mpFull->setClearColor(StateGX::getEfbClearColor());
                mpFull->capture(rEfb.vp.x, rEfb.vp.y, false, 0x27);
            }
        } else {
            mpFull = TextureBuffer::alloc(rEfb.vp.width, rEfb.vp.height,
                                          GX_TF_RGBA8);
            mpFull->enableColorUpdate();
            mpFull->setClearColor(StateGX::getEfbClearColor());
            mpFull->capture(rEfb.vp.x, rEfb.vp.y);
        }

        if (mpFull != NULL) {
            mpFull->disablePixModeSync();
        }

        if (0 <= mFogIndex && mFogIndex < 32) {
            mpFogZ = TextureBuffer::alloc(rEfb.vp.width, rEfb.vp.height,
                                          GX_TF_RGBA8);
            mpFogZ->capture(rEfb.vp.x, rEfb.vp.y, false, GX_TF_Z24X8);
            mpFogZ->setMinFilter(GX_NEAR);
            mpFogZ->setMagFilter(GX_NEAR);
        }

        IDrawGX::disableColorUpdate();
        IDrawGX::enableAlphaUpdate();
        DrawGX::BeginDrawShadowVolume(0);
        break;
    }

    case cStep_2: {
        IDrawGX::setDrawFlag(mDrawFlag);
        StateGX::GXSetColorUpdate_(true);
        StateGX::GXSetAlphaUpdate_(false);

        half = mSVFlags & 1;
        TextureBuffer* pBuffer = TextureBuffer::alloc(
            !half ? rEfb.vp.width : rEfb.vp.width / 2,
            !half ? rEfb.vp.height : rEfb.vp.height / 2, GX_TF_I8);
        pBuffer->setVFilterBlur();

        if (isAlpha) {
            if (!(mSVFlags & 2)) {
                GXColor clear = {0, 0, 0, 0};
                clear.a = mClearAlpha;
                pBuffer->setClearColor(clear);
                pBuffer->enableAlphaUpdate();
            }
            pBuffer->capture(rEfb.vp.x, rEfb.vp.y, half, 0x27);
        } else {
            pBuffer->capture(rEfb.vp.x, rEfb.vp.y, half, 0x28);
        }

        GXInvalidateTexAll();
        PostEffectBase::setProjection(rScreen);

        PostEffectSimple simple;
        simple.configure();

        if (isAlpha) {
            if (mSVFlags & 2) {
#line 171
                EGG_ASSERT(mpFull);
                StateGX::AutoColorUpdate colorUpdate(false);
                StateGX::AutoAlphaUpdate alphaUpdate(true);
                mpFull->setLoadMap(GX_TEXMAP0);
                simple.setCapTexture(mpFull);
                simple.draw(rScreen.GetWidth(), rScreen.GetHeight());
                mpFull->free();
            }
        } else {
            if (!(mSVFlags & 0x20)) {
                simple.setCapTexture(mpFull);
                simple.draw(GetScreen().GetWidth(), GetScreen().GetHeight());
            }
            mpFull->free();
        }

        if (!(mSVFlags & 8)) {
            pBuffer->load(DrawGX::GetTexMapDefault());

            nw4r::math::MTX34 mtx;
            PSMTXScale(mtx, GetScreen().GetWidth(), GetScreen().GetHeight(),
                       1.0f);

            DrawGX::BeginDrawShadowVolumeTexture(true, mShadowColor,
                                                 cBlendTable[mBlendType]);

            if (mpFogZ != NULL) {
                nw4r::g3d::G3DState::LoadFog(mFogIndex);
                mpFogZ->load(GX_TEXMAP1);

                GXSetNumTevStages(2);
                GXSetTevDirect(GX_TEVSTAGE1);
                GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
                GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1,
                              GX_COLOR_NULL);
                GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO,
                                GX_CC_ZERO, GX_CC_CPREV);
                GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO,
                                GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
                GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO,
                                GX_CA_ZERO, GX_CA_APREV);
                GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO,
                                GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
                DrawGX::SetZMode(DrawGX::ZMODE_ALWAYS);
                GXSetZTexture(GX_ZT_REPLACE, GX_TF_Z24X8, 0);
                GXSetZCompLoc(GX_FALSE);
                DrawGX::DrawScreenTexture(mtx);
                GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);
                GXSetZCompLoc(GX_TRUE);
                mpFogZ->free();
                mpFogZ = NULL;
            } else {
                DrawGX::DrawScreenTexture(mtx);
            }
        }

        pBuffer->free();
        break;
    }
    }
}

u16 DrawPathShadowVolume::getNumStep() const {
    return cStep_Max;
}

} // namespace EGG
