// Table Tennis player: racket and Mii hand, Remote pointer / CPU movement, swing samples, racket
// against ball and serve. Unit 0x802274C0-0x8022AA04 (dtk/game_units.tsv). Functions in reverse
// address order.
#include <Pack/RPParty/RPPnpScene/RPPnpPlayer.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPPartyCommon/RPBase8034F6B0.h>
#include <Pack/RPParty/RPPnpScene/RPPnpBall.h>
#include <Pack/RPParty/RPPnpScene/RPPnpField.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPParty/RPPnpScene/RPPnpHud.h>
#include <Pack/RPParty/RPPnpScene/RPPnpLevelTable.h>
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>
#include <Pack/RPParty/RPPnpScene/RPPnpPhase.h>
#include <Pack/RPParty/RPPnpScene/RPPnpSound.h>
#include <Pack/RPUtility/RPUtlRandom.h>

#include <egg/core.h>
#include <egg/gfxe.h>
#include <egg/math.h>

#include <cmath>


enum PnpPlayerGameMode { PnpPlayerSingle, PnpPlayerMulti };
enum PnpPlayerBallState { PnpPlayerBallIdle, PnpPlayerBallServe = 2, PnpPlayerBallHit = 7,
                          PnpPlayerBallOut = 8 };
enum PnpPlayerSide { PnpPlayerNear, PnpPlayerFar, PnpPlayerNoSide };

s32 RPPnpPlayer::sHitWindowFrames = 6;
f32 RPPnpPlayer::sRacketScale = 25.0f;
f32 RPPnpPlayer::sPlayerHeight = 40.0f;
f32 RPPnpPlayer::sCpuReturnSpeed = 1.65f;
f32 RPPnpPlayer::sTossOffset = 20.0f;
f32 RPPnpPlayer::sServeHitHeight = 10.0f;
f32 RPPnpPlayer::sReturnSpinScale = 0.7f;
f32 RPPnpPlayer::sSwingSpeedScale = 4.0f;
f32 RPPnpPlayer::sSlowSwingSpeed = 1.0f;
f32 RPPnpPlayer::sHumanFollowRate = 0.35f;
f32 RPPnpPlayer::sHumanMaxX = 170.0f;
f32 RPPnpPlayer::sHumanMinX = -155.0f;
f32 RPPnpPlayer::sHumanMoveSpeed = 0.5f;
f32 RPPnpPlayer::sSmashSpeedScale = 1.5f;
f32 RPPnpPlayer::sSmashGravityScale = 2.0f;
f32 RPPnpPlayer::sSmashWindow = 0.9f;
f32 RPPnpPlayer::sCpuLandingOffset = 60.0f;

nw4r::math::VEC3 RPPnpPlayer::sServeBox(100.0f, 0.03f, 100.0f);
nw4r::math::VEC3 RPPnpPlayer::sHumanBox(80.0f, 90.0f, 0.03f);
nw4r::math::VEC3 RPPnpPlayer::sCpuBox(48.0f, 96.0f, 0.03f);
nw4r::math::VEC3 RPPnpPlayer::sSpinFactors(0.09f, 0.0f, 0.25f);

bool RPPnpPlayer::sSwingHintShown;
RPGrpHandle RPPnpPlayer::sHandAnmHandles[3];

// The averaged swing velocity; in the points game its x is clamped and eased by a sine
inline nw4r::math::VEC3 RPPnpPlayer::GetSwing() const {
    nw4r::math::VEC3 swing = mSwingVecs[0];
    PnpPlayerGameMode mode = (PnpPlayerGameMode)RPPnpGame::GetInstance()->IsMultiPlayer();
    if (mode == PnpPlayerMulti) {
        f32 max = RPPnpPlayer::sSwingSpeedScale / RPPnpPlayer::sSpinFactors.x;
        if (swing.x > max) {
            swing.x = max;
        } else if (swing.x < -max) {
            swing.x = -max;
        }
        swing.x *= (f32)std::sin(3.1415927f * (f32)std::fabs(swing.x / max) / 2.0f);
    }
    return swing;
}

RPPnpPlayer::~RPPnpPlayer() {}

void RPPnpPlayer::RecordSwingSample(nw4r::math::VEC3 pos, bool valid, bool swing,
                                    bool spin) {
    int prev = (mSampleCount + 9) % 10;
    SwingSample& rSample = mSamples[mSampleCount % 10];

    rSample.mPos = pos;
    rSample.mValid = valid;
    rSample.unk25 = swing;
    rSample.unk26 = spin;

    if (valid) {
        rSample.mVel = rSample.mPos - mSamples[prev].mPos;
        rSample.mAccel = rSample.mVel - mSamples[prev].mVel;
    } else {
        rSample.mVel = sVecZero;
        rSample.mAccel = sVecZero;
    }

    if (++mSampleCount >= 10) {
        mSampleCount = 0;
    }

    nw4r::math::VEC3 unused = sVecZero;
    nw4r::math::VEC3 vel = sVecZero;
    nw4r::math::VEC3 accel = sVecZero;
    int num = 0;
    for (int i = 0; i < 10; i++) {
        if (mSamples[i].mValid && mSamples[i].unk25) {
            num++;
            vel += mSamples[i].mVel;
            accel += mSamples[i].mAccel;
        }
    }
    if (num != 0) {
        vel /= num;
        accel /= num;
    }
    mSwingVecs[0] = vel;
    mSwingVecs[1] = accel;

    if (spin) {
        vel = sVecZero;
        accel = sVecZero;
        num = 0;
        for (int i = 0; i < 10; i++) {
            if (mSamples[i].mValid && mSamples[i].unk26) {
                num++;
                vel += mSamples[i].mVel;
                accel += mSamples[i].mAccel;
            }
        }
        if (num != 0) {
            vel /= num;
            accel /= num;
        }
        mSwingVecs[2] = vel;
        mSwingVecs[3] = accel;
    }
}

