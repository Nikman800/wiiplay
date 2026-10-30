#include <Pack/RPParty/RPWlyScene/RPWlyLyt_5.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPWlyScene/RPWlySound.h>

#include <cwchar>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLyt.h>

namespace {
inline nw4r::math::VEC2 operator*(const nw4r::math::VEC2& rVec, f32 f) {
    return nw4r::math::VEC2(f * rVec.x, f * rVec.y);
}
}

void RPWlyTimerLyt::Init() {
    mState = 0;
    mTimer = 0;
    mScale.x = mScale.y = 1.0f;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpInAnmObj = NULL;
    mpOutAnmObj = NULL;
    mpSwitchAnmObj = NULL;
    mpPlusAnmObj = NULL;
    mpMinusAnmObj = NULL;
}

void RPWlyTimerLyt::Create() {
    mpResAccessor = RPCreateLocalLytResAccessor("wally.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "wallyTime_00.brlyt");
    mpInAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor, "wallyTime_00_inTime_00.brlan");
    mpOutAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor, "wallyTime_00_outTime_00.brlan");
    mpSwitchAnmObj =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyTime_00_switchTime_00.brlan");
    mpPlusAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor, "wallyTime_00_switchPlus_00.brlan");
    mpMinusAnmObj =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyTime_00_switchMinus_00.brlan");

    mpLayout->findPane("N_timeSingleC_00")->SetVisible(false);
    mpLayout->findPane("N_timeDubleC_00")->SetVisible(true);

    RPSysMessage* message = RPWlyGameMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_timeNumS_00")->setMessage(message, 0x968, 0, 0);
    mpLayout->findTextBox("T_timeNumD_00")->setMessage(message, 0x968, 0, 0);
}

void RPWlyTimerLyt::Reset() {
    for (int i = 0; i < 5; i++) {
        (&mpInAnmObj)[i]->resetFrame();
        (&mpInAnmObj)[i]->getFrameCtrl()->reinit();
    }
    mpLayout->unbindAllAnmObj();
    if (RPWlyGameMgr::sInstance->mStageNo == 1) {
        mpLayout->bindAnmObj(mpInAnmObj);
    } else {
        mpLayout->bindAnmObj(mpOutAnmObj);
    }
    mState = 0;
    mTimer = 0;
    unk10 = -1;

    if (mpLayout != NULL) {
        std::swprintf(mNumberText, 4, L"%d", 120);
        for (int i = 0; i < 4; i++) {
            if (mNumberText[i] == 0) {
                break;
            }
            mNumberText[i] += 0xFEE0;
        }
        mpLayout->findTextBox("T_timeNumS_00")->setText(mNumberText, 0);
        mpLayout->findTextBox("T_timeNumD_00")->setText(mNumberText, 0);
    }
}

void RPWlyTimerLyt::Calculate() {
    switch (mState) {
    case 0:
        mTimer = 0;
        mState = 1;
        break;
    case 1:
        break;
    case 2:
        if (mpInAnmObj->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    case 3:
        if (mpOutAnmObj->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    case 4:
        if (mpPlusAnmObj->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    case 5:
        if (mpMinusAnmObj->isFinished()) {
            mState = 1;
            mTimer = 0;
        }
        break;
    }

    s32 timer = RPWlyGameMgr::sInstance->mTimer;
    s32 seconds;
    if (timer <= 0) {
        seconds = 0;
    } else {
        seconds = (timer - 1) / 60 + 1;
    }

    std::swprintf(mNumberText, 4, L"%d", seconds);
    for (int i = 0; i < 4; i++) {
        if (mNumberText[i] == 0) {
            break;
        }
        mNumberText[i] += 0xFEE0;
    }
    mpLayout->findTextBox("T_timeNumS_00")->setText(mNumberText, 0);
    mpLayout->findTextBox("T_timeNumD_00")->setText(mNumberText, 0);

    RPWlyGameMgr::sInstance->mpTimerSe->Update(seconds);

    switch (timer) {
    case 0:
    case 60:
    case 120:
    case 180:
    case 240:
    case 300:
        if (RPWlyLayoutSet::sInstance->mpPointPopups->AreAllIdle()) {
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpSwitchAnmObj);
            mpSwitchAnmObj->restart();
        }
        break;
    }

    nw4r::lyt::Pane* pScalePane = mpLayout->findPane("N_timeD_00");
    nw4r::math::VEC2 scale = pScalePane->GetScale();
    nw4r::math::VEC2 newScale;
    pScalePane->SetScale(newScale = (mScale - scale) * 0.5f + scale);

    nw4r::lyt::Pane* pRootPane = mpLayout->findPane("RootPane");
    nw4r::math::VEC3 trans = pRootPane->GetTranslate();
    trans.y = (newScale.x - 1.0f) * 12.0f;
    pRootPane->SetTranslate(trans);

    mpLayout->calc();
}

void RPWlyTimerLyt::Destroy() {}

void RPWlyTimerLyt::Draw() {
    mpLayout->draw();
}

void RPWlyTimerLyt::PlayIn() {
    if (RPWlyGameMgr::sInstance->mStageNo == 1) {
        mState = 2;
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpInAnmObj);
        mpInAnmObj->restart();
    }
}

void RPWlyTimerLyt::PlayOut() {
    mState = 3;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpOutAnmObj);
    mpOutAnmObj->restart();
}

void RPWlyTimerLyt::PlayPlus() {
    mState = 4;
    mScale.x = mScale.y = 1.5f;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpPlusAnmObj);
    mpPlusAnmObj->restart();
}

void RPWlyTimerLyt::PlayMinus() {
    mState = 5;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpMinusAnmObj);
    mpMinusAnmObj->restart();
}

void RPWlyTimerLyt::UpdateWarnAnim() {
    if (RPWlyGameMgr::sInstance->mTimer > 300) {
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpOutAnmObj);
        mpOutAnmObj->stop();
    } else {
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpSwitchAnmObj);
        mpSwitchAnmObj->restart();
    }
}
