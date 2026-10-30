// Table Tennis crowd: 17 seats of two spectators built from the Mii databases, turned towards
// the ball, cheering on points. Unit 0x8022E67C-0x8022FA20 (dtk/game_units.tsv). Functions in
// reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpAudience.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel/RPSysKokeshiManager.h>
#include <Pack/RPKernel/RPSysMiddleDB.h>
#include <Pack/RPKernel/RPSysMiddleDBGenInfo.h>
#include <Pack/RPParty/RPPnpScene/RPPnpBall.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPUtility/RPUtlRandom.h>

#include <algorithm>
#include <cmath>

// RPSysKokeshi.cpp (Wii Play, stubbed for release)
bool Mii_isSameKokeshiLocation(const RPSysKokeshiLocation* pLhs, const RPSysKokeshiLocation* pRhs);

// Keep the zero-vector temporary inside the inlined assignment.
inline void ClearAudienceTarget(nw4r::math::VEC3& target) {
    target = nw4r::math::VEC3(0.0f, 0.0f, 0.0f);
}

s32 RPPnpAudience::sMaxOfficialMii = 6;
RPPnpAudience* RPPnpAudience::sInstance;

void RPPnpAudience::CreateInstance() {
    sInstance = new RPPnpAudience();
}

void RPPnpAudience::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPPnpAudience::RPPnpAudience() {
    for (int i = 0; i < SEAT_NUM; i++) {
        for (int j = 0; j < 2; j++) {
            mSeats[i].mpSpectators[j] = NULL;
        }
    }

    s32 validNum = RPSysKokeshiManager::GetOfficialDBValidNum();
    u16 officialNum = std::min(sMaxOfficialMii, validNum);

    if (validNum != 0) {
        RPSysMiddleDBGenInfo info;
        info.SetType(static_cast<RFLMiddleDBType>(3));
        info.SetSize(officialNum);
        mpOfficialDB = new RPSysMiddleDB(&info);
    }

    u16 defaultNum;
    {
        RPSysMiddleDBGenInfo info;
        info.SetType(static_cast<RFLMiddleDBType>(0));
        info.SetSize(7);
        mpDefaultDB = new RPSysMiddleDB(&info);
        defaultNum = mpDefaultDB->GetStoredNum();
    }

    {
        RPSysMiddleDBGenInfo info;
        info.SetType(static_cast<RFLMiddleDBType>(3));
        info.SetSize(SEAT_NUM);
        mpExtraDB = new RPSysMiddleDB(&info);
    }

    RPSysKokeshiLocation locations[3];
    locations[0].SetDataSource(static_cast<RFLDataSource>(0));
    locations[0].SetMiddleDB(mpOfficialDB);
    locations[1].SetDataSource(static_cast<RFLDataSource>(7));
    locations[1].SetMiddleDB(mpDefaultDB);
    locations[2].SetDataSource(static_cast<RFLDataSource>(7));
    locations[2].SetMiddleDB(mpExtraDB);

    for (int i = 0; i < SEAT_NUM; i++) {
    retry:
        mGenInfos[i].SetBodyType(0);
        mGenInfos[i].SetGenType(RPSysKokeshiManager::GenType_Friend);
        mGenInfos[i].SetExpressionFlag(0xB);

        int source;
        if (officialNum != 0) {
            source = 0;
            officialNum--;
        } else if (defaultNum != 0) {
            source = 1;
            defaultNum--;
        } else {
            source = 2;
        }
        mGenInfos[i].SetLocation(locations[source]);

        if (mSeats[i].mpSpectators[0] != NULL) {
            delete mSeats[i].mpSpectators[0];
        }

        for (int j = 0; j < 2; j++) {
            if (j == 0) {
                RPPnpSpectator* pSpectator = new RPPnpSpectator(&mGenInfos[i], i, j);
                mSeats[i].mpSpectators[j] = pSpectator;
                mGenInfos[i] = pSpectator->GetGenInfo();
                mGenInfos[i].SetGenType(RPSysKokeshiManager::GenType_Direct);

                for (int k = 0; k < i; k++) {
                    if (Mii_isSameKokeshiLocation(&mGenInfos[i].GetLocation(),
                                                  &mGenInfos[k].GetLocation())) {
                        goto retry;
                    }
                }
            } else {
                mSeats[i].mpSpectators[j] = new RPPnpSpectator(&mGenInfos[i], i, j);
            }
        }
    }

    for (int i = SEAT_NUM - 1; i > 0; i--) {
        u32 j = RPUtlRandom::getU32(i + 1);

        RPPnpSpectator* pTemp = mSeats[j].mpSpectators[0];
        mSeats[j].mpSpectators[0] = mSeats[i].mpSpectators[0];
        mSeats[i].mpSpectators[0] = pTemp;

        pTemp = mSeats[j].mpSpectators[1];
        mSeats[j].mpSpectators[1] = mSeats[i].mpSpectators[1];
        mSeats[i].mpSpectators[1] = pTemp;
    }
}

