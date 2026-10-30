// Find Mii avatar manager: builds the stage's avatars from the Mii databases, their
// animations, and the cursor hit tests. Functions in reverse address order.

#include <Pack/RPParty/RPWlyScene/RPWlyCursor.h>
#include <Pack/RPParty/RPWlyScene/RPWlyAvatar.h>
#include <Pack/RPParty/RPWlyScene/RPWlyField_5.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>

#include <Pack/RPParty/RPWlyScene/RPWlyAvatarMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameConfig.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLevelTable.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel/RPSysKokeshiManager.h>
#include <Pack/RPKernel/RPSysMiddleDB.h>
#include <Pack/RPKernel/RPSysMiddleDBGenInfo.h>
#include <Pack/RPKernel/IRPSysKokeshiBodyManager.h>
#include <Pack/RPUtility/RPUtlRandom.h>

#include <egg/gfxe/eggScreen.h>

#include <algorithm>


// RPSysKokeshi.cpp (Wii Play, stubbed for release)
bool Mii_isSameKokeshiLocation(const RPSysKokeshiLocation* pLhs, const RPSysKokeshiLocation* pRhs);



f32 RPWlyAvatarMgr::sHitOffsetY = 22.4f;
f32 RPWlyAvatarMgr::sHitScale = 37.5f;

RPWlyAvatarMgr* RPWlyAvatarMgr::sInstance;

void RPWlyAvatarMgr::CreateInstance() {
    sInstance = new RPWlyAvatarMgr();
}

void RPWlyAvatarMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

void RPWlyAvatarMgr::CreateAvatarsSpecial() {
    mpMiddleDB = RPSceneInfo::spInstance->mpMiddleDB;

    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 7) {
        RPWlyGameConfig config = RPSceneInfo::spInstance->mMiiEntry.GetConfig();
        mGenInfos[0] = config.mGenInfo;
        mpAvatars[0] = new RPWlyAvatar(&mGenInfos[0], 0, TRUE);
    } else {
        mGenInfos[0].SetBodyType(6);
        mGenInfos[0].SetGenType(RPSysKokeshiManager::GenType_Friend);
        mGenInfos[0].SetResolution(static_cast<RFLResolution>(0x80));
        mGenInfos[0].SetExpressionFlag(0x2B);
        RPSysKokeshiLocation location;
        location.SetDataSource(RFLDataSource_Middle);
        location.SetMiddleDB(mpMiddleDB);
        mGenInfos[0].SetLocation(location);
        mpAvatars[0] = new RPWlyAvatar(&mGenInfos[0], 0, TRUE);
        mGenInfos[0] = mpAvatars[0]->GetGenInfo();
        mGenInfos[0].SetGenType(RPSysKokeshiManager::GenType_Direct);
    }

    for (s32 i = 1; i < mpStage->mTargetNum; i++) {
        mpAvatars[i] = new RPWlyAvatar(&mGenInfos[0], i, TRUE);
        mGenInfos[i] = mGenInfos[0];
    }

    for (s32 i = mpStage->mTargetNum; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
    retry:
        if (mpAvatars[i] != NULL) {
            delete mpAvatars[i];
        }

        mGenInfos[i].SetBodyType(6);
        mGenInfos[i].SetGenType(RPSysKokeshiManager::GenType_Friend);
        mGenInfos[i].SetResolution(static_cast<RFLResolution>(0x80));
        mGenInfos[i].SetExpressionFlag(0x2B);
        {
            RPSysKokeshiLocation location;
            location.SetDataSource(RFLDataSource_Middle);
            location.SetMiddleDB(mpMiddleDB);
            mGenInfos[i].SetLocation(location);
            mpAvatars[i] = new RPWlyAvatar(&mGenInfos[i], i, TRUE);
            mGenInfos[i] = mpAvatars[i]->GetGenInfo();
            mGenInfos[i].SetGenType(RPSysKokeshiManager::GenType_Direct);
        }

        if (Mii_isSameKokeshiLocation(&mGenInfos[i].GetLocation(), &mGenInfos[0].GetLocation())) {
            goto retry;
        }
    }
}

