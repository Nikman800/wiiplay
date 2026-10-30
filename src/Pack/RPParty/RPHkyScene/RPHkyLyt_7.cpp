// Laser Hockey two-player message layout (airHockeyMsg_03.brlyt).
// Unit 0x802240E4-0x802244D0 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>

void RPHkyMsg03Lyt::Create() {
    mState = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_SWITCH] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
}

void RPHkyMsg03Lyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("airHockey.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "airHockeyMsg_03.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyMsg_03_inMsg_00.brlan");
    mpAnmObjs[ANM_SWITCH] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyMsg_03_switchMsg_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "airHockeyMsg_03_outMsg_00.brlan");
    mpLayout->findTextBox("T_msgText_00")
        ->setMessage(RPHkyGame::GetInstance()->GetMessage(), 0xA2D, 0, 0);
}

void RPHkyMsg03Lyt::Reset() {
    mState = 0;
    mSwitchCount = 0;

    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
}

void RPHkyMsg03Lyt::Calculate() {
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
            mpLayout->bindAnmObj(mpAnmObjs[ANM_SWITCH]);
            mpAnmObjs[ANM_SWITCH]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
            mpAnmObjs[ANM_SWITCH]->restart();
        }
        break;
    case 3:
        if (mpAnmObjs[ANM_SWITCH]->isFinished()) {
            if (mSwitchCount++ == 5 || RPHkyGame::GetInstance()->GetState() == 5) {
                mState = 4;
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
                mpAnmObjs[ANM_OUT]->restart();
            } else {
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObj(mpAnmObjs[ANM_SWITCH]);
                mpAnmObjs[ANM_SWITCH]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
                mpAnmObjs[ANM_SWITCH]->restart();
            }
        }
        break;
    case 4:
        if (mpAnmObjs[ANM_OUT]->isFinished()) {
            mState = 1;
        }
        break;
    }

    mpLayout->calc();
}

void RPHkyMsg03Lyt::Exit() {}

void RPHkyMsg03Lyt::Draw() {
    mpLayout->draw();
}

void RPHkyMsg03Lyt::PlayIn() {
    mState = 2;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
    mpAnmObjs[ANM_IN]->restart();
}
