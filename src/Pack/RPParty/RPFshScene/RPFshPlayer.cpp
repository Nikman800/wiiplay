// Fishing player (RPFshPlayer): the hand avatar, aiming and casting with the pointer, the fight
// with a hooked fish and landing it.
// Unit 0x802067E4-0x80208BA4, including the three compiler adjustment thunks.
// Functions in reverse address order.
#define RP_FSH_PLAYER_SOUND_PLAYER_TEMP
#include <Pack/RPParty/RPFshScene/RPFshPlayer.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpDrawPathLightMap.h>
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpLightManager.h>
#include <Pack/RPKernel/RPSysKokeshiGenInfo.h>
#include <Pack/RPKernel/RPSysKokeshiManager.h>
#include <Pack/RPParty/RPFshScene/RPFshBonusFish.h>
#include <Pack/RPParty/RPFshScene/RPFshCamera.h>
#include <Pack/RPParty/RPFshScene/RPFshFish.h>
#include <Pack/RPParty/RPFshScene/RPFshFishMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayerMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshPrompt.h>
#include <Pack/RPParty/RPFshScene/RPFshRod.h>
#include <Pack/RPParty/RPFshScene/RPFshScoreLayout.h>
#include <Pack/RPParty/RPFshScene/RPFshStartLayout.h>
#include <Pack/RPParty/RPFshScene/RPFshTimer.h>
#include <Pack/RPParty/RPPartyCommon/RPBase8034F6B0.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPCurve.h>
#include <Pack/RPParty/RPPartyCommon/RPInterpCamera.h>
#include <Pack/RPParty/RPPartyCommon/RPLeadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPPauseText.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPUtility.h>

#include <egg/core.h>
#include <egg/gfxe.h>
#include <egg/math.h>

#include <revolution/PAD.h>


f32 RPFshPlayer::sSteadyAccelMin = -1.4f;
f32 RPFshPlayer::sSteadyAccelMax = -0.5f;
f32 RPFshPlayer::sCastDist = 30.0f;
f32 RPFshPlayer::sMoveRate = 0.1f;
f32 RPFshPlayer::sAimBaseY = -80.0f;
f32 RPFshPlayer::sAimDistScale = 2500.0f;
f32 RPFshPlayer::sAimBaseZ = 100.0f;
f32 RPFshPlayer::sAimRangeZ = 110.0f;
f32 RPFshPlayer::sSpacing = 200.0f;
f32 RPFshPlayer::sLandRate = 0.5f;
f32 RPFshPlayer::sWaitRate = 0.4f;
f32 RPFshPlayer::sAimMinY = -50.0f;
f32 RPFshPlayer::sPosPlayRadius = 0.005f;
f32 RPFshPlayer::sDistRadiusMin = 0.05f;
f32 RPFshPlayer::sDistRadiusMax = 0.2f;
f32 RPFshPlayer::sRemoteDistNear = 1.0f;
f32 RPFshPlayer::sRemoteDistFar = 2.5f;
f32 RPFshPlayer::sIdleMaxX = 500.0f;
s32 RPFshPlayer::sLandFrames = 100;

f32 RPFshPlayer::sAimRangeMin = sAimBaseZ - sAimRangeZ;
f32 RPFshPlayer::sAimRangeMax = sAimBaseZ + sAimRangeZ;
EGG::Vector3f RPFshPlayer::sAimBasePos(0.0f, sAimBaseY, sAimBaseZ);
EGG::Vector2f RPFshPlayer::sBasePos(0.0f, -80.0f);
EGG::Vector3f RPFshPlayer::sLandOffset(0.0f, -6.0f, 5.0f);
EGG::Vector3f RPFshPlayer::sLandScale(10.0f, 10.0f, 3.0f);
// Unreferenced: one array (both registration nodes come before it in .bss), not the header's
// sUnusedPosL/sUnusedPosR pair.
static EGG::Vector2f sUnusedPos[2] = {EGG::Vector2f(-0.25f, -0.4f), EGG::Vector2f(0.25f, -0.4f)};

