#include <Pack/RPParty/RPWlyScene/RPWlyLyt_7.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLevelTable.h>
#include <Pack/RPSystem/RPSysProjectLocal.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyAvatar.h>
#include <Pack/RPParty/RPWlyScene/RPWlyAvatarMgr.h>

namespace {
// Singleton accessor: ShowIn (opt_propagation off) keeps the inline call's register order.
inline RPWlyGameMgr* GetGameMgr() {
    return RPWlyGameMgr::sInstance;
}
}

void RPWlyQuestionLyt::Init() {
    mState = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpInAnmObj = NULL;
    mpSwitchAnmObj = NULL;
    mpOutAnmObj = NULL;
}

#pragma push
#pragma opt_propagation off
void RPWlyQuestionLyt::Create() {
    mpResAccessor = RPCreateLocalLytResAccessor("wally.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "wallyQue_00.brlyt");
    mpInAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor, "wallyQue_00_inQue_00.brlan");
    mpSwitchAnmObj =
        mpLayout->createAnmObj(NULL, mpResAccessor, "wallyQue_00_switchFaceWin_00.brlan");
    mpOutAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor, "wallyQue_00_outQue_00.brlan");

    RPSysMessage* message = GetGameMgr()->mpMessage;
    RPWlyLevelData* level = RPWlyLevelTable::sInstance->GetCurrentLevel();
    s32 type = level->mQuestionType;
    switch (type) {
    case 0:
    case 8: {
        const wchar_t* name = RPWlyAvatarMgr::sInstance->mpAvatars[0]->mAdditionalInfo.name;
        mpLayout->findTextBox("T_miiName_00")->setText(name, 0);
        mpLayout->findTextBox("T_miiName_01")->setText(name, 0);
        mpLayout->findTextBox("T_miiNameG_00")->setText(name, 0);
        mpLayout->findTextBox("T_miiNameG_01")->setText(name, 0);
        mpLayout->findTextBox("T_queMsgS_01")
            ->setMessage(message, 0x960, level->mQuestionType, 0);
        mpLayout->findPane("N_miiName_00")->SetVisible(true);
        break;
    }
    default:
        mpLayout->findTextBox("T_queMsgL_00")->setMessage(message, 0x960, type, 0);
        mpLayout->findTextBox("T_queMsgS_00")
            ->setMessage(message, 0x960, level->mQuestionType, 0);
        mpLayout->findPane("N_miiName_00")->SetVisible(false);
        break;
    }

    switch (level->mQuestionType) {
    case 7:
    case 8:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
        mpLayout->findPane("T_queMsgL_00")->SetVisible(false);
        mpLayout->findPane("T_queMsgS_00")->SetVisible(true);
        break;
    default:
        mpLayout->findPane("T_queMsgL_00")->SetVisible(true);
        mpLayout->findPane("T_queMsgS_00")->SetVisible(false);
        break;
    }

    if (RPSysProjectLocal::instance()->getLocale() == RPSysProjectLocal::EArea_Spain) {
        mpLayout->findPane("T_queMsgL_00")->SetVisible(false);
        mpLayout->findPane("T_queMsgS_00")->SetVisible(true);
    }

    switch (level->mQuestionType) {
    case 0:
    case 8:
        mpLayout->findPane("N_queFaceC_00")->SetVisible(true);
        mpLayout->findPane("T_queMsgL_00")->SetVisible(false);
        mpLayout->findPane("T_queMsgS_00")->SetVisible(false);
        mpLayout->findPane("T_queMsgS_01")->SetVisible(true);
        break;
    default:
        mpLayout->findPane("N_queFaceC_00")->SetVisible(false);
        mpLayout->findPane("T_queMsgS_01")->SetVisible(false);
        break;
    }
}

#pragma pop

void RPWlyQuestionLyt::Reset() {
    for (int i = 0; i < 3; i++) {
        (&mpInAnmObj)[i]->resetFrame();
        (&mpInAnmObj)[i]->getFrameCtrl()->reinit();
    }
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpInAnmObj);
    mState = 0;
    mOutLocked = true;
}

void RPWlyQuestionLyt::Calculate() {
    switch (mState) {
    case 0:
        mState = 1;
        break;
    case 1:
        if (mpSwitchAnmObj->isFinished()) {
            mpSwitchAnmObj->restart();
        }
        break;
    case 2:
        if (mpInAnmObj->isFinished()) {
            mState = 1;
            switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
            case 0:
            case 8:
                RPGrpRenderer::GetCurrent()->GetView(4)->SetFlag(1);
                break;
            }
            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpSwitchAnmObj);
            mpSwitchAnmObj->restart();
        }
        break;
    case 3:
        if (mpOutAnmObj->isFinished()) {
            mState = 1;
            mOutLocked = true;
        }
        break;
    }
    mpLayout->calc();
}

void RPWlyQuestionLyt::Destroy() {}

void RPWlyQuestionLyt::Draw() {
    mpLayout->draw();
}

#pragma push
#pragma opt_propagation off
void RPWlyQuestionLyt::ShowIn() {
    if (mOutLocked && mState == 1) {
        RPWlyLevelData* level = RPWlyLevelTable::sInstance->GetCurrentLevel();
        mpLayout->findPane("N_queBgA_00")->SetVisible(false);
        mpLayout->findPane("N_queBgB_00")->SetVisible(false);
        mpLayout->findPane("N_queBgAG_00")->SetVisible(false);
        mpLayout->findPane("N_queBgBG_00")->SetVisible(false);
        mpLayout->findPane("N_miiName_00")->SetVisible(false);
        mpLayout->findPane("N_miiNameG_00")->SetVisible(false);
        if (GetGameMgr()->mState == 2) {
            switch (level->mQuestionType) {
            case 0:
            case 8:
                mpLayout->findPane("N_queBgB_00")->SetVisible(true);
                mpLayout->findPane("N_miiName_00")->SetVisible(true);
                break;
            default:
                mpLayout->findPane("N_queBgA_00")->SetVisible(true);
                break;
            }
        } else {
            switch (level->mQuestionType) {
            case 0:
            case 8:
                mpLayout->findPane("N_queBgBG_00")->SetVisible(true);
                mpLayout->findPane("N_miiNameG_00")->SetVisible(true);
                break;
            default:
                mpLayout->findPane("N_queBgAG_00")->SetVisible(true);
                break;
            }
        }
        mState = 2;
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpInAnmObj);
        mpInAnmObj->restart();
        mOutLocked = false;
    }
}

#pragma pop

void RPWlyQuestionLyt::StartOut() {
    if (!mOutLocked && mState == 1) {
        mState = 3;
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObj(mpOutAnmObj);
        mpOutAnmObj->restart();
        switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
        case 0:
        case 8:
            RPGrpRenderer::GetCurrent()->GetView(4)->ClearFlag(1);
            break;
        }
    }
}

bool RPWlyQuestionLyt::IsOutDone() const {
    return mpOutAnmObj->isFinished();
}
