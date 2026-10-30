// Table Tennis arrows over the players (pingpongArrow_00.brlyt, one layout per player).
// Unit 0x80230F84-0x802313D0 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPParty/RPPnpScene/RPPnpPlayer.h>

void RPPnpArrowLyt::Create() {
    mpResAccessor = NULL;
    mpLayouts[0] = NULL;
    mpAnmObjs[0][ANM_UP_DOWN] = NULL;
    mpAnmObjs[0][ANM_BLUE] = NULL;
    mpAnmObjs[0][ANM_RED] = NULL;
    mpLayouts[1] = NULL;
    mpAnmObjs[1][ANM_UP_DOWN] = NULL;
    mpAnmObjs[1][ANM_BLUE] = NULL;
    mpAnmObjs[1][ANM_RED] = NULL;
}

void RPPnpArrowLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("pingpong.arc");

    for (int i = 0; i < 2; i++) {
        mpLayouts[i] = RPCreateLayout(mpResAccessor, "pingpongArrow_00.brlyt");
        mpRootPanes[i] = mpLayouts[i]->findPane("RootPane");

        // Wii Play: the same three animations, three times
        for (int j = 0; j < ANM_NUM; j++) {
            mpAnmObjs[i][ANM_UP_DOWN] = mpLayouts[i]->createAnmObj(
                NULL, mpResAccessor, "pingpongArrow_00_arrowUpDown_00.brlan");
            mpAnmObjs[i][ANM_BLUE] = mpLayouts[i]->createAnmObj(
                NULL, mpResAccessor, "pingpongArrow_00_arrowBlue_00.brlan");
            mpAnmObjs[i][ANM_RED] = mpLayouts[i]->createAnmObj(
                NULL, mpResAccessor, "pingpongArrow_00_arrowRed_00.brlan");
        }
    }
}

void RPPnpArrowLyt::Reset() {
    mShown = false;

    for (int i = 0; i < 2; i++) {
        mpRootPanes[i]->SetVisible(false);

        for (int j = 0; j < ANM_NUM; j++) {
            mpAnmObjs[i][j]->resetFrame();
            mpAnmObjs[i][j]->getFrameCtrl()->reinit();
        }
    }
}

void RPPnpArrowLyt::Calculate() {
    RPGrpCamera* pCamera = RPPnpGame::GetInstance()->GetCamera();
    RPGrpScreen* pScreen = RPPnpGame::GetInstance()->GetScreen();

    for (int i = 0; i < 2; i++) {
        nw4r::math::VEC3 screenPos;
        pCamera->GetWorldToScreen(&screenPos, RPPnpGame::GetInstance()->GetPlayer(i)->GetPos(),
                                  pScreen);
        mpRootPanes[i]->SetTranslate(screenPos);
        mpLayouts[i]->calc();
    }
}

void RPPnpArrowLyt::Exit() {}

void RPPnpArrowLyt::Draw() {
    for (int i = 0; i < 2; i++) {
        if (!RPPnpGame::GetInstance()->GetPlayer(i)->IsHidden()) {
            mpLayouts[i]->draw();
        }
    }
}

void RPPnpArrowLyt::Show() {
    if (!mShown) {
        mpRootPanes[0]->SetVisible(true);
        mpLayouts[0]->unbindAllAnmObj();
        mpLayouts[0]->bindAnmObj(mpAnmObjs[0][ANM_BLUE]);
        mpLayouts[0]->bindAnmObj(mpAnmObjs[0][ANM_UP_DOWN]);
        mpAnmObjs[0][ANM_BLUE]->restart();
        mpAnmObjs[0][ANM_UP_DOWN]->restart();

        mpRootPanes[1]->SetVisible(true);
        mpLayouts[1]->unbindAllAnmObj();
        mpLayouts[1]->bindAnmObj(mpAnmObjs[1][ANM_RED]);
        mpLayouts[1]->bindAnmObj(mpAnmObjs[1][ANM_UP_DOWN]);
        // Wii Play: restarts the first layout's red arrow, not the second's
        mpAnmObjs[0][ANM_RED]->restart();
        mpAnmObjs[1][ANM_UP_DOWN]->restart();

        mShown = true;
    }
}
