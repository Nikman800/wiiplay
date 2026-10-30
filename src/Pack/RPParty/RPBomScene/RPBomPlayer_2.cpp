#include <Pack/RPParty/RPBomScene/RPBomPlayer_2.h>

void RPBomPlayerManager::CreateInstance() {
    RPBomPlayerManager* manager = new RPBomPlayerManager;
    sInstance = manager;
}

void RPBomPlayerManager::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBomPlayerManager::~RPBomPlayerManager() {
}

void RPBomPlayerManager::CreatePlayers(s32 num) {
    mPlayerNum = num;

    s32 player = 0;
    do {
        mpPlayers[player] = new RPBomPlayer;
        mpPlayers[player]->mPlayer = static_cast<u16>(player);
        mpPlayers[player]->Create();
    } while (++player < mPlayerNum);
}

void RPBomPlayerManager::Load() {
    s32 player = 0;
    do {
        mpPlayers[player]->Load();
    } while (++player < mPlayerNum);
}

void RPBomPlayerManager::Reset() {
    s32 player = 0;
    do {
        mpPlayers[player]->Reset();
    } while (++player < mPlayerNum);

    if (RPBomGameManager::sInstance->mMode == 0) {
        mActiveNum = 1;
    } else if (RPBomGameManager::sInstance->mMode == 1) {
        mActiveNum = 2;
    }
}

void RPBomPlayerManager::CalcState() {
    s32 player = 0;
    do {
        mpPlayers[player]->CalcState();
    } while (++player < mActiveNum);
}

void RPBomPlayerManager::CalcMove() {
    s32 player = 0;
    do {
        mpPlayers[player]->CalcMove();
    } while (++player < mActiveNum);
}

void RPBomPlayerManager::UpdateModels() {
    s32 player = 0;
    do {
        mpPlayers[player]->UpdateModel();
    } while (++player < mActiveNum);
}

void RPBomPlayerManager::SetAllIdle() {
    s32 player = 0;
    do {
        mpPlayers[player]->SetStateIdle();
    } while (++player < mActiveNum);
}

void RPBomPlayerManager::StartAllPosing(f32 scale, s32 pose) {
    s32 player = 0;
    do {
        mpPlayers[player]->StartPosing(pose, scale);
    } while (++player < mActiveNum);
}

void RPBomPlayerManager::RestartAllPosing2() {
    s32 player = 0;
    do {
        mpPlayers[player]->RestartPosing2();
    } while (++player < mActiveNum);
}

void RPBomPlayerManager::SetStateIraByMask(u32 mask) {
    s32 player = 0;
    do {
        mpPlayers[player]->SetStateIra((mask & (1 << player)) != 0);
    } while (++player < mActiveNum);
}

void RPBomPlayerManager::RestartAllPosing() {
    s32 player = 0;
    do {
        mpPlayers[player]->RestartPosing();
    } while (++player < mActiveNum);
}

void RPBomPlayerManager::SetPlayerStateWin(s32 player, BOOL playAnim) {
    mpPlayers[player]->SetStateWin(playAnim);
}

void RPBomPlayerManager::SetAllStateResult() {
    s32 player = 0;
    do {
        mpPlayers[player]->SetStateResult();
    } while (++player < mActiveNum);
}

void RPBomPlayerManager::PlayResultAnims(s32 result) {
    s32 mode = RPBomGameManager::sInstance->mMode;
    switch (mode) {
    case 0:
        if (result == 0) {
            mpPlayers[0]->PlayResultAnim(FALSE);
        } else {
            if (result == 1) {
                mpPlayers[0]->PlayResultAnim(TRUE);
            }
        }
        break;

    case 1:
        if (result == 0) {
            mpPlayers[0]->PlayResultAnim(TRUE);
            mpPlayers[1]->PlayResultAnim(FALSE);
        } else {
            if (result == 1) {
            mpPlayers[0]->PlayResultAnim(FALSE);
            mpPlayers[1]->PlayResultAnim(TRUE);
            }
        }
        break;
    }
}
