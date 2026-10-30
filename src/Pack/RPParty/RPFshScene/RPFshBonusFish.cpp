// Fishing bonus fish indicator (RPFshBonusFish): a spinning fish model in the fishingBonus_00
// layout; catching a fish of its type doubles the points.
// Unit 0x802136D4-0x80214E78. Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshBonusFish.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpDrawPathXluSnap.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPFshScene/RPFshCamera.h>
#include <Pack/RPParty/RPFshScene/RPFshFish.h>
#include <Pack/RPParty/RPFshScene/RPFshFishMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshResultLayout.h>
#include <Pack/RPParty/RPFshScene/RPFshSound.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPSystem.h>
#include <Pack/RPUtility.h>

#include <egg/gfxe.h>
#include <egg/math.h>

// Countdown sound table (RPSndRangeTable::Info: 2 entries, up to 0 -> 0x2C, up to 3 -> 0x2B)
static RPSndRangeTable::Entry sStartSoundTable[3] = {{2, 0}, {0x2C, 3}, {0x2B, 0}};

inline f32 BonusRadians(f32 degrees) {
    return (degrees / 180.0f) * 3.1415927f;
}

const f32 RPFshBonusFish::sSpinStart = BonusRadians(10.0f);
const f32 RPFshBonusFish::sSpinMin = BonusRadians(2.0f);
f32 RPFshBonusFish::sSpinStep = BonusRadians(5.0f);
const f32 RPFshBonusFish::sSpinMax = BonusRadians(100.0f);

RPFshBonusFish* RPFshBonusFish::sInstance;

inline s32 RandomBonusType() {
    bool retry = true;
    s32 type = -1;
    while (retry) {
        type = RPFshBonusFish::FISH_TYPE_NUM * RPUtlRandom::getF32();
        if (type != RPFshFish::TYPE_SUKA_A && type != RPFshFish::TYPE_RARE_A) {
            retry = false;
        }
    }
    return type;
}

inline f32 BonusAbs(f32 x) {
    return x > 0.0f ? x : -x;
}

class BonusVector : public EGG::Vector3f {
public:
    BonusVector(const nw4r::math::VEC3& v) : EGG::Vector3f(v.x, v.y, v.z) {}
};

extern "C" EGG::Vector3f RP_mtx33MultVec(const EGG::Matrix34f&, const EGG::Vector3f&);

void RPFshBonusFish::CreateInstance() {
    sInstance = new RPFshBonusFish();
}

void RPFshBonusFish::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshBonusFish::~RPFshBonusFish() {}

#pragma auto_inline off
RPFshBonusFish::RPFshBonusFish() {
    mpStateMachine = new RPStateMachine<RPFshBonusFish>(this, STATE_NUM);

    RPFshStateFuncs<RPFshBonusFish> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPFshBonusFish::FadeInEnter, &RPFshBonusFish::FadeInCalc},
        {&RPFshBonusFish::ShowEnter, &RPFshBonusFish::ShowCalc},
        {&RPFshBonusFish::SpinDownEnter, &RPFshBonusFish::SpinDownCalc},
        {&RPFshBonusFish::SpinUpEnter, &RPFshBonusFish::SpinUpCalc},
        {&RPFshBonusFish::FadeOutEnter, &RPFshBonusFish::FadeOutCalc},
    };
    for (int i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    mpResAccessor = RPSysLytResAccessor::create(NULL);
    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));
}
#pragma auto_inline on

