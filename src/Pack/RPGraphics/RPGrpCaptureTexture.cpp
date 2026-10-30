#include <Pack/RPGraphics.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPGraphics/RPGrpCaptureTexture.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

#include <egg/gfxe.h>

/**
 * @brief Constructor
 *
 * @param pScreen Capture source
 * @param format Texture format
 * @param flags Creation flags
 */
RPGrpCaptureTexture::RPGrpCaptureTexture(RPGrpScreen* pScreen, GXTexFmt format,
                                         u32 flags)
    : mEfbFlags(0), mTexFlags(0), mpScreen(pScreen) {

    if (flags & EFlag_MipMap) {
        mTexFlags |= ETexFlag_MipMap;
    }

    bool mipmap = IsMipMapEnable();
    const RPGrpScreen::DataEfb& rEfb = pScreen->GetDataEfb();

    mpCapTexture = new EGG::CapTexture(
        mipmap ? rEfb.vp.width / 2 : rEfb.vp.width,
        mipmap ? rEfb.vp.height / 2 : rEfb.vp.height, format);

    mpCapTexture->configure();
    mpCapTexture->alloc();
}

/**
 * @brief Captures the EFB contents as a texture
 *
 * @param format Texture format override (optional)
 */
void RPGrpCaptureTexture::Capture(int format) {
    if (mEfbFlags & EEfbFlag_ColorUpdate) {
        mpCapTexture->enableColorUpdate();
    } else {
        mpCapTexture->disableColorUpdate();
    }

    if (mEfbFlags & EEfbFlag_AlphaUpdate) {
        mpCapTexture->enableAlphaUpdate();
    } else {
        mpCapTexture->disableAlphaUpdate();
    }

    if (mEfbFlags & EEfbFlag_ZBufferUpdate) {
        mpCapTexture->enableZBufferUpdate();
    } else {
        mpCapTexture->disableZBufferUpdate();
    }

    const RPGrpScreen::DataEfb& rEfb = mpScreen->GetDataEfb();
    mpCapTexture->capture(rEfb.vp.x, rEfb.vp.y, IsMipMapEnable(), format);

    GXInvalidateTexAll();
}

// Wii Play: not in Wii Sports
void RPGrpCaptureTexture::SetFilter(int filter) {
    switch (filter) {
    case 0:
        mpCapTexture->setMinFilter(GX_LINEAR);
        mpCapTexture->setMagFilter(GX_LINEAR);
        break;
    case 1:
        mpCapTexture->setMinFilter(GX_NEAR);
        mpCapTexture->setMagFilter(GX_NEAR);
        break;
    }
}
