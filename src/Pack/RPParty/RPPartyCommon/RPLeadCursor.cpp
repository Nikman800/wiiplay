// Off-screen pointer guide (RPLeadCursorLyt, leadCursor_00.brlyt) and the Mii blink control
// (RPFaceCtrl). Unit 0x801F8650-0x801F97D4. Functions in reverse address order; the state
// machine instantiation follows at the end of the file.
#include <Pack/RPParty/RPPartyCommon/RPFaceCtrl.h>
#include <Pack/RPParty/RPPartyCommon/RPLeadCursor.h>

#include <Pack/RPKernel/RPSysKokeshi.h>
#include <Pack/RPKernel/RPSysKokeshiManager.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytPicture.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPUtility/RPUtlRandom.h>

// RPMessageWindow.cpp
extern "C" void Lyt_setPaneColorFromTable(nw4r::lyt::Pane* pPane, u32 player);

namespace {
struct RPLeadCursorLytStateFuncs {
    RPLeadCursorSeq::StateFunc mEnter; // at 0x0
    RPLeadCursorSeq::StateFunc mCalc;  // at 0xC
};
} // namespace

// Defined first: its tables come first in .rodata.
namespace {
struct RPFaceCtrlBlinkRange {
    s32 mMin; // at 0x0
    s32 mMax; // at 0x4
};
} // namespace

inline void RPFaceCtrl::InitBlink(s32 expression) {
    s32 intervals[7] = {180, -1, 180, 100, 180, -1, 200};
    RPFaceCtrlBlinkRange ranges[7] = {{1, 2}, {0, 0}, {1, 2}, {2, 3}, {2, 2}, {0, 0}, {1, 2}};

    if (intervals[expression] == -1 || expression >= 7) {
        mInterval = -1;
        mBlinkNum = 0;
    } else {
        mInterval = intervals[expression] + RPUtlRandom::getU32(intervals[expression] / 2) -
                    intervals[expression] / 4;
        mBlinkNum = ranges[expression].mMin +
                    RPUtlRandom::getU32(ranges[expression].mMax - ranges[expression].mMin);
    }
}

RPLeadCursorLyt::RPLeadCursorLyt(s32 playerNum) {
    mPlayerNum = playerNum;

    for (s32 i = 0; i < SIDE_NUM; i++) {
        mpStateMachines[i] = new RPLeadCursorSeq(this, 4);

        RPLeadCursorLytStateFuncs funcs[4] = {
            {&RPLeadCursorLyt::EnterIdle, &RPLeadCursorLyt::CalcIdle},
            {&RPLeadCursorLyt::EnterIn, &RPLeadCursorLyt::CalcIn},
            {&RPLeadCursorLyt::EnterLoop, &RPLeadCursorLyt::CalcLoop},
            {&RPLeadCursorLyt::EnterOut, &RPLeadCursorLyt::CalcOut},
        };
        for (s32 j = 0; j < 4; j++) {
            mpStateMachines[i]->SetStateFuncs(funcs[j].mEnter, funcs[j].mCalc, j);
        }
    }

    mpResAccessor = RPSysLytResAccessor::create(NULL);
}

void RPLeadCursorLyt::Load() {
    RPLytObjBase::AttachArcFile(RPSysResourceManager::instance()->GetStaticLocalArchive(),
                                "leadCursor.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "leadCursor_00.brlyt");

    const char* anmNames[3] = {"leadCursor_00_in_00.brlan", "leadCursor_00_roop_00.brlan",
                               "leadCursor_00_out_00.brlan"};
    for (s32 i = 0; i < SIDE_NUM; i++) {
        for (s32 j = 0; j < 3; j++) {
            mpAnmObjs[i][j] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[j]);
        }
    }

    mpBasePanes[0] = mpLayout->findPane("N_base_01");
    mpBasePanes[1] = mpLayout->findPane("N_base_00");

    for (s32 i = 0; i < SIDE_NUM; i++) {
        for (s32 j = 0; j < PICTURE_NUM; j++) {
            const char* pictureNames[SIDE_NUM][PICTURE_NUM] = {
                {"P_arrrowL_01", "P_arrrowR_01", "P_lineUL_01", "P_lineUR_01", "P_lineDL_01",
                 "P_lineDR_01"},
                {"P_arrrowL_00", "P_arrrowR_00", "P_lineUL_00", "P_lineUR_00", "P_lineDL_00",
                 "P_lineDR_00"},
            };
            mpPictures[i][j] = mpLayout->findPicture(pictureNames[i][j]);
        }
    }

    void* pMsgRes = RPSysResourceManager::GetMessageResource("party_message.bmg");
    mpMessage = new RPSysMessage(pMsgRes, NULL);

    mpTextBoxes[0] = mpLayout->findTextBox("T_mes_01");
    mpTextBoxes[1] = mpLayout->findTextBox("T_mes_00");
}

void RPLeadCursorLyt::Reset() {
    for (s32 i = 0; i < SIDE_NUM; i++) {
        mpStateMachines[i]->Reset(0);
        mShowRequest[i] = false;
        mHideRequest[i] = false;
        mPlayer[i] = -1;
        mRequestPlayer[i] = -1;
    }

    for (s32 i = 0; i < SIDE_NUM; i++) {
        for (s32 j = 0; j < 3; j++) {
            mpAnmObjs[i][j]->resetFrame();
            mpAnmObjs[i][j]->stop();
            mpAnmObjs[i][j]->setFrame(0.0f);
        }
    }

    mpLayout->unbindAllAnmObj();

    for (s32 i = 0; i < SIDE_NUM; i++) {
        mpBasePanes[i]->UnbindAllAnimation(true);
    }
}

