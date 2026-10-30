// The original included RPDucSound.h after the RPDucGame class definition: CodeWarrior
// emits vtables in reverse order of class definition, so this puts the weak sound actor
// vtables (dropped at link) before __vt__9RPDucGame.
#define RP_PARTY_DUC_SOUND_H
class RPDucSndActor;
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#undef RP_PARTY_DUC_SOUND_H
// The original also instantiated TRPSndObject<4> here: its weak vtables are the 0x168 zero bytes
// (a duplicate of RPDucScene's copy) before __vt__9RPDucGame.
#define RP_DUC_SOUND_INSTANTIATE_SND_OBJECT
#include <Pack/RPParty/RPDucScene/RPDucSound.h>
#undef RP_DUC_SOUND_INSTANTIATE_SND_OBJECT
#include <Pack/RPParty/RPDucScene/RPDucCamera.h>
#include <Pack/RPParty/RPDucScene/RPDucHud.h>
#include <Pack/RPParty/RPDucScene/RPDucShot.h>
#include <Pack/RPParty/RPDucScene/RPDucSpawner.h>
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPLeadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPPauseText.h>
#include <Pack/RPParty/RPPartyCommon/RPResultMenu.h>
#include <Pack/RPParty/RPPartyCommon/RPMedalResult.h>
#include <Pack/RPParty/RPPartyCommon/RPVsWinner.h>
#include <Pack/RPParty/RPPartyCommon/RPTextWriter.h>
#include <egg/gfxe/eggScreen.h>

inline f32 DucRandom() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) RP_randSeedY ^= 0x11020;
    RP_randSeedY >>= 1;
    return static_cast<f32>((RP_randSeedX ^ RP_randSeedY) & 0x7FFFFF) * 1.192093e-7f;
}

inline u32 DucRemoteFlag() {
    if (RPDucGame::sInstance->GetMode() != 1 && !RPDucGame::sInstance->IsP2Joined())
        return 1;
    return 3;
}

inline RPDucTargetRegistry* DucTargets() {
    return RPDucTargetRegistry::sInstance;
}

inline RPDucHud* DucHud() {
    return RPDucHud::sInstance;
}

// The TRPSndObject<0> virtuals are instantiated here as weak copies from RPSndObject.h (the
// vtable is needed by the inlined RPDucSndActor constructor). The linker keeps RPFshFish's
// copies and drops these from .text, but their RPSndObject.h assert strings stay in .data.

RPDucGame* RPDucGame::sInstance;

void RPDucGame::CreateInstance() { sInstance = new RPDucGame; }

void RPDucGame::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucGame::~RPDucGame() {}

