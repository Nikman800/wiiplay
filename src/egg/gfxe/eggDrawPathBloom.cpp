#include <egg/gfxe.h>

#include <revolution/GX.h>


namespace EGG {

template <> const char* IBinary<DrawPathBloom>::spExtension = "bblm";

DrawPathBloom::DrawPathBloom() : mFlags(7), mpTexBuffer(NULL) {
    mpMask = new PostEffectMask();
    mpMask->configure();

    mpBlurSimple = new PostEffectBlurSimple();
    mpBlurSimple->configure();

    mpBlur1 = new PostEffectBlur();
    mpBlur1->configure();

    mpBlur2 = new PostEffectBlur();
    mpBlur2->configure();

    mpBlurGather = new PostEffectBlurGather();
    mpBlurGather->configure();
    mpBlurGather->setNumBlurGather(2);

    mpSimple = new PostEffectSimple();
    mpSimple->configure();
    mpSimple->setSimpleBlendMode(0);

    mData.mThreshold = 0.5f;
    mData.mMaskColor = DrawGX::WHITE;
    mData.mColor = DrawGX::WHITE;
    mData.mFlags = 0xB;
    mData.mBlur0.mRadius = 1.0f;
    mData.mBlur0.mIntensity = 0.3f;
    mData.mBlur1.mRadius = 2.0f;
    mData.mBlur1.mIntensity = 0.08f;
    mData.mBlur2.mRadius = 4.0f;
    mData.mBlur2.mIntensity = 0.04f;
    mData.mNumBlur = 2;
    mData.mBlendModeIdx = 1;
    mData.mGatherScale0 = mData.mGatherScale1 = 0.0f;
}

DrawPathBloom::~DrawPathBloom() {
    delete mpMask;
    mpMask = NULL;

    delete mpBlurSimple;
    mpBlurSimple = NULL;

    delete mpBlur1;
    mpBlur1 = NULL;

    delete mpBlur2;
    mpBlur2 = NULL;

    delete mpBlurGather;
    mpBlurGather = NULL;

    delete mpSimple;
    mpSimple = NULL;
}

void DrawPathBloom::internalCalc() {
    static const PostEffectBase::BlendMode BLEND_MODES[] = {
        PostEffectBase::cBlendMode_Add, PostEffectBase::cBlendMode_4,
        PostEffectBase::cBlendMode_AddAlpha, PostEffectBase::cBlendMode_Normal};

    if (mFlags & 1) {
        mpMask->setIntensity((16.0f + 219.0f * mData.mThreshold) / 255.0f);
        mpMask->setColor(mData.mMaskColor);

        u8 intensity = mData.mBlur0.mIntensity * 255.0f;
        mpBlurSimple->setIntensity(mData.mBlur0.mIntensity * 255.0f);
        mpBlurSimple->setRadius(mData.mBlur0.mRadius);

        mpBlur1->setNumBlurData(1);
        mpBlur1->getBlurData(0)->BYTE_0x0 = 8;
        PostEffectBlur::BlurData* pData = mpBlur1->getBlurData(0);
        pData->mColor.r = intensity;
        pData->mColor.g = intensity;
        pData->mColor.b = intensity;
        pData->mColor.a = intensity;
        mpBlur1->getBlurData(0)->mRadius = mData.mBlur0.mRadius;

        mData.mBlur2.mIntensity = mData.mBlur1.mIntensity * 0.5f;
        mData.mBlur2.mRadius = mData.mBlur1.mRadius * 2.0f;

        mpBlur2->setNumBlurData(mData.mNumBlur);
        for (int i = 0; i < mData.mNumBlur; i++) {
            u8 c = (&mData.mBlur1)[i].mIntensity * 255.0f;
            PostEffectBlur::BlurData* pData = mpBlur2->getBlurData(i);
            pData->mColor.r = c;
            pData->mColor.g = c;
            pData->mColor.b = c;
            pData->mColor.a = c;
            mpBlur2->getBlurData(i)->mRadius = mData.mBlur1.mRadius;
            mpBlur2->getBlurData(i)->BYTE_0x0 = (i + 1) * 8;
        }

        mpBlurGather->getBlurGatherData(0).mColorScale = mData.mGatherScale0;
        mpBlurGather->getBlurGatherData(1).mColorScale = mData.mGatherScale1;

        mpSimple->setColor(mData.mColor);
        mpSimple->setBlendMode(BLEND_MODES[mData.mBlendModeIdx]);
    }

    mpMask->setUnk20((mFlags & 2) != 0);
}

void DrawPathBloom::internalDraw(u16 step) {
    const Screen& rScreen = IDrawGX::getScreen();

    switch (step) {
    case cStep_0: {
        TextureBuffer* pEfb = captureEfb(cBufferType_3, isFlag8());

        bool large = mData.mFlags & 4;
        int div = 4;
        if (large) {
            div = 2;
        }

        bool half = false;
        if ((mFlags & 4) && !large) {
            half = true;
        }

        bool vfilter = (mData.mFlags & 8) != 0;

        if (!isGlbFlag1()) {
            pushWorkBuffer(half && !isFlag8() ? cWorkBuffer_1 : cWorkBuffer_0);
        }

        const Screen::DataEfb& rEfb = rScreen.GetDataEfb();
        u16 texW = rEfb.vp.width / div;
        u16 texH = rEfb.vp.height / div;
        u16 texHalfW = texW / 2;
        u16 texHalfH = texH / 2;
        u16 drawW = rScreen.GetWidth() / div;
        u16 drawH = rScreen.GetHeight() / div;
        u32 drawHalfW = drawW >> 1;
        u32 drawHalfH = drawH >> 1;

        PostEffectBase::setProjection(rScreen);

        const f32* pVp = shiftWorkSpaceViewportGX();
        u16 x = pVp[0];
        u16 y = pVp[1];

        GXTexFmt fmt = GX_TF_I8;
        if (mFlags & 2) {
            fmt = GX_TF_IA8;
        }

        TextureBuffer* pMaskBuf = TextureBuffer::alloc(texW, texH, fmt);
        mpTexBuffer = TextureBuffer::alloc(texW, texH, GX_TF_RGBA8);

        StateGX::AutoAlphaUpdate alphaLock(true);
        StateGX::AutoDither ditherLock(false);
        GXInvalidateTexAll();

        pEfb->setLoadMap(GX_TEXMAP0);

        PostEffectSimple simple;
        simple.configure();
        simple.setSimpleBlendMode(1);
        simple.setCapTexture(pEfb);
        f32 scale = 0.5f;
        if (half) {
            simple.setScale(scale, scale);
        }
        simple.draw(scale * rScreen.GetWidth(), scale * rScreen.GetHeight());

        pMaskBuf->capture(x, y);
        pMaskBuf->setLoadMap(GX_TEXMAP1);
        mpMask->setCapTexture(pMaskBuf);
        mpMask->draw(drawW, drawH);

        TextureBuffer* pBlurBuf = TextureBuffer::alloc(texW, texH, GX_TF_RGBA8);
        TextureBuffer* pGather0 = TextureBuffer::alloc(texW, texH, GX_TF_RGBA8);
        if (vfilter) {
            pBlurBuf->setVFilterBlur();
            pGather0->setVFilterBlur();
        }

        {
            StateGX::AutoDither ditherLock2(false);

            pBlurBuf->capture(x, y);
            StateGX::setPixelFormatGX(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
            mpBlurSimple->setCapTexture(pBlurBuf);
            mpBlur1->setCapTexture(pBlurBuf);

            if (vfilter) {
                mpBlur1->draw(drawW, drawH);
            } else {
                mpBlurSimple->setUnk30(0);
                mpBlurSimple->setBlendMode(PostEffectBase::cBlendMode_None);
                mpBlurSimple->draw(drawW, drawH);

                if (mData.mFlags & 2) {
                    pBlurBuf->capture(x, y);
                    pBlurBuf->setLoadMap(GX_TEXMAP1);
                    mpBlurSimple->setUnk30(1);
                    mpBlurSimple->setBlendMode(PostEffectBase::cBlendMode_Normal);
                    mpBlurSimple->draw(drawW, drawH);
                }
            }

            pGather0->capture(x, y);

            TextureBuffer* pBlur2Buf = TextureBuffer::alloc(texHalfW, texHalfH, GX_TF_RGBA8);
            TextureBuffer* pGather1 = TextureBuffer::alloc(texHalfW, texHalfH, GX_TF_RGBA8);
            if (vfilter) {
                pBlur2Buf->setVFilterBlur();
                pGather1->setVFilterBlur();
            }

            if (mData.mFlags & 1) {
                pBlur2Buf->capture(x, y, true);
                mpBlur2->setCapTexture(pBlur2Buf);
                mpBlur2->draw(drawHalfW, drawHalfH);
                pGather1->capture(x, y);
            }

            pMaskBuf->setLoadMap(GX_TEXMAP2);
            mpBlurGather->setCapTexture(pMaskBuf);
            mpBlurGather->getBlurGatherData(0).mpCapTexture = pGather0;
            mpBlurGather->getBlurGatherData(1).mpCapTexture = pGather1;
            mpBlurGather->setNumBlurGather((mData.mFlags & 1) + 1);
            pGather0->setLoadMap(GX_TEXMAP0);
            pGather1->setLoadMap(GX_TEXMAP1);
            mpBlurGather->draw(drawW, drawH);

            mpTexBuffer->enableAlphaUpdate();
            GXColor clear = {0, 0, 0, 0};
            mpTexBuffer->setClearColor(clear);
            mpTexBuffer->capture(x, y);

            StateGX::setPixelFormatGX(StateGX::getDefaultPixelFormat(),
                                      StateGX::getDefaultZFmt16());

            pMaskBuf->free();
            pBlurBuf->free();
            pGather0->free();
            pBlur2Buf->free();
            pGather1->free();
        }
        break;
    }

    case cStep_1: {
        popWorkBuffer(false);
        releaseEfb(cBufferType_3);
        break;
    }

    case cStep_2: {
        StateGX::AutoColorUpdate colorLock(true);
        StateGX::AutoAlphaUpdate alphaLock(false);
        StateGX::AutoDither ditherLock(true);

        PostEffectBase::setProjection(rScreen);
        mpSimple->setCapTexture(mpTexBuffer);
        mpSimple->draw(rScreen.GetWidth(), rScreen.GetHeight());

        mpTexBuffer->free();
        mpTexBuffer = NULL;
        break;
    }
    }
}

const char* DrawPathBloom::GetBinaryType() const {
    return "PBLM";
}

void DrawPathBloom::SetBinaryInner(const Bin& rBin) {
    switch (rBin.header.version) {
    case 0: {
        mData = rBin.data;
        mData.mGatherScale0 = mData.mGatherScale1 = 0.0f;
        break;
    }
    case 1: {
        mData = rBin.data;
        break;
    }
    }
}

void DrawPathBloom::GetBinaryInner(Bin* pData) const {
#line 445
    EGG_ASSERT(pData);
    pData->data = mData;
}

} // namespace EGG

