// Fishing point popup (RPFshPointPopup): the caught fish's name, points and bonus line
// (fishingPoint_00.brlyt).
// Unit 0x8020F668-0x8020FEE8. Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshPointPopup.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPFshScene/RPFshBonusFish.h>
#include <Pack/RPParty/RPFshScene/RPFshFish.h>
#include <Pack/RPParty/RPFshScene/RPFshFishMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshScoreLayout.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPSystem.h>

RPFshPointPopup* RPFshPointPopup::sInstance;

void RPFshPointPopup::Exit() {}

inline RPFshPointPopup::RPFshPointPopup() {
    mpResAccessor = RPSysLytResAccessor::create(NULL);
    RPGrpRenderer::GetCurrent()->AppendDrawObject(*this);
}

void RPFshPointPopup::CreateInstance() {
    sInstance = new RPFshPointPopup();
}

void RPFshPointPopup::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshPointPopup::~RPFshPointPopup() {}

// String-pool symbol names are verified by the linked DOL comparison.
void RPFshPointPopup::Create() {
    RPLytObjBase::AttachArcFile(RPSysResourceManager::LoadGameLocalArchive(), "fishing.arc",
                                mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "fishingPoint_00.brlyt");

    const char* anmNames[2] = {"fishingPoint_00_inAll_00.brlan",
                               "fishingPoint_00_outAll_00.brlan"};
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    const char* bonusNames[2] = {"T_bonus_00", "T_bonus_01"};
    for (int i = 0; i < 2; i++) {
        mpLayout->findTextBox(bonusNames[i])->setMessage(pMessage, 0x805, 0, 0);
    }
}

void RPFshPointPopup::Reset() {
    mState = STATE_NONE;
    mNextState = STATE_NONE;
    mIsChanging = false;
    mStateTimer = 0;
}

void RPFshPointPopup::Calculate() {
    RPFshStateFuncs<RPFshPointPopup> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPFshPointPopup::InEnter, &RPFshPointPopup::InCalc},
        {&RPFshPointPopup::ShowEnter, &RPFshPointPopup::ShowCalc},
        {&RPFshPointPopup::OutEnter, &RPFshPointPopup::OutCalc},
    };

    if (mIsChanging) {
        mState = mNextState;
        mIsChanging = false;
        mStateTimer = 0;

        RPFshStateFuncs<RPFshPointPopup>::Func enter = funcs[mState].mEnter;
        if (enter != NULL) {
            (this->*enter)();
        }
    }

    RPFshStateFuncs<RPFshPointPopup>::Func calc = funcs[mState].mCalc;
    if (calc != NULL) {
        (this->*calc)();
    }

    if (mState != STATE_NONE) {
        mpLayout->calc();
    }
}

void RPFshPointPopup::InEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[0]);
    mpAnmObjs[0]->restart();
}

void RPFshPointPopup::InCalc() {
    if (mpAnmObjs[0]->isFinished()) {
        mNextState = STATE_SHOW;
        mIsChanging = true;
    }
}

void RPFshPointPopup::ShowEnter() {}

void RPFshPointPopup::ShowCalc() {
    mStateTimer++;
}

void RPFshPointPopup::OutEnter() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
}

void RPFshPointPopup::OutCalc() {
    if (mpAnmObjs[1]->isFinished()) {
        mNextState = STATE_NONE;
        mIsChanging = true;
    }
}

void RPFshPointPopup::Show(s32 /* player */, s32 fishIdx) {
    mNextState = STATE_IN;
    mIsChanging = true;

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    RPFshFish* pFish = RPFshFishMgr::GetInstance()->GetFish(fishIdx);
    s32 points = RPFshScoreLayout::CalcFishPoints(pFish->GetType(), pFish->GetScale());

    if (pFish->GetType() == RPFshBonusFish::GetInstance()->GetType()) {
        mpLayout->findPane("N_bonusOya_00")->SetVisible(true);
        mpLayout->findPane("N_B2_00")->SetVisible(true);
    } else {
        mpLayout->findPane("N_bonusOya_00")->SetVisible(false);
        mpLayout->findPane("N_B2_00")->SetVisible(false);
    }

    wchar_t text[10];
    RPSysStringUtility::GetStringHalfSizeNumber(points, text, 10, false);

    s32 nameIdx = 0;
    if (RPSysProjectLocal::instance()->getLocale() != RPSysProjectLocal::EArea_Japan) {
        nameIdx = 1;
    }

    const char* nameTexts[2] = {"T_fishName_00", "T_fishName_01"};
    for (int i = 0; i < 2; i++) {
        RPSysLytTextBox* pText = mpLayout->findTextBox(nameTexts[i]);
        if (i == nameIdx) {
            pText->setMessage(pMessage, 0x802, pFish->GetType(), 0);
            pText->SetVisible(true);
        } else {
            pText->SetVisible(false);
        }
    }

    const char* pointTexts[2][2] = {{"T_fishPointA_00", "T_fishPointA_01"},
                                    {"T_fishPointB_00", "T_fishPointB_01"}};
    const char* unitTexts[2][2] = {{"T_ptsA_00", "T_ptsA_01"}, {"T_ptsB_00", "T_ptsB_01"}};

    s32 plural = 1;
    if (RPSysStringUtility::IsSingular(points)) {
        plural = 0;
    }

    s32 row = 0;
    if (pFish->GetType() == RPFshFish::TYPE_SUKA_A) {
        row = 1;
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if (i == row) {
                mpLayout->findTextBox(pointTexts[i][j])->SetVisible(true);
                mpLayout->findTextBox(pointTexts[i][j])->setText(text, 0);
                mpLayout->findTextBox(unitTexts[i][j])->SetVisible(true);
                mpLayout->findTextBox(unitTexts[i][j])->setMessage(pMessage, 0x80A, plural, 0);
            } else {
                mpLayout->findTextBox(pointTexts[i][j])->SetVisible(false);
                mpLayout->findTextBox(unitTexts[i][j])->SetVisible(false);
            }
        }
    }
}

s32 RPFshPointPopup::GetOutAnmLength() const {
    return mpAnmObjs[1]->getFrameMax();
}

void RPFshPointPopup::Close() {
    mNextState = STATE_OUT;
    mIsChanging = true;
}

void RPFshPointPopup::UserDraw() {
    if (mState != STATE_NONE && RPGrpRenderer::GetCurrentViewNo() == 4 &&
        RPGrpRenderer::GetDrawStep() == 5) {
        mpLayout->draw();
    }
}