#pragma push
#pragma always_inline on
#pragma opt_propagation off
static inline bool PnpPlayerProjectPointer(const EGG::CoreStatus* pStatus, f32 tableHeight,
                                          nw4r::math::VEC3& target,
                                          nw4r::math::VEC3& planePos,
                                          nw4r::math::VEC3& planeNrm) {
    f32 y, x, planeHeight;
    EGG::Screen::TVMode mode = EGG::Screen::GetTVMode();
    RPGrpRenderer* const renderer = RPGrpRenderer::GetCurrent();
    u16 halfWidth = (u16)(EGG::Screen::GetSizeXMax(mode) / 2);
    u16 halfHeight = (u16)(EGG::Screen::GetSizeYMax(mode) / 2);
    x = pStatus->pos.x;
    y = -pStatus->pos.y;
    const RPGrpScreen* pScreen = renderer->GetView(0)->GetScreen();
    planeHeight = tableHeight + RPPnpPlayer::sPlayerHeight;
    planePos.x = 0.0f;
    planePos.y = planeHeight;
    planePos.z = 0.0f;
    planeNrm.x = 0.0f;
    planeNrm.y = 1.0f;
    planeNrm.z = 0.0f;
    return RPPnpGame::GetInstance()->GetCamera()->GetScreenToPlane(
        &target, planePos, planeNrm, pScreen, x * halfWidth, y * halfHeight);
}
#pragma opt_propagation reset
#pragma pop

void RPPnpPlayer::UpdateHuman() {
    if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_RESULT) {
        if (!unkCE) {
            mHidden = false;
        }
        return;
    }

    f32 tableHeight = RPPnpGame::GetInstance()->GetTable()->GetHeight();
    const EGG::CoreStatus* pStatus =
        EGG::CoreControllerMgr::instance()->getNthController(mSide)->getCoreStatus();

    if (pStatus->getDPDValidFlag() > 0) {
        nw4r::math::VEC3 target;
        nw4r::math::VEC3 planePos;
        nw4r::math::VEC3 planeNrm;
        unkCE = PnpPlayerProjectPointer(pStatus, tableHeight, target, planePos, planeNrm);
    
        target.z *= sHumanFollowRate;
        f32 base;
        if (mSide == 0) {
            base = sHumanMaxX;
        } else {
            base = sHumanMinX;
        }
        nw4r::math::VEC3 camTarget = RPPnpGame::GetInstance()->GetCamera()->GetTarget();
        base -= ((sHumanMaxX + sHumanMinX) * (camTarget.z + RPPnpGame::sCameraOffsetFar)) /
                (RPPnpGame::sCameraOffsetFar - RPPnpGame::sCameraOffsetNear);
        target.z += base;
        RecordSwingSample(target, unkCE, true, true);
    
        bool hold = false;
        bool serving = false;
        RPPnpBall* pBall = RPPnpGame::GetInstance()->GetBall();
        if (pBall->GetState() == 2) {
            PnpPlayerSide targetSide = (PnpPlayerSide)pBall->mTargetSide;
            if (mSide == targetSide) {
                serving = true;
            }
        }
        if (serving && !EGG::CoreControllerMgr::instance()->getNthController(mSide)->down(0xC00)) {
            hold = true;
        }
    
        if (!hold) {
            nw4r::math::VEC3 move = target - mTarget;
            if (PSVECMag(move) < 0.0001f) {
                mTarget = target;
            } else {
                mTarget += move * sHumanMoveSpeed;
            }
        }
    
        if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
            if (!unk98 && mSide == 0) {
                f32 x = mTarget.x;
                f32 z = mTarget.z;
                if (RPPnpGame::GetInstance()->GetState() != RPPnpGame::STATE_INTRO && -15.0f < x &&
                    x < 15.0f && 130.0f < z && z < 190.0f) {
                    unk98 = true;
                } else {
                    mHidden = false;
                }
            }
        } else if (!unk98) {
            if (mSide == 0) {
                f32 x = mTarget.x;
                f32 z = mTarget.z;
                if (RPPnpGame::GetInstance()->GetState() != RPPnpGame::STATE_INTRO && -15.0f < x &&
                    x < 15.0f && 130.0f < z && z < 190.0f) {
                    unk98 = true;
                } else {
                    mHidden = false;
                }
            } else {
                f32 x = mTarget.x;
                f32 z = mTarget.z;
                if (RPPnpGame::GetInstance()->GetState() != RPPnpGame::STATE_INTRO && -15.0f < x &&
                    x < 15.0f && -190.0f < z && z < -130.0f) {
                    unk98 = true;
                } else {
                    mHidden = false;
                }
            }
        }
    
        if (unk98) {
            mPos = mTarget;
        }
    } else {
        bool hold = false;
        bool serving = false;
        RPPnpBall* pBall = RPPnpGame::GetInstance()->GetBall();
        if (pBall->GetState() == 2) {
            PnpPlayerSide targetSide = (PnpPlayerSide)pBall->mTargetSide;
            if (mSide == targetSide) {
                serving = true;
            }
        }
        if (serving && !EGG::CoreControllerMgr::instance()->getNthController(mSide)->down(0xC00)) {
            hold = true;
        }
        if (hold) {
            mHidden = true;
            unkCE = true;
        } else {
            mHidden = false;
            unkCE = false;
        }

    }
}

