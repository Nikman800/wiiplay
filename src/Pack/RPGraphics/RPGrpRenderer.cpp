#include <Pack/RPGraphics.h>
#include <Pack/RPKernel/RPSysEffectMgr.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysSystem.h>
#include <Pack/RPParty/RPPartyCommon/RPScnProcDrawHook_2.h>

#include <egg/gfxe.h>

#include <nw4r/g3d.h>
#include <nw4r/math.h>

// Wii Play: a plain store, no assert (EGG::Frustum::SetProjectionType would leave its assert
// strings in .data); same as RPGrpView.cpp
namespace {
struct RendererScreen : EGG::Screen {
    using EGG::Screen::mProjType;
};
} // namespace

// EGG::StateGX::s_flag (ledger name; RPGrpView uses it by this name too)

// The draw hook every model scene's draw path manager calls; its .sbss slot comes
// before the renderer's statics
RPScnProcDrawHook RPGrpRenderer::sScnProcDrawHook;

// Clear colour passed to the system
GXColor RPGrpRenderer::sClearColor = {0, 0, 0, 255};
s32 RPGrpRenderer::sDrawStep = 12;

RPGrpRenderer* RPGrpRenderer::spCurrent;
// Frames rendered since Begin
u8 RPGrpRenderer::sRenderFrameCount;
u8 RPGrpRenderer::sCurrentViewNo;
RPGrpView* RPGrpRenderer::spCurrentView;
RPGrpScreen* RPGrpRenderer::spCurrentScreen;
// Camera of the view being rendered
RPGrpCamera* RPGrpRenderer::spCurrentCamera;
// Set to 8 whenever a renderer becomes current (ledger name). An 8-byte object: CW
// aligns it to 8 in .sbss, which leaves the original's 4-byte gap after RPGrpRenderer::spCurrentCamera; only
// its first byte is ever written
u8 lbl_8043C5B8[8];

// View matrix the effects are drawn with (a POD so it needs no initializer)
nw4r::math::_MTX34 RPGrpRenderer::sEffectDrawMtx;

static inline nw4r::math::MTX34& EffectDrawMtx() {
    return static_cast<nw4r::math::MTX34&>(RPGrpRenderer::sEffectDrawMtx);
}


inline void RPGrpRenderer::UserDrawObjects() {
    for (IRPGrpDrawObject* pObject = mpDrawList; pObject != NULL; pObject = pObject->mpNext) {
        pObject->UserDraw();
    }
}

RPGrpRenderer::~RPGrpRenderer() {}

void RPGrpRenderer::Initialize(u16 width, u16 height) {
    EGG::GfxEngine::initialize(width, height, NULL);
    EGG::StateGX::setDefaultPixelFormat(GX_PF_RGBA6_Z24);
    EGG::StateGX::s_flag |= EGG::StateGX::cFlag_EnableCache;
    nw4r::g3d::G3dInit(false);

    EGG::IBinary<EGG::DrawPathBloom>::SetBinaryExtension("pblm");
    EGG::IBinary<EGG::LightManager>::SetBinaryExtension("plight");
    EGG::IBinary<EGG::LightObject>::SetBinaryExtension("plobj");
    EGG::IBinary<EGG::LightTexture>::SetBinaryExtension("pltex");
    EGG::IBinary<EGG::LightTextureManager>::SetBinaryExtension("plmap");

    RPGrpCamera::Initialize();
    const u16 maxX[] = {608, 832};
    u16 maxY[] = {height, height};
    RPGrpScreen::Initialize(maxX, maxY);
    RPGrpModelManager::ClearCurrent();
}

void RPGrpRenderer::SetClearColor(u8 r, u8 g, u8 b) {
    sClearColor.r = r;
    sClearColor.g = g;
    sClearColor.b = b;
    sClearColor.a = 0;
    RPSysSystem::instance()->setDisplayClearColor(sClearColor);
}

void RPGrpRenderer::Begin() {
    sRenderFrameCount = 0;
    sDrawStep = 11;
    spCurrentScreen = &RPGrpScreen::GetRootScreen();
    EGG::GfxEngine::beginDraw();
    EGG::StateGX::setModifyRatio(RPSysSystem::instance()->getModifyRatio());
}

void RPGrpRenderer::StartDebugDraw() {
    RPGrpScreen screen(RPGrpScreen::GetRootScreen());
    screen.SetNearZ(0.0f);
    screen.SetFarZ(1.0f);
    static_cast<RendererScreen&>(static_cast<EGG::Screen&>(screen)).mProjType =
        EGG::Screen::PROJ_ORTHO;
    screen.SetProjectionGX();

    nw4r::math::MTX34 mtx;
    PSMTXIdentity(mtx);
    GXLoadPosMtxImm(mtx, GX_PNMTX0);
    GXLoadNrmMtxImm(mtx, GX_PNMTX0);
}

void RPGrpRenderer::End() {
    sDrawStep = 12;
}

