// Table Tennis player balloons with the Mii icons (pingpongBalloon_00.brlyt, one layout per
// player). Unit 0x80232F0C-0x80233788 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPParty/RPPnpScene/RPPnpPlayer.h>

void RPPnpBalloonLyt::Create() {
    mpResAccessor = NULL;

    for (int i = 0; i < 2; i++) {
        mStates[i] = 0;
        mpLayouts[i] = NULL;
        mpAnmObjs[i][ANM_IN] = NULL;
        mpAnmObjs[i][ANM_UP_DOWN] = NULL;
        mpAnmObjs[i][ANM_OUT] = NULL;

        for (int j = 0; j < 2; j++) {
            mpCenterPanes[i][j] = NULL;
            mIcons[i][j].mpIcon = NULL;
            mIcons[i][j].mpPane = NULL;
            mIcons[i][j].mpPane2 = NULL;
        }

        for (int j = 0; j < RPSceneInfo::spInstance->GetPlayerNum(); j++) {
            RPSysKokeshiGenInfo info;
            GXColor bgColor = {0, 0, 0, 0};

            info.SetGenType(RPSysKokeshiManager::GenType_Player);
            info.SetPlayerNo(i);
            info.SetIconWidth(0x60);
            info.SetIconHeight(0x60);
            info.SetIconBGType(RFLIconBG_Direct);
            info.SetIconBGColor(bgColor);
            info.SetIconMipMap(true);

            mIcons[i][j].mpIcon = new RPSysKokeshiIcon(&info);
        }
    }
}

void RPPnpBalloonLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("pingpong.arc");

    for (int i = 0; i < 2; i++) {
        mpLayouts[i] = RPCreateLayout(mpResAccessor, "pingpongBalloon_00.brlyt");
        mpAnmObjs[i][ANM_IN] =
            mpLayouts[i]->createAnmObj(NULL, mpResAccessor, "pingpongBalloon_00_inAll_00.brlan");
        mpAnmObjs[i][ANM_UP_DOWN] = mpLayouts[i]->createAnmObj(
            NULL, mpResAccessor, "pingpongBalloon_00_UpDownAll_00.brlan");
        mpAnmObjs[i][ANM_OUT] =
            mpLayouts[i]->createAnmObj(NULL, mpResAccessor, "pingpongBalloon_00_outAll_00.brlan");

        RPSysMessage* pMessage = RPPnpGame::GetInstance()->GetMessage();
        mpLayouts[i]->findTextBox("T_1pPlayer_00")->setMessage(pMessage, 0xA2C, 0, 0);
        mpLayouts[i]->findTextBox("T_1pPlayer_01")->setMessage(pMessage, 0xA2C, 0, 0);
        mpLayouts[i]->findTextBox("T_2pPlayer_00")->setMessage(pMessage, 0xA2C, 1, 0);
        mpLayouts[i]->findTextBox("T_2pPlayer_01")->setMessage(pMessage, 0xA2C, 1, 0);

        mpCenterPanes[i][0] = mpLayouts[i]->findPane("N_1pCenter_00");
        mpCenterPanes[i][1] = mpLayouts[i]->findPane("N_2pCenter_00");
        mIcons[i][0].mpPane = mpLayouts[i]->findPane("N_1pFace_00");
        mIcons[i][1].mpPane = mpLayouts[i]->findPane("N_2pFace_00");
        mIcons[i][0].mpPane2 = mpLayouts[i]->findPane("N_1pFace_01");
        mIcons[i][1].mpPane2 = mpLayouts[i]->findPane("N_2pFace_01");
    }
}

void RPPnpBalloonLyt::Reset() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < ANM_NUM; j++) {
            mpAnmObjs[i][j]->resetFrame();
            mpAnmObjs[i][j]->getFrameCtrl()->reinit();
        }

        mpLayouts[i]->unbindAllAnmObj();
        mpLayouts[i]->bindAnmObj(mpAnmObjs[i][ANM_IN]);
        mpLayouts[i]->findPane("N_1pCenter_00")->SetVisible(false);
        mpLayouts[i]->findPane("N_2pCenter_00")->SetVisible(false);
        mStates[i] = 0;
        mOutRequested[i] = false;
    }
}

