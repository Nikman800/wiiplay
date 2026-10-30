#include <Pack/RPParty/RPCowScene/RPCowHud_12.h>

#include <Pack/RPKernel/RPSysKokeshiIcon.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPCowScene/RPCowCamera.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPCowScene/RPCowPlayer.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>

namespace {
inline RPCowCamera* GetCamera(RPCowGame* pGame, u16 idx) {
    return &static_cast<RPCowCamera*>(pGame->GetSlotActor(0))[idx];
}
inline RPCowPlayer* GetPlayer(RPCowGame* pGame, u16 idx) {
    return &static_cast<RPCowPlayer*>(pGame->GetSlotActor(7))[idx];
}
}

void RPCowBalloon::Show() {
    mpPanes[0]->SetVisible(true);
    if (GetUnk1A() == 0) {
        mpPanes[1]->SetVisible(true);
        mpPanes[3]->SetVisible(true);
    } else if (GetUnk1A() == 1) {
        mpPanes[2]->SetVisible(true);
        mpPanes[4]->SetVisible(true);
    }
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[0], mpPanes[0], true);
    mpAnmObjs[0]->restart();
    mState = 1;
}

void RPCowBalloon::Hide() {
    mpLayout->unbindAnmObj(mpAnmObjs[1]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[1], mpPanes[0], true);
    mpAnmObjs[1]->restart();
    mState = 2;
}

void RPCowBalloon::VF_0x3C() {
    RPSysKokeshiGenInfo info;
    info.SetGenType(RPSysKokeshiManager::GenType_Player);
    info.SetPlayerNo(GetUnk1A());
    GXColor bgColor = {0, 0, 0, 0};
    info.SetIconWidth(0x40);
    info.SetIconHeight(0x40);
    info.SetIconBGType(RFLIconBG_Direct);
    info.SetIconBGColor(bgColor);
    info.SetIconMipMap(true);

    mpIcon = new RPSysKokeshiIcon(&info);
}

void RPCowBalloon::VF_0x40() {}

void RPCowBalloon::LoadResource() {
    mpIcon->LoadResource();
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpLayout = RPSysLayout::create(NULL, pGame->GetLytAccessor(), "cowBalloon_00.brlyt");
    const char* anm[] = {"cowBalloon_00_in_00.brlan", "cowBalloon_00_out_00.brlan"};
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, pGame->GetLytAccessor(), anm[i]);
    }
    const char* pane[] = {"N_cow_00", "N_balloon_00", "N_balloon_01", "N_player_00", "N_player_01"};
    for (int i = 0; i < 5; i++) {
        mpPanes[i] = mpLayout->findPane(pane[i]);
    }
    RPSysMessage* pMsg = RPCls803572E0::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_balloon00_00")->setMessage(pMsg, 0xA8E, 0, 0);
    mpLayout->findTextBox("T_balloon01_00")->setMessage(pMsg, 0xA8E, 0, 0);
    mpLayout->findTextBox("T_balloon00_01")->setMessage(pMsg, 0xA8E, 1, 0);
    mpLayout->findTextBox("T_balloon01_01")->setMessage(pMsg, 0xA8E, 1, 0);
}

void RPCowBalloon::OnStart() {
    for (int i = 0; i < 5; i++) {
        mpPanes[i]->SetVisible(false);
    }
    unk4C = 0;
    mState = 0;
    mpLayout->unbindAllAnmObj();
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i]->resetFrame();
    }
}

void RPCowBalloon::OnStop() {}

void RPCowBalloon::OnActivate() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    if (mpPanes[0]->IsVisible()) {
        RPCowCamera* pCamera = GetCamera(pGame, GetUnk1A());
        EGG::Vector3f pos = GetPlayer(pGame, GetUnk1A())->GetPos();
        pos.y += 11.3f;
        pos = pCamera->ProjectToScreen(pos);
        mpPanes[0]->SetTranslate(pos);
    }
    mpLayout->calc();
    switch (mState) {
    case 1:
        if (mpPanes[0]->IsVisible() && mpAnmObjs[0]->isFinished()) {
        } else if (mpAnmObjs[1]->isFinished()) {
        }
        break;
    case 2:
        if (mpPanes[0]->IsVisible() && mpAnmObjs[1]->isFinished()) {
            mpPanes[0]->SetVisible(false);
            RPCowGame::GetInstance()->StopActor(this);
        }
        break;
    }
}

void RPCowBalloon::DrawScreen(u8 pass) {
    if (mState == 1 || mState == 2) {
        if (GetUnk1A() == 0 && pass == 2) {
            mpLayout->draw();
            RPDrawKokeshiIcon(mpIcon, mpPanes[3], FALSE, NULL);
        } else if (GetUnk1A() == 1 && pass == 3) {
            mpLayout->draw();
            RPDrawKokeshiIcon(mpIcon, mpPanes[4], FALSE, NULL);
        }
    }
}