void RPWlyAvatarMgr::CreateAvatars() {
    s32 avatarNum = RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum;
    s32 targetNum = mpStage->mTargetNum;

    s32 validNum = RPSysKokeshiManager::GetOfficialDBValidNum();
    s32 maxOfficial = avatarNum * 5 / 10;
    u16 officialNum = std::min(maxOfficial, validNum);
    u16 hiddenNum = avatarNum * 3 / 10;

    if (validNum != 0) {
        RPSysMiddleDBGenInfo info;
        info.SetType(static_cast<RFLMiddleDBType>(3));
        info.SetSize(officialNum);
        mpOfficialDB = new RPSysMiddleDB(&info);
    }

    if (hiddenNum != 0) {
        RPSysMiddleDBGenInfo info;
        info.SetType(static_cast<RFLMiddleDBType>(0));
        info.SetSize(hiddenNum + 2);
        mpHiddenDB = new RPSysMiddleDB(&info);
        hiddenNum = mpHiddenDB->GetStoredNum();
    }

    {
        RPSysMiddleDBGenInfo info;
        info.SetType(static_cast<RFLMiddleDBType>(3));
        info.SetSize(100);
        mpMiddleDB = new RPSysMiddleDB(&info);
    }

    u32 rnd = RPUtlRandom::getU32(avatarNum);
    s32 source;
    if (validNum != 0 && hiddenNum != 0) {
        if (rnd < officialNum) {
            source = 0;
        } else if (rnd < officialNum + hiddenNum) {
            source = 1;
        } else {
            source = 2;
        }
    } else if (validNum == 0 && hiddenNum != 0) {
        if (rnd < hiddenNum) {
            source = 1;
        } else {
            source = 2;
        }
    } else if (validNum != 0 && hiddenNum == 0) {
        if (rnd < officialNum) {
            source = 0;
        } else {
            source = 2;
        }
    } else {
        source = 2;
    }

    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 8) {
        source = 0;
    }

    RPSysKokeshiLocation locations[3];
    locations[0].SetDataSource(static_cast<RFLDataSource>(0));
    locations[0].SetMiddleDB(mpOfficialDB);
    locations[1].SetDataSource(static_cast<RFLDataSource>(7));
    locations[1].SetMiddleDB(mpHiddenDB);
    locations[2].SetDataSource(static_cast<RFLDataSource>(7));
    locations[2].SetMiddleDB(mpMiddleDB);

    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
    case 21:
    case 22:
        mGenInfos[0].SetGenType(RPSysKokeshiManager::GenType_Player);
        mGenInfos[0].SetPlayerNo(0);
        break;
    default:
        mGenInfos[0].SetGenType(RPSysKokeshiManager::GenType_Friend);
        break;
    }

    mGenInfos[0].SetBodyType(6);
    mGenInfos[0].SetResolution(static_cast<RFLResolution>(0x80));
    mGenInfos[0].SetExpressionFlag(0x2B);
    mGenInfos[0].SetLocation(locations[source]);
    mpAvatars[0] = new RPWlyAvatar(&mGenInfos[0], 0, TRUE);
    mGenInfos[0] = mpAvatars[0]->GetGenInfo();
    mGenInfos[0].SetGenType(RPSysKokeshiManager::GenType_Direct);

    if (source == 0) {
        officialNum--;
    } else if (source == 1) {
        hiddenNum--;
    }

    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
    case 9:
    case 10:
    case 11:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 22:
        targetNum = 1;
        break;
    }

    for (s32 i = 1; i < targetNum; i++) {
        mpAvatars[i] = new RPWlyAvatar(&mGenInfos[0], i, TRUE);
        mGenInfos[i] = mGenInfos[0];
    }

    for (s32 i = targetNum; i < avatarNum; i++) {
    retry:
        s32 src;
        if (officialNum != 0) {
            src = 0;
            officialNum--;
        } else if (hiddenNum != 0) {
            src = 1;
            hiddenNum--;
        } else {
            src = 2;
        }

        if (mpAvatars[i] != NULL) {
            delete mpAvatars[i];
        }

        if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 22 && i == 1) {
            mGenInfos[1].SetGenType(RPSysKokeshiManager::GenType_Player);
            mGenInfos[1].SetPlayerNo(1);
        } else {
            mGenInfos[i].SetGenType(RPSysKokeshiManager::GenType_Friend);
        }

        mGenInfos[i].SetBodyType(6);
        mGenInfos[i].SetResolution(static_cast<RFLResolution>(0x80));
        mGenInfos[i].SetExpressionFlag(0x2B);
        mGenInfos[i].SetLocation(locations[src]);
        mpAvatars[i] = new RPWlyAvatar(&mGenInfos[i], i, TRUE);
        mGenInfos[i] = mpAvatars[i]->GetGenInfo();
        mGenInfos[i].SetGenType(RPSysKokeshiManager::GenType_Direct);

        if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType != 22 || i != 1) {
            for (s32 j = 0; j < i; j++) {
                if (Mii_isSameKokeshiLocation(&mGenInfos[i].GetLocation(),
                                              &mGenInfos[j].GetLocation())) {
                    goto retry;
                }
            }
        }
    }
}