namespace {

// RPActorBase keeps its position as a VEC3; the player moves it with EGG vector math
inline EGG::Vector3f& AsVector3f(nw4r::math::VEC3& rVec) {
    return static_cast<EGG::Vector3f&>(rVec);
}

inline f32 MaxValue(f32 a, f32 b) {
    return a > b ? a : b;
}

inline void CopyVector(const nw4r::math::VEC3& in, EGG::Vector3f* out) {
    out->set(in.x, in.y, in.z);
}

} // namespace

// Wii Play: the destructor comes after the constructor in the binary
RPFshPlayer::~RPFshPlayer() {}

// Wii Play: the reference wrapper in TRPSndAudio3DActor and disabled copy propagation
// reproduce the registers and scheduling of the inlined sound construction.
#pragma opt_propagation off
RPFshPlayer::RPFshPlayer(s32 playerIndex) : mPlayerIndex(playerIndex) {
    mScale = 30.0f;
    mpRod = new RPFshRod(this);

    RPSysKokeshiGenInfo info;
    info.SetGenType(RPSysKokeshiManager::GenType_Player);
    info.SetPlayerNo(mPlayerIndex);
    info.SetHandType(0);
    mpAvatar = new RPCls8034F6B0(&info);

    mpSnd = new RPFshFishSnd();
    if (mPlayerIndex == 0) {
        mpSnd->SetRemoteFlag(1);
    } else {
        mpSnd->SetRemoteFlag(2);
    }

    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));
}
#pragma opt_propagation reset

// Only the string-pool label names differ (pLight declared first for the original registers).
void RPFshPlayer::Create() {
    void* pLight;
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPSysKokeshiManager::CreateLightTexture(1);

    pLight = RPSysResourceManager::GetFileFromArchive(pArchive, "Environment/scene_Light.plight");
    RPGrpModelManager::GetCurrent()->GetLightManager(1)->SetBinary(pLight);

    void* pLightMap =
        RPSysResourceManager::GetFileFromArchive(pArchive, "Environment/scene_Lm.plmap");
    RPGrpDrawPathLightMap* pLightMapPath =
        RPGrpModelManager::GetCurrent()->GetDrawPathManager(1)->GetDrawPathLightMap();
    pLightMapPath->GetLightTextureManager()->SetBinary(pLightMap);
    pLightMapPath->GetLightTextureManager()->onFlag2();

    mpAvatar->LoadResource();
    mpAvatar->ApplyLightTexture(1);
    mpModel = mpAvatar->GetHandModel();
    mpModel->SetDrawScene(1);
    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
    mpModel->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_Chr, 0, 0xF, "fsh_hand_R_grip");

    mpRod->Create();
}

void RPFshPlayer::Reset() {
    mState = STATE_NONE;
    mPrevState = STATE_NONE;
    mIsChanging = false;
    mStateTimer = 0;
    mAxisZ = EGG::Vector3f(0.0f, 0.0f, 1.0f);
    mAxisY = EGG::Vector3f(0.0f, 1.0f, 0.0f);
    mAxisX = EGG::Vector3f(1.0f, 0.0f, 0.0f);

    f32 playerOffset = mPlayerIndex -
                      (RPFshPlayerMgr::GetInstance()->GetPlayerNum() - 1.0f) / 2.0f;
    mBasePos = sBasePos;
    mBasePos.x += sSpacing * playerOffset;
    mPointer.x = mBasePos.x;
    mPointer.y = mBasePos.y;
    mPointer.z = 1.0f;
    mPrevPointer.x = mBasePos.x;
    mPrevPointer.y = mBasePos.y;
    mPrevPointer.z = 1.0f;

    AsVector3f(mPos) = mAimPos = CalcAimPos(false);
    mHookedFish = -1;
    mCaughtNum = 0;
    mIsLanding = false;
    mStatusHead = 0;
    mStatusNum = 0;

    mpRod->Reset();
    mpSnd->set3DPosition(mPos);
}

void RPFshPlayer::Exit() {
    mpRod->Exit();
    PADControlMotor(mControllerNo, PAD_MOTOR_STOP);
}

