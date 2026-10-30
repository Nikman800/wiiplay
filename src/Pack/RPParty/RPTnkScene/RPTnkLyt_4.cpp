// Tanks hit marker popup at a destroyed tank (tnkHit_00.brlyt).
// Unit 0x80270470-0x80270940 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkLyt.h>

#include <Pack/RPKernel.h>
#include <egg/gfxe/eggScreen.h>
#include <nw4r/lyt.h>
#include <egg/math.h>

RPTnkHitPopup::RPTnkHitPopup() : mActive(false) {}

RPTnkHitPopup::~RPTnkHitPopup() {}

void RPTnkHitPopup::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "tank.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "tnkHit_00.brlyt");
    mpAnmObjs[0] = mpLayout->createAnmObj(NULL, mpResAccessor, "tnkHit_00_hit_00.brlan");
    mpAnmObjs[0]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
}

void RPTnkHitPopup::Reset() {
    mpLayout->reset();
    s32 i = 0;
    do {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    } while (++i < 1);

    RPSysMessage* pMsg = RPTnkLayoutManager::sInstance->GetMessage();
    mpLayout->findTextBox("T_hit00_00")->setMessage(pMsg, 0xB5C, 0, 0);
    mpLayout->findTextBox("T_hit01_00")->setMessage(pMsg, 0xB5C, 0, 0);
    mpLayout->findTextBox("T_hit00_01")->setMessage(pMsg, 0xB5C, 0, 0);
    mpLayout->findTextBox("T_hit01_01")->setMessage(pMsg, 0xB5C, 0, 0);
    mpLayout->unbindAllAnmObj();

    s32 j = 0;
    do {
        mNumText[j] = 0;
    } while (++j < 4);
    mActive = false;
}

void RPTnkHitPopup::Calculate() {
    if (mActive && mpAnmObjs[0]->isFinished()) {
        RPTnkLayoutManager::sInstance->DecHitPopupNum();
        mActive = false;
    }
    mpLayout->calc();
}

void RPTnkHitPopup::Draw() {
    mpLayout->draw();
}

void RPTnkHitPopup::VF_0x18() {}

void RPTnkHitPopup::Start(nw4r::math::VEC2 rScreenPos, s32 num, s32 type) {
    RPSysStringUtility::GetStringHalfSizeNumber(num, mNumText, 4, false);
    const char* paneNames[2] = {"N_hit_00", "N_hit_01"};
    if (type == 0) {
        mpLayout->findPane(paneNames[0])->SetVisible(true);
        mpLayout->findPane(paneNames[1])->SetVisible(false);
    } else {
        mpLayout->findPane(paneNames[0])->SetVisible(false);
        mpLayout->findPane(paneNames[1])->SetVisible(true);
    }

    rScreenPos.x = EGG::Mathf::min(0.9f, EGG::Mathf::max(-0.9f, rScreenPos.x));
    rScreenPos.y = EGG::Mathf::min(0.9f, EGG::Mathf::max(-0.9f, rScreenPos.y));
    nw4r::lyt::Pane* pPane = mpLayout->findPane("N_tnk_00");
    const nw4r::math::VEC3& rTrans = pPane->GetTranslate();
    nw4r::math::VEC3 trans(rTrans.x, rTrans.y, rTrans.z);
    trans.x = rScreenPos.x * (0.5f * EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD));
    trans.y = rScreenPos.y * (0.5f * EGG::Screen::GetSizeYMax(EGG::Screen::TV_MODE_STD));
    pPane->SetTranslate(trans);

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObjToPane(mpAnmObjs[0], paneNames[type], true);
    mpAnmObjs[0]->restart();
    mActive = true;
}
