#include <Pack/RPParty/RPWlyScene/RPWlyLyt_10.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>

void RPWlyEndScoreLyt::Init() {
    mState = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpInAnmObj = NULL;
    mpOutAnmObj = NULL;
    mpIcon = NULL;
    mpIconPanes[0] = NULL;
    mpIconPanes[1] = NULL;

    RPSysKokeshiGenInfo info;
    GXColor bgColor = {0, 0, 0, 0};

    info.SetGenType(RPSysKokeshiManager::GenType_Player);
    info.SetPlayerNo(0);
    info.SetIconWidth(0x60);
    info.SetIconHeight(0x60);
    info.SetIconBGType(RFLIconBG_Direct);
    info.SetIconBGColor(bgColor);
    info.SetIconMipMap(true);

    mpIcon = new RPSysKokeshiIcon(&info);
}

void RPWlyEndScoreLyt::Create() {
    mpResAccessor = RPCreateLocalLytResAccessor("wally.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "wallyEndScore_00.brlyt");
    mpInAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor, "wallyEndScore_00_inScore_00.brlan");
    mpOutAnmObj =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyEndScore_00_outScore_00.brlan");

    RPSysMessage* message = RPWlyGameMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_1pText_00")->setMessage(message, 0x969, 0, 0);
    mpLayout->findTextBox("T_1pText_01")->setMessage(message, 0x969, 0, 0);

    mpIconPanes[0] = mpLayout->findPane("N_1pFace_00");
    mpIconPanes[1] = mpLayout->findPane("N_1pFace_01");
}

void RPWlyEndScoreLyt::Reset() {
    for (int i = 0; i < 2; i++) {
        (&mpInAnmObj)[i]->resetFrame();
        (&mpInAnmObj)[i]->getFrameCtrl()->reinit();
    }
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpInAnmObj);
    mState = 0;
}

void RPWlyEndScoreLyt::Calculate() {
    switch (mState) {
    case 0:
        mState = 1;
        break;
    case 2:
        if (mpInAnmObj->isFinished()) {
            mState = 1;
        }
        break;
    case 3:
        if (mpOutAnmObj->isFinished()) {
            mState = 1;
        }
        break;
    }
    mpLayout->calc();
}

void RPWlyEndScoreLyt::Destroy() {}

void RPWlyEndScoreLyt::Draw() {
    mpLayout->draw();
    RPDrawKokeshiIcon(mpIcon, mpIconPanes[1], TRUE, NULL);
    RPDrawKokeshiIcon(mpIcon, mpIconPanes[0], FALSE, NULL);
}

void RPWlyEndScoreLyt::Show() {
    wchar_t text[4];
    s32 score = RPSceneInfo::spInstance->GetUnk1C() - 1;
    RPSysStringUtility::GetStringHalfSizeNumber(score, text, 3, false);
    mpLayout->findTextBox("T_scoreNum_00")->setText(text, 0);
    mpLayout->findTextBox("T_scoreNum_01")->setText(text, 0);

    RPSysMessage* message = RPWlyGameMgr::sInstance->mpMessage;
    if (RPSysStringUtility::IsSingular(score)) {
        mpLayout->findTextBox("T_scoreUnit_00")->setMessage(message, 0x96A, 0, 0);
        mpLayout->findTextBox("T_scoreUnit_01")->setMessage(message, 0x96A, 0, 0);
    } else {
        mpLayout->findTextBox("T_scoreUnit_00")->setMessage(message, 0x96A, 1, 0);
        mpLayout->findTextBox("T_scoreUnit_01")->setMessage(message, 0x96A, 1, 0);
    }

    mState = 2;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpInAnmObj);
    mpInAnmObj->restart();
}

void RPWlyEndScoreLyt::PlayOut() {
    mState = 3;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpOutAnmObj);
    mpOutAnmObj->restart();
}