inline RPGrpRenderer::RPGrpRenderer(EGG::Allocator* pAllocator, u16 id)
    : mID(id), mFlags(0), unk4(1), unk6((GXColor){0, 0, 0, 0x80}), mpViewList(NULL) {
    mpModelManager = new RPGrpModelManager(pAllocator, pAllocator);

    for (int i = 0; i < MAX_VIEW; i++) {
        mpDrawViews[i] = NULL;
    }

    mpDrawList = mpDrawListTail = NULL;
}

RPGrpRenderer* RPGrpRenderer::Construct(EGG::Allocator* pAllocator, u16 id) {
    RPGrpRenderer* pRenderer = new RPGrpRenderer(pAllocator, id);
    pRenderer->BecomeCurrent();
    sDrawStep = 12;
    return pRenderer;
}

void RPGrpRenderer::Destruct() {
    mpModelManager->TerminateResources();
}

void RPGrpRenderer::BecomeCurrent() {
    spCurrent = this;
    mpModelManager->BecomeCurrent();
    lbl_8043C5B8[0] = 8;
}

void RPGrpRenderer::PreDraw() {
    mpModelManager->ResetViews();
    sDrawStep = 0;
    UserDrawObjects();
}

void RPGrpRenderer::Rendering() {
    spCurrent = this;
    sRenderFrameCount++;
    mpModelManager->BecomeCurrent();
    lbl_8043C5B8[0] = 8;
    sCurrentViewNo = MAX_VIEW;
    PSMTXIdentity(EffectDrawMtx());
    spCurrentScreen = NULL;
    spCurrentView = NULL;

    for (RPGrpView* pView = mpViewList; pView != NULL;
         pView = pView->mpNextView) {
        if (!(pView->mFlags & RPGrpView::EFlag_SysCalc)) {
            continue;
        }

        spCurrentView = pView;
        sCurrentViewNo = pView->mViewNo;
        spCurrentCamera = pView->GetEnableCamera();
        spCurrentScreen = pView->GetEnableScreen();
        PSMTXCopy(spCurrentCamera->GetSavedCameraMatrix(), EffectDrawMtx());
        EGG::DrawGX::SetCameraMtx(EffectDrawMtx());

        sScnProcDrawHook.SetEnabled(pView->mFlags & RPGrpView::EFlag_9);
        EGG::StateGX::GXSetColorUpdate_(true);
        EGG::StateGX::GXSetAlphaUpdate_(false);

        sDrawStep = 1;
        UserDrawObjects();
        pView->PrepareDraw();

        sDrawStep = 2;
        spCurrentView->PrepareDrawGX();
        UserDrawObjects();
        pView->Draw();

        if (pView->GetType() == RPGrpView::EType_Render2D &&
            (pView->mFlags & RPGrpView::EFlag_SysDraw)) {
            EGG::StateGX::resetGX();
            RPSysLayout::drawInit();

            sDrawStep = 5;
            spCurrentView->PrepareDrawGX();
            UserDrawObjects();

            // a reference bound to a converted copy: a plain u32 local took r30 (named locals are
            // coloured first), the original's r24 needs a compiler temporary
            const u32& drawScene = pView->mDrawScene;
            if (drawScene != 0xFF) {
                EGG::Screen screen(*spCurrentScreen);
                screen.SetNearZ(-10000.0f);
                screen.SetFarZ(10000.0f);
                static_cast<RendererScreen&>(screen).mProjType = EGG::Screen::PROJ_ORTHO;
                screen.SetProjectionGX();

                EGG::StateGX::AutoDither dither(true);
                EGG::StateGX::AutoAlphaUpdate alphaUpdate(false);

                if (pView->mFlags & 0x20) {
                    const EGG::Screen::DataEfb& rDataEfb = spCurrentScreen->GetDataEfb();

                    RPSysEffectMgr::EfbTexData texData;
                    texData.format = GX_TF_RGB565;
                    texData.width = rDataEfb.vp.width / 2.0f;
                    texData.height = rDataEfb.vp.height / 2.0f;

                    EGG::TextureBuffer* pTexture =
                        EGG::TextureBuffer::alloc(texData.width, texData.height, GX_TF_RGB565);

                    pTexture->capture(rDataEfb.vp.x, rDataEfb.vp.y, true, -1);
                    RPSysEffectMgr::instance()->setCaptureEfb(pTexture->getBuffer(), texData);
                    pTexture->free();

                    RPSysEffectMgr::instance()->draw(EffectDrawMtx(), drawScene, 0x30);
                }

                RPSysEffectMgr::instance()->draw(EffectDrawMtx(), drawScene, 0x20);
            }
        }

        sDrawStep = 3;
        spCurrentView->PrepareDrawGX();
        UserDrawObjects();
    }

    mpModelManager->DrawPost();

    nw4r::math::MTX34 mtx;
    PSMTXIdentity(mtx);
    EGG::DrawGX::SetCameraMtx(mtx);

    mFlags |= 2;
    sDrawStep = 11;
}

void RPGrpRenderer::PreCalculate() {
    mpModelManager->ClearDrawEntries();
}