void RPPnpPlayer::UpdateCpu() {
    if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_POINT) {
        return;
    }

    f32 tableHeight = RPPnpGame::GetInstance()->GetTable()->GetHeight();
    unkCE = true;

    nw4r::math::VEC3 target = RPPnpGame::GetInstance()->GetBall()->GetPos();
    if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_SERVE && mIsCpu == TRUE) {
        target.x = mCpuServeX;
    }

    f32 rate = 1.0f;
    if (!RPPnpGame::GetInstance()->IsMultiPlayer() &&
        RPPnpGame::GetInstance()->GetState() != RPPnpGame::STATE_RALLY) {
        rate = 0.2f;
    }

    nw4r::math::VEC3 move;
    nw4r::math::VEC3 landing;
    move.x = mPos.x - target.x;
    if ((f32)std::fabs(move.x) < 0.005f) {
        mPos.x = target.x;
    } else {
        move.x *= rate;
        mPos.x -= move.x;
    }

    mPos.y = 50.0f + tableHeight;
    f32 follow = 0.2f;

    switch (mSide) {
    case 0: {
        RPPnpBall* pBall = RPPnpGame::GetInstance()->GetBall();
        bool lands = pBall->mWillLandOnTable;
        if (lands) {
            landing = pBall->mLandingPos;
        } else {
            landing = nw4r::math::VEC3(0.0f, 0.0f, 0.0f);
        }
        if (lands && landing.z > 0.0f) {
            move.z = mPos.z - (landing.z + sCpuLandingOffset);
            move.z *= follow;
            mPos.z -= move.z;
        } else {
            move.z = (mPos.z - 160.0f) * follow;
            mPos.z -= move.z;
        }
        break;
    }
    case 1: {
        RPPnpBall* pBall = RPPnpGame::GetInstance()->GetBall();
        bool lands = pBall->mWillLandOnTable;
        if (lands) {
            landing = pBall->mLandingPos;
        } else {
            landing = nw4r::math::VEC3(0.0f, 0.0f, 0.0f);
        }
        if (lands && landing.z < 0.0f) {
            move.z = mPos.z - (landing.z - sCpuLandingOffset);
            move.z *= follow;
            mPos.z -= move.z;
        } else {
            move.z = (mPos.z - -160.0f) * follow;
            mPos.z -= move.z;
        }
        break;
    }
    }

    mTarget = mPos;
}

#pragma push
#pragma always_inline on
static inline u32 PnpPlayerPickTargetIndex(f32 ballX, f32 left, f32 right,
                                          const s32* pCpuLevel) {
    u32 nearWeight, near, far, center;
    if (ballX < left) {
        near = 0;
        center = 1;
        far = 2;
    } else if (ballX > right) {
        near = 2;
        center = 1;
        far = 0;
    } else if (ballX < 0.0f) {
        near = 1;
        center = 0;
        far = 2;
    } else {
        near = 1;
        center = 2;
        far = 0;
    }


    const RPPnpLevelTable::LevelEntry* pEntry =
        RPPnpLevelTable::GetInstance()->GetLevelEntry(*pCpuLevel);
    nearWeight = pEntry->unk0;
    u32 centerWeight = nearWeight + pEntry->unk4;
    u32 total = pEntry->unk8 + centerWeight;
    u32 pick = (f32)total * RPUtlRandom::getF32();
    if (pick < nearWeight) {
        return near;
    }
    if (pick < centerWeight) {
        far = center;
    }
    return far;
}
#pragma pop

#pragma push
#pragma auto_inline off
f32 RPPnpPlayer::PickCpuTargetX(f32 ballX) {
    f32 width = RPPnpGame::GetInstance()->GetTable()->GetWidth();
    f32 left = -width / 3.0f;
    f32 right = width / 3.0f;

    u32 near = PnpPlayerPickTargetIndex(ballX, left, right, &mCpuLevel);

    f32 x = left;
    switch (near) {
    case 0:
        x = left;
        break;
    case 1:
        x = 0.0f;
        break;
    case 2:
        x = right;
        break;
    }
    return x;
}
#pragma pop

nw4r::math::VEC3 RPPnpPlayer::CalcCpuReturnVelocity() {
    nw4r::math::VEC3 ballPos = RPPnpGame::GetInstance()->GetBall()->GetPos();

    nw4r::math::VEC3 dir;
    switch (mSide) {
    case 0:
        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = -1.0f;
        break;
    case 1:
        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = 1.0f;
        break;
    }

    nw4r::math::VEC3 opponentPos;
    switch (mSide) {
    case 0:
        opponentPos = RPPnpGame::GetInstance()->GetPlayer(1)->GetPos();
        break;
    case 1:
        opponentPos = RPPnpGame::GetInstance()->GetPlayer(0)->GetPos();
        break;
    }

    f32 targetZ = dir.z * RPPnpGame::GetInstance()->GetTable()->GetLength() / 2.0f;
    f32 targetX = PickCpuTargetX(opponentPos.x);

    nw4r::math::VEC3 vel;
    vel.x = 0.0342f * (targetX - ballPos.x);
    vel.y = 5.6595f * sCpuReturnSpeed;
    vel.z = 0.0342f * (targetZ - ballPos.z);
    return vel;
}

