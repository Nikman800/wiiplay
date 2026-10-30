#include <Pack/RPParty/RPWlyScene/RPWlyLyt_4.h>

#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysStringUtility.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>

#include <egg/gfxe.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyAvatar.h>
#include <Pack/RPParty/RPWlyScene/RPWlyAvatarMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLyt.h>

#pragma opt_unroll_loops off

void RPWlyPointPopup::UpdatePos() {
    if (mMode == 0) {
        if (mPoints < 0) {
            s32 frame = ++mTimer;
            RPWlyGameMgr* gameMgr = RPWlyGameMgr::sInstance;
            s32 timer = gameMgr->mTimer;
            switch (frame) {
            case 8:
            case 16:
            case 24:
            case 32:
            case 40:
                gameMgr->mTimer = timer - 60;
                break;
            case 48:
                RPWlyLayoutSet::sInstance->mpTimer->UpdateWarnAnim();
                break;
            }
        }
        nw4r::math::VEC3 pos = RPWlyAvatarMgr::sInstance->mpAvatars[mAvatarIdx]->mPopupPos;
        pos.x *= static_cast<f32>(EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD)) /
                 static_cast<f32>(RPWlyGameMgr::sInstance->mpScreen->GetWidth());
        mpLayout->findPane("N_centerU_00")->SetTranslate(pos);
    } else {
        for (int i = 0; i < 2; i++) {
            nw4r::math::VEC3 pos = RPWlyAvatarMgr::sInstance->mpAvatars[mAvatarIdx]->mPopupPos;
            pos.x *= static_cast<f32>(EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD)) /
                     static_cast<f32>(RPWlyGameMgr::sInstance->mpScreen->GetWidth());
            mpCenterPanes[i]->SetTranslate(pos);
        }
    }
    if (mpAnmObj->isFinished()) {
        mState = 1;
    }
}

void RPWlyPointPopup::Init() {
    mMode = RPWlyGameMgr::sInstance->mMode;
    mState = 0;
    mAvatarIdx = -1;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObj = NULL;
}

void RPWlyPointPopup::Create() {
    mpResAccessor = RPCreateLocalLytResAccessor("wally.arc");
    mpLayout = RPCreateLayout(mpResAccessor,
                              mMode == 0 ? "wallyTimePoint_00.brlyt" : "wallyPoint_00.brlyt");
    mpAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor,
                                      mMode == 0 ? "wallyTimePoint_00_inOutAll_00.brlan"
                                                 : "wallyPoint_00_inOutAll_00.brlan");
    mpTextBoxes[0] = mpLayout->findTextBox("T_numberU_00");
    mpTextBoxes[1] = mpLayout->findTextBox("T_numberU_01");
    mpTextBoxes[2] = mpLayout->findTextBox("T_numberD_00");
    mpTextBoxes[3] = mpLayout->findTextBox("T_numberD_01");

    RPSysMessage* message = RPWlyGameMgr::sInstance->mpMessage;
    static_cast<RPSysLytTextBox*>(mpTextBoxes[0])->setMessage(message, 0x963, 0, 0);
    static_cast<RPSysLytTextBox*>(mpTextBoxes[1])->setMessage(message, 0x963, 0, 0);
    static_cast<RPSysLytTextBox*>(mpTextBoxes[2])->setMessage(message, 0x963, 1, 0);
    static_cast<RPSysLytTextBox*>(mpTextBoxes[3])->setMessage(message, 0x963, 1, 0);

    mpCenterPanes[0] = mpLayout->findPane("N_centerU_00");
    mpCenterPanes[1] = mpLayout->findPane("N_centerD_00");
}

void RPWlyPointPopup::Reset() {
    for (int i = 0; i < 1; i++) {
        (&mpAnmObj)[i]->resetFrame();
        (&mpAnmObj)[i]->getFrameCtrl()->reinit();
    }
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObj);
    mpCenterPanes[0]->SetVisible(false);
    mpCenterPanes[1]->SetVisible(false);
    mState = 0;
}

void RPWlyPointPopup::Calculate() {
    switch (mState) {
    case 0:
        mState = 1;
        mAvatarIdx = -1;
        break;
    case 1:
        break;
    case 2:
        UpdatePos();
        break;
    }
    if (mState != 1) {
        mpLayout->calc();
    }
}

void RPWlyPointPopup::Destroy() {}

void RPWlyPointPopup::Draw() {
    if (mState == 1) {
        return;
    }
    if (RPWlyGameMgr::sInstance->mState == 3 && mPoints <= 0) {
        return;
    }
    if (RPWlyGameMgr::sInstance->mState != 4) {
        mpLayout->draw();
        return;
    }
    if (mAvatarIdx == -1) {
        return;
    }
    if (!RPWlyAvatarMgr::sInstance->mpAvatars[mAvatarIdx]->mVisible) {
        return;
    }
    mpLayout->draw();
}

void RPWlyPointPopup::Show(s32 player, s32 avatarIdx, s32 points) {
    mAvatarIdx = avatarIdx;
    mPlayer = player;
    mPoints = points;
    mState = 2;
    RPSysStringUtility::GetStringHalfSizeNumber(points, mNumberText, 4, true);
    static_cast<RPSysLytTextBox*>(mpTextBoxes[0])->setText(mNumberText, 0);
    static_cast<RPSysLytTextBox*>(mpTextBoxes[1])->setText(mNumberText, 0);
    static_cast<RPSysLytTextBox*>(mpTextBoxes[2])->setText(mNumberText, 0);
    static_cast<RPSysLytTextBox*>(mpTextBoxes[3])->setText(mNumberText, 0);

    if (mMode == 0) {
        mTimer = 0;
        if (points >= 0) {
            mpCenterPanes[0]->SetVisible(true);
            mpCenterPanes[1]->SetVisible(false);
            RPWlyLayoutSet::sInstance->mpTimer->PlayPlus();
        } else {
            mpCenterPanes[0]->SetVisible(false);
            mpCenterPanes[1]->SetVisible(true);
            RPWlyLayoutSet::sInstance->mpTimer->PlayMinus();
        }
    } else if (points >= 0) {
        mpCenterPanes[0]->SetVisible(true);
        mpCenterPanes[1]->SetVisible(false);
        if (player == 0) {
            mpLayout->changeFontColor("T_numberU_00", 0);
        } else {
            mpLayout->changeFontColor("T_numberU_00", 2);
        }
    } else {
        mpCenterPanes[0]->SetVisible(false);
        mpCenterPanes[1]->SetVisible(true);
        if (player == 0) {
            mpLayout->changeFontColor("T_numberD_00", 1);
        } else {
            mpLayout->changeFontColor("T_numberD_00", 3);
        }
    }

    for (int i = 0; i < 2; i++) {
        nw4r::math::VEC3 pos = RPWlyAvatarMgr::sInstance->mpAvatars[mAvatarIdx]->mPopupPos;
        pos.x *= static_cast<f32>(EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD)) /
                 static_cast<f32>(RPWlyGameMgr::sInstance->mpScreen->GetWidth());
        mpCenterPanes[i]->SetTranslate(pos);
    }

    mpAnmObj->resetFrame();
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObj);
    mpAnmObj->restart();
}
