#include <Pack/RPParty/RPWlyScene/RPWlyLyt_11.h>

#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>

void RPWlyCompleteLyt::Init() {
    mState = 0;
    mpResAccessor = NULL;
    mpLayout = NULL;
    mpInAnmObj = NULL;
    mpOutAnmObj = NULL;
}

void RPWlyCompleteLyt::Create() {
    mpResAccessor = RPCreateLocalLytResAccessor("wally.arc");
    mpLayout = RPCreateLayout(mpResAccessor, "wallyComp_00.brlyt");
    mpInAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor,
                                      "wallyComp_00_in_00.brlan");
    mpOutAnmObj = mpLayout->createAnmObj(NULL, mpResAccessor,
                                       "wallyComp_00_out_00.brlan");
    RPSysMessage* message = RPWlyGameMgr::sInstance->mpMessage;
    mpLayout->findTextBox("T_complete00_00")->setMessage(message, 0x96B, 0, 0);
    mpLayout->findTextBox("T_complete01_00")->setMessage(message, 0x96B, 0, 0);
}

void RPWlyCompleteLyt::Reset() {
    for (int i = 0; i < 2; ++i) {
        (&mpInAnmObj)[i]->resetFrame();
        (&mpInAnmObj)[i]->getFrameCtrl()->reinit();
    }
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpInAnmObj);
    mState = 0;
}

void RPWlyCompleteLyt::Calculate() {
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

void RPWlyCompleteLyt::Destroy() {}

void RPWlyCompleteLyt::Draw() {
    mpLayout->draw();
}

void RPWlyCompleteLyt::ShowAllClear() {
    mState = 2;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpInAnmObj);
    mpInAnmObj->restart();

    nw4r::math::VEC3 pos = RPSimpleModel::sVecZero;
    nw4r::math::VEC3 scale(1.0f, 1.0f, 1.0f);
    RPSysEffect effect("Wtd_all_clear", 5, 0);
    effect.create();
    effect.setPos(pos);
    effect.setScale(scale);
    effect.update();
}

void RPWlyCompleteLyt::PlayOut() {
    mState = 3;
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpOutAnmObj);
    mpOutAnmObj->restart();
}