void RPPnpBalloonLyt::Calculate() {
    mOutRequested[0] = RPPnpGame::GetInstance()->GetPlayer(0)->IsBalloonOut();
    mOutRequested[1] = RPPnpGame::GetInstance()->GetPlayer(1)->IsBalloonOut();

    for (int i = 0; i < 2; i++) {
        switch (mStates[i]) {
        case 0:
            mStates[i] = 1;
            break;
        case 1:
            break;
        case 2:
            if (mpAnmObjs[i][ANM_IN]->isFinished()) {
                mStates[i] = 3;
                mpLayouts[i]->unbindAllAnmObj();
                mpLayouts[i]->bindAnmObj(mpAnmObjs[i][ANM_UP_DOWN]);
                mpAnmObjs[i][ANM_UP_DOWN]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
                mpAnmObjs[i][ANM_UP_DOWN]->restart();
            }
            break;
        case 3:
            if (mpAnmObjs[i][ANM_UP_DOWN]->isFinished()) {
                if (mOutRequested[i]) {
                    mStates[i] = 4;
                    mpLayouts[i]->unbindAllAnmObj();
                    mpLayouts[i]->bindAnmObj(mpAnmObjs[i][ANM_OUT]);
                    mpAnmObjs[i][ANM_OUT]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
                    mpAnmObjs[i][ANM_OUT]->restart();
                } else {
                    mpLayouts[i]->unbindAllAnmObj();
                    mpLayouts[i]->bindAnmObj(mpAnmObjs[i][ANM_UP_DOWN]);
                    mpAnmObjs[i][ANM_UP_DOWN]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
                    mpAnmObjs[i][ANM_UP_DOWN]->restart();
                }
            }
            break;
        case 4:
            if (mpAnmObjs[i][ANM_OUT]->isFinished()) {
                mStates[i] = 1;
            }
            break;
        }

        RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
        RPGrpCamera* pCamera = RPPnpGame::GetInstance()->GetCamera();

        for (int j = 0; j < RPSceneInfo::spInstance->GetPlayerNum(); j++) {
            nw4r::math::VEC3 screenPos;
            pCamera->GetWorldToScreen(&screenPos, RPPnpGame::GetInstance()->GetPlayer(j)->GetPos(),
                                      pScreen);
            mpCenterPanes[i][j]->SetTranslate(screenPos);
            mpCenterPanes[i][j]->SetTranslate(screenPos);
        }

        mpLayouts[i]->calc();
    }
}

void RPPnpBalloonLyt::Exit() {}

void RPPnpBalloonLyt::Draw() {
    for (int i = 0; i < RPSceneInfo::spInstance->GetPlayerNum(); i++) {
        mpLayouts[i]->draw();

        for (int j = 0; j < RPSceneInfo::spInstance->GetPlayerNum(); j++) {
            if (i == j) {
                RPDrawKokeshiIcon(mIcons[i][j].mpIcon, mIcons[i][j].mpPane2, TRUE, NULL);
                RPDrawKokeshiIcon(mIcons[i][j].mpIcon, mIcons[i][j].mpPane, FALSE, NULL);
            }
        }
    }
}

void RPPnpBalloonLyt::Show() {
    for (int i = 0; i < 2; i++) {
        mStates[i] = 2;

        if (i == 0) {
            mpLayouts[i]->findPane("N_1pCenter_00")->SetVisible(true);
            mpLayouts[i]->findPane("N_2pCenter_00")->SetVisible(false);
        } else {
            mpLayouts[i]->findPane("N_1pCenter_00")->SetVisible(false);
            mpLayouts[i]->findPane("N_2pCenter_00")->SetVisible(true);
        }

        mpLayouts[i]->unbindAllAnmObj();
        mpLayouts[i]->bindAnmObj(mpAnmObjs[i][ANM_IN]);
        mpAnmObjs[i][ANM_IN]->restart();
    }
}
