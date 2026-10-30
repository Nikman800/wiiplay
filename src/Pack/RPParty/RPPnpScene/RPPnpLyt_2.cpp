// Table Tennis server marker (pingpongServer_00.brlyt, message 0x8DE), two players.
// Unit 0x80230878-0x80230F84 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPParty/RPPnpScene/RPPnpPlayer.h>

// Not inlined into Calculate in the original
#pragma push
#pragma auto_inline off
void RPPnpServerLyt::ApplyPending() {
    if (mPending != 1 && mState != mPending) {
        mState = 1;
        mAnim = mPending;
        mPending = 1;

        switch (mAnim) {
        case 2:
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
            mpLayout->bindAnmObj(mpAnmObjs[ANM_UP_DOWN]);
            mpAnmObjs[ANM_IN]->restart();
            mpAnmObjs[ANM_UP_DOWN]->restart();
            break;
        case 3:
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
            mpLayout->bindAnmObj(mpAnmObjs[ANM_UP_DOWN]);
            mpAnmObjs[ANM_OUT]->restart();
            mpAnmObjs[ANM_UP_DOWN]->restart();
            break;
        }
    }
}
#pragma pop

void RPPnpServerLyt::Create() {
    mAnim = 0;
    mSide = 0;
    mPlayer = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_UP_DOWN] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
}

void RPPnpServerLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("pingpong.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "pingpongServer_00.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongServer_00_inAll_00.brlan");
    mpAnmObjs[ANM_UP_DOWN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongServer_00_upDownAll_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongServer_00_outAll_00.brlan");
    mpRootPane = mpLayout->findPane("RootPane");

    RPSysMessage* pMessage = RPPnpGame::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_textU_00")->setMessage(pMessage, 0x8DE, 0, 0);
    mpLayout->findTextBox("T_textU_01")->setMessage(pMessage, 0x8DE, 0, 0);
    mpLayout->findTextBox("T_textD_00")->setMessage(pMessage, 0x8DE, 0, 0);
    mpLayout->findTextBox("T_textD_01")->setMessage(pMessage, 0x8DE, 0, 0);
}

void RPPnpServerLyt::Reset() {
    mAnim = 0;

    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->findPane("N_serverWU_00")->SetVisible(false);
    mpLayout->findPane("N_serverWD_00")->SetVisible(false);
}

void RPPnpServerLyt::Calculate() {
    switch (mAnim) {
    case 0:
        mState = 3;
        mPending = 1;
        mAnim = 1;
        break;
    case 1:
        ApplyPending();
        break;
    case 2:
        if (mpAnmObjs[ANM_IN]->isFinished()) {
            mState = mAnim;
            mAnim = 1;
        }
        break;
    case 3:
        if (mpAnmObjs[ANM_OUT]->isFinished()) {
            mState = mAnim;
            mAnim = 1;
        }
        break;
    }

    RPGrpCamera* pCamera = RPPnpGame::GetInstance()->GetCamera();
    RPGrpScreen* pScreen = RPPnpGame::GetInstance()->GetScreen();
    nw4r::math::VEC3 screenPos;
    pCamera->GetWorldToScreen(&screenPos, RPPnpGame::GetInstance()->GetPlayer(mPlayer)->GetPos(),
                              pScreen);
    mpRootPane->SetTranslate(screenPos);
    mpLayout->calc();
}

void RPPnpServerLyt::Exit() {}

void RPPnpServerLyt::Draw() {
    mpLayout->draw();
}

void RPPnpServerLyt::Show(s32 side, s32 player) {
    mSide = side;
    mPlayer = player;

    if (side == 0) {
        mpLayout->findPane("N_serverWU_00")->SetVisible(true);
        mpLayout->findPane("N_serverWD_00")->SetVisible(false);
    } else {
        mpLayout->findPane("N_serverWU_00")->SetVisible(false);
        mpLayout->findPane("N_serverWD_00")->SetVisible(true);
    }

    if (mAnim != 1) {
        mPending = 2;
    } else if (mState != 2) {
        mAnim = 2;
        mPending = 1;
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
        mpLayout->bindAnmObj(mpAnmObjs[ANM_UP_DOWN]);
        mpAnmObjs[ANM_IN]->restart();
        mpAnmObjs[ANM_UP_DOWN]->restart();
    }
}

void RPPnpServerLyt::Hide() {
    if (mAnim != 1) {
        mPending = 3;
    } else if (mState != 3) {
        mAnim = 3;
        mPending = 1;
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
        mpLayout->bindAnmObj(mpAnmObjs[ANM_UP_DOWN]);
        mpAnmObjs[ANM_OUT]->restart();
        mpAnmObjs[ANM_UP_DOWN]->restart();
    }
}