void RPLeadCursorLyt::Calculate() {
    for (s32 i = 0; i < SIDE_NUM; i++) {
        mCurSide = i;
        mpStateMachines[i]->Calculate();
    }

    bool visible = false;
    for (s32 i = 0; i < SIDE_NUM; i++) {
        if (mpStateMachines[i]->GetState() != 0) {
            visible = true;
        }
    }

    if (visible) {
        mpLayout->calc();
    }
}

void RPLeadCursorLyt::EnterIdle() {
    mpBasePanes[mCurSide]->SetVisible(false);
}

void RPLeadCursorLyt::CalcIdle() {
    if (mShowRequest[mCurSide]) {
        mpStateMachines[mCurSide]->ChangeState(1);
        mPlayer[mCurSide] = mRequestPlayer[mCurSide];
    }
}

void RPLeadCursorLyt::EnterIn() {
    mpBasePanes[mCurSide]->SetVisible(true);
    mShowRequest[mCurSide] = false;
    mpBasePanes[mCurSide]->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[mCurSide][0], mpBasePanes[mCurSide], true);
    mpAnmObjs[mCurSide][0]->restart();

    for (s32 i = 0; i < PICTURE_NUM; i++) {
        Lyt_setPaneColorFromTable(mpPictures[mCurSide][i], mPlayer[mCurSide]);
    }

    if (mPlayerNum == 1) {
        static_cast<RPSysLytTextBox*>(mpTextBoxes[mCurSide])->setMessage(mpMessage, 0x2382, 0, 0);
    } else if (mPlayer[mCurSide] == 0) {
        static_cast<RPSysLytTextBox*>(mpTextBoxes[mCurSide])->setMessage(mpMessage, 0x2382, 1, 0);
    } else {
        static_cast<RPSysLytTextBox*>(mpTextBoxes[mCurSide])->setMessage(mpMessage, 0x2382, 2, 0);
    }
}

void RPLeadCursorLyt::CalcIn() {
    if (mpAnmObjs[mCurSide][0]->isFinished()) {
        mpStateMachines[mCurSide]->ChangeState(2);
    }
}

void RPLeadCursorLyt::EnterLoop() {
    mpBasePanes[mCurSide]->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[mCurSide][1], mpBasePanes[mCurSide], true);
    mpAnmObjs[mCurSide][1]->restart();
}

void RPLeadCursorLyt::CalcLoop() {
    if (mHideRequest[mCurSide]) {
        mpStateMachines[mCurSide]->ChangeState(3);
    }
}

void RPLeadCursorLyt::EnterOut() {
    mHideRequest[mCurSide] = false;
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[mCurSide][2], mpBasePanes[mCurSide], true);
    mpAnmObjs[mCurSide][2]->restart();
}

void RPLeadCursorLyt::CalcOut() {
    if (mpAnmObjs[mCurSide][2]->isFinished()) {
        mpStateMachines[mCurSide]->ChangeState(0);
    }
}

void RPLeadCursorLyt::Draw() {
    bool visible = false;
    for (s32 i = 0; i < SIDE_NUM; i++) {
        if (mpStateMachines[i]->GetState() != 0) {
            visible = true;
        }
    }

    if (visible) {
        mpLayout->draw();
    }
}

void RPLeadCursorLyt::Show(s32 player, s32 side) {
    if (mpStateMachines[side]->GetState() == 0) {
        mRequestPlayer[side] = player;
        mShowRequest[side] = true;
        mHideRequest[side] = false;
    }
}

void RPLeadCursorLyt::Hide(s32 player, s32 side) {
    if (mpStateMachines[side]->GetState() != 0 && player == mPlayer[side]) {
        mHideRequest[side] = true;
        mShowRequest[side] = false;
    }
}

RPFaceCtrl::RPFaceCtrl() {}

void RPFaceCtrl::Reset(RPSysKokeshi* pKokeshi) {
    s32 expression = RPSysKokeshiManager::GetExpression(pKokeshi->GetNigaoeModel());
    InitBlink(expression);
    mExpression = expression;
    mTimer = 0;
    mBlinkTimer = 0;
    mBlinkCount = 0;
}

void RPFaceCtrl::Calculate(RPSysKokeshi* pKokeshi) {
    if (mInterval == -1) {
        return;
    }

    if (mBlinkTimer > 0) {
        mBlinkTimer--;
        RPSysKokeshiManager::ChangeExpression(pKokeshi->GetNigaoeModel(), RFLExp_Blink);
    } else if (mTimer > mInterval) {
        if (mBlinkCount < mBlinkNum) {
            mInterval = 12;
            mBlinkCount++;
        } else {
            InitBlink(mExpression);
            mBlinkCount = 1;
        }

        RPSysKokeshiManager::ChangeExpression(pKokeshi->GetNigaoeModel(), RFLExp_Blink);
        mTimer = 0;
        mBlinkTimer = 4;
    } else {
        RPSysKokeshiManager::ChangeExpression(pKokeshi->GetNigaoeModel(),
                                              static_cast<RFLExpression>(mExpression));
        mTimer++;
    }
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPLeadCursorLyt>::RPStateMachine(RPLeadCursorLyt*, s32);
template void RPStateMachine<RPLeadCursorLyt>::SetStateFuncs(const StateFunc&, const StateFunc&,
                                                             s32);
template void RPStateMachine<RPLeadCursorLyt>::Reset(s32);
template void RPStateMachine<RPLeadCursorLyt>::ChangeState(s32);
template void RPStateMachine<RPLeadCursorLyt>::Calculate();
template void RPStateMachine<RPLeadCursorLyt>::CallEnter();
template void RPStateMachine<RPLeadCursorLyt>::CallCalc();
template void RPStateMachine<RPLeadCursorLyt>::CallExit();