// Instruction-identical apart from string-pool symbol names.
bool RPPnpPlayer::CheckServeHit() {
    RPPnpBall* pBall = RPPnpGame::GetInstance()->GetBall();
    nw4r::math::VEC3 prevPos = pBall->GetPrevPos();
    nw4r::math::VEC3 pos = pBall->GetPos();
    nw4r::math::VEC3 vel = pBall->GetVelocity();

    f32 offset;
    nw4r::math::VEC3 dir;
    switch (mSide) {
    case 0:
        offset = -sServeHitHeight;
        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = -1.0f;
        break;
    case 1:
        offset = sServeHitHeight;
        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = 1.0f;
        break;
    }

    if (vel.y >= 0.0f) {
        return false;
    }

    if (mSide != pBall->mTargetSide) {
        return false;
    }

    nw4r::math::VEC3 diff(pos.x - mPos.x, pos.y - mPos.y, pos.z - mPos.z);
    if (nw4r::math::VEC3LenSq(&diff) <= unk84 * unk84 && unkFC != 1) {
        unkFC = 1;
    }

    for (int i = 0; i < 10; i++) {
        int sampleIndex = (mSampleCount + i - 1) % 10;
        SwingSample sample = mSamples[sampleIndex];
        sample.mPos = mTarget;

        if (sample.mValid) {
            nw4r::math::VEC3 before(0.0f, prevPos.y - sample.mPos.y, 0.0f);
            nw4r::math::VEC3 after(0.0f, pos.y - sample.mPos.y, 0.0f);
            if (nw4r::math::VEC3Dot(&before, &after) < 0.0f &&
                (f32)std::fabs((pos.x + prevPos.x) / 2.0f - sample.mPos.x) < sServeBox.x / 2.0f &&
                offset + (f32)std::fabs((pos.z + prevPos.z) / 2.0f - sample.mPos.z) < sServeBox.z) {
                nw4r::math::VEC3 vel;
                nw4r::math::VEC3 grav;
                nw4r::math::VEC3 swing = GetSwing();
                f32 center = 0.0f;
                nw4r::math::VEC3 ret;
                ret.x = 0.03f * (center - sample.mPos.x);
                ret.y = -2.0f;
                ret.z = 0.0684f * (center - pos.z);
                nw4r::math::VEC3 gravity;
                gravity.x = 0.0f;
                gravity.y = -1.323f;
                gravity.z = 0.0f;
                vel.x = swing.x * sSpinFactors.x + ret.x;
                vel.y = ret.y;
                vel.z = ret.z;
                pBall->SetVelocity(vel);
                grav.x = gravity.x;
                grav.y = gravity.y;
                grav.z = gravity.z;
                pBall->SetGravity(grav);

                if (mIsCpu != TRUE) {
                    EGG::CoreControllerMgr::instance()->getNthController(mSide)->startSimpleRumble(
                        0xFFFFFFFF, sHitWindowFrames, 32);
                }

                unkFC = 1;
                RPPnpGame::GetInstance()->GetBall()->OnEvent(7, mSide);

                if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_RALLY) {
                    nw4r::math::VEC3 effectPos;
                    effectPos.x = pos.x;
                    effectPos.y = pos.y;
                    effectPos.z = pos.z;
                    nw4r::math::VEC3 effectScale(1.0f, 1.0f, 1.0f);
                    RPSysEffect effect("Pnp_shot", 0, 0);
                    effect.create();
                    effect.setPos(effectPos);
                    effect.setScale(effectScale);
                    effect.update();
                }

                return true;
            }
        }
    }

    return false;
}

RPPnpPlayer::RPPnpPlayer(s32 side, BOOL isCpu) {
    mSide = side;
    mDebugFlags = 0;
    mHidden = false;
    unkCE = false;
    mIsCpu = isCpu;
    mCpuLevel = 0;
    unkFC = 0;
    unk100 = 0.0f;
}

void RPPnpPlayer::Create() {
    RPSysKokeshiGenInfo info;
    info.SetGenType(RPSysKokeshiManager::GenType_Player);
    info.SetPlayerNo(mSide);
    info.SetHandType(0);
    mpHand = new RPCls8034F6B0(&info);

    mpTrail = new RPPnpRacketTrail();
    if (mSide == 0) {
        mpTrail->SetRemoteFlag(1);
    } else if (mSide == 1 && RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE) {
        mpTrail->SetRemoteFlag(2);
    }

    unk84 = 64.0f;
    mPos = sVecZero;
    mTarget = sVecZero;
    mSampleCount = 0;
    for (int i = 0; i < 10; i++) {
        mSamples[i].mValid = false;
    }
    mHitRadius = sRacketScale;
    mHitBoxes[0] = sHumanBox;
    mHitBoxes[1] = sCpuBox;

    sSpinFactors.x = !RPPnpGame::GetInstance()->IsMultiPlayer() ? 0.09f : 0.4f;
}

