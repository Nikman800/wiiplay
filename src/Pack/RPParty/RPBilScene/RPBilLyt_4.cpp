// Billiards stroke guide (RPBilStrokeGuide, billiard_05.brlyt, singleton RPBilStrokeGuide::sInstance): the
// pull / push messages. Unit 0x802D20CC-0x802D2558 (ends with the IRPGrpDrawObject thunks).
// Functions in reverse address order.
#include <Pack/RPParty/RPBilScene/RPBilLyt_4.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPSystem.h>

RPBilStrokeGuide* RPBilStrokeGuide::sInstance;

inline RPBilStrokeGuide::RPBilStrokeGuide() {
    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));
}

void RPBilStrokeGuide::CreateInstance() {
    sInstance = new RPBilStrokeGuide();
}

void RPBilStrokeGuide::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilStrokeGuide::~RPBilStrokeGuide() {}

void RPBilStrokeGuide::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "billiard.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "billiard_05.brlyt");
    mpAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor, "billiard_05_waitMsg_00.brlan");
    mpMsgPanes[MODE_PULL] = mpLayout->findPane("N_msgL_00");
    mpMsgPanes[MODE_PUSH] = mpLayout->findPane("N_msgR_00");

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_msgPull_00")->setMessage(pMessage, 0x8FC, 5, 0);
    mpLayout->findTextBox("T_msgPull_01")->setMessage(pMessage, 0x8FC, 5, 0);
    mpLayout->findTextBox("T_msgSubPull_00")->setMessage(pMessage, 0x8FC, 2, 0);
    mpLayout->findTextBox("T_msgSubPull_01")->setMessage(pMessage, 0x8FC, 2, 0);
    mpLayout->findTextBox("T_msgPush_00")->setMessage(pMessage, 0x8FC, 6, 0);
    mpLayout->findTextBox("T_msgPush_01")->setMessage(pMessage, 0x8FC, 6, 0);
    mpLayout->findTextBox("T_msgSubPush_00")->setMessage(pMessage, 0x8FC, 3, 0);
    mpLayout->findTextBox("T_msgSubPush_01")->setMessage(pMessage, 0x8FC, 3, 0);
}

void RPBilStrokeGuide::Reset() {
    mMode = MODE_NONE;
    mpMsgPanes[MODE_PULL]->SetVisible(false);
    mpMsgPanes[MODE_PUSH]->SetVisible(false);

    mpAnmObj->resetFrame();
    mpAnmObj->stop();
    mpAnmObj->setFrame(0.0f);

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObj);
}

void RPBilStrokeGuide::Calculate() {
    if (mMode == MODE_NONE) {
        return;
    }
    mpLayout->calc();
}

void RPBilStrokeGuide::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() != 9) {
        return;
    }
    if (RPGrpRenderer::GetDrawStep() != 5) {
        return;
    }
    if (mMode == MODE_NONE) {
        return;
    }
    mpLayout->draw();
}

void RPBilStrokeGuide::SetMode(s32 mode) {
    if (mMode == mode) {
        return;
    }

    mMode = mode;
    for (int i = 0; i < MODE_NUM; i++) {
        if (i == mode) {
            mpMsgPanes[i]->SetVisible(true);
        } else {
            mpMsgPanes[i]->SetVisible(false);
        }
    }

    if (mode != MODE_NONE) {
        mpAnmObj->restart();
    } else {
        mpAnmObj->stop();
        mpAnmObj->setFrame(0.0f);
    }
}
