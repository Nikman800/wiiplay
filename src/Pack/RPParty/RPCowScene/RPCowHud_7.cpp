#include <Pack/RPParty/RPCowScene/RPCowHud_7.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>

void RPCowStartMsg::Show() {
    mpPane->SetVisible(true);
    mpLayout->unbindAnmObj(mpAnmObj);
    RPSysLayout::bindAnmObjToPane(mpAnmObj, mpPane, true);
    mpAnmObj->restart();
    mTimer = 150;
    if (mState != 1) {
        mState = 1;
    }
}

RPCowStartMsg::RPCowStartMsg() {}

RPCowStartMsg::~RPCowStartMsg() {}

void RPCowStartMsg::LoadResource() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpLayout = RPSysLayout::create(NULL, pGame->GetLytAccessor(), "cowStart_00.brlyt");
    const char* anm[] = {"cowStart_00_start_00.brlan"};
    for (int i = 0; i < 1; i++) {
        (&mpAnmObj)[i] = mpLayout->createAnmObj(NULL, pGame->GetLytAccessor(), anm[i]);
    }
    const char* pane[] = {"N_start_00"};
    for (int i = 0; i < 1; i++) {
        (&mpPane)[i] = mpLayout->findPane(pane[i]);
    }
    RPSysMessage* pMsg = RPCls803572E0::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_start_00")->setMessage(pMsg, 0xA92, 0, 0);
    mpLayout->findTextBox("T_start_01")->setMessage(pMsg, 0xA92, 0, 0);
}

void RPCowStartMsg::OnStart() {
    mpPane->SetVisible(false);
    mTimer = 0;
    mState = 0;
    mpLayout->unbindAllAnmObj();
    mpAnmObj->resetFrame();
}

void RPCowStartMsg::OnStop() {}

void RPCowStartMsg::OnActivate() {
    if (mpPane->IsVisible() && mpAnmObj->isFinished()) {
        mpPane->SetVisible(false);
    }
    mpLayout->calc();
}

void RPCowStartMsg::DrawScreen(u8 pass) {
    if (pass == 4) {
        mpLayout->draw();
    }
}