#pragma push
#pragma opt_propagation off
// opt_propagation off keeps the racket model in r28 (shared with the hand animation names)
void RPPnpPlayer::Load() {
    RPGrpHandle file =
        RPSimpleModel::LoadResource(mSide == 0 ? "/G3D/pin_racket0.brres" : "/G3D/pin_racket1.brres");
    RPSimpleModel::LoadModel(mSide == 0 ? "pin_racket0" : "pin_racket1", file, 1, 0, 3);

    RPGrpModel* pModel = mpModel;
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    pModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_TexPat, 1);
    RPGrpHandle anm = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResAnmTexPat,
        pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmTexPat, file,
                        mSide == 0 ? "pin_racket0" : "pin_racket1"));
    pModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_TexPat, 0, anm);
    pModel->SetDrawScene(0);
    pModel->CreateBoundingInfo(1);

    mpHand->LoadResource();
    mpHand->ApplyLightTexture(0);

    RPGrpModel* pHand = mpHand->GetHandModel();
    pHand->SetDrawScene(0);
    pHand->SetBaseScale(1.5f);

    const char* anmNames[] = {"pin_hand_R_fore_wait", "pin_hand_R_fore", "pin_hand_R_toss"};
    if (mSide == 0) {
        RPGrpHandle anmFile = RPSimpleModel::LoadResource("/G3D/pin_hand_R_anim.brres");
        for (u16 i = 0; i < 3; i++) {
            sHandAnmHandles[i] = pResMgr->CreateData(
                RPGrpModelResManager::Type_ResAnmChr,
                pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmChr, anmFile, anmNames[i]));
        }
    }

    pHand->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 3);
    for (u16 i = 0; i < 3; i++) {
        pHand->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, i, sHandAnmHandles[i]);
    }
    pHand->CreateBoundingInfo(1);

    mpRacketEffect =
        new RPSysEffect(mSide == 0 ? "Pnp_racket_tail_b" : "Pnp_racket_tail_r", 0, 0);
}
#pragma pop

void RPPnpPlayer::Reset() {
    sSwingHintShown = false;
    mSampleCount = 0;
    for (int i = 0; i < 10; i++) {
        mSamples[i].mValid = false;
    }
    unk94 = 0;
    unk98 = false;
    unkCD = false;
    mHidden = false;
    unkCF = false;

    if (!RPPnpGame::GetInstance()->IsMultiPlayer() && mSide == 1) {
        unk98 = true;
        unkCD = true;
        mHidden = true;
    }

    mCpuServeX = 0.0f;
    mPos.x = 0.0f;
    mPos.y = sPlayerHeight + RPPnpGame::GetInstance()->GetTable()->GetHeight();
    if (mSide == 0) {
        mPos.z = 160.0f;
    } else {
        mPos.z = -160.0f;
    }

    if (mSide == 0) {
        mRacketMtx._00 = -0.917345f;
        mRacketMtx._01 = 0.395881f;
        mRacketMtx._02 = 0.03887502f;
        mRacketMtx._10 = -0.383416f;
        mRacketMtx._11 = -0.853946f;
        mRacketMtx._12 = -0.351459f;
        mRacketMtx._20 = -0.105952f;
        mRacketMtx._21 = -0.337356f;
        mRacketMtx._22 = 0.935265f;
    } else {
        mRacketMtx._00 = 0.917345f;
        mRacketMtx._01 = -0.395881f;
        mRacketMtx._02 = -0.03887502f;
        mRacketMtx._10 = -0.383416f;
        mRacketMtx._11 = -0.853946f;
        mRacketMtx._12 = -0.351459f;
        mRacketMtx._20 = 0.105952f;
        mRacketMtx._21 = 0.337356f;
        mRacketMtx._22 = -0.935265f;
    }

    mpRacketEffect->create();
    unk80 = false;
}

