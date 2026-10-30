// Billiards hint text layout (RPBilHintLyt, billiard_04.brlyt), faded by its alpha.
// Unit 0x802D1FE4-0x802D20CC. Functions in reverse address order.
#include <Pack/RPParty/RPBilScene/RPBilLyt.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPSystem.h>

void RPBilHintLyt::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "billiard.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "billiard_04.brlyt");
    mpPane = mpLayout->findPane("N_msg_00");

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x8FC, 0, 0);
    mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x8FC, 0, 0);
}

void RPBilHintLyt::Reset() {
    mAlpha = 0;
}

void RPBilHintLyt::Calculate() {
    mpPane->SetAlpha(mAlpha);
    mpLayout->calc();
}

void RPBilHintLyt::Draw() {
    mpLayout->draw();
}