RPWlyAvatarMgr::RPWlyAvatarMgr() {
    mpStage = new RPWlyStage();

    for (s32 i = 0; i < 100; i++) {
        mpAvatars[i] = NULL;
    }

    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
    case 6:
    case 7:
        CreateAvatarsSpecial();
        break;
    default:
        CreateAvatars();
        break;
    }

    mpQuestionAvatar =
        new RPWlyAvatar(&mGenInfos[0], RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum, FALSE);
}

RPWlyAvatarMgr::~RPWlyAvatarMgr() {
    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 7) {
        mpMiddleDB->UpdateRandomDBAsync();
    }

    mpQuestionAvatar = NULL;
}

bool RPWlyAvatarMgr::TryWrongReaction(s32 idx) {
    bool result = false;
    if (idx >= 0) {
        result = mpAvatars[idx]->StartWrongReaction();
    }
    return result;
}

void RPWlyAvatarMgr::SetAvatarFound(s32 idx) {
    if (idx >= 0) {
        mpAvatars[idx]->SetFound();
    }
}

void RPWlyAvatarMgr::MarkFound(s32 idx) {
    mFoundIdx = idx;
    mpStage->MarkTargetFound(idx);
    mpAvatars[idx]->OnFound();

    switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
    case 6:
        mpStage->RecordFoundMii(idx);
        break;
    }
}

void RPWlyAvatarMgr::HideNonTargets() {
    for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
        case 6:
            if (i != mpStage->mTargetIdx) {
                mpAvatars[i]->mVisible = false;
            }
            break;
        default:
            if (!mpStage->IsTargetNotFound(i) && !mpStage->IsTargetFound(i)) {
                mpAvatars[i]->mVisible = false;
            }
            break;
        }
    }
}

void RPWlyAvatarMgr::HideAll() {
    for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        mpAvatars[i]->mVisible = false;
    }
}

void RPWlyAvatarMgr::ShowAll() {
    for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        mpAvatars[i]->mVisible = true;
    }
}

bool RPWlyAvatarMgr::IsWrongReacting(s32 idx) const {
    return mpAvatars[idx]->mReaction == 1;
}

bool RPWlyAvatarMgr::IsAnyWrongReacting() const {
    bool result = false;
    for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        if (mpAvatars[i]->mReaction == 1) {
            result = true;
            break;
        }
    }
    return result;
}

void RPWlyAvatarMgr::Init() {}