void RPFshBonusFish::Create() {
    for (int i = 0; i < FISH_TYPE_NUM; i++) {
        const char* modelNames[FISH_TYPE_NUM] = {"fsh_fish_A",     "fsh_fish_B",
                                                 "fsh_fish_C",     "fsh_fish_sukaA",
                                                 "fsh_fish_rareA", "fsh_fish_rareB"};
        // Wii Play: the (handle, int) overload is called with the model name
        mpModels[i] = RPGrpModel::Construct(i + 8, modelNames[i], 1, 0, 0);
        mpModels[i]->SetDrawScene(2);
        mpModels[i]->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
        mpModels[i]->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_Chr, 0, 0xE,
                                                      "fsh_fish_swimanim");
        mpModels[i]->SetDrawGroup(1);
    }

    RPLytObjBase::AttachArcFile(RPSysResourceManager::LoadGameLocalArchive(), "fishing.arc",
                                mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "fishingBonus_00.brlyt");

    const char* anmNames[3] = {"fishingBonus_00_inAll_00.brlan",
                               "fishingBonus_00_changeAll_00.brlan",
                               "fishingBonus_00_outAll_00.brlan"};
    for (int i = 0; i < 3; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    mpFishPosPane = mpLayout->findPane("N_fishPosi_00");
    mpUnderPane = mpLayout->findPane("N_under_00");
    mpAbovePane = mpLayout->findPane("N_above_00");

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_bonusText_00")->setMessage(pMessage, 0x804, 0, 0);
    mpLayout->findTextBox("T_bonusText_01")->setMessage(pMessage, 0x804, 0, 0);
    mpLayout->findTextBox("T_bonusText_02")->setMessage(pMessage, 0x804, 0, 0);
}

void RPFshBonusFish::Reset() {
    mpStateMachine->Reset(STATE_NONE);

    mType = RandomBonusType();
    while (mType == RPFshFish::TYPE_RARE_B) {
        mType = RandomBonusType();
    }

    mNextType = -1;
    mpModel = mpModels[mType];
    mChangeEnabled = true;
    mFrame = 0;
    mInterval = 900;
    mSpinAngle = 0.0f;
    mSpinSpeed = 0.0f;
    mSpinDir = 0;
    mBonus = 2.0f;

    mpLayout->unbindAllAnmObj();
    for (int i = 0; i < 3; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }

    mpLayout->bindAnmObj(mpAnmObjs[1]);
    mpAnmObjs[1]->restart();
    mpAnmObjs[1]->setRate(0.0f);

    for (int i = 0; i < FISH_TYPE_NUM; i++) {
        mpModels[i]->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
    }
}

void RPFshBonusFish::Calculate() {
    mpStateMachine->Calculate();

    if (RPFshResultLayout::GetInstance()->GetState() != RPFshResultLayout::STATE_NONE) {
        Hide();
    }

    if (mpStateMachine->GetState() != STATE_NONE) {
        mSpinSpeed += sSpinMin * mSpinDir;

        if (mSpinAngle >= 6.2831855f) {
            mSpinAngle -= 6.2831855f;
        }
        if (mSpinAngle <= -6.2831855f) {
            mSpinAngle += 6.2831855f;
        }
        if (mSpinSpeed >= sSpinMax) {
            mSpinSpeed = sSpinMax;
        }
        if (mSpinDir < 0 && mSpinSpeed < sSpinStep) {
            mSpinSpeed = sSpinStep;
        }

        mSpinAngle += mSpinSpeed;
    }

    if (mpStateMachine->GetState() != STATE_NONE) {
        mpUnderPane->SetVisible(true);
        mpAbovePane->SetVisible(true);
        mpLayout->calc();

        if (mpModel != NULL) {
            const nw4r::math::MTX34& rPaneMtx = mpFishPosPane->GetGlobalMtx();
            mScreenPos.x = rPaneMtx._03;
            mScreenPos.y = rPaneMtx._13;

            RPFshCamera* pCamera = RPFshCameraMgr::GetInstance()->GetCamera();
            BonusVector dir(pCamera->GetTargetRef() - pCamera->GetPosition());
            dir.normalise();

            BonusVector base(pCamera->GetPosition() + dir * 100.0f);
            mPos = base + BonusVector(pCamera->GetRight() * mScreenPos.x) +
                   BonusVector(pCamera->GetUp() * mScreenPos.y);

            // VEC3's own assignment: an EGG::Vector3f temporary would take another 0xC of stack.
            static_cast<nw4r::math::VEC3&>(mAxisX) = pCamera->GetPosition() - pCamera->GetTargetRef();
            mAxisX.normalise();

            const nw4r::math::VEC3& rRight = pCamera->GetRight();
            mAxisZ = EGG::Vector3f(-rRight.x, -rRight.y, -rRight.z);
            mAxisY = mAxisZ.cross(mAxisX);

            EGG::Matrix34f rot;
            rot.setAxisRotation(mAxisY, mSpinAngle);
            mAxisX = RP_mtx33MultVec(rot, mAxisX);
            mAxisZ = RP_mtx33MultVec(rot, mAxisZ);

            const RPFshFishTypeParam* pParam = RPFshFish::GetTypeParam(mType);
            if (mType == RPFshFish::TYPE_RARE_A || mType == RPFshFish::TYPE_RARE_B) {
                mScale = 120.0f / (2.0f * pParam->mLength);
            } else {
                mScale = 90.0f / (2.0f * pParam->mLength);
            }

            RPGrpModelAnm* pAnm = mpModel->GetModelAnm();
            f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, 0);
            if (frame >= pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, 0) - 1 - 1.0f) {
                pAnm->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
            }

            EGG::DrawPathXluSnap* pSnap = RPGrpModelManager::GetCurrent()
                ->GetDrawPathManager(2)
                ->GetDrawPathXluSnap()
                ->GetXluSnap();
            pSnap->setAlpha(mpFishPosPane->GetGlbAlpha());

            CalcModelMtx();
        }
    }
}