void RPFshPlayer::Calculate() {
    if (mState != STATE_NONE) {
        EGG::CoreControllerMgr::instance()->getNthController(mControllerNo)->setPosParam(
            sPosPlayRadius, 1.0f);

        f32 radius = sDistRadiusMin + (sDistRadiusMax - sDistRadiusMin) *
                                          ((mPointer.z - sRemoteDistNear) /
                                           (sRemoteDistFar - sRemoteDistNear));
        if (radius < sDistRadiusMin) {
            radius = sDistRadiusMin;
        } else if (radius > sDistRadiusMax) {
            radius = sDistRadiusMax;
        }
        EGG::CoreControllerMgr::instance()->getNthController(mControllerNo)->setDistParam(radius,
                                                                                         1.0f);

        s32 num =
            EGG::CoreControllerMgr::instance()->getNthController(mControllerNo)->getReadLength();
        for (int i = 0; i < num; i++) {
            s32 idx = (mStatusHead + num - 1 - i) % STATUS_HISTORY_NUM;
            mStatusHistory[idx] = *EGG::CoreControllerMgr::instance()
                                       ->getNthController(mControllerNo)
                                       ->getCoreStatus(i);
        }

        mStatusHead = (mStatusHead + num) % STATUS_HISTORY_NUM;
        mStatusNum += num;
        if (mStatusNum > STATUS_HISTORY_NUM) {
            mStatusNum = STATUS_HISTORY_NUM;
        }

        mPrevPointer = mPointer;
        if (EGG::CoreControllerMgr::instance()->getNthController(mControllerNo)->getDpdValidFlag() > 0) {
            if (!RPFshTimer::GetInstance()->IsTimeUp()) {
                if (RPFshStartLayout::GetInstance()->IsStarted()) {
                    EGG::Vector2f pos = EGG::CoreControllerMgr::instance()
                                            ->getNthController(mControllerNo)
                                            ->getDpdPos();
                    mPointer.x = pos.x * (u16)(EGG::Screen::GetSizeXMax() / 2);
                    mPointer.y = -pos.y * (u16)(EGG::Screen::GetSizeYMax() / 2);
                }

                mPointer.z = EGG::CoreControllerMgr::instance()
                                 ->getNthController(mControllerNo)
                                 ->getDpdDistance();
            }
        }
    }

    RPFshStateFuncs<RPFshPlayer> funcs[STATE_NUM] = {
        {NULL, NULL},
        {&RPFshPlayer::AimEnter, &RPFshPlayer::AimCalc},
        {&RPFshPlayer::WaitEnter, &RPFshPlayer::WaitCalc},
        {&RPFshPlayer::FightEnter, &RPFshPlayer::FightCalc},
        {&RPFshPlayer::LandEnter, &RPFshPlayer::LandCalc},
        {&RPFshPlayer::ReturnEnter, &RPFshPlayer::ReturnCalc},
        {&RPFshPlayer::IdleEnter, &RPFshPlayer::IdleCalc},
    };

    if (mIsChanging) {
        mPrevState = mState;
        mState = mNextState;
        mIsChanging = false;
        mStateTimer = 0;

        RPFshStateFuncs<RPFshPlayer>::Func enter = funcs[mState].mEnter;
        if (enter != NULL) {
            (this->*enter)();
        }
    }

    RPFshStateFuncs<RPFshPlayer>::Func calc = funcs[mState].mCalc;
    if (calc != NULL) {
        (this->*calc)();
    }

    if (mState != STATE_NONE) {
        mpRod->Calculate();
    }

    bool cursorEnabled[STATE_NUM] = {false, true, true, false, false, false, false};
    RPCommon::GetInstance()->GetLeadCursor()->SetPlayerEnabled(mPlayerIndex,
                                                               cursorEnabled[mState]);
}

void RPFshPlayer::AimEnter() {
    mIsAiming = true;
}

