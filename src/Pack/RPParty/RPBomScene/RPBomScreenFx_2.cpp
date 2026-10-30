// Pose Mii full-screen effects (RPBomScreenFx): the screen capture and the masks drawn by
// RPBomScene::UserDraw. The second source file of the old RPBomScreenFx range (its own
// __sinit): 0x802DDCEC-0x802DE8C8. Functions in reverse address order.
#include <Pack/RPParty/RPBomScene/RPBomScreenFx.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpCaptureTexture.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPParty/RPBomScene/RPBomGameManager.h>
#include <Pack/RPParty/RPBomScene/RPBomPlayer.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <egg/gfxe/eggDrawGX.h>
#include <nw4r/math.h>

extern "C" {
// RPBomCamera_2
extern RPAxes Bom_rotXNeg90;
f32 Bom_getScreenWidth();
f32 Bom_getScreenHeight();
}

static nw4r::math::VEC3 sCapturePos(0.0f, 100.0f, 0.0f);



void RPBomScreenFx::CreateInstance() {
    sInstance = new RPBomScreenFx;
}

void RPBomScreenFx::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBomScreenFx::~RPBomScreenFx() {}

void RPBomScreenFx::StateNop5() {}

void RPBomScreenFx::StateNop4() {}

void RPBomScreenFx::StateNop3() {}

void RPBomScreenFx::DrawFullMask() {
    RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
    nw4r::math::MTX34 mtx;
    nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
    GXColor color = {0, 0xFF, 0, 0xFF};
    nw4r::math::VEC3 scale(pScreen->GetWidth(), pScreen->GetHeight(), 1.0f);
    RPAxes rot = Bom_rotXNeg90;
    RPDucMakeMtx(&mtx, rot, scale, pos);
    EGG::DrawGX::BeginDrawQuad(EGG::DrawGX::COLORCHAN_COLOR, EGG::DrawGX::ZMODE_ALWAYS,
                               EGG::DrawGX::BLEND_NORMAL, false, true);
    EGG::DrawGX::DrawDLWorld(EGG::DrawGX::DL_QUAD, mtx, color);
}

void RPBomScreenFx::DrawPlayerSpotlights() {
    RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
    nw4r::math::MTX34 mtx;
    nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
    GXColor color = {0, 0xFF, 0, 0xFF};
    nw4r::math::VEC3 scale(pScreen->GetWidth(), pScreen->GetHeight(), 1.0f);
    RPAxes rot = Bom_rotXNeg90;
    RPDucMakeMtx(&mtx, rot, scale, pos);
    EGG::DrawGX::BeginDrawQuad(EGG::DrawGX::COLORCHAN_COLOR, EGG::DrawGX::ZMODE_ALWAYS,
                               EGG::DrawGX::BLEND_NORMAL, false, true);
    EGG::DrawGX::DrawDLWorld(EGG::DrawGX::DL_QUAD, mtx, color);

    GXColor lightColor = {0, 0, 0, 0xFF};
    s32 playerNum = 1;
    if (RPBomGameManager::sInstance->mMode == 1) {
        playerNum = 2;
    }
    s32 i = 0;
    do {
        nw4r::math::VEC3 playerPos = RPBomPlayerManager::sInstance->mpPlayers[i]->mPos;
        pos.x = playerPos.x * mAlpha;
        pos.z = playerPos.z * mAlpha;
        scale.y = scale.x = 2.0f * unk10 * mAlpha;
        RPDucMakeMtx(&mtx, rot, scale, pos);
        EGG::DrawGX::BeginDrawCircleZ(EGG::DrawGX::COLORCHAN_COLOR, EGG::DrawGX::ZMODE_ALWAYS);
        EGG::DrawGX::DrawDLWorld(EGG::DrawGX::s_circleFillDLTbl[EGG::DrawGX::CIRCLE_32], mtx, lightColor);
    } while (++i < playerNum);
}

void RPBomScreenFx::StateNop2() {}

void RPBomScreenFx::StateNop1() {}

void RPBomScreenFx::DrawCapturedScreen() {
    nw4r::math::MTX34 mtx;
    nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
    pos = sCapturePos;
    f32 inv = 1.0f / mAlpha;
    RPGrpScreen* const pScreen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
    f32 w = inv * pScreen->GetWidth();
    f32 h = inv * pScreen->GetHeight();
    nw4r::math::VEC3 scale(w, h, 1.0f);
    RPAxes rot = Bom_rotXNeg90;
    RPDucMakeMtx(&mtx, rot, scale, pos);
    EGG::DrawGX::BeginDrawQuad(EGG::DrawGX::COLORCHAN_COLOR, EGG::DrawGX::ZMODE_XLU,
                               EGG::DrawGX::BLEND_NORMAL, true, false);
    mpCapture->SetFilter(0);
    mpCapture->GetCapTexture()->load(GX_TEXMAP0);
    EGG::DrawGX::DrawDLWorld(EGG::DrawGX::DL_QUAD_TEXTURE, mtx, EGG::DrawGX::WHITE);
}

void RPBomScreenFx::StateNop0() {}

void RPBomScreenFx::Create() {
    RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
    mpCapture = new RPGrpCaptureTexture(pScreen, GX_TF_IA8);
}

void RPBomScreenFx::Load() {}

void RPBomScreenFx::Reset() {
    mAlpha = 1.0f;
    mState = 0;
    unk10 = 10.0f;
    f32 width = Bom_getScreenWidth();
    f32 height = Bom_getScreenHeight();
    mScreenDiag = nw4r::math::FSqrt(width * width + height * height);
}

void RPBomScreenFx::Calculate() {
    (this->*mCalcFuncs[mState])();
}

void RPBomScreenFx::CaptureState() {
    (this->*mCaptureFuncs[mState])();
    mpCapture->Capture(0x2B);
}

void RPBomScreenFx::DrawState() {
    (this->*mDrawFuncs[mState])();
}

void RPBomScreenFx::SetAlpha(f32 alpha) {
    mAlpha = alpha;
}

// Defined last: CreateInstance inlines it, and this out-of-line copy (compiled first, then
// dead-stripped by the linker) puts the first set of state-function constants in .data.
RPBomScreenFx::RPBomScreenFx() {
    mCalcFuncs[0] = &RPBomScreenFx::StateNop5;
    mCalcFuncs[1] = &RPBomScreenFx::StateNop4;
    mCalcFuncs[2] = &RPBomScreenFx::StateNop3;
    mCaptureFuncs[0] = &RPBomScreenFx::DrawFullMask;
    mCaptureFuncs[1] = &RPBomScreenFx::DrawPlayerSpotlights;
    mCaptureFuncs[2] = &RPBomScreenFx::StateNop2;
    mDrawFuncs[0] = &RPBomScreenFx::StateNop1;
    mDrawFuncs[1] = &RPBomScreenFx::DrawCapturedScreen;
    mDrawFuncs[2] = &RPBomScreenFx::StateNop0;
}
