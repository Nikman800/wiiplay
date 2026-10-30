// Table Tennis help message layout (pingpongMsg_01.brlyt, messages 0x898/0x899).
// Unit 0x80234AC8-0x802350F4 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>

// Not inlined into its callers in the original
#pragma push
#pragma auto_inline off
void RPPnpHelpMsgLyt::ApplyText() {
    RPSysMessage* pMessage = RPPnpGame::GetInstance()->GetMessage();

    switch (mMessage) {
    case 0:
        mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x898, 0, 0);
        mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x898, 0, 0);
        break;
    case 1:
        mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x898, 1, 0);
        mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x898, 1, 0);
        break;
    case 2:
        mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x899, 0, 0);
        mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x899, 0, 0);
        break;
    case 3:
        mpLayout->findTextBox("T_msgText_00")->setMessage(pMessage, 0x898, 2, 0);
        mpLayout->findTextBox("T_msgText_01")->setMessage(pMessage, 0x898, 2, 0);
        break;
    }
}
#pragma pop

void RPPnpHelpMsgLyt::Create() {
    mAnim = 0;
    mMessage = 4;
    mPrevMessage = 4;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpAnmObjs[ANM_IN] = NULL;
    mpAnmObjs[ANM_WAIT] = NULL;
    mpAnmObjs[ANM_OUT] = NULL;
}

void RPPnpHelpMsgLyt::Load() {
    mpResAccessor = RPCreateLocalLytResAccessor("pingpong.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "pingpongMsg_01.brlyt");
    mpAnmObjs[ANM_IN] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongMsg_01_inAll_00.brlan");
    mpAnmObjs[ANM_WAIT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongMsg_01_waitAll_00.brlan");
    mpAnmObjs[ANM_OUT] =
        mpLayout->createAnmObj(NULL, mpResAccessor, "pingpongMsg_01_outAll_00.brlan");
}

void RPPnpHelpMsgLyt::Reset() {
    mAnim = 0;

    for (int i = 0; i < ANM_NUM; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->getFrameCtrl()->reinit();
    }

    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
}

void RPPnpHelpMsgLyt::Calculate() {
    // enum-typed local: the state is loaded into r4, not r0 (see MATCHING.md, enum-typed values)
    RPPnpGame::State state = (RPPnpGame::State)RPPnpGame::GetInstance()->GetState();
    if (state != RPPnpGame::STATE_SERVE) {
        RequestHide();
    }

    switch (mAnim) {
    case 0:
        mState = 4;
        mPending = 1;
        mAnim = 1;
        break;
    case 1:
        break;
    case 2:
        if (mpAnmObjs[ANM_IN]->isFinished()) {
            mState = mAnim;
            if (mPending == 1) {
                mAnim = 3;
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObj(mpAnmObjs[ANM_WAIT]);
                mpAnmObjs[ANM_WAIT]->restart();
            } else {
                mAnim = 4;
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
                mpAnmObjs[ANM_OUT]->restart();
            }
        }
        break;
    case 3:
        if (mPending != 1) {
            mAnim = 4;
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[ANM_OUT]);
            mpAnmObjs[ANM_OUT]->restart();
        }
        break;
    case 4:
        if (mpAnmObjs[ANM_OUT]->isFinished()) {
            mState = mAnim;
            mAnim = 1;
            if (mPending == 2) {
                mState = mAnim;
                mAnim = mPending;
                mPending = 1;
                ApplyText();
                mpLayout->unbindAllAnmObj();
                mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
                mpAnmObjs[ANM_IN]->restart();
            }
        }
        break;
    }

    mpLayout->calc();
}

void RPPnpHelpMsgLyt::Exit() {}

void RPPnpHelpMsgLyt::Draw() {
    mpLayout->draw();
}

void RPPnpHelpMsgLyt::SetMessage(s32 message) {
    mPrevMessage = mMessage;
    mMessage = message;

    if (mAnim == 1) {
        mState = mAnim;
        mAnim = 2;
        mPending = 1;
        ApplyText();
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpAnmObjs[ANM_IN]);
        mpAnmObjs[ANM_IN]->restart();
    } else {
        mPending = 2;
    }
}

void RPPnpHelpMsgLyt::RequestHide() {
    mPending = 4;
}