void RPFshPlayer::AimCalc() {
    if (RPFshTimer::GetInstance()->IsTimeUp()) {
        ChangeState(STATE_IDLE);
        return;
    }

    if (!RPFshStartLayout::GetInstance()->IsStarted()) {
        return;
    }

    EGG::Vector3f aim = CalcAimPos(false);
    AsVector3f(mPos) = AsVector3f(mPos) * (1.0f - sMoveRate) + aim * sMoveRate;

    RPGrpView* pView = RPGrpRenderer::GetCurrent()->GetView(1);
    RPGrpCamera* pCamera = pView->GetCamera();
    RPGrpScreen* pScreen = pView->GetScreen();

    EGG::Vector3f handScreen;
    pCamera->GetWorldToScreen(&handScreen, mPos, pScreen);
    EGG::Vector3f gripScreen;
    pCamera->GetWorldToScreen(&gripScreen, mpRod->GetGripPos(), pScreen);

    EGG::Vector3f diff = handScreen - gripScreen;
    diff.z = 0.0f;
    if (diff.length() < sCastDist) {
        ChangeState(STATE_WAIT);
        mCastDist = mPointer.z;
        mpSnd->startSound(0U);
    }
}

void RPFshPlayer::WaitEnter() {
    mpRod->SetUseKcl(true);
    mIsAiming = false;
}

void RPFshPlayer::WaitCalc() {
    if (RPFshTimer::GetInstance()->IsTimeUp()) {
        ChangeState(STATE_IDLE);
        return;
    }

    EGG::Vector3f aim = CalcAimPos(true);
    AsVector3f(mPos) = AsVector3f(mPos) * (1.0f - sMoveRate) + aim * sMoveRate;
    mpRod->SetPosFromGrip(AsVector3f(mPos));

    if (IsHookWaiting()) {
        for (int i = 0; i < RPFshFishMgr::FISH_NUM; i++) {
            RPFshFish* pFish = RPFshFishMgr::GetInstance()->GetFish(i);
            if (pFish->GetState() != RPFshFish::STATE_NONE && pFish->IsBiting() &&
                mHookedFish == -1 &&
                pFish->IsMouthNearPoint(mpRod->GetHookRadius(),
                                        mpRod->GetRope()->GetPoint(RPFshRod::ROPE_JOINT_NUM - 1))) {
                ChangeState(STATE_FIGHT);
                pFish->ChangeState(RPFshFish::STATE_HOOKED);
                mHookedFish = i;
            }
        }
    }

    f32 scale = sWaitRate * ((sAimBaseZ - mPos.z) / sAimRangeZ) + 1.0f;
    scale = 1.0f > scale ? 1.0f : scale;
    mpRod->SetLengthScale(scale);
}

void RPFshPlayer::FightEnter() {
    if (RPFshFishMgr::GetInstance()->GetFish(mHookedFish)->GetType() !=
        RPFshFish::TYPE_SUKA_A) {
        RPFshYankPrompt::GetInstance()->Show(mPlayerIndex);
    }

    mpRod->SetUseKcl(false);
}

void RPFshPlayer::FightCalc() {
    RPFshFish* pFish = RPFshFishMgr::GetInstance()->GetFish(mHookedFish);
    if (pFish->GetState() == RPFshFish::STATE_JUMP || pFish->GetState() == RPFshFish::STATE_SWIM) {
        ChangeState(STATE_WAIT);
        mHookedFish = -1;
        RPFshYankPrompt::GetInstance()->Hide(mPlayerIndex);
        Rumble(8.0f);
        return;
    }

    f32 random = 0.3f * RPUtlRandom::getF32();
    f32 rate = 1.0f + random;
    AsVector3f(mPos) -=
        (static_cast<const EGG::Vector3f&>(pFish->GetAxisZ()) * pFish->GetPullSpeed()) * rate;
    mPos.y += 0.4f * (rate * pFish->GetPullSpeed());
    mpRod->SetPosFromGrip(AsVector3f(mPos));

    EGG::Vector3f front = mpRod->GetFrontDir();
    EGG::Vector3f toFish = static_cast<const EGG::Vector3f&>(pFish->GetPos()) - mpRod->GetTipPos();
    toFish.normalise();
    f32 step = 0.1f; // unused: only places 0.1f before the epsilon in .sdata2
    bool zero = toFish.squaredLength() <= 1.1920929e-7f;
    if (!zero) {
        front += toFish * 0.1f;
        front.normalise();
        if (front.z < -0.2f && front.y > 0.2f) {
            mpRod->SetFrontDir(front);
        }
    }

    if (IsRemoteSteady() && RPFshPlayerMgr::GetInstance()->GetCatchingPlayer() == -1) {
        RPFshPlayerMgr::GetInstance()->SetCatchingIdx(mPlayerIndex);
        RPFshPlayerMgr::GetInstance()->SetCatchingPlayer(mPlayerIndex);
        RPFshFishMgr::GetInstance()->SetFishToSwim(mHookedFish);
        RPFshFishMgr::GetInstance()->SetUnk10(mHookedFish);
        RPFshYankPrompt::GetInstance()->Hide(mPlayerIndex);
    }

    const RPFshFishTypeParam* pParam = RPFshFish::GetTypeParam(pFish->GetType());
    s32 strength = pParam->unk24 * pFish->GetScale();
    if (strength > 10) {
        strength = 10;
    }
    if (mStateTimer % 12 == 0) {
        Rumble(strength);
    }

    mStateTimer++;
}

