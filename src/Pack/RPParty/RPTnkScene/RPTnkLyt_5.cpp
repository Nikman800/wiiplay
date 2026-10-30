// Tanks control hint balloon (tnkEx_00.brlyt).
// Unit 0x80270940-0x80270E38 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkLyt.h>

#include <Pack/RPKernel.h>

void RPTnkHideExMsg() {
    RPTnkLayoutManager::sInstance->GetExMsg()->PlayOut();
}

RPTnkExMsg::RPTnkExMsg() {}

RPTnkExMsg::~RPTnkExMsg() {}

void RPTnkExMsg::Load(EGG::Archive* pArchive) {
    const char* anmNames[3] = {"tnkEx_00_in_00.brlan", "tnkEx_00_out_00.brlan",
                               "tnkEx_00_outAll_00.brlan"};
    AttachArcFile(pArchive, "tank.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "tnkEx_00.brlyt");
    s32 i = 0;
    do {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
        mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    } while (++i < 3);
}

void RPTnkExMsg::Reset() {
    mState = 1;
    unk1C = 0;
    unk20 = 0;
    unk24 = false;
    mpLayout->reset();
    s32 i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 3);

    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
}

void RPTnkExMsg::Calculate() {
    mpLayout->calc();
}

void RPTnkExMsg::Draw() {
    mpLayout->draw();
}

void RPTnkExMsg::VF_0x18() {}

void RPTnkExMsg::Show(s32 msg, f32) {
    RPSysMessage* pMsg = RPTnkLayoutManager::sInstance->GetMessage();
    switch (msg) {
    case 0:
        mpLayout->findTextBox("T_ex_00")->setMessage(pMsg, 0xB7C, 0, 0);
        break;
    case 1:
        mpLayout->findTextBox("T_ex_00")->setMessage(pMsg, 0xB7C, 1, 0);
        break;
    case 2:
        mpLayout->findTextBox("T_ex_00")->setMessage(pMsg, 0xB7C, 2, 0);
        break;
    case 3:
        mpLayout->findTextBox("T_ex_00")->setMessage(pMsg, 0xB7C, 3, 0);
        break;
    }

    if (mState == 0 || mState == 2) {
        return;
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
    mState = 2;
}

#pragma auto_inline off
void RPTnkExMsg::PlayOut() {
    if (mState == 1 || mState == 3) {
        return;
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
    mState = 3;
}
#pragma auto_inline reset

void RPTnkExMsg::PlayOutAll() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[2]);
    mpAnmObjs[2]->restart();
    mState = 3;
}