void RPPnpPlayer::Calculate() {
    if (mHidden) {
        unk98 = true;
    }

    if (unk98 && mSide == 0 &&
        RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_SERVE) {
        if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
            if (!sSwingHintShown) {
                RPPnpHud::GetInstance()->GetStartMsgLyt()->PlayOut();
                RPPnpHud::GetInstance()->GetCountLyt()->PlayIn();
                RPPnpGame::GetInstance()->SetUnk18(true);
                RPPnpHud::GetInstance()->GetArrowLyt()->Show();
                sSwingHintShown = true;
            }
        } else if (RPPnpGame::GetInstance()->GetPlayer(1)->unk98 && !sSwingHintShown) {
            RPPnpHud::GetInstance()->GetStartMsgLyt()->PlayOut();
            RPPnpHud::GetInstance()->GetScoreLyt()->PlayIn();
            RPPnpHud::GetInstance()->GetArrowLyt()->Show();
            sSwingHintShown = true;
        }
    }

    if (unkCE) {
        RPPnpGame* pGame = RPPnpGame::GetInstance();
        f32 margin = 50.0f;
        if ((f32)std::fabs(mTarget.x) < pGame->GetTable()->GetWidth() / 2.0f + margin &&
            (f32)std::fabs(mTarget.z) < pGame->GetTable()->GetLength() / 2.0f + margin) {
            pGame->GetShadowTexture()->AddModel(mpHand->GetHandModel(), 1.0f);
        }
    }
    RPPnpGame::GetInstance()->GetShadowTexture()->AddModel(mpModel, 1.0f);

    RPGrpModelAnm* pAnm = mpHand->GetHandModel()->GetModelAnm();
    s16 playIdx = pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr);
    switch (unkFC) {
    case 0:
        if (playIdx != 0) {
            pAnm->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
        } else {
            u16 idx = playIdx;
            f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, idx);
            if (!(frame < pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, idx) - 1.0f)) {
                pAnm->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
            }
        }
        break;
    case 1:
        if (playIdx != 1) {
            pAnm->Start(RPGrpModelAnm::Anm_Chr, 1, 0.0f);
        } else {
            u16 idx = playIdx;
            f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, idx);
            if (!(frame < pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, idx) - 1.0f)) {
                pAnm->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
                unkFC = 0;
            }
        }
        break;
    case 2:
        if (playIdx != 2) {
            pAnm->Start(RPGrpModelAnm::Anm_Chr, 2, 0.0f);
        } else {
            u16 idx = playIdx;
            f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, idx);
            if (!(frame < pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, idx) - 1.0f)) {
                pAnm->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
                unkFC = 0;
            }
        }
        break;
    }

    RPGrpModel* pHand = mpHand->GetHandModel();
    EGG::Vector3f scale(1.0f, 1.0f, 1.0f);
    EGG::Vector3f rot(0.0f, mSide == 0 ? 3.1415927f : 0.0f, 0.0f);
    EGG::Vector3f trans(mTarget.x, mTarget.y, mTarget.z);
    EGG::Matrix34f handMtx;
    handMtx.makeSRT(scale, rot, trans);
    if (unkCE) {
        pHand->GetModelEx()->setLocalMtx(&handMtx);
        pHand->SetBaseScale(30.0f);
        pHand->UpdateFrame();
        pHand->Calc();
        pHand->Entry();
    }

    if (mHidden) {
        nw4r::math::MTX34 racketMtx;
        mpHand->GetItemJointMtx(&racketMtx);
        mpModel->GetModelEx()->setLocalMtx(&racketMtx);
        mpModel->SetBaseScale(mHitRadius);
        mpModel->UpdateFrame();
        mpModel->Calc();
        mpModel->Entry();
    } else {
        nw4r::math::MTX34 racketMtx;
        if (unk98) {
            racketMtx = *mpModel->GetScnObj()->GetMtxPtr(nw4r::g3d::ScnObj::MTX_LOCAL);
            racketMtx._03 = mPos.x;
            racketMtx._13 = mPos.y;
            racketMtx._23 = mPos.z;
        } else {
            mRacketMtx._03 = mPos.x;
            mRacketMtx._13 = mPos.y;
            mRacketMtx._23 = mPos.z;
            racketMtx = mRacketMtx;
        }
        mpModel->GetModelEx()->setLocalMtx(&racketMtx);
        mpModel->SetBaseScale(mHitRadius);
        mpModel->UpdateFrame();
        mpModel->Calc();
        mpModel->Entry();
    }

    mpTrail->set3DPosition(mPos);

    nw4r::math::MTX34 modelMtx = *mpModel->GetScnObj()->GetMtxPtr(nw4r::g3d::ScnObj::MTX_LOCAL);
    mEffectMtx._00 = modelMtx._00;
    mEffectMtx._10 = modelMtx._10;
    mEffectMtx._20 = modelMtx._20;
    mEffectMtx._01 = modelMtx._01;
    mEffectMtx._11 = modelMtx._11;
    mEffectMtx._21 = modelMtx._21;
    mEffectMtx._02 = modelMtx._02;
    mEffectMtx._12 = modelMtx._12;
    mEffectMtx._22 = modelMtx._22;
    mEffectMtx._03 = modelMtx._03;
    mEffectMtx._13 = modelMtx._13;
    mEffectMtx._23 = modelMtx._23;
    mpRacketEffect->setMtx(mEffectMtx);
    mpRacketEffect->update();

    if (unk80) {
        nw4r::math::VEC3 effectScale(1.0f, 1.0f, 1.0f);
        RPSysEffect effect("Pnp_smashshot", 0, 0);
        effect.create();
        effect.setMtx(mEffectMtx);
        effect.setScale(effectScale);
        effect.update();
    }
    unk80 = false;

    if (!mHidden && mHidden != unkCD) {
        mpTrail->startSound(0x29U);
    } else if (mHidden && mHidden != unkCD) {
        mpTrail->startSound(mSide == 0 ? 0x27U : 0x28U);
    }
    unkCD = mHidden;
}

void RPPnpPlayer::Exit() {}

