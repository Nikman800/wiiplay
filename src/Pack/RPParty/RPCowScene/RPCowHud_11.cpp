#include <Pack/RPParty/RPCowScene/RPCowHud_11.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPParty/RPCowScene/RPCowCamera.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPCowScene/RPCowPlayer.h>
#include <egg/gfxe/eggScreen.h>

namespace {
inline RPCowCamera* GetCamera(RPCowGame* pGame, u16 idx) {
    return &static_cast<RPCowCamera*>(pGame->GetSlotActor(0))[idx];
}
inline RPCowPlayer* GetPlayer(RPCowGame* pGame, u16 idx) {
    return &static_cast<RPCowPlayer*>(pGame->GetSlotActor(7))[idx];
}
}

void RPCowTargetMarker::Show() {
    mpPane->SetVisible(true);
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[0], mpPane, true);
    mpAnmObjs[0]->restart();
    mState = 1;
}

void RPCowTargetMarker::RestartAnim() {
    mpAnmObjs[1]->restart();
    mState = 0;
}

RPCowTargetMarker::RPCowTargetMarker() {}

RPCowTargetMarker::~RPCowTargetMarker() {}

void RPCowTargetMarker::LoadResource() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpLayout = RPSysLayout::create(NULL, pGame->GetLytAccessor(), "cowOrder_01.brlyt");
    const char* anm[] = {"cowOrder_01_in_00.brlan", "cowOrder_01_out_00.brlan",
                         "cowOrder_01_cursor_00.brlan"};
    for (int i = 0; i < 3; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, pGame->GetLytAccessor(), anm[i]);
    }
    const char* pane[] = {"N_cow_00"};
    for (int i = 0; i < 1; i++) {
        (&mpPane)[i] = mpLayout->findPane(pane[i]);
    }
}

void RPCowTargetMarker::OnStart() {
    mpPane->SetVisible(false);
    unk3C = 0;
    mState = 0;
    mpLayout->unbindAllAnmObj();
}

void RPCowTargetMarker::OnStop() {}

void RPCowTargetMarker::OnActivate() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    if (mpPane->IsVisible()) {
        RPCowCamera* pCamera = GetCamera(pGame, GetUnk1A());
        EGG::Vector3f pos = GetPlayer(pGame, GetUnk1A())->GetTargetPos();
        pos.y += 20.0f;
        pos = pCamera->ProjectToScreen(pos);
        pos.x *= (f32)EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD) / (f32)EGG::Screen::GetSizeXMax();
        pos.y *= (f32)EGG::Screen::GetSizeYMax(EGG::Screen::TV_MODE_STD) / (f32)EGG::Screen::GetSizeYMax();
        mpPane->SetTranslate(pos);
    }
    mpLayout->calc();
    switch (mState) {
    case 1:
        if (mpPane->IsVisible() && mpAnmObjs[0]->isFinished()) {
            mpLayout->unbindAnmObj(mpAnmObjs[1]);
            RPSysLayout::bindAnmObjToPane(mpAnmObjs[1], mpPane, true);
            mpAnmObjs[1]->resetFrame();
            mpLayout->unbindAnmObj(mpAnmObjs[2]);
            RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[2], mpPane, true);
            mpAnmObjs[2]->restart();
            mState = 2;
        }
        break;
    case 0:
        if (mpPane->IsVisible() && mpAnmObjs[1]->isFinished()) {
            mpPane->SetVisible(false);
            RPCowGame::GetInstance()->StopActor(this);
        }
        break;
    }
}

void RPCowTargetMarker::DrawScreen(u8 pass) {
    if ((GetUnk1A() == 0 && pass == 2) || (GetUnk1A() == 1 && pass == 3)) {
        mpLayout->draw();
    }
}