RPPnpAudience::~RPPnpAudience() {
    for (int i = 0; i < SEAT_NUM; i++) {
        for (int j = 0; j < 2; j++) {
            mSeats[i].mpSpectators[j] = NULL;
        }
    }
}

void RPPnpAudience::Create() {}

void RPPnpAudience::Load() {
    const char* anmNames[2] = {"pin_A0_jumpR", "pin_A0_waitR"};
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/pin_avatar_anim.brres");
    RPGrpHandle anms[2];

    for (u32 i = 0; i < 2; i++) {
        RPGrpHandle anm = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResAnmChr,
            pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmChr, file, anmNames[(u16)i]));
        anms[(u16)i] = anm;
    }

    for (int i = 0; i < SEAT_NUM; i++) {
        for (int j = 0; j < 2; j++) {
            if (j == 0) {
                mSeats[i].mpSpectators[j]->LoadResource(NULL);
                mOverloadInfos[i].SetClothesType(
                    mSeats[i].mpSpectators[j]->GetOverloadInfo().GetClothesType());
            } else {
                mSeats[i].mpSpectators[j]->LoadResource(&mOverloadInfos[i]);
                mSeats[i].mpSpectators[j]->GetNigaoeModel()->SetDrawGroup(2);
                mSeats[i].mpSpectators[j]->GetNigaoeModel()->SetUnk3(0);
                mSeats[i].mpSpectators[j]->GetNigaoeModel()->SetUnk4(0);
                mSeats[i].mpSpectators[j]->GetNigaoeModel()->SetUnkCFlags(0x20);
                mSeats[i].mpSpectators[j]->GetBodyModel()->SetDrawGroup(2);
                mSeats[i].mpSpectators[j]->GetBodyModel()->SetUnk3(0);
                mSeats[i].mpSpectators[j]->GetBodyModel()->SetUnk4(0);
            }

            mSeats[i].mpSpectators[j]->ApplyLightTexture(0);

            RPGrpModel* pBody = mSeats[i].mpSpectators[j]->GetBodyModel();
            pBody->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 2);
            for (u32 k = 0; k < 2; k++) {
                pBody->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, k, anms[(u16)k]);
            }

            mSeats[i].mpSpectators[j]->GetBodyModel()->GetModelAnm()->Start(
                RPGrpModelAnm::Anm_Chr, 1, 0.0f);
        }
    }
}

void RPPnpAudience::Reset() {
    for (int i = 0; i < SEAT_NUM; i++) {
        for (int j = 0; j < 2; j++) {
            nw4r::math::VEC3 specPos = mSeats[i].mpSpectators[j]->GetPos();
            nw4r::math::VEC3 ballPos = RPPnpGame::GetInstance()->GetBall()->GetPos();
            nw4r::math::VEC3 dir = ballPos - specPos;
            f32 rotY = atan2f(dir.x, dir.z);
            mSeats[i].mpSpectators[j]->SetRotY(rotY);
            mSeats[i].mpSpectators[j]->SetTargetRotY(rotY);
            mSeats[i].mpSpectators[j]->Reset();

            RPGrpModelAnm* pAnm = mSeats[i].mpSpectators[j]->GetBodyModel()->GetModelAnm();
            pAnm->Start(RPGrpModelAnm::Anm_Chr, 1, 0.0f);
            pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, 1, 120.0f * RPUtlRandom::getF32());
        }

        mSeats[i].mAnim = 1;
        mSeats[i].mCheerTimer = 0;
    }
}

