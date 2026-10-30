#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPMessageWindow.h>
#include <Pack/RPSystem.h>

RPSysCommonObject::RPSysCommonObject() {
    RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();

    RPGrpScreen* pScreen = new RPGrpScreen();
    pScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    pRenderer->CreateView2D(EView_2D_Cursor, pScreen);
    RPGrpRenderer::GetCurrent()->GetView(EView_2D_Cursor)->SetDrawScene(15);

    pScreen = new RPGrpScreen();
    pScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    pRenderer->CreateView2D(EView_2D_31, pScreen);

    RPGrpRenderer::GetCurrent()->AppendDrawObject(this);
}

void RPSysCommonObject::init() {
    RPMessageWindow::GetInstance()->Reset();
    RPSysSystemWinMgr::instance()->reset();
    RPSysCursorDrawMgr::instance()->init();
    RPSysHomeMenuMgr::instance()->init();
    RPSysPauseMgr::instance()->init();
}

void RPSysCommonObject::preUpdate() {
    if (RPSysSceneMgr::instance()->isNormalState() &&
        !RPSysHomeMenuMgr::instance()->isActive()) {
        RPSysCursorDrawMgr::instance()->preUpdate();

        if (RPSysSystemWinMgr::instance()->getState() == 0) {
            RPSysPauseMgr::instance()->calc();
        }
    }

    RPSysHomeMenuMgr::instance()->update();
}

void RPSysCommonObject::postUpdate() {
    if (RPSysSceneMgr::instance()->isNormalState() &&
        !RPSysHomeMenuMgr::instance()->isActive()) {
        RPSysCursorDrawMgr::instance()->postUpdate();

        if (RPSysPauseMgr::instance()->isPause()) {
            RPSysEffectMgr::instance()->update(
                RPSysEffectMgr::ECalcSetting_Cursor);
        } else {
            RPMessageWindow::GetInstance()->Calculate();
            RPSysSystemWinMgr::instance()->calc();
            RPSysEffectMgr::instance()->update(RPSysEffectMgr::ECalcSetting_All);
            RPSysSceneMgr::instance()->calcSceneFader();
        }
    }
}

namespace {

inline void DrawCommon(s32 viewNo, s32 drawStep) {
    if (viewNo == RPSysCommonObject::EView_2D_Cursor) {
        if (drawStep == 1) {
            RPSysSceneMgr::instance()->drawSceneFader();
        } else if (drawStep == 5) {
            RPMessageWindow::GetInstance()->Draw();
            RPSysPauseMgr::instance()->draw();
            RPSysSystemWinMgr::instance()->draw();
        }
    } else if (viewNo == RPSysCommonObject::EView_2D_31) {
        if (drawStep == 5) {
            RPSysHomeMenuMgr::instance()->draw();
        }
    }
}

} // namespace

void RPSysCommonObject::UserDraw() {
    DrawCommon(RPGrpRenderer::GetCurrentViewNo(), RPGrpRenderer::GetDrawStep());
}