void RPGrpRenderer::PostCalculate() {
    mpModelManager->SetCalcDisabled(!(unk4 & 1));
    mpModelManager->CalcModelScenes();

    if (mpDrawListTail != NULL) {
        mpDrawListTail->mpNext = NULL;
    }

    for (RPGrpView* pView = mpViewList; pView != NULL;
         pView = pView->mpNextView) {
        pView->Calc();
    }
}

void RPGrpRenderer::CalculateInPause() {
    mpModelManager->CalcWorldModelScenes();

    for (RPGrpView* pView = mpViewList; pView != NULL;
         pView = pView->mpNextView) {
        pView->Calc();
    }
}

void RPGrpRenderer::PostDraw() {
    sDrawStep = 4;
    UserDrawObjects();
}

void RPGrpRenderer::CreateView3D(u8 viewNo, RPGrpCamera* pCamera, RPGrpScreen* pScreen) {
    RPGrpViewRender3D* pView = new RPGrpViewRender3D(viewNo);
    pView->Configure();
    pView->AttachCamera(pCamera);
    pView->AttachScreen(pScreen);
    mpDrawViews[viewNo] = pView;
}

void RPGrpRenderer::CreateView2D(u8 viewNo, RPGrpScreen* pScreen) {
    RPGrpView* pView = new RPGrpViewRender2D(viewNo);
    pView->Configure();
    pView->AttachScreen(pScreen);
    pView->SetDrawScene(0xFF);
    mpDrawViews[viewNo] = pView;
}

void RPGrpRenderer::LinkViews() {
    for (int i = 0; i < 8; i++) {
        if (mpModelManager->GetModelScene((u8)i) != NULL) {
            mpModelManager->GetDrawPathManager(i)->SetDrawHook(&sScnProcDrawHook);
        }
    }

    RPGrpView* pPrev = NULL;
    for (int i = 0; i < MAX_VIEW; i++) {
        RPGrpView* pView = mpDrawViews[i];
        if (pView != NULL) {
            if (pPrev != NULL) {
                pPrev->mpNextView = pView;
            } else {
                mpViewList = pView;
            }
            pPrev = pView;
        }
    }

    if (pPrev != NULL) {
        pPrev->mpNextView = NULL;
    }

    mFlags |= 1;
}

void RPGrpRenderer::AppendDrawObject(IRPGrpDrawObject* pObject) {
    if (mpDrawListTail != NULL) {
        mpDrawListTail->mpNext = pObject;
        mpDrawListTail = pObject;
    } else {
        mpDrawListTail = pObject;
        mpDrawList = pObject;
    }
}

void RPGrpView::PrepareDrawGX() {
    SetProjectionGX();
}

// Draw step for each scene-renderer step (13: none)
const s32 RPScnProcDrawHook::sDrawSteps[] = {6, 7, 10, 8, 9, 13};

inline void RPGrpRenderer::DrawObjectsForStep(s32 step) {
    sDrawStep = step;
    RPGrpRenderer* pRenderer = GetCurrent();
    spCurrentView->PrepareDrawGX();
    pRenderer->UserDrawObjects();
}

inline void RPGrpRenderer::DrawEffects() {
    RPGrpView* pView = GetCurrentView();
    EGG::StateGX::AutoColorUpdate colorUpdate(true);
    EGG::StateGX::AutoAlphaUpdate alphaUpdate(false);
    EGG::StateGX::AutoDither dither(true);

    RPGrpScreen* pScreen = GetCurrentScreen();
    u8 drawScene = pView->GetDrawScene();
    if (pView->mFlags & 0x20) {
        const EGG::Screen::DataEfb& rDataEfb = pScreen->GetDataEfb();

        RPSysEffectMgr::EfbTexData texData;
        texData.format = GX_TF_RGB565;
        texData.width = rDataEfb.vp.width / 2.0f;
        texData.height = rDataEfb.vp.height / 2.0f;

        EGG::TextureBuffer* pTexture =
            EGG::TextureBuffer::alloc(texData.width, texData.height, GX_TF_RGB565);

        pTexture->capture(rDataEfb.vp.x, rDataEfb.vp.y, true, -1);
        RPSysEffectMgr::instance()->setCaptureEfb(pTexture->getBuffer(), texData);
        pTexture->free();

        RPSysEffectMgr::instance()->draw(EffectDrawMtx(), drawScene, 0);
    }

    RPSysEffectMgr::instance()->draw(EffectDrawMtx(), drawScene, 0x10);
}

// The model scene comes through RPGrpView::GetDrawScene() as a u8: a direct mDrawScene read (or a
// u32 local) put the s_cacheGX base of DrawEffects' Auto* objects in r27 instead of r26.
void RPScnProcDrawHook::Draw(u16 step) {
    s32 drawStep = sDrawSteps[step];
    if (drawStep != 13) {
        RPGrpRenderer::DrawObjectsForStep(drawStep);
    }

    if (step == 5 && mEnabled) {
        RPGrpRenderer::DrawEffects();
    }
}

void RPScnProcDrawHook::VF_0x10() {}

