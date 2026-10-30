// Billiards pocket-camera sub windows (RPBilPocketCamWindow, singleton RPBilPocketCamWindow::sInstance): two
// picture-in-picture windows showing a pocket after a ball drops in, and the pocket-entry test.
// Unit 0x802CB464-0x802CBE88. Functions in reverse address order.
#include <Pack/RPParty/RPBilScene/RPBilPocketCamWindow.h>

#include <Pack/RPParty/RPBilScene/RPBilCamera.h>
#include <Pack/RPParty/RPBilScene/RPBilScene.h>
#include <Pack/RPParty/RPBilScene/RPBilTable.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPSystem.h>
#include <Pack/RPUtility/RPUtlRandom.h>

#include <egg/gfxe.h>
#include <egg/math.h>

s32 RPBilPocketCamWindow::sPocketWindowFrames = 120;
s32 RPBilPocketCamWindow::sPocketWindowClamp = 60;
s32 RPBilPocketCamWindow::sPocketWindowStep = 1;
f32 RPBilPocketCamWindow::sPocketEnterParamA = 10.0f;
f32 RPBilPocketCamWindow::sPocketEnterParamB = 30.0f;
s32 RPBilPocketCamWindow::sSubWindowDivisor = 4;

s32 RPBilPocketCamWindow::sPocketCamParam = RPBilPocketCamWindow::sPocketWindowStep + 15;
f32 RPBilPocketCamWindow::sPocketAngle60 = EGG::Math<f32>::pi() / 3.0f;
f32 RPBilPocketCamWindow::sPocketAngle80 = EGG::Math<f32>::pi() * 4.0f / 9.0f;
f32 RPBilPocketCamWindow::sPocketAngle120 = EGG::Math<f32>::pi() * 2.0f / 3.0f;

// Window positions, in screen sizes
EGG::Vector2f RPBilPocketCamWindow::sSubWindowPos[RPBilPocketCamWindow::WINDOW_NUM] = {
    EGG::Vector2f(-0.2f, 0.3f), EGG::Vector2f(0.2f, 0.3f)};

RPBilPocketCamWindow* RPBilPocketCamWindow::sInstance;

namespace {

inline bool IsStill(const EGG::Vector3f& rVel) {
    return rVel.squaredLength() <= FLT_EPSILON;
}

} // namespace

u16 RPBilPocketCamWindow::GetSubWindowWidth() {
    return EGG::Screen::GetSizeXMax() / sSubWindowDivisor;
}

u16 RPBilPocketCamWindow::GetSubWindowHeight() {
    return EGG::Screen::GetSizeYMax() / sSubWindowDivisor;
}

inline RPBilPocketCamWindow::RPBilPocketCamWindow() {
    mpResAccessor = RPSysLytResAccessor::create(NULL);
    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));
}

void RPBilPocketCamWindow::CreateInstance() {
    sInstance = new RPBilPocketCamWindow();
}

void RPBilPocketCamWindow::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilPocketCamWindow::~RPBilPocketCamWindow() {}