void RPDucGame::Start() {
    RPDucCamera::sInstance->SetMode(0);
    RPDucShotMgr* shots = RPDucShotMgr::sInstance;
    if (mMode == 2 && !mP2Joined) shots->SetCursorMode(0, 1);
    else shots->SetCursorModeAll(1);
    mpSpawner->StartPattern(8, 10);
    mWaves = 1;
    mTimer = 0;
    mState = 1;
    mRound = 1;
    mRoundNum = 5;
    mpScoreMgr->ClearScores();
    mpScoreMgr->ClearHits();
    mpScoreMgr->ClearStreaks();
    mpAmmoMgr->Setup(2);
    mInPlay = false;
    if (mMode != 1) RPCommon::GetInstance()->GetPauseText()->SetEnabled(1, true);
    SetRoundParams(mRound);
}
void RPDucGame::SetRoundParams(s32 round) {
    switch (round) {
    case 1: mSpawnPattern = 0; mRoundAmmo = 10; mSpawnCount = 2; unk38 = 7; mWaves = 3; break;
    case 2: mSpawnPattern = 2; mRoundAmmo = 10; mSpawnCount = 13; unk38 = 60; mWaves = 5; break;
    case 3: mSpawnPattern = 3; mRoundAmmo = 10; mSpawnCount = 8; unk38 = 22; mWaves = 1; break;
    case 4: mSpawnPattern = 9; mRoundAmmo = 10; mSpawnCount = 1; unk38 = 7; mWaves = 3; break;
    case 5: mSpawnPattern = 6; mRoundAmmo = 10; mSpawnCount = 1; unk38 = 4; mWaves = 4; break;
    case 6: mSpawnPattern = 5; mRoundAmmo = 10; mSpawnCount = 2; unk38 = 16; mWaves = 4; break;
    case 7: mSpawnPattern = 7; mRoundAmmo = 10; mSpawnCount = 1; unk38 = 10; mWaves = 4; break;
    }
    mpAmmoMgr->mMaxAmmo = mRoundAmmo;
}
void RPDucGame::SpawnAvatars() {
    RPDucTargetRegistry* targets;
    u32 side = mMode == 1;
    f32 scale;
    if (EGG::Screen::GetTVMode() != EGG::Screen::TV_MODE_STD) scale = 1.0f;
    else scale = 0.75f;
    RPDucAvatarMgr::sFinalRoundFlag = false;
    targets = RPDucTargetRegistry::sInstance;
    nw4r::math::VEC3 pos(0.0f, -15.0f, 0.0f);
    pos.x = -80.0f * scale;
    pos.z = DucRandom() * 20.0f + -40.0f;
    targets->Spawn(6, pos, 0);
    pos.x = -50.0f * scale;
    pos.z = DucRandom() * 30.0f + -60.0f;
    targets->Spawn(6, pos, side);
    pos.x = -20.0f * scale;
    pos.z = DucRandom() * 20.0f + -70.0f;
    targets->Spawn(6, pos, 0);
    pos.x = 20.0f * scale;
    pos.z = DucRandom() * 20.0f + -70.0f;
    targets->Spawn(6, pos, side);
    pos.x = 50.0f * scale;
    pos.z = DucRandom() * 30.0f + -60.0f;
    targets->Spawn(6, pos, 0);
    pos.x = 80.0f * scale;
    pos.z = DucRandom() * 20.0f + -40.0f;
    targets->Spawn(6, pos, side);
}
void RPDucGame::StateNone() {}
void RPDucGame::StateIntro() {
    mpSpawner->Calculate();
    ++mTimer;
    if (mTimer == 1) {
        RPSndAudioMgr::instance()->startBgm(0x64u);
        u32 remote = DucRemoteFlag();
        RPSndAudioMgr::instance()->startSystemSeWithRemote(0x2fu, remote);
    }
    if (RPDucCamera::sInstance->GetMode() != 0) {
        RPDucShotMgr* shots = RPDucShotMgr::sInstance;
        if (mMode == 2 && !mP2Joined) shots->SetCursorMode(0, 2);
        else shots->SetCursorModeAll(2);
        RPDucStopJingle();
        RPDucHud::sInstance->SetHitCounterVisible(true);
        RPDucHud::sInstance->SetStartBannerVisible(true);
        u32 remote = DucRemoteFlag();
        if (mRound != mRoundNum) RPSndAudioMgr::instance()->startSystemSeWithRemote(0x23u, remote);
        else RPSndAudioMgr::instance()->startSystemSeWithRemote(0x24u, remote);
        if (mRound == mRoundNum) RPSndAudioMgr::instance()->startSystemSe(0x66u);
        else if (mRound != 1) RPSndAudioMgr::instance()->startSystemSe(0x65u);
        mTimer = 180;
        mState = 2;
    }
}
void RPDucGame::StateRoundStart() {
    RPDucHud* hud = RPDucHud::sInstance;
    if (--mTimer == 0) {
        mpAmmoMgr->ClearShotCounts();
        mpSpawner->StartPattern(mSpawnPattern, mSpawnCount);
        mInPlay = true;
        mState = 3;
    } else if (mTimer == 10) {
        hud->SetHitCounterVisible(true);
    } else if (mTimer == 20) {
        hud->SetStartBannerVisible(false);
    } else if (mTimer == 120) {
        u32 remote = DucRemoteFlag();
        RPSndAudioMgr::instance()->startSystemSe(0x25u);
        RPSndAudioMgr::instance()->startSystemSeWithRemote(0x26u, remote);
        mpAmmoMgr->Refill();
    } else if (mTimer == 60) {
        u32 remote = DucRemoteFlag();
        RPSndAudioMgr::instance()->startSystemSe(0x27u);
        RPSndAudioMgr::instance()->startSystemSeWithRemote(0x27u, remote);
        if (mRound == mRoundNum) SpawnAvatars();
    }
}
void RPDucGame::StateRoundPlay() {
    mpSpawner->Calculate();
    RPDucTargetRegistry& targets = *DucTargets();
    s32 active = targets.CountActiveExclDucksAvatars();
    if (!mpSpawner->IsActive() && active == 0) {
        if (mWaves == 1) {
            if (mRound == mRoundNum) mInPlay = false;
            if (targets.CountActive() == 0) {
                mWaves = 0;
                if (mRound == mRoundNum) {
                    if (AwardStreakBonuses()) mTimer = 120;
                    else mTimer = 40;
                } else mTimer = 120;
                mState = 4;
            }
        } else {
            --mWaves;
            mpSpawner->AddCount(0);
        }
    }
}
void RPDucGame::StateRoundEnd() {
    u32 remote;
    RPDucHud* hud = DucHud();
    if (--mTimer == 0) {
        RPDucTargetRegistry::sInstance->RemoveAll();
        if (mRound == mRoundNum) {
            RPDucStartJingle();
            hud->GetEndLayout()->PlayIn();
            bool tie = mpScoreMgr->GetLeader() == -1;
            if (tie) RPSndAudioMgr::instance()->startBgm(0x68u);
            else RPSndAudioMgr::instance()->startBgm(0x67u);
            mState = 5;
        } else {
            if (mMode != 1 && mRound == 1)
                RPCommon::GetInstance()->GetPauseText()->SetEnabled(1, false);
            SetRoundParams(++mRound);
            mInPlay = false;
            hud->SetStartBannerVisible(true);
            remote = DucRemoteFlag();
            if (mRound != mRoundNum) RPSndAudioMgr::instance()->startSystemSeWithRemote(0x23u, remote);
            else RPSndAudioMgr::instance()->startSystemSeWithRemote(0x24u, remote);
            if (mRound == mRoundNum) RPSndAudioMgr::instance()->startSystemSe(0x66u);
            else if (mRound != 1) RPSndAudioMgr::instance()->startSystemSe(0x65u);
            mTimer = 180;
            mState = 2;
        }
    } else if (mTimer == 10) {
        hud->SetHitCounterVisible(false);
    }
}
void RPDucGame::StateWaitScore() {
    if (RPDucHud::sInstance->IsEndAnimDone()) {
        RPDucShotMgr::sInstance->SetCursorModeAll(0);
        switch (mMode) {
        case 0:
        case 2:
            RPCommon::GetInstance()->GetMedalResult()->Start(mpScoreMgr->GetScore(0));
            mState = 6;
            break;
        case 1:
            s32 leader = mpScoreMgr->GetLeader();
            RPCommon::GetInstance()->GetVsWinner()->Start(leader);
            mState = 6;
            break;
        }
    }
}
void RPDucGame::StateShowResults() {
    switch (mMode) {
    case 0:
    case 2:
        if (RPCommon::GetInstance()->GetMedalResult()->GetState() == 4) {
            RPCommon::GetInstance()->GetResultMenu()->Start(1, -1);
            mState = 7;
        }
        break;
    case 1:
        if (RPCommon::GetInstance()->GetVsWinner()->GetState() == 5) {
            s32 leader = mpScoreMgr->GetLeader();
            RPCommon::GetInstance()->GetResultMenu()->Start(2, leader);
            mState = 7;
        } else if (RPCommon::GetInstance()->GetVsWinner()->IsStateEntered(3)) {
            RPDucHud::sInstance->GetEndLayout()->PlayOut();
        }
        break;
    }
}
void RPDucGame::StateEnd() {
    switch (mMode) {
    case 0:
    case 2:
        if (RPCommon::GetInstance()->GetResultMenu()->GetState() == 4) {
            switch (RPCommon::GetInstance()->GetResultMenu()->GetChoice()) {
            case 0: mQuit = true; break;
            case 1: mRetry = true; break;
            }
            RPSndAudioMgr::instance()->stopBgm(10);
        }
        break;
    case 1:
        if (RPCommon::GetInstance()->GetResultMenu()->GetState() == 4) {
            switch (RPCommon::GetInstance()->GetResultMenu()->GetChoice()) {
            case 0: mQuit = true; break;
            case 1: mRetry = true; break;
            }
            RPSndAudioMgr::instance()->stopBgm(10);
        }
        break;
    }
}
void RPDucGame::Create() {
    RPDucSpawner::CreateInstance();
    mpSpawner = RPDucSpawner::sInstance;
    mpSpawner->Create();
    RPDucScoreMgr::CreateInstance();
    mpScoreMgr = RPDucScoreMgr::sInstance;
    RPDucAmmoMgr::CreateInstance();
    mpAmmoMgr = RPDucAmmoMgr::sInstance;
    mpSndActor = new RPDucSndActor;
}
void RPDucGame::Load() {}

