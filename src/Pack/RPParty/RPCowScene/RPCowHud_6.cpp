#include <Pack/RPParty/RPCowScene/RPCowHud_6.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>

namespace {
inline RPCowTargetMarker* GetTargetMarker(RPCowGame* pGame, u16 idx) {
    return &static_cast<RPCowTargetMarker*>(pGame->GetSlotActor(22))[idx];
}
inline void RestartAllMarkers(RPCowGame* pGame) {
    s32 playerNum = pGame->GetPlayerNum();
    for (int i = 0; i < playerNum; i++) {
        GetTargetMarker(pGame, i)->RestartAnim();
    }
}
inline RPCowBalloon* GetBalloon(RPCowGame* pGame, u16 idx) {
    return &static_cast<RPCowBalloon*>(pGame->GetSlotActor(23))[idx];
}
}

void RPCowOrderMsg::Show() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpRootPane->SetVisible(true);
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[0], mpRootPane, true);
    mpAnmObjs[0]->restart();
    int playerNum = pGame->GetPlayerNum();
    if (playerNum >= 2) {
        for (int i = 0; i < playerNum; i++) {
            GetBalloon(pGame, i)->Show();
        }
    }
    RPSysMessage* pMsg = RPCls803572E0::GetInstance()->GetMessage();
    if (playerNum <= 1) {
        mpTextBoxes[0]->setMessage(pMsg, 0xA91, 0, 0);
        mpTextBoxes[1]->setMessage(pMsg, 0xA91, 0, 0);
    } else {
        mpTextBoxes[0]->setMessage(pMsg, 0xA91, 1, 0);
        mpTextBoxes[1]->setMessage(pMsg, 0xA91, 1, 0);
    }
    mTimer = 180;
    if (mState != 1) {
        mState = 1;
    }
}

void RPCowOrderMsg::ShowAllTargetMarkers() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    s32 playerNum = pGame->GetPlayerNum();
    for (int i = 0; i < playerNum; i++) {
        GetTargetMarker(pGame, i)->Show();
    }
}

void RPCowOrderMsg::HideAllBalloons() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    s32 playerNum = pGame->GetPlayerNum();
    for (int i = 0; i < playerNum; i++) {
        GetBalloon(pGame, i)->Hide();
    }
}

RPCowOrderMsg::RPCowOrderMsg() {}

RPCowOrderMsg::~RPCowOrderMsg() {}

void RPCowOrderMsg::LoadResource() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpLayout = RPSysLayout::create(NULL, pGame->GetLytAccessor(), "cowOrder_00.brlyt");
    const char* anm[] = {"cowOrder_00_in_00.brlan", "cowOrder_00_out_00.brlan"};
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, pGame->GetLytAccessor(), anm[i]);
    }
    const char* pane[] = {"N_order_00"};
    for (int i = 0; i < 1; i++) {
        (&mpRootPane)[i] = mpLayout->findPane(pane[i]);
    }
    const char* text[] = {"T_order_00", "T_order_01"};
    for (int i = 0; i < 2; i++) {
        mpTextBoxes[i] = mpLayout->findTextBox(text[i]);
    }
}

void RPCowOrderMsg::OnStart() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpRootPane->SetVisible(false);
    if (pGame->GetPlayerNum() <= 1) {
        mpRootPane->SetTranslate(nw4r::math::VEC3(0.0f, -105.0f, 0.0f));
    } else {
        mpRootPane->SetTranslate(nw4r::math::VEC3(0.0f, 0.0f, 0.0f));
    }
    mTimer = 0;
    mState = 0;
    mpLayout->unbindAllAnmObj();
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i]->resetFrame();
    }
}

void RPCowOrderMsg::OnStop() {}

void RPCowOrderMsg::OnActivate() {
    if (mTimer > 0) {
        if (--mTimer <= 0) {
            RPCowGame* pGame = RPCowGame::GetInstance();
            mpLayout->unbindAnmObj(mpAnmObjs[1]);
            RPSysLayout::bindAnmObjToPane(mpAnmObjs[1], mpRootPane, true);
            mpAnmObjs[1]->restart();
            RestartAllMarkers(pGame);
        }
    } else if (mpRootPane->IsVisible() && mpAnmObjs[1]->isFinished()) {
        mpRootPane->SetVisible(false);
    }
    mpLayout->calc();
}

void RPCowOrderMsg::DrawScreen(u8 pass) {
    if (pass == 4) {
        mpLayout->draw();
    }
}
