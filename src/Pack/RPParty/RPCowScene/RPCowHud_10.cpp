#include <Pack/RPParty/RPCowScene/RPCowHud_10.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysStringUtility.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>

void RPCowVsResult::Show() {
    mpPanes[0]->SetVisible(true);
    mpPanes[1]->SetVisible(true);
    mpPanes[2]->SetVisible(true);
    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[0], mpPanes[0], true);
    mpAnmObjs[0]->restart();
    mpLayout->unbindAnmObj(mpAnmObjs[2]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[2], mpPanes[1], true);
    mpAnmObjs[2]->setRate(0.0f);
    mpAnmObjs[2]->restart();
    mpLayout->unbindAnmObj(mpAnmObjs[3]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[3], mpPanes[2], true);
    mpAnmObjs[3]->setRate(0.0f);
    mpAnmObjs[3]->restart();
    mState = 1;
}

void RPCowVsResult::PlayOut() {
    mpLayout->unbindAnmObj(mpAnmObjs[1]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[1], mpPanes[0], true);
    mpAnmObjs[1]->restart();
    mState = 2;
}

void RPCowVsResult::ShowWinner(s32 player) {
    mpLayout->unbindAnmObj(mpAnmObjs[2]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[2], mpPanes[1], true);
    mpLayout->unbindAnmObj(mpAnmObjs[3]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[3], mpPanes[2], true);
    switch (player) {
    case 0:
        mpAnmObjs[2]->setRate(1.0f);
        mpAnmObjs[3]->setRate(0.0f);
        break;
    case 1:
        mpAnmObjs[2]->setRate(0.0f);
        mpAnmObjs[3]->setRate(1.0f);
        break;
    }
    mpAnmObjs[2]->restart();
    mpAnmObjs[3]->restart();
}

void RPCowVsResult::SetScore(s32 player, s32 score) {
    wchar_t buf[8];
    RPSysStringUtility::GetStringFullSizeNumber(score, buf, 4);
    switch (player) {
    case 0:
        mpTextBoxes[0]->setText(buf, 0);
        mpTextBoxes[1]->setText(buf, 0);
        break;
    case 1:
        mpTextBoxes[2]->setText(buf, 0);
        mpTextBoxes[3]->setText(buf, 0);
        break;
    }
}

RPCowVsResult::RPCowVsResult() {}

RPCowVsResult::~RPCowVsResult() {}

void RPCowVsResult::LoadResource() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpLayout = RPSysLayout::create(NULL, pGame->GetLytAccessor(), "cowVsResult_00.brlyt");
    const char* anm[] = {"cowVsResult_00_in_00.brlan", "cowVsResult_00_out_00.brlan",
                         "cowVsResult_00_win1p_00.brlan", "cowVsResult_00_win2p_00.brlan"};
    for (int i = 0; i < 4; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, pGame->GetLytAccessor(), anm[i]);
    }
    const char* pane[] = {"N_result_00", "N_score_00", "N_score_01"};
    for (int i = 0; i < 3; i++) {
        mpPanes[i] = mpLayout->findPane(pane[i]);
    }
    const char* text[] = {"T_score00_00", "T_score01_00", "T_score00_01", "T_score01_01"};
    for (int i = 0; i < 4; i++) {
        mpTextBoxes[i] = mpLayout->findTextBox(text[i]);
    }
    RPSysMessage* pMsg = RPCls803572E0::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_player00_00")->setMessage(pMsg, 0xA98, 0, 0);
    mpLayout->findTextBox("T_player00_01")->setMessage(pMsg, 0xA98, 1, 0);
}

void RPCowVsResult::OnStart() {
    for (int i = 0; i < 3; i++) {
        mpPanes[i]->SetVisible(false);
    }
    unk58 = 0;
    mState = 2;
    mpLayout->unbindAllAnmObj();
    for (int i = 0; i < 4; i++) {
        mpAnmObjs[i]->resetFrame();
    }
}

void RPCowVsResult::OnStop() {}

void RPCowVsResult::OnActivate() {
    switch (mState) {
    case 2:
        if (mpPanes[0]->IsVisible() && mpAnmObjs[1]->isFinished()) {
            mpPanes[0]->SetVisible(false);
        }
        break;
    }
    mpLayout->calc();
}

void RPCowVsResult::DrawScreen(u8 pass) {
    if (pass == 4) {
        mpLayout->draw();
    }
}