void RPPnpAudience::Calculate() {
    RPPnpGame::State state;
    for (int i = 0; i < SEAT_NUM; i++) {
        if (mSeats[i].mCheerTimer != 0) {
            if (mSeats[i].mCheerTimer++ >= 120) {
                mSeats[i].mAnim = 1;
                mSeats[i].mCheerTimer = 0;
            }
        }

        for (int j = 0; j < 2; j++) {
            RPGrpModelAnm* pAnm = mSeats[i].mpSpectators[j]->GetBodyModel()->GetModelAnm();
            int idx = pAnm->GetPlayIndex(RPGrpModelAnm::Anm_Chr);
            f32 frame = pAnm->GetFrame(RPGrpModelAnm::Anm_Chr, idx);
            if (frame < pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, idx) - 1.0f) {
                pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, idx, 1.0f + frame);
            } else {
                if (mSeats[i].mAnim == 1) {
                    state = static_cast<RPPnpGame::State>(
                        RPPnpGame::GetInstance()->GetState());
                    RPSysKokeshiManager::ChangeExpression(
                        mSeats[i].mpSpectators[j]->GetNigaoeModel(),
                        state == RPPnpGame::STATE_RESULT ? static_cast<RFLExpression>(3)
                                                        : static_cast<RFLExpression>(0));
                }
                pAnm->Start(RPGrpModelAnm::Anm_Chr, mSeats[i].mAnim, 0.0f);
            }
        }
    }

    for (int i = 0; i < SEAT_NUM; i++) {
        for (int j = 0; j < 2; j++) {
            RPPnpGame* pGame = RPPnpGame::GetInstance();
            if (pGame->GetState() == RPPnpGame::STATE_RESULT) {
                if (!pGame->IsMultiPlayer()) {
                    nw4r::math::VEC3 pos = mSeats[i].mpSpectators[j]->GetPos();
                    nw4r::math::VEC3 target(0.0f, 0.0f, 160.0f);
                    nw4r::math::VEC3 dir = target - pos;
                    mSeats[i].mpSpectators[j]->SetTargetRotY(atan2f(dir.x, dir.z));
                } else {
                    bool near = i >= 0 && i < 5;
                    if (near) {
                        nw4r::math::VEC3 pos = mSeats[i].mpSpectators[j]->GetPos();
                        nw4r::math::VEC3 target(0.0f, 0.0f, 160.0f);
                        nw4r::math::VEC3 dir = target - pos;
                        mSeats[i].mpSpectators[j]->SetTargetRotY(atan2f(dir.x, dir.z));
                    } else {
                        nw4r::math::VEC3 pos = mSeats[i].mpSpectators[j]->GetPos();
                        nw4r::math::VEC3 target(0.0f, 0.0f, -160.0f);
                        nw4r::math::VEC3 dir = target - pos;
                        mSeats[i].mpSpectators[j]->SetTargetRotY(atan2f(dir.x, dir.z));
                    }
                }
            } else {
                RPPnpBall* pBall = pGame->GetBall();
                nw4r::math::VEC3 pos = mSeats[i].mpSpectators[j]->GetPos();
                nw4r::math::VEC3 target;
                if (pBall->WillLandOnTable()) {
                    target = pBall->GetLandingPosRef();
                } else {
                    ClearAudienceTarget(target);
                }
                if (!pBall->WillLandOnTable()) {
                    target = pGame->GetBall()->GetPos();
                }

                nw4r::math::VEC3 dir = target - pos;
                mSeats[i].mpSpectators[j]->SetTargetRotY(atan2f(dir.x, dir.z));
            }

            mSeats[i].mpSpectators[j]->Calculate();
        }
    }
}