void RPPnpPlayer::UpdateInput() {
    mHidden = unk98;

    switch (mIsCpu) {
    case FALSE:
        UpdateHuman();
        break;
    case TRUE:
        UpdateCpu();
        break;
    }

    switch (mSide) {
    case 0:
        if (mPos.z <= 20.0f) {
            mPos.z = 20.0f;
            mHidden = false;
        }
        break;
    case 1:
        if (mPos.z >= -20.0f) {
            mPos.z = -20.0f;
            mHidden = false;
        }
        break;
    }

    mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexPat, 0, (s32)!unkCE);
    mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_TexPat, 0, 0.0f);

    if (mIsCpu == TRUE) {
        RecordSwingSample(mPos, unkCE, true, true);
    }

    if (unk94 < 0) {
        unk94 = 0;
    }

    if (!mHidden) {
        return;
    }

    s32 server = RPPnpGame::GetInstance()->GetPointPhase()->GetServerSide();
    if (mSide == server) {
        if (mIsCpu == TRUE) {
            if (RPPnpGame::GetInstance()->GetBall()->GetState() == 1 && mPos.x == mCpuServeX) {
                if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
                    if (EGG::CoreControllerMgr::instance()->getNthController(0)->downTrigger(
                            0xC00) &&
                        RPPnpGame::GetInstance()->GetPlayer(0)->mHidden) {
                        f32 offset = sTossOffset;
                        nw4r::math::VEC3 pos = mPos;
                        f32 z = pos.z;
                        unkFC = 2;
                        pos.z = z + offset;
                        RPPnpGame::GetInstance()->GetBall()->TossServe(nw4r::math::VEC3(pos));
                    }
                } else {
                    unkFC = 2;
                    RPPnpGame::GetInstance()->GetBall()->TossServe(nw4r::math::VEC3(mPos));
                }
            }
        } else if (EGG::CoreControllerMgr::instance()->getNthController(mSide)->downTrigger(0xC00) &&
                   RPPnpGame::GetInstance()->GetPlayer(0)->unk98 &&
                   RPPnpGame::GetInstance()->GetPlayer(1)->unk98 &&
                   (f32)std::fabs(mPos.z) >=
                       RPPnpGame::GetInstance()->GetTable()->GetLength() / 3.0f) {
            nw4r::math::VEC3 pos = mPos;
            switch (mSide) {
            case 0:
                pos.z -= sTossOffset;
                break;
            case 1:
                pos.z += sTossOffset;
                break;
            }

            if (RPPnpGame::GetInstance()->GetBall()->GetState() == 1) {
                mpTrail->startSound(mSide == 0 ? 0x2BU : 0x2CU);
                unkFC = 2;
            }

            RPPnpGame::GetInstance()->GetBall()->TossServe(nw4r::math::VEC3(pos));
        }
    }
}

// Instruction-identical apart from string-pool symbol names.
bool RPPnpPlayer::CheckBallHit() {
    if (!RPPnpGame::GetInstance()->IsMultiPlayer() &&
        RPPnpGame::sRallyLimit == RPPnpGame::GetInstance()->GetBall()->GetHitCount() / 2) {
        return false;
    }
    if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_RESULT) {
        return false;
    }
    if (!mHidden) {
        return false;
    }
    if (!unkCE) {
        return false;
    }

    RPPnpBall* pBall = RPPnpGame::GetInstance()->GetBall();
    PnpPlayerBallState ballState = (PnpPlayerBallState)pBall->GetState();
    if (ballState == PnpPlayerBallServe) {
        return CheckServeHit();
    }

    nw4r::math::VEC3 prevPos = pBall->GetPrevPos();
    nw4r::math::VEC3 pos = pBall->GetPos();
    nw4r::math::VEC3 vel = pBall->GetVelocity();

    nw4r::math::VEC3 dir;
    switch (mSide) {
    case 0:
        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = -1.0f;
        break;
    case 1:
        dir.x = 0.0f;
        dir.y = 0.0f;
        dir.z = 1.0f;
        break;
    }

    if (vel.z * dir.z >= 0.0f) {
        return false;
    }

    if (ballState == PnpPlayerBallHit && mSide == pBall->mHitterSide) {
        return false;
    }

    nw4r::math::VEC3 diff(pos.x - mPos.x, pos.y - mPos.y, pos.z - mPos.z);
    if (nw4r::math::VEC3LenSq(&diff) <= unk84 * unk84 && unkFC != 1) {
        unkFC = 1;
    }

    if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_POINT) {
        return false;
    }

    for (int i = 0; i < 10; i++) {
        SwingSample sample = mSamples[(mSampleCount + i - 1) % 10];

        if ((i == 0 || RPPnpGame::GetInstance()->GetBall()->GetState() == 6) && sample.mValid) {
            nw4r::math::VEC3 before(0.0f, 0.0f, prevPos.z - sample.mPos.z);
            nw4r::math::VEC3 after(0.0f, 0.0f, pos.z - sample.mPos.z);
            const nw4r::math::VEC3& rBox = mIsCpu == FALSE ? mHitBoxes[0] : mHitBoxes[1];
            nw4r::math::VEC3 box = rBox;

            f32 dx;
            if (nw4r::math::VEC3Dot(&before, &after) < 0.0f &&
                (dx = (f32)std::fabs((pos.x + prevPos.x) / 2.0f - sample.mPos.x), dx < box.x / 2.0f) &&
                (f32)std::fabs((pos.y + prevPos.y) / 2.0f - sample.mPos.y) < box.y / 2.0f) {
                if (RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE) {
                    unkCF = !RPPnpGame::GetInstance()->GetBall()->mVisible;
                    bool center = dx < sSmashWindow * box.x / 2.0f;
                    if (!center) {
                        unkCF = false;
                    }
                    RPPnpGame::GetInstance()->GetBall()->mVisible = center;
                    RPPnpGame::GetInstance()->GetBall()->mHidden = unkCF;
                }

                nw4r::math::VEC3 newVel;
                nw4r::math::VEC3 gravity;
                nw4r::math::VEC3 swing = GetSwing();
                nw4r::math::VEC3 ret;
                nw4r::math::VEC3 baseGravity;

                f32 targetZ = dir.z * RPPnpGame::GetInstance()->GetTable()->GetLength() / 2.0f;
                f32 targetX = pos.x / 8.0f;
                if (mIsCpu == TRUE) {
                    ret = CalcCpuReturnVelocity();
                } else {
                    ret.x = 0.0342f * (targetX - pos.x);
                    ret.y = 6.174f * sCpuReturnSpeed;
                    ret.z = 0.0342f * (targetZ - pos.z);
                    if ((f32)std::fabs(swing.x) < sSlowSwingSpeed) {
                        f32 halfWidth = RPPnpGame::GetInstance()->GetTable()->GetWidth() / 2.0f;
                        ret.x += 0.0342f *
                                 -(halfWidth / 2.0f - halfWidth * RPUtlRandom::getF32());
                    }
                }

                newVel.x = swing.x * sSpinFactors.x + ret.x;
                newVel.y = ret.y;
                if (mIsCpu == TRUE) {
                    newVel.z = ret.z;
                } else if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
                    newVel.z = ret.z;
                } else {
                    newVel.z = swing.z * sSpinFactors.z + ret.z;
                }

                if (unkCF) {
                    newVel.x *= sSmashSpeedScale;
                    newVel.z *= sSmashSpeedScale;
                }
                pBall->SetVelocity(newVel);

                baseGravity.x = 0.0f;
                baseGravity.y = -0.588f * sCpuReturnSpeed;
                baseGravity.z = 0.0f;
                gravity.x = 0.0f;
                gravity.y = sReturnSpinScale * baseGravity.y *
                                (f32)std::fabs((swing.z * sSpinFactors.z) / ret.z) +
                            baseGravity.y;
                gravity.z = 0.0f;
                if (unkCF) {
                    gravity.y *= sSmashGravityScale;
                }
                pBall->SetGravity(gravity);

                if (mIsCpu != TRUE) {
                    EGG::CoreControllerMgr::instance()->getNthController(mSide)->startSimpleRumble(
                        0xFFFFFFFF, sHitWindowFrames, 32);
                }

                unkFC = 1;
                RPPnpGame::GetInstance()->GetBall()->OnEvent(7, mSide);

                if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_RALLY) {
                    if (unkCF) {
                        unk80 = true;
                    } else {
                        nw4r::math::VEC3 effectPos;
                        effectPos.x = pos.x;
                        if (!RPPnpGame::GetInstance()->IsMultiPlayer() && mSide == 1) {
                            effectPos.y = (pos.y + mPos.y) / 2.0f;
                        } else {
                            effectPos.y = pos.y;
                        }
                        effectPos.z = pos.z;
                        nw4r::math::VEC3 effectScale(1.0f, 1.0f, 1.0f);
                        RPSysEffect effect("Pnp_shot", 0, 0);
                        effect.create();
                        effect.setPos(effectPos);
                        effect.setScale(effectScale);
                        effect.update();
                    }
                }

                unk94 = 10;
                return true;
            }
        }
    }

    return false;
}