void RPFshPlayer::LandEnter() {
    RPCommon::GetInstance()->GetPauseText()->SetEnabled(false);
}

void RPFshPlayer::LandCalc() {
    RPFshFish* pFish = RPFshFishMgr::GetInstance()->GetFish(mHookedFish);
    EGG::Vector3f front = mpRod->GetFrontDir();

    if (pFish->IsHooked()) {
        mpRod->SetFollowRest(false);
        front.y += sLandRate;
        if (front.z > -0.2f) {
            front.z = -0.2f;
        }
        front.normalise();
        mpRod->SetFrontDir(front);

        mPos.x += sLandOffset.x;
        mPos.y += sLandOffset.y;
        mPos.z += sLandOffset.z;
        mpRod->SetPosFromGrip(AsVector3f(mPos));
        Rumble(16.0f);
    } else {
        if (mStateTimer < sLandFrames) {
            mIsLanding = true;
            f32 t = RPRange::CalcEase((f32)mStateTimer / sLandFrames, 1);
            f32 side = 1.0f - t;
            f32 up = 1.0f - 2.0f * t;

            mLandPos = mpRod->GetRope()->GetPoint(RPFshRod::ROPE_JOINT_NUM - 1);
            EGG::Vector3f offset = sLandScale;
            const nw4r::math::VEC3& tip =
                mpRod->GetRope()->GetPoint(RPFshRod::ROPE_JOINT_NUM - 1);
            if (tip.x > mpRod->GetRopePoint(0).x) {
                offset.x = -offset.x;
            }
            offset.x *= side;
            offset.z *= side;
            offset.y *= up;
            mLandPos.x += offset.x;
            mLandPos.y += offset.y;
            mLandPos.z += offset.z;

            EGG::Vector3f move = offset * 1.5f;
            move.y = -move.y;
            mPos.x += move.x;
            mPos.y += move.y;
            mPos.z += move.z;
            mStateTimer++;
        } else {
            mIsLanding = false;
            mpRod->SetFollowRest(true);
            AsVector3f(mPos) = AsVector3f(mPos) * (1.0f - sMoveRate) + mAimPos * sMoveRate;
        }

        mpRod->SetPosFromGrip(AsVector3f(mPos));
    }

    if (RPFshPlayerMgr::GetInstance()->GetCatchingPlayer() == -1 &&
        !RPFshCameraMgr::GetInstance()->GetCamera()->IsActive()) {
        ChangeState(STATE_WAIT);
        mpRod->SetAllRopeTargets(EGG::Vector3f::zero);
        mpRod->SetFollowRest(true);
        mHookedFish = -1;
    }
}

void RPFshPlayer::ReturnEnter() {
    mHookedFish = -1;
    RPFshYankPrompt::GetInstance()->Hide(mPlayerIndex);
    RPFshScoreLayout::GetInstance()->RequestHide(mPlayerIndex);
}