void RPDucGame::Setup(s32 playMode) {
    if (playMode == 2) mPlayerNum = 2;
    else mPlayerNum = 1;
    mTimer = 0;
    mInPlay = false;
    if (playMode == 1) mMode = 0;
    else if (playMode == 2) mMode = 1;
    else if (playMode == 3) mMode = 2;
    mP2Joined = false;
    mQuit = false;
    mRetry = false;
    mpSpawner->Reset();
    mpScoreMgr->Setup(2);
    mpAmmoMgr->Setup(2);
}

void RPDucGame::ResetGame() {
    RPCommon::GetInstance()->GetLeadCursor()->SetTimers(15);
    mInPlay = false;
    Start();
}
void RPDucGame::Calculate() {
    (this->*mStateFuncs[mState])();
    if (mMode == 2 && !mP2Joined) {
        if (mMode == 2 && !mP2Joined &&
            EGG::CoreControllerMgr::instance()->getNthController(1)->down(0xC00)) {
            mP2Joined = true;
            RPDucShotMgr::sInstance->PlayControllerSoundAll();
            if (mState == 1) RPDucShotMgr::sInstance->SetCursorMode(1, 1);
            else RPDucShotMgr::sInstance->SetCursorMode(1, 2);
        }
    }
    RPCommon::GetInstance()->GetLeadCursor()->SetAllEnabled(mInPlay);
    nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
    mpSndActor->set3DPosition(pos);
}
void RPDucGame::Draw() {
    f32 halfWidth = static_cast<u16>(static_cast<u32>(EGG::Screen::GetSizeXMax()) >> 1);
    f32 halfHeight = static_cast<u16>(static_cast<u32>(EGG::Screen::GetSizeYMax()) >> 1);
    RPTextWriter writer;
    writer.Begin(1);
    writer.SetDrawFlag(0x110);
    writer.End();
}
void RPDucGame::Exit() {
    RPDucSpawner::DestroyInstance();
    RPDucScoreMgr::DestroyInstance();
    RPDucAmmoMgr::DestroyInstance();
}

s32 RPDucGame::GetCurrentRound() const {
    switch (mState) {
    case 1:
    case 2:
    case 3:
    case 4:
        return mRound;
    default:
        return 0;
    }
}

s32 RPDucGame::GetRoundNum() const { return mRoundNum; }

void RPDucGame::PlayRemoteAlertSound(s32 type) {
    RPSndAudioMgr* audio;
    u32 remote = DucRemoteFlag();
    if (type == 0) {
        audio = RPSndAudioMgr::instance();
        audio->startSystemSeWithRemote(0x2fu, remote);
    } else {
        audio = RPSndAudioMgr::instance();
        audio->startSystemSeWithRemote(0x2eu, remote);
    }
}
void RPDucGame::PlayStreakSound() { RPSndAudioMgr::instance()->startSystemSe(0x69u); }
void RPDucGame::PlayStreakBonusSound() { RPSndAudioMgr::instance()->startSystemSe(0x6au); }