void RPBilPocketCamWindow::Load(EGG::Archive* pArchive) {
    RPLytObjBase::AttachArcFile(pArchive, "billiard.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "billiard_03.brlyt");
    mpWindowPane = mpLayout->findPane("W_subWindow_00");
}

void RPBilPocketCamWindow::Reset() {
    for (int i = 0; i < WINDOW_NUM; i++) {
        mPockets[i] = -1;
        mTimers[i] = 0;
        mDrawn[i] = false;
    }
}

void RPBilPocketCamWindow::Exit() {}

void RPBilPocketCamWindow::Calculate() {
    for (int i = 0; i < WINDOW_NUM; i++) {
        mDrawn[i] = false;
    }

    for (int i = 0; i < WINDOW_NUM; i++) {
        if (mTimers[i] > 0) {
            mTimers[i] -= sPocketWindowStep;
            if (mTimers[i] <= 0) {
                mPockets[i] = -1;
                mTimers[i] = 0;
            }
        }
    }

    mpLayout->calc();
}

s32 RPBilPocketCamWindow::FindEnteredPocket(const EGG::Vector3f& rPos,
                                            const EGG::Vector3f& rVel) {
    for (int i = 0; i < 6; i++) {
        EGG::Vector3f pocketPos = RPBilTable::sInstance->GetPocketPos(i);

        EGG::Vector3f diff = pocketPos - rPos;
        diff.y = 0.0f;
        f32 dist = diff.length();
        if (dist < sPocketEnterParamB) {
            if (IsStill(rVel)) {
                continue;
            }

            EGG::Vector3f dir = RPBilTable::GetPocketDirection(i);
            f32 dot = rVel.dot(dir);
            f32 angle = EGG::Math<f32>::abs(EGG::Math<f32>::atan2(rVel.cross(dir).length(), dot));

            if (angle > EGG::Math<f32>::pi()) {
                angle = EGG::Math<f32>::pi2() - angle;
            }

            f32 limit = sPocketAngle120;
            if (i == 1 || i == 4) {
                limit = sPocketAngle80;
            }

            if (dist > sPocketEnterParamA) {
                limit = (limit - sPocketAngle60) * ((dist - sPocketEnterParamA) /
                                                       (sPocketEnterParamB - sPocketEnterParamA)) +
                        sPocketAngle60;
            }

            if (angle < limit) {
                return i;
            }
        }
    }

    return -1;
}

bool RPBilPocketCamWindow::ShowPocket(s32 pocket) {
    for (int i = 0; i < WINDOW_NUM; i++) {
        if (pocket == mPockets[i]) {
            if (mDrawn[i]) {
                return true;
            }

            mDrawn[i] = true;
            mTimers[i] += sPocketCamParam;
            if (mTimers[i] > sPocketWindowFrames) {
                mTimers[i] = sPocketWindowFrames;
            }
            return true;
        }
    }

    s32 free[WINDOW_NUM];
    s32* pFree = free;
    u32 freeNum = 0;
    for (int i = 0; i < WINDOW_NUM; i++) {
        if (mPockets[i] == -1) {
            *pFree++ = i;
            freeNum++;
        }
    }

    if (freeNum != 0) {
        s32 window = free[(u32)(freeNum * RPUtlRandom::getF32())];
        mPockets[window] = pocket;
        mTimers[window] = 0;
        RPBilCameraMgr::sInstance->GetPocketCam(window)->LookAtPocket(pocket);
        return true;
    }

    return false;
}

void RPBilPocketCamWindow::ClampTimers() {
    for (int i = 0; i < WINDOW_NUM; i++) {
        mTimers[i] = mTimers[i] < sPocketWindowClamp ? mTimers[i] : sPocketWindowClamp;
    }
}

void RPBilPocketCamWindow::UserDraw() {
    RPBilScene* pScene = static_cast<RPBilScene*>(RPSysSceneMgr::instance()->getCurrentSceneRP());

    for (int i = 0; i < WINDOW_NUM; i++) {
        if (i + 2 == RPGrpRenderer::GetCurrentViewNo() && RPGrpRenderer::GetDrawStep() == 3) {
            pScene->GetSubWindowCapture(i)->Capture();
        }

        if (mPockets[i] != -1 && RPGrpRenderer::GetCurrentViewNo() == 8 &&
            RPGrpRenderer::GetDrawStep() == 5) {
            RPGrpScreen* pScreen = RPGrpRenderer::GetCurrentScreen();

            EGG::Matrix34f mtx;
            mtx.makeIdentity();
            mtx(0, 0) = GetSubWindowWidth();
            mtx(1, 1) = GetSubWindowHeight();
            mtx(0, 3) = sSubWindowPos[i].x * pScreen->GetWidth();
            mtx(1, 3) = sSubWindowPos[i].y * pScreen->GetHeight();

            EGG::DrawGX::BeginDrawQuad(EGG::DrawGX::COLORCHAN_COLOR, EGG::DrawGX::ZMODE_ALWAYS,
                                       EGG::DrawGX::BLEND_NORMAL, true, false);
            pScene->GetSubWindowCapture(i)->GetCapTexture()->load(GX_TEXMAP0);

            s32 alpha = mTimers[i] * 255 / sPocketWindowClamp;
            if (alpha < 0) {
                alpha = 0;
            } else if (alpha > 255) {
                alpha = 255;
            }

            GXColor color = {255, 255, 255, alpha};
            EGG::DrawGX::DrawDL(EGG::DrawGX::DL_QUAD_TEXTURE, mtx, color);

            nw4r::math::MTX34 paneMtx = mpWindowPane->GetGlobalMtx();
            paneMtx._03 = sSubWindowPos[i].x * pScreen->GetWidth();
            paneMtx._13 = sSubWindowPos[i].y * pScreen->GetHeight();
            mpWindowPane->SetGlobalMtx(paneMtx);
            mpWindowPane->SetGlbAlpha(alpha);
            mpLayout->draw();
        }
    }
}