void RPPnpPlayer::PickCpuServeX() {
    if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
        mCpuServeX = 0.0f;
    } else if (mIsCpu == TRUE) {
        f32 width = RPPnpGame::GetInstance()->GetTable()->GetWidth();
        mCpuServeX = width * RPUtlRandom::getF32() - width / 2.0f;
    }
}

void RPPnpPlayer::DrawDebug() {
    if (RPGrpRenderer::GetCurrentViewNo() == 0 && RPGrpRenderer::GetDrawStep() == 3) {
        if ((mDebugFlags & 1) && mHidden && unkCE) {
            EGG::Vector3f scale;
            EGG::Vector3f trans;
            if (RPPnpGame::GetInstance()->GetBall()->GetState() == 2) {
                scale.set(sServeBox.x, sServeBox.y, sServeBox.z);
                trans.set(mPos.x, mPos.y, mPos.z - sServeHitHeight);
            } else {
                nw4r::math::VEC3 box = mIsCpu == FALSE ? mHitBoxes[0] : mHitBoxes[1];
                scale.set(box.x, box.y, box.z);
                trans.set(mPos.x, mPos.y, mPos.z);
            }

            EGG::Matrix34f mtx;
            mtx.makeST(scale, trans);
            EGG::DrawGX::BeginDrawCube();
            GXColor color = mSide == 0 ? EGG::DrawGX::RED : EGG::DrawGX::BLUE;
            EGG::DrawGX::DrawDLWorld(EGG::DrawGX::DL_CUBE, mtx, color);
        }

        if (mDebugFlags & 2) {
            EGG::Vector3f scale(unk84, unk84, unk84);
            EGG::Vector3f trans(mPos.x, mPos.y, mPos.z);
            EGG::Matrix34f mtx;
            mtx.makeST(scale, trans);
            GXColor color = {0xFF, 0xFF, 0xFF, 0x40};
            EGG::DrawGX::BeginDrawCube();
            EGG::DrawGX::DrawDLWorld(EGG::DrawGX::DL_SPHERE_8X16, mtx, color);
        }
    }

    if (RPGrpRenderer::GetCurrentViewNo() == 1 && RPGrpRenderer::GetDrawStep() == 3 &&
        (mDebugFlags & 4)) {
        RPSysTextWriter* pWriter = RPSysTextWriter::instance();
        pWriter->Begin();
        pWriter->SetTextColor(nw4r::ut::Color(0xFFFF00FF));
        if (unkCF) {
            pWriter->Printf(0.0f, 0.0f, "SMASH");
        }
        if (mSide == 0) {
            nw4r::math::VEC3 swing = GetSwing();
            pWriter->Printf(50.0f, 50.0f, "V %f", PSVECMag(swing));
        }
        pWriter->End();
    }
}