void RPFshBonusFish::FadeInEnter() {
    mpAnmObjs[0]->restart();
    mSpinAngle = 0.0f;
    mSpinSpeed = 0.0f;
    mSpinDir = 0;
}

void RPFshBonusFish::FadeInCalc() {
    s32 alpha = 255.0f * mpAnmObjs[0]->getFrameCtrl()->getFrame() /
                mpAnmObjs[0]->getFrameCtrl()->getEndFrame();
    if (alpha < 0) {
        alpha = 0;
    } else if (alpha > 255) {
        alpha = 255;
    }

    mpUnderPane->SetAlpha(alpha);
    mpAbovePane->SetAlpha(alpha);

    if (mpAnmObjs[0]->isFinished() && !mpStateMachine->IsChanging()) {
        mpStateMachine->ChangeState(STATE_SHOW);
    }
}

void RPFshBonusFish::ShowEnter() {}

void RPFshBonusFish::ShowCalc() {
    if (mChangeEnabled) {
        if (RPFshFishMgr::GetInstance()->CountFishOfType(mType) == 0) {
            mFrame = mInterval;
        }

        if (mFrame >= mInterval && !mpStateMachine->IsChanging()) {
            if (mNextType == -1) {
                mNextType = RandomBonusType();
                for (int i = 0; mNextType == mType ||
                                RPFshFishMgr::GetInstance()->CountFishOfType(mNextType) == 0;
                     i++) {
                    mNextType = RandomBonusType();
                    if (i > 20) {
                        break;
                    }
                }
            }

            mpStateMachine->ChangeState(STATE_SPIN_UP);
        }
    }

    mFrame++;
}

void RPFshBonusFish::SpinDownEnter() {
    mSpinDir = -1;
}

void RPFshBonusFish::SpinDownCalc() {
    if (BonusAbs(mSpinAngle) <= sSpinStep || BonusAbs(mSpinAngle) >= 6.2831855f - sSpinStep) {
        if (mSpinSpeed <= 1.5f * sSpinStep && !mpStateMachine->IsChanging()) {
            mpStateMachine->ChangeState(STATE_SHOW);
            mSpinDir = 0;
            mSpinAngle = 0.0f;
            mSpinSpeed = 0.0f;

            mpLayout->unbindAllAnmObj();
            mpLayout->bindAnmObj(mpAnmObjs[1]);
            mpAnmObjs[1]->restart();
            mpAnmObjs[1]->setRate(1.0f);

            RPSndAudioMgr::instance()->startSystemSe(0x39U);
        }
    }
}