void RPFshPlayer::ReturnCalc() {
    f32 rate = 2.0f * sMoveRate;
    AsVector3f(mPos) = AsVector3f(mPos) * (1.0f - rate) + mAimPos * rate;
    mpRod->SetPosFromGrip(AsVector3f(mPos));

    if (RPFshPlayerMgr::GetInstance()->GetCatchingPlayer() == -1 &&
        !RPFshCameraMgr::GetInstance()->GetCamera()->IsActive()) {
        if (mPrevState == STATE_AIM) {
            ChangeState(STATE_AIM);
        } else {
            ChangeState(STATE_WAIT);
        }

        mpRod->SetAllRopeTargets(EGG::Vector3f::zero);
        mpRod->SetFollowRest(true);
        mHookedFish = -1;
        RPFshScoreLayout::GetInstance()->RequestShow(mPlayerIndex);
    }
}

// The split merges IdleEnter into IdleCalc (the state table points to IdleCalc+0x64). Defined
// inline here, its weak copy comes right after IdleCalc, the first function that refers to it.
inline void RPFshPlayer::IdleEnter() {}

void RPFshPlayer::IdleCalc() {
    RPFshStateFuncs<RPFshPlayer>::Func enter = &RPFshPlayer::IdleEnter;
    (void)enter;
    f32 step = 10.0f;
    if (mPos.x < 0.0f) {
        mPos.x -= step;
    } else {
        mPos.x += step;
    }

    if (mPos.x < -sIdleMaxX) {
        mPos.x = -sIdleMaxX;
    }
    if (mPos.x > sIdleMaxX) {
        mPos.x = sIdleMaxX;
    }

    mpRod->SetPosFromGrip(AsVector3f(mPos));
}

void RPFshPlayer::UpdateRod() {
    if ((mState == STATE_FIGHT || mState == STATE_LAND) && mHookedFish != -1) {
        RPFshFish* pFish = RPFshFishMgr::GetInstance()->GetFish(mHookedFish);
        if (pFish->IsHooked()) {
            EGG::Vector3f move = pFish->GetMouthPos() -
                                 mpRod->GetRope()->GetPoint(RPFshRod::ROPE_JOINT_NUM - 1);
            mpRod->Translate(move);
            for (int i = 0; i < RPFshRod::ROPE_JOINT_NUM; i++) {
                mpRod->SetRopeTargetPoint(i, (move * (f32)i) / 7.0f);
            }
        }
    }

    if (mIsLanding) {
        EGG::Vector3f move =
            mLandPos - mpRod->GetRope()->GetPoint(RPFshRod::ROPE_JOINT_NUM - 1);
        mpRod->Translate(move);
    }

    if (mState == STATE_NONE) {
        return;
    }

    if (mIsAiming) {
        RPFshCamera* pCamera = RPFshCameraMgr::GetInstance()->GetCamera();
        EGG::Vector3f view;
        CopyVector(pCamera->GetTargetRef() - pCamera->GetPosition(), &view);
        view.normalise();
        mAxisZ = -view;
        mAxisY = EGG::Vector3f(0.0f, 1.0f, 0.0f);
        mAxisX = mAxisY.cross(mAxisZ);
        mAxisX.normalise();
        mAxisY = mAxisZ.cross(mAxisX);
    } else {
        mAxisZ = -mpRod->GetFrontDir();
        mAxisY = mpRod->GetUpDir();
        mAxisX = mAxisY.cross(mAxisZ);
        mPos = mpRod->GetGripPos();
    }

    if (mIsAiming) {
        mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
    } else {
        mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 1.0f);
    }
    mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, 0.0f);

    CalcModelMtx();
    mpRod->CalcHookWater();
    mpSnd->set3DPosition(mPos);
}

bool RPFshPlayer::IsHookWaiting() const {
    if (mState == STATE_WAIT && !mIsChanging && mpRod->IsHookInWater()) {
        return true;
    }
    return false;
}

void RPFshPlayer::Init(u32 arg, s32 controllerNo) {
    unk64 = arg;
    ChangeState(STATE_AIM);
    mControllerNo = controllerNo;
}