void RPPnpAudience::Exit() {}

void RPPnpAudience::Draw() {
    for (int i = 0; i < SEAT_NUM; i++) {
        for (int j = 0; j < 2; j++) {
            mSeats[i].mpSpectators[j]->Draw();
        }
    }
}

void RPPnpAudience::CheerTimed(s32 seat) {
    for (int i = 0; i < SEAT_NUM; i++) {
        if (seat == -1 || i == seat) {
            mSeats[i].mCheerTimer = 1;
            mSeats[i].mAnim = 0;

            f32 rand = RPUtlRandom::getF32();
            f32 numFrame = mSeats[i].mpSpectators[0]->GetBodyModel()->GetModelAnm()->GetNumFrame(
                RPGrpModelAnm::Anm_Chr, mSeats[i].mAnim);
            f32 frame = numFrame * rand;

            for (int j = 0; j < 2; j++) {
                RPSysKokeshiManager::ChangeExpression(mSeats[i].mpSpectators[j]->GetNigaoeModel(),
                                                      static_cast<RFLExpression>(1));
                mSeats[i].mpSpectators[j]->GetBodyModel()->GetModelAnm()->Start(
                    RPGrpModelAnm::Anm_Chr, 0, 0.0f);
                mSeats[i].mpSpectators[j]->GetBodyModel()->GetModelAnm()->SetFrame(
                    RPGrpModelAnm::Anm_Chr, 0, frame);
            }
        }
    }
}

void RPPnpAudience::PlayLoopAnimB(s32 seat) {
    for (int i = 0; i < SEAT_NUM; i++) {
        if (seat == -1 || i == seat) {
            mSeats[i].mAnim = 0;

            f32 rand = RPUtlRandom::getF32();
            f32 numFrame = mSeats[i].mpSpectators[0]->GetBodyModel()->GetModelAnm()->GetNumFrame(
                RPGrpModelAnm::Anm_Chr, mSeats[i].mAnim);
            f32 frame = numFrame * rand;

            for (int j = 0; j < 2; j++) {
                RPSysKokeshiManager::ChangeExpression(mSeats[i].mpSpectators[j]->GetNigaoeModel(),
                                                      static_cast<RFLExpression>(1));
                mSeats[i].mpSpectators[j]->GetBodyModel()->GetModelAnm()->Start(
                    RPGrpModelAnm::Anm_Chr, 0, 0.0f);
                mSeats[i].mpSpectators[j]->GetBodyModel()->GetModelAnm()->SetFrame(
                    RPGrpModelAnm::Anm_Chr, 0, frame);
            }
        }
    }
}

void RPPnpAudience::PlayLoopAnimA(s32 seat) {
    for (int i = 0; i < SEAT_NUM; i++) {
        if (seat == -1 || i == seat) {
            mSeats[i].mAnim = 1;

            f32 rand = RPUtlRandom::getF32();
            f32 numFrame = mSeats[i].mpSpectators[0]->GetBodyModel()->GetModelAnm()->GetNumFrame(
                RPGrpModelAnm::Anm_Chr, mSeats[i].mAnim);
            f32 frame = numFrame * rand;

            for (int j = 0; j < 2; j++) {
                RPSysKokeshiManager::ChangeExpression(mSeats[i].mpSpectators[j]->GetNigaoeModel(),
                                                      static_cast<RFLExpression>(3));
                mSeats[i].mpSpectators[j]->GetBodyModel()->GetModelAnm()->Start(
                    RPGrpModelAnm::Anm_Chr, 1, 0.0f);
                mSeats[i].mpSpectators[j]->GetBodyModel()->GetModelAnm()->SetFrame(
                    RPGrpModelAnm::Anm_Chr, 1, frame);
            }
        }
    }
}