void RPFshBonusFish::SpinUpEnter() {
    if (mSpinSpeed < sSpinStart) {
        mSpinSpeed = sSpinStart;
    }
    mSpinDir = 1;

    RPSndAudioMgr::instance()->startSystemSe(0x38U);
}

void RPFshBonusFish::SpinUpCalc() {
    if (mSpinSpeed >= sSpinMax && !mpStateMachine->IsChanging()) {
        mpStateMachine->ChangeState(STATE_SPIN_DOWN);
        mType = mNextType;
        mNextType = -1;
        mpModel = mpModels[mType];
        mFrame = 0;
    }
}

void RPFshBonusFish::FadeOutEnter() {
    mpAnmObjs[2]->restart();
}

void RPFshBonusFish::FadeOutCalc() {
    s32 alpha = 255.0f - 255.0f * mpAnmObjs[2]->getFrameCtrl()->getFrame() /
                             mpAnmObjs[2]->getFrameCtrl()->getEndFrame();
    if (alpha < 0) {
        alpha = 0;
    } else if (alpha > 255) {
        alpha = 255;
    }

    mpUnderPane->SetAlpha(alpha);
    mpAbovePane->SetAlpha(alpha);

    if (mpAnmObjs[2]->isFinished() && !mpStateMachine->IsChanging()) {
        mpStateMachine->ChangeState(STATE_NONE);
    }
}

void RPFshBonusFish::Show(s32 changeType) {
    if (changeType != 0) {
        mType = RandomBonusType();
        mpModel = mpModels[mType];
    }

    s32 state = mpStateMachine->GetState();
    if (state != STATE_FADE_IN && state != STATE_SHOW) {
        mpStateMachine->ChangeState(STATE_FADE_IN);
    }
}

void RPFshBonusFish::Hide() {
    s32 state = mpStateMachine->GetState();
    if (state != STATE_FADE_OUT && state != STATE_NONE) {
        mpStateMachine->ChangeState(STATE_FADE_OUT);
    }
}

void RPFshBonusFish::SetNextType(s32 type) {
    mNextType = type;
    mFrame = mInterval;
}

void RPFshBonusFish::UserDraw() {
    if (mpStateMachine->GetState() != STATE_NONE) {
        if (RPGrpRenderer::GetCurrentViewNo() == 2) {
            if (RPGrpRenderer::GetDrawStep() == 5) {
                mpUnderPane->SetVisible(true);
                mpAbovePane->SetVisible(false);
                mpLayout->draw();
            }
        } else if (RPGrpRenderer::GetCurrentViewNo() == 4 && RPGrpRenderer::GetDrawStep() == 5) {
            mpUnderPane->SetVisible(false);
            mpAbovePane->SetVisible(true);
            mpLayout->draw();
        }
    }
}

const RPSndRangeTable::Info* RPFshGetStartSoundTable() {
    return reinterpret_cast<const RPSndRangeTable::Info*>(sStartSoundTable);
}

// Flush the functions above, then emit the state machine in the original's address order
// (implicit instantiation puts CallExit before CallEnter); see RPFshGame.cpp.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPFshBonusFish>::RPStateMachine(RPFshBonusFish*, s32);
template void RPStateMachine<RPFshBonusFish>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template void RPStateMachine<RPFshBonusFish>::Reset(s32);
template void RPStateMachine<RPFshBonusFish>::ChangeState(s32);
template void RPStateMachine<RPFshBonusFish>::Calculate();
template void RPStateMachine<RPFshBonusFish>::CallEnter();
template void RPStateMachine<RPFshBonusFish>::CallCalc();
template void RPStateMachine<RPFshBonusFish>::CallExit();
