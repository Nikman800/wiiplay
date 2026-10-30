// Tanks "Start!" message (tnkMsg_01.brlyt).
// Unit 0x802719A8-0x80271D2C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkLyt.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel.h>

RPTnkStartMsg::RPTnkStartMsg() {}

RPTnkStartMsg::~RPTnkStartMsg() {}

void RPTnkStartMsg::Load(EGG::Archive* pArchive) {
    const char* anmNames[1] = {"tnkMsg_01_start_00.brlan"};
    AttachArcFile(pArchive, "tank.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "tnkMsg_01.brlyt");
    s32 i = 0;
    do {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    } while (++i < 1);
}

void RPTnkStartMsg::Reset() {
    mpLayout->reset();
    s32 i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 1);

    RPSysMessage* pMsg = RPTnkLayoutManager::sInstance->GetMessage();
    mpLayout->findTextBox("T_start_00")->setMessage(pMsg, 0xB5B, 0, 0);
    mpLayout->findTextBox("T_start_01")->setMessage(pMsg, 0xB5B, 0, 0);
    mpLayout->findTextBox("T_start_02")->setMessage(pMsg, 0xB5B, 0, 0);

    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
}

void RPTnkStartMsg::Calculate() {
    mpLayout->calc();
}

void RPTnkStartMsg::Draw() {
    mpLayout->draw();
}

void RPTnkStartMsg::VF_0x18() {}

void RPTnkStartMsg::Play() {
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    RPSndAudioMgr::instance()->startSystemSe(0x23);
}
