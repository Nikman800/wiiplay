// Shooting Range shot manager (RPDucShotMgr): one RPDucShot cursor per player.
// Unit 0x80244648-0x80244BB8 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucShot.h>

#include <Pack/RPParty/RPDucScene/RPDucGame.h>

RPDucShotMgr* RPDucShotMgr::sInstance;

void RPDucShotMgr::CreateInstance() {
    sInstance = new RPDucShotMgr();
}

void RPDucShotMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucShotMgr::~RPDucShotMgr() {}

void RPDucShotMgr::CreateShots(s32 num) {
    mShotNum = num;
    s32 i = 0;
    do {
        mpShots[i] = new RPDucShot();
        mpShots[i]->Create();
        mpShots[i]->CreateCursorEffects(i);
    } while (++i < mShotNum);
}

void RPDucShotMgr::Load() {}

void RPDucShotMgr::Reset() {
    s32 i = mShotNum - 1;
    do {
        mpShots[i]->Reset();
    } while (--i >= 0);

    i = mShotNum - 1;
    do {
        mpShots[i]->RestartTailEffect();
    } while (--i >= 0);

    i = mShotNum - 1;
    do {
        mpShots[i]->RestartMainEffect();
    } while (--i >= 0);

    s32 mode = RPDucGame::sInstance->GetMode();
    if (mode == 0) {
        mActiveNum = 1;
    } else if (mode == 1) {
        mActiveNum = 2;
    } else if (mode == 2) {
        mActiveNum = 2;
    }

    if (mode == 2) {
        mpShots[0]->SetCursorMode(2);
        mpShots[1]->SetCursorMode(0);
    } else {
        i = mActiveNum - 1;
        do {
            mpShots[i]->SetCursorMode(2);
        } while (--i >= 0);
    }
}

void RPDucShotMgr::Calculate() {
    s32 i = 0;
    do {
        mpShots[i]->Calculate();
    } while (++i < mActiveNum);
}

void RPDucShotMgr::Exit() {
    s32 i = 0;
    do {
        mpShots[i]->Exit();
    } while (++i < mShotNum);
}

void RPDucShotMgr::Draw() {
    s32 i = 0;
    do {
        mpShots[i]->Draw();
    } while (++i < mShotNum);
}

void RPDucShotMgr::SetCursorMode(s32 player, s32 mode) {
    mpShots[player]->SetCursorMode(mode);
}

void RPDucShotMgr::SetCursorModeAll(s32 mode) {
    s32 i = mActiveNum - 1;
    do {
        mpShots[i]->SetCursorMode(mode);
    } while (--i >= 0);
}

void RPDucShotMgr::ProcessHitsAll() {
    s32 i = 0;
    do {
        mpShots[i]->ProcessHits();
    } while (++i < mActiveNum);
}

void RPDucShotMgr::PlayControllerSoundAll() {
    s32 i = 0;
    do {
        mpShots[i]->PlayControllerSound();
    } while (++i < mActiveNum);
}
