// Fishing message balloon fishingMsg_01 (RPFshMsg01, the "pull" warning).
// Unit 0x802161E8-0x80216B44 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshMsg.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPSystem.h>

RPFshMsg01* RPFshMsg01::sInstance;

inline RPFshMsg01::RPFshMsg01() {
    mpStateMachine = new RPStateMachine<RPFshMsg01>(this, STATE_NUM);

    RPFshStateFuncs<RPFshMsg01> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPFshMsg01::InEnter, &RPFshMsg01::InCalc},
        {&RPFshMsg01::WaitEnter, &RPFshMsg01::WaitCalc},
        {&RPFshMsg01::OutEnter, &RPFshMsg01::OutCalc},
    };
    for (int i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    mpResAccessor = RPSysLytResAccessor::create(NULL);
    RPGrpRenderer::GetCurrent()->AppendDrawObject(this);
}

void RPFshMsg01::CreateInstance() {
    sInstance = new RPFshMsg01();
}

void RPFshMsg01::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshMsg01::~RPFshMsg01() {}

void RPFshMsg01::Create() {
    RPLytObjBase::AttachArcFile(RPSysResourceManager::LoadGameLocalArchive(), "fishing.arc",
                                mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "fishingMsg_01.brlyt");

    const char* anmNames[3] = {"fishingMsg_01_inAll_00.brlan", "fishingMsg_01_waitAll_00.brlan",
                               "fishingMsg_01_outAll_00.brlan"};
    for (int i = 0; i < 3; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x7D2, 0, 0);
    mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x7D2, 0, 0);
}

void RPFshMsg01::Reset() {
    mpStateMachine->Reset(STATE_NONE);

    for (int i = 0; i < 3; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }

    mpLayout->unbindAllAnmObj();
}

void RPFshMsg01::Calculate() {
    mpStateMachine->Calculate();

    if (mpStateMachine->GetState() != STATE_NONE) {
        mpLayout->calc();
    }
}

void RPFshMsg01::InEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
}

void RPFshMsg01::InCalc() {
    if (mpAnmObjs[0]->isFinished()) {
        mpStateMachine->ChangeState(STATE_WAIT);
    }
}

void RPFshMsg01::WaitEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
}

void RPFshMsg01::WaitCalc() {}

void RPFshMsg01::OutEnter() {
    mpLayout->bindAnmObj(mpAnmObjs[2]);
    mpAnmObjs[2]->restart();
}

void RPFshMsg01::OutCalc() {
    if (mpAnmObjs[2]->isFinished()) {
        mpStateMachine->ChangeState(STATE_NONE);
    }
}

void RPFshMsg01::UserDraw() {
    if (mpStateMachine->GetState() != STATE_NONE && RPGrpRenderer::GetCurrentViewNo() == 2 &&
        RPGrpRenderer::GetDrawStep() == 5) {
        mpLayout->draw();
    }
}

void RPFshMsg01::Open() {
    if (mpStateMachine->IsStateSettled(STATE_NONE)) {
        mpStateMachine->ChangeState(STATE_IN);
    }
}

void RPFshMsg01::Close() {
    if (mpStateMachine->IsStateSettled(STATE_WAIT)) {
        mpStateMachine->ChangeState(STATE_OUT);
    }
}