void RPWlyAvatarMgr::Create() {
    const char* anmNames[13] = {
        "wtd_waitR",      "wtd_walkR",        "wtd_jogR",       "wtd_happyR",
        "wtd_selectR",    "wtd_swim1R",       "wtd_swim2R",     "wtd_swim_hitR",
        "wtd_swim_selectR", "wtd_spacewalkR", "wtd_space_hitR", "wtd_space_selectR",
        "wtd_sleepR",
    };
    RPGrpHandle anms[13];
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/wtd_avatar_anim.brres");

    for (u32 i = 0; i < 13; i++) {
        RPGrpHandle anm = pResMgr->CreateData(
            RPGrpModelResManager::Type_ResAnmChr,
            pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmChr, file, anmNames[(u16)i]));
        anms[(u16)i] = anm;
    }

    IRPSysKokeshiBodyManager* pBodyMgr = RPSysKokeshiManager::instance()->GetBodyManager();

    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 7) {
        RPWlyGameConfig config = RPSceneInfo::spInstance->mMiiEntry.GetConfig();
        mOverloadInfos[0].SetClothesType(config.mOverloadInfo.GetClothesType());
        mpAvatars[0]->LoadResource(&mOverloadInfos[0]);
        mpAvatars[0]->ApplyLightTexture(0);
    } else {
        switch (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType) {
        case 21:
        case 22: {
            u32 clothes = RPUtlRandom::getU32(pBodyMgr->GetFriendClothesIndexRange());
            u16 start = pBodyMgr->GetFriendClothesStartIndex();
            mOverloadInfos[0].SetClothesType(start + clothes);
            mpAvatars[0]->LoadResource(&mOverloadInfos[0]);
            break;
        }
        default:
            mpAvatars[0]->LoadResource(NULL);
            mOverloadInfos[0].SetClothesType(mpAvatars[0]->GetOverloadInfo().GetClothesType());
            break;
        }
        mpAvatars[0]->ApplyLightTexture(0);
    }

    for (s32 i = 1; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mQuestionType == 22 && i == 1) {
            u32 clothes = RPUtlRandom::getU32(pBodyMgr->GetFriendClothesIndexRange());
            u16 start = pBodyMgr->GetFriendClothesStartIndex();
            mOverloadInfos[1].SetClothesType(start + clothes);
            mpAvatars[1]->LoadResource(&mOverloadInfos[1]);
        } else {
            mpAvatars[i]->LoadResource(NULL);
            mOverloadInfos[i].SetClothesType(mpAvatars[i]->GetOverloadInfo().GetClothesType());
        }
        mpAvatars[i]->ApplyLightTexture(0);
    }

    for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        RPGrpModel* pBody = mpAvatars[i]->GetBodyModel();
        pBody->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 13);
        for (u32 j = 0; j < 13; j++) {
            pBody->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, j, anms[(u16)j]);
        }
        pBody->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
        pBody->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, 1.0f);
        pBody->GetModelAnm()->SetFrame(RPGrpModelAnm::Anm_Chr, 3, 30.0f);
    }

    mpQuestionAvatar->LoadResource(&mOverloadInfos[0]);
    mpQuestionAvatar->ApplyLightTexture(0);
    RPGrpModel* pBody = mpQuestionAvatar->GetBodyModel();
    pBody->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 13);
    for (u32 j = 0; j < 13; j++) {
        pBody->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Chr, j, anms[(u16)j]);
    }
    pBody->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
    pBody->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
    RPGrpModelAnm* pAnm = pBody->GetModelAnm();
    pAnm->SetFrame(RPGrpModelAnm::Anm_Chr, 0, pAnm->GetNumFrame(RPGrpModelAnm::Anm_Chr, 0) / 4.0f);
}

void RPWlyAvatarMgr::Reset() {
    mMotionFlags[0] = TRUE;
    mMotionFlags[1] = TRUE;
    mMotionFlags[2] = TRUE;

    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum >= 40) {
        mMotionFlags[2] = FALSE;
    }

    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum >= 50 &&
        RPWlyLevelTable::sInstance->IsSpotlightStage()) {
        mMotionFlags[0] = FALSE;
        mMotionFlags[1] = FALSE;
    }

    for (s32 i = 0; i < 100; i++) {
        if (mpAvatars[i] != NULL) {
            mpAvatars[i]->mSwayX = 0.0f;
            mpAvatars[i]->mSwaySpeedX = 0.0f;
            mpAvatars[i]->ResetState();
        }
    }

    unk28F8 = 0;
    mFoundIdx = -1;
    mHoverIdx[0] = -1;
    mLastHoverIdx[0] = -1;
    mHoverIdx[1] = -1;
    mLastHoverIdx[1] = -1;
}

