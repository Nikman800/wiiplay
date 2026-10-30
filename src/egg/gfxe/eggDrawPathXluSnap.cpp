#include <egg/gfxe.h>

#include <revolution/GX.h>
#include <revolution/MTX.h>

namespace EGG {

DrawPathXluSnap::DrawPathXluSnap() : BYTE_0x7C(2), BYTE_0x7D(0xff), WORD_0x80(0),
                                     BUF_0x84(NULL), BUF_0x88(NULL),
                                     BUF_0x8C(NULL) {}

DrawPathXluSnap::~DrawPathXluSnap() {}

void DrawPathXluSnap::internalCalc() {
    BUF_0x84 = NULL;
    BUF_0x88 = NULL;
    BUF_0x8C = NULL;
}

void DrawPathXluSnap::internalDraw(u16 step) {
    const Screen::DataEfb& rEfb = IDrawGX::getScreen().GetDataEfb();

    switch (step) {
    case cStep_0:
        if (BYTE_0x7C & 1) {
            BUF_0x8C = TextureBuffer::alloc(rEfb.vp.width, rEfb.vp.height,
                                            GX_TF_RGBA8);
            BUF_0x8C->enableColorUpdate();
            BUF_0x8C->enableAlphaUpdate();
            BUF_0x8C->capture(rEfb.vp.x, rEfb.vp.y);
        }
        break;

    case cStep_1: {
        if (WORD_0x80 == 2) {
            BUF_0x84 = TextureBuffer::alloc(rEfb.vp.width, rEfb.vp.height,
                                            GX_TF_RGB565);
        } else {
            BUF_0x84 = TextureBuffer::alloc(rEfb.vp.width, rEfb.vp.height,
                                            GX_TF_RGBA8);
        }
        BUF_0x84->enableColorUpdate();
        BUF_0x84->enableAlphaUpdate();

        if ((BYTE_0x7C & 2) == 0) {
            BUF_0x84->enableZBufferUpdate();
        }

        u16 x = rEfb.vp.x;
        u16 y = rEfb.vp.y;
        BUF_0x84->capture(x, y);

        if (BUF_0x8C != NULL) {
            PostEffectBase::setProjection(GetScreen());
            nw4r::math::MTX34 mtx;
            PSMTXScale(mtx, GetScreen().GetWidth(), GetScreen().GetHeight(),
                       1.0f);
            BUF_0x8C->load(GX_TEXMAP0);

            DrawGX::BeginDrawScreen(true, true, false);
            GXSetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                           GX_LO_CLEAR);

            StateGX::AutoColorUpdate colorLock(true);
            StateGX::AutoAlphaUpdate alphaLock(true);
            DrawGX::DrawScreenTexture(mtx, DrawGX::WHITE);

            BUF_0x8C->free();
            BUF_0x8C = NULL;
        }

        if (BYTE_0x7C & 2) {
            BUF_0x88 = TextureBuffer::alloc(rEfb.vp.width, rEfb.vp.height,
                                            GX_TF_Z24X8);
            BUF_0x88->enableZBufferUpdate();
            if (isFlag1()) {
                BUF_0x88->enablePixModeSync();
            } else {
                BUF_0x88->disablePixModeSync();
            }
            BUF_0x88->capture(x, y);
        }
        break;
    }

    case cStep_2:
    case cStep_3:
        if (BUF_0x84 != NULL &&
            ((step == cStep_2 && (BYTE_0x7C & 4) == 0) ||
             (step == cStep_3 && (BYTE_0x7C & 4) != 0))) {

            if (BYTE_0x7D != 0) {
                BUF_0x84->setMinFilter(GX_NEAR);
                BUF_0x84->setMagFilter(GX_NEAR);
                BUF_0x84->load(GX_TEXMAP0);

                PostEffectBase::setProjection(GetScreen());
                nw4r::math::MTX34 mtx;
                PSMTXScale(mtx, GetScreen().GetWidth(),
                           GetScreen().GetHeight(), 1.0f);

                GXColor color = {255, 255, 255, 0};
                color.a = BYTE_0x7D;

                StateGX::AutoColorUpdate colorLock(true);
                StateGX::AutoAlphaUpdate alphaLock(false);
                StateGX::AutoDither ditherLock(true);

                if (BUF_0x88 != NULL) {
                    BUF_0x88->setMinFilter(GX_NEAR);
                    BUF_0x88->setMagFilter(GX_NEAR);
                    BUF_0x88->load(GX_TEXMAP1);

                    DrawGX::SetMat_ColorChannel(DrawGX::COLORCHAN_COLOR);
                    DrawGX::SetMat_TexGen(DrawGX::TEXGEN_ENABLE);
                    DrawGX::SetMat_Ind();

                    GXColor tevColor = {0, 0, 0, 0};
                    GXSetTevKColor(GX_KCOLOR0, tevColor);
                    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN,
                                          GX_CH_BLUE, GX_CH_ALPHA);
                    GXSetNumTevStages(2);
                    GXSetTevDirect(GX_TEVSTAGE0);
                    GXSetTevDirect(GX_TEVSTAGE1);
                    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
                    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);

                    if (WORD_0x80 == 0) {
                        GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
                        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0,
                                      GX_COLOR0A0);
                        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC,
                                        GX_CC_RASC, GX_CC_ZERO);
                        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
                        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_KONST,
                                        GX_CA_RASA, GX_CA_ZERO);
                        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_COMP_A8_GT,
                                        GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                                        GX_TEVPREV);
                    } else {
                        GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
                        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0,
                                      GX_COLOR0A0);
                        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO,
                                        GX_CC_ZERO, GX_CC_TEXC);
                        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
                        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                                        GX_CA_ZERO, GX_CA_RASA);
                        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
                    }

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

                    GXSetZTexture(GX_ZT_REPLACE, GX_TF_Z24X8, 0);
                    DrawGX::SetZMode(DrawGX::ZMODE_OPA);
                    GXSetZCompLoc(GX_FALSE);
                    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
                    DrawGX::SetBlendMode(DrawGX::BLEND_NORMAL);
                    DrawGX::SetVtxState(DrawGX::VTX_TYPE_SCREEN_TEXTURE_LU);
                } else {
                    DrawGX::BeginDrawScreen(true, true, false);
                    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
                    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO,
                                    GX_CA_RASA, GX_CA_ZERO);
                    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_COMP_A8_GT,
                                    GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                                    GX_TEVPREV);
                }

                DrawGX::DrawScreenTexture(mtx, color);
                GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);
            }

            if (BUF_0x88 != NULL) {
                BUF_0x88->free();
                BUF_0x88 = NULL;
            }

            BUF_0x84->free();
            BUF_0x84 = NULL;
        }
        break;

    default:
        break;
    }
}

u16 DrawPathXluSnap::getNumStep() const {
    return cStep_Max;
}

} // namespace EGG
