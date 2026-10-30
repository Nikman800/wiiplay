// Fishing message balloon fishingMsg_00 (RPFshMsg00, message 0x7D3).
// Original text: 0x8021588C-0x802161E8. Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshMsg.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPSystem.h>

RPFshMsg00* RPFshMsg00::sInstance;

inline RPFshMsg00::RPFshMsg00() {
    mpStateMachine = new RPStateMachine<RPFshMsg00>(this, STATE_NUM);

    RPFshStateFuncs<RPFshMsg00> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPFshMsg00::InEnter, &RPFshMsg00::InCalc},
        {&RPFshMsg00::WaitEnter, &RPFshMsg00::WaitCalc},
        {&RPFshMsg00::OutEnter, &RPFshMsg00::OutCalc},
    };
    for (int i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    mpResAccessor = RPSysLytResAccessor::create(NULL);
    RPGrpRenderer::GetCurrent()->AppendDrawObject(this);
}

void RPFshMsg00::CreateInstance() {
    sInstance = new RPFshMsg00();
}

void RPFshMsg00::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshMsg00::~RPFshMsg00() {}

void RPFshMsg00::Create() {
    RPLytObjBase::AttachArcFile(RPSysResourceManager::LoadGameLocalArchive(), "fishing.arc",
                                mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "fishingMsg_00.brlyt");

    const char* anmNames[3] = {"fishingMsg_00_inAll_00.brlan", "fishingMsg_00_waitAll_00.brlan",
                               "fishingMsg_00_outAll_00.brlan"};
    for (int i = 0; i < 3; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x7D3, 0, 0);
    mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x7D3, 0, 0);
}

void RPFshMsg00::Reset() {
    mpStateMachine->Reset(STATE_NONE);

    for (int i = 0; i < 3; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }

    mpLayout->unbindAllAnmObj();
}

void RPFshMsg00::Calculate() {
    mpStateMachine->Calculate();

    if (mpStateMachine->GetState() != STATE_NONE) {
        mpLayout->calc();
    }
}

void RPFshMsg00::InEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
}

void RPFshMsg00::InCalc() {
    if (mpAnmObjs[0]->isFinished()) {
        mpStateMachine->ChangeState(STATE_WAIT);
    }
}

void RPFshMsg00::WaitEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
}

void RPFshMsg00::WaitCalc() {}

void RPFshMsg00::OutEnter() {
    mpLayout->bindAnmObj(mpAnmObjs[2]);
    mpAnmObjs[2]->restart();
}

void RPFshMsg00::OutCalc() {
    if (mpAnmObjs[2]->isFinished()) {
        mpStateMachine->ChangeState(STATE_NONE);
    }
}

void RPFshMsg00::UserDraw() {
    if (mpStateMachine->GetState() != STATE_NONE && RPGrpRenderer::GetCurrentViewNo() == 2 &&
        RPGrpRenderer::GetDrawStep() == 5) {
        mpLayout->draw();
    }
}

void RPFshMsg00::Open() {
    if (mpStateMachine->IsStateSettled(STATE_NONE)) {
        mpStateMachine->ChangeState(STATE_IN);
    }
}

void RPFshMsg00::Close() {
    if (mpStateMachine->IsStateSettled(STATE_WAIT)) {
        mpStateMachine->ChangeState(STATE_OUT);
    }
}