void RPFshPlayer::AddCaughtFish(s32 fishIdx) {
    RPFshFish* pFish = RPFshFishMgr::GetInstance()->GetFish(fishIdx);
    RPFshCaughtFish& rCaught = mCaughtFish[mCaughtNum];
    rCaught.mType = pFish->GetType();
    rCaught.mSize = pFish->GetScale();
    if (pFish->GetType() == RPFshBonusFish::GetInstance()->GetType()) {
        rCaught.mBonus = RPFshBonusFish::GetInstance()->GetBonus();
    } else {
        rCaught.mBonus = 1.0f;
    }
    mCaughtNum++;
}

void RPFshPlayer::Rumble(f32 strength) {
    if (mState != STATE_IDLE) {
        if (strength < 4.0f) {
            strength = 4.0f;
        }
        EGG::CoreControllerMgr::instance()->getNthController(mControllerNo)->startSimpleRumble(
            0xFFFFFFFF, strength, 32);
    }
}

// Matches with copy propagation off (otherwise the return pointer and pView swap r29/r30).
#pragma opt_propagation off
EGG::Vector3f RPFshPlayer::CalcAimPos(bool adjust) {
    RPFshCamera* pCamera = RPFshCameraMgr::GetInstance()->GetCamera();
    EGG::Vector3f normal;
    EGG::Vector3f base = sAimBasePos;
    normal = static_cast<const EGG::Vector3f&>(pCamera->GetTargetRef() - pCamera->GetPosition());
    normal.normalise();

    RPGrpView* pView = RPGrpRenderer::GetCurrent()->GetView(0);
    EGG::Vector3f pos;
    if (pCamera->GetScreenToPlane(&pos, base, normal, pView->GetScreen(), mPointer.x,
                                  mPointer.y)) {
        if (adjust) {
            pos -= (normal * (mPointer.z - mCastDist)) * sAimDistScale;
        }

        if (pos.z < sAimRangeMin) {
            mCastDist += (pos.z - sAimRangeMin) / sAimDistScale;
            pos.z = sAimRangeMin;
        }
        if (pos.z > sAimRangeMax) {
            mCastDist += (pos.z - sAimRangeMax) / sAimDistScale;
            pos.z = sAimRangeMax;
        }

        pos.y = MaxValue(sAimMinY * (mpRod->GetLengthScale() - 1.0f) + sAimBaseY, pos.y);

        EGG::Vector3f screen;
        pCamera->GetWorldToScreen(&screen, pos, pView->GetScreen());
        f32 left = 0.5f * -(u16)pView->GetScreen()->GetSize().x;
        screen.x = screen.x > left ? screen.x : left;
        f32 right = 0.5f * (u16)pView->GetScreen()->GetSize().x;
        screen.x = screen.x < right ? screen.x : right;
        f32 bottom = 0.5f * (u16)pView->GetScreen()->GetSize().y;
        screen.y = screen.y < bottom ? screen.y : bottom;
        pCamera->GetScreenToWorld(&pos, screen, pView->GetScreen());
        return pos;
    }

    return EGG::Vector3f(mPos);
}
#pragma opt_propagation reset

bool RPFshPlayer::IsRemoteSteady() {
    EGG::Vector3f accel =
        EGG::CoreControllerMgr::instance()->getNthController(mControllerNo)->getAccel();
    if (accel.y < sSteadyAccelMin) {
        for (int i = 0; i < 20; i++) {
            if (i < mStatusNum) {
                bool unstable =
                    mStatusHistory[(mStatusHead + STATUS_HISTORY_NUM - 1 - i) % STATUS_HISTORY_NUM]
                            .getAccel().y > sSteadyAccelMax;
                if (unstable) {
                    return false;
                }
            }
        }
        return true;
    }

    return false;
}

// The shadow draw after the IsStarted check is not in this build; the check itself survives
// (the original keeps the predicate in r4 and returns either way).
void RPFshPlayer::UserDraw() {
    if (!RPFshStartLayout::GetInstance()->IsStarted()) {
        return;
    }
}
