#include <Pack/RPParty/RPCowScene/RPCowHud_9.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>

void RPCowLimitCount::ShowFinish() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpPanes[0]->SetVisible(true);
    if (pGame->GetPlayerNum() <= 1) {
        mpPanes[1]->SetVisible(true);
    } else {
        mpPanes[2]->SetVisible(true);
    }
    mpLayout->unbindAnmObj(mpAnmObjs[1]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[1], mpPanes[0], true);
    mpAnmObjs[1]->restart();
    mTimer = 180;
    mState = 2;
}

void RPCowLimitCount::Start() {
    for (int i = 0; i < 14; i++) {
        mpPanes[i]->SetVisible(false);
    }
    mpPanes[0]->SetVisible(true);
    mpPanes[3]->SetVisible(true);
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[0], mpPanes[3], true);
    mpAnmObjs[0]->restart();
    mTimer = 0;
    mState = 1;
}

void RPCowLimitCount::HideMessage() {
    mpPanes[3]->SetVisible(false);
}

void RPCowLimitCount::SetCount(s32 count) {
    s32 show[] = {-1, -1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    s32 hide[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, -1, -1};
    if (show[count] >= 0) {
        mpPanes[show[count]]->SetVisible(true);
    }
    if (hide[count] >= 0) {
        mpPanes[hide[count]]->SetVisible(false);
    }
}

RPCowLimitCount::RPCowLimitCount() {}

RPCowLimitCount::~RPCowLimitCount() {}

void RPCowLimitCount::LoadResource() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpLayout = RPSysLayout::create(NULL, pGame->GetLytAccessor(), "cowLimit_00.brlyt");
    const char* anm[] = {"cowLimit_00_count_00.brlan", "cowLimit_00_in_00.brlan",
                         "cowLimit_00_out_00.brlan"};
    for (int i = 0; i < 3; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, pGame->GetLytAccessor(), anm[i]);
    }
    const char* pane[] = {"N_messageAll_00", "N_message_00", "N_message_01", "N_countAll_00",
                          "N_count_00",      "N_count_01",   "N_count_02",   "N_count_03",
                          "N_count_04",      "N_count_05",   "N_count_06",   "N_count_07",
                          "N_count_08",      "N_count_09"};
    for (int i = 0; i < 14; i++) {
        mpPanes[i] = mpLayout->findPane(pane[i]);
    }
    RPSysMessage* pMsg = RPCls803572E0::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_message00_00")->setMessage(pMsg, 0xA96, 0, 0);
    mpLayout->findTextBox("T_message01_00")->setMessage(pMsg, 0xA96, 0, 0);
    mpLayout->findTextBox("T_message00_01")->setMessage(pMsg, 0xA97, 0, 0);
    mpLayout->findTextBox("T_message01_01")->setMessage(pMsg, 0xA97, 0, 0);
}

void RPCowLimitCount::OnStart() {
    for (int i = 0; i < 14; i++) {
        mpPanes[i]->SetVisible(false);
    }
    mTimer = 0;
    mState = 0;
    mpLayout->unbindAllAnmObj();
    for (int i = 0; i < 3; i++) {
        mpAnmObjs[i]->resetFrame();
    }
}

void RPCowLimitCount::OnStop() {}

void RPCowLimitCount::OnActivate() {
    switch (mState) {
    case 2:
        if (mTimer > 0) {
            if (--mTimer <= 0) {
                mpPanes[0]->SetVisible(true);
                mpLayout->unbindAnmObj(mpAnmObjs[2]);
                RPSysLayout::bindAnmObjToPane(mpAnmObjs[2], mpPanes[0], true);
                mpAnmObjs[2]->restart();
                mTimer = 0;
                mState = 3;
            }
        }
        break;
    case 3:
        if (mpAnmObjs[2]->isFinished()) {
            mpPanes[0]->SetVisible(false);
            mState = 0;
        }
        break;
    }
    mpLayout->calc();
}

void RPCowLimitCount::DrawScreen(u8 pass) {
    if (pass == 4) {
        mpLayout->draw();
    }
}
