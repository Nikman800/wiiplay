// Laser Hockey player balloons with the players' Mii icons (airHockeyBalloon_00.brlyt).
// Unit 0x802228D8-0x80223060 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPHkyScene/RPHkyPaddle.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>

#include <egg/gfxe.h>

void RPHkyBalloonLyt::Create() {
    mState = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_WAIT] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;

    for (int i = 0; i < 2; i++) {
        mpCenterPanes[i] = NULL;
        mIcons[i].mpIcon = NULL;
        mIcons[i].mpPane = NULL;
    }

    for (int i = 0; i < RPSceneInfo::spInstance->GetPlayerNum(); i++) {
        RPSysKokeshiGenInfo info;
        GXColor bgColor = {0, 0, 0, 0};

        info.SetGenType(RPSysKokeshiManager::GenType_Player);
        info.SetPlayerNo(i);
        info.SetIconWidth(0x60);
        info.SetIconHeight(0x60);
        info.SetIconBGType(RFLIconBG_Direct);
        info.SetIconBGColor(bgColor);
        info.SetIconMipMap(true);

        mIcons[i].mpIcon = new RPSysKokeshiIcon(&info);
    }
}

void RPHkyBalloonLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("airHockey.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "airHockeyBalloon_00.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyBalloon_00_inAll_00.brlan");
    mpAnmObjs[ANM_WAIT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyBalloon_00_waitAll_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyBalloon_00_outAll_00.brlan");

    RPSysMessage* pMessage = RPHkyGame::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_1pText_00")->setMessage(pMessage, 0xA2C, 0, 0);
    mpLayout->findTextBox("T_1pText_01")->setMessage(pMessage, 0xA2C, 0, 0);
    mpLayout->findTextBox("T_2pText_00")->setMessage(pMessage, 0xA2C, 1, 0);
    mpLayout->findTextBox("T_2pText_01")->setMessage(pMessage, 0xA2C, 1, 0);

    mpCenterPanes[0] = mpLayout->findPane("N_1pCenter_00");
    mpCenterPanes[1] = mpLayout->findPane("N_2pCenter_00");
    mIcons[0].mpPane = mpLayout->findPane("N_1pFace_00");
    mIcons[1].mpPane = mpLayout->findPane("N_2pFace_00");
}

void RPHkyBalloonLyt::Reset() {
    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpLayout->findPane("N_1pCenter_00")->SetVisible(false);
    mpLayout->findPane("N_2pCenter_00")->SetVisible(false);
    mState = 0;
    mOutRequested = false;
}

void RPHkyBalloonLyt::Calculate() {
    switch (mState) {
    case 0:
        mState = 1;
        break;
    case 1:
        break;
    case 2:
        if (mpAnmObjs[ANM_IN]->isFinished()) {
            mState = 3;
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[ANM_WAIT]);
            mpAnmObjs[ANM_WAIT]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
            mpAnmObjs[ANM_WAIT]->restart();
        }
        break;
    case 3:
        if (mpAnmObjs[ANM_WAIT]->isFinished()) {
            if (mOutRequested) {
                mState = 4;
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
                mpAnmObjs[ANM_OUT]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
                mpAnmObjs[ANM_OUT]->restart();
            } else {
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObj(mpAnmObjs[ANM_WAIT]);
                mpAnmObjs[ANM_WAIT]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
                mpAnmObjs[ANM_WAIT]->restart();
            }
        }
        break;
    case 4:
        if (mpAnmObjs[ANM_OUT]->isFinished()) {
            mState = 1;
        }
        break;
    }

    RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
    RPGrpCamera* pCamera = RPHkyGame::GetInstance()->GetCamera();

    for (int i = 0; i < RPSceneInfo::spInstance->GetPlayerNum(); i++) {
        nw4r::math::VEC3 screenPos;
        pCamera->GetWorldToScreen(&screenPos, RPHkyGame::GetInstance()->GetPaddle(i)->GetPos(),
                                  pScreen);
        screenPos.x *= static_cast<f32>(EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD)) /
                       static_cast<f32>(pScreen->GetWidth());
        mpCenterPanes[i]->SetTranslate(screenPos);
    }

    mpLayout->calc();
}

void RPHkyBalloonLyt::Exit() {}

void RPHkyBalloonLyt::Draw() {
    mpLayout->draw();

    for (int i = 0; i < RPSceneInfo::spInstance->GetPlayerNum(); i++) {
        RPDrawKokeshiIcon(mIcons[i].mpIcon, mIcons[i].mpPane, FALSE, NULL);
    }
}

void RPHkyBalloonLyt::Show() {
    mState = 2;

    if (RPSceneInfo::spInstance->GetPlayerNum() == 1) {
        mpLayout->findPane("N_1pCenter_00")->SetVisible(true);
        mpLayout->findPane("N_2pCenter_00")->SetVisible(false);
    } else {
        mpLayout->findPane("N_1pCenter_00")->SetVisible(true);
        mpLayout->findPane("N_2pCenter_00")->SetVisible(true);
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpAnmObjs[ANM_IN]->restart();
}

void RPHkyBalloonLyt::RequestOut() {
    mOutRequested = true;
}