s32 RPWlyAvatarMgr::FindAvatarUnderCursor(s32 player) {
    s32 result = -1;
    f32 minDist = 10000.0f;
    EGG::Vector2f pos = RPWlyGameMgr::sInstance->mpCursors[player]->GetPos();
    pos.x *= (f32)(u16)(EGG::Screen::GetSizeXMax() / 2);
    pos.y *= (f32)(u16)(EGG::Screen::GetSizeYMax() / 2);

    for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        if (!mpStage->IsTargetFound(i) && mpAvatars[i]->unkDC && mpStage->IsTargetNotFound(i)) {
            RPWlyAvatar* pAvatar = mpAvatars[i];
            nw4r::math::VEC3 screen = pAvatar->mScreenPos;
            f32 dx = pos.x - screen.x;
            f32 dy = pos.y - screen.y;
            f32 scale = pAvatar->mScale;
            f32 radius = sHitOffsetY + sHitScale * scale / screen.z;
            f32 dist = dx * dx + dy * dy;
            if (dist < radius * radius) {
                if (result != -1) {
                    if (dist < minDist) {
                        minDist = dist;
                        result = i;
                    }
                } else {
                    result = i;
                    minDist = dist;
                }
            }
        }
    }

    return result;
}

// RPWlyCursor::GetPos, by value (hidden result pointer in r3, cursor in r4). RPWlyCursor.h
// declares it with an out parameter and RPWlyField_4 calls it under its ledger name, so it is
// defined under that name here.
EGG::Vector2f RPWlyCursor::GetPos() const {
    return mPos;
}

s32 RPWlyAvatarMgr::FindAvatarNearCursor(s32 player) {
    s32 result = -1;
    f32 minDist = 10000.0f;
    EGG::Vector2f pos = RPWlyGameMgr::sInstance->mpCursors[player]->GetPos();
    pos.x *= (f32)(u16)(EGG::Screen::GetSizeXMax() / 2);
    pos.y *= (f32)(u16)(EGG::Screen::GetSizeYMax() / 2);

    for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
        if (!mpStage->IsTargetFound(i) && mpAvatars[i]->unkDC) {
            for (s32 j = 0; j < 2; j++) {
                if (mpStage->IsTargetNotFound(i) && j == 0) {
                    continue;
                }
                nw4r::math::VEC3 screen = (&mpAvatars[i]->mScreenPos)[j];
                RPWlyAvatar* pAvatar = mpAvatars[i];
                f32 dx = pos.x - screen.x;
                f32 dy = pos.y - screen.y;
                f32 scale = pAvatar->mScale;
                f32 radius = sHitOffsetY + sHitScale * scale / screen.z;
                f32 dist = dx * dx + dy * dy;
                if (dist < radius * radius) {
                    if (result != -1) {
                        if (dist < minDist) {
                            minDist = dist;
                            result = i;
                        }
                    } else {
                        result = i;
                        minDist = dist;
                    }
                }
            }
        }
    }

    return result;
}

void RPWlyAvatarMgr::Calculate() {
    if (mpStage->mActive) {
        mpStage->Calculate();

        for (s32 i = 0; i < RPWlyGameMgr::sInstance->mPlayerNum; i++) {
            mHoverIdx[i] = FindAvatarUnderCursor(i);
            if (mHoverIdx[i] == -1) {
                mHoverIdx[i] = FindAvatarNearCursor(i);
            }

            s32 idx = mHoverIdx[i];
            if (idx != -1) {
                if (idx != mLastHoverIdx[i]) {
                    mLastHoverIdx[i] = idx;
                    mpAvatars[idx]->PlayHoverSound();
                }
            } else {
                mLastHoverIdx[i] = -1;
            }
        }
    }
}

void RPWlyAvatarMgr::Destroy() {}

void RPWlyAvatarMgr::Draw() {
    if (mpStage->mActive) {
        for (s32 i = 0; i < RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum; i++) {
            mpAvatars[i]->Draw();
        }
    }
}

void RPWlyAvatarMgr::SetPaused(bool paused) {
    s32 avatarNum = RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum;
    for (s32 i = 0; i < avatarNum; i++) {
        mpAvatars[i]->SetPaused(paused);
    }
}

void RPWlyAvatarMgr::StartSwimEffects() {
    s32 avatarNum = RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum;
    for (s32 i = 0; i < avatarNum; i++) {
        if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum < 40) {
            if (!RPWlyGameMgr::sInstance->mpWipe->mDone) {
                mpAvatars[i]->StartSwimEffect();
            }
        } else {
            if (RPWlyGameMgr::sInstance->mpWipe->mDone) {
                mpAvatars[i]->StartSwimEffect();
            }
        }
    }
}
