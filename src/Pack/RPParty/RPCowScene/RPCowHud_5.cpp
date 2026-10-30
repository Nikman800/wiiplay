#include <Pack/RPParty/RPCowScene/RPCowHud_5.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>

namespace {
// Layout element of a player: one-player layout, or player 1 / player 2 of the versus layout
inline s32 GetElement(s32 player, s32 single, s32 first, s32 second) {
    if (RPCowGame::GetInstance()->GetPlayerNum() <= 1) {
        return single;
    }
    if (player == 0) {
        return first;
    }
    if (player == 1) {
        return second;
    }
    return 0;
}
}

inline void RPCowExMsg::BindAnm(u32 anm, s32 pane) {
    mpLayout->unbindAnmObj(mpAnmObjs[anm]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[anm], mpPanes[pane], true);
}

inline void RPCowExMsg::PlayOut(s32 msg) {
    s32 out[] = {1, 3, 5};
    BindAnm(out[msg], msg);
    mpAnmObjs[out[msg]]->restart();
    mStates[msg] = 0;
}

void RPCowExMsg::ShowInputMsg(s32 player) {
    s32 text = GetElement(player, 0, 1, 2);
    s32 msg = GetElement(player, 0, 1, 2);
    s32 anm = GetElement(player, 0, 2, 4);
    mpTextBoxes[text]->setMessage(RPCls803572E0::GetInstance()->GetMessage(), 0xA8F, 0, 0);
    mpPanes[msg]->SetVisible(true);
    mpLayout->unbindAnmObj(mpAnmObjs[anm]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[anm], mpPanes[msg], true);
    mTimers[msg] = 30;
    if (mStates[msg] != 1) {
        mpAnmObjs[anm]->restart();
        mStates[msg] = 1;
    }
}

void RPCowExMsg::ShowMoveMsg(s32 player) {
    s32 text = GetElement(player, 0, 1, 2);
    s32 msg = GetElement(player, 0, 1, 2);
    s32 anm = GetElement(player, 0, 2, 4);
    mpTextBoxes[text]->setMessage(RPCls803572E0::GetInstance()->GetMessage(), 0xA8F, 2, 0);
    mpPanes[msg]->SetVisible(true);
    mpLayout->unbindAnmObj(mpAnmObjs[anm]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[anm], mpPanes[msg], true);
    mTimers[msg] = 0;
    if (mStates[msg] != 2) {
        mpAnmObjs[anm]->restart();
        mStates[msg] = 2;
    }
}

void RPCowExMsg::ShowCollisionMsg(s32 player) {
    s32 text = GetElement(player, 0, 1, 2);
    s32 msg = GetElement(player, 0, 1, 2);
    s32 anm = GetElement(player, 0, 2, 4);
    mpTextBoxes[text]->setMessage(RPCls803572E0::GetInstance()->GetMessage(), 0xA8F, 1, 0);
    mpPanes[msg]->SetVisible(true);
    mpLayout->unbindAnmObj(mpAnmObjs[anm]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[anm], mpPanes[msg], true);
    mTimers[msg] = 180;
    if (mStates[msg] != 3) {
        mpAnmObjs[anm]->restart();
        mStates[msg] = 3;
    }
}

void RPCowExMsg::Hide(s32 player) {
    PlayOut(GetElement(player, 0, 1, 2));
}

RPCowExMsg::RPCowExMsg() {}

RPCowExMsg::~RPCowExMsg() {}

void RPCowExMsg::LoadResource() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpLayout = RPSysLayout::create(NULL, pGame->GetLytAccessor(), "cowEx_00.brlyt");
    const char* anm[] = {"cowEx_00_in_00.brlan", "cowEx_00_out_00.brlan", "cowEx_00_in_00.brlan",
                         "cowEx_00_out_00.brlan", "cowEx_00_in_00.brlan", "cowEx_00_out_00.brlan"};
    for (int i = 0; i < 6; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, pGame->GetLytAccessor(), anm[i]);
    }
    const char* pane[] = {"N_cowEx_00", "N_cowEx_01", "N_cowEx_02"};
    for (int i = 0; i < 3; i++) {
        mpPanes[i] = mpLayout->findPane(pane[i]);
    }
    const char* text[] = {"T_ex_00", "T_ex_01", "T_ex_02"};
    for (int i = 0; i < 3; i++) {
        mpTextBoxes[i] = mpLayout->findTextBox(text[i]);
    }
}

void RPCowExMsg::OnStart() {
    for (int i = 0; i < 3; i++) {
        mpPanes[i]->SetVisible(false);
        mTimers[i] = 0;
        mStates[i] = 0;
    }
    mpLayout->unbindAllAnmObj();
    for (int i = 0; i < 6; i++) {
        mpAnmObjs[i]->resetFrame();
    }
}

void RPCowExMsg::OnStop() {}

void RPCowExMsg::OnActivate() {
    s32 out[] = {1, 3, 5};
    for (int i = 0; i < 3; i++) {
        switch (mStates[i]) {
        case 1:
        case 2:
        case 3:
            if (mTimers[i] > 0) {
                if (--mTimers[i] <= 0) {
                    PlayOut(i);
                }
            }
            break;
        case 4:
            if (mpAnmObjs[out[i]]->isFinished()) {
                mpPanes[i]->SetVisible(false);
                mStates[i] = 0;
            }
            break;
        }
    }
    mpLayout->calc();
}

void RPCowExMsg::DrawScreen(u8 pass) {
    if (pass == 4) {
        mpLayout->draw();
    }
}
