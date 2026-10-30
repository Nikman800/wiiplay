// Billiards player holder (RPBilPlayerMgr, singleton RPBilPlayerMgr::sInstance): turn order, per-player
// statistics and input. Unit 0x802C1138-0x802C16F4. Functions in reverse address order.
#include <Pack/RPParty/RPBilScene/RPBilPlayer.h>

#include <Pack/RPParty/RPBilScene/RPBilGameMgr.h>
#include <Pack/RPParty/RPBilScene/RPBilHud.h>

#include <egg/core.h>

// RPBilPlayerMgr::sInstance (.sbss 0x8043C9F0) belongs to this file but still sits in an auto split; it is
// declared in RPBilPlayer.h and should be defined here (RPBilPlayerMgr* RPBilPlayerMgr::sInstance;) once
// dtk/data_owners.tsv gives 0x8043C9F0-0x8043C9F8 to this unit.

inline RPBilPlayerMgr::RPBilPlayerMgr() {
    for (int i = 0; i < PLAYER_MAX; i++) {
        mpPlayers[i] = new RPBilPlayer(i);
    }

    mPlayerNum = RPBilGameMgr::sPlayerNum;
}

void RPBilPlayerMgr::CreateInstance() {
    sInstance = new RPBilPlayerMgr();
}

void RPBilPlayerMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBilPlayerMgr::~RPBilPlayerMgr() {}

void RPBilPlayerMgr::Load() {
    for (int i = 0; i < PLAYER_MAX; i++) {
        mpPlayers[i]->Load();
    }
}

void RPBilPlayerMgr::Reset() {
    mCurrentPlayer = -1;
    unkC = true;
    unkD = false;
    unkE = true;
    unkF = false;

    for (int i = 0; i < PLAYER_MAX; i++) {
        mpPlayers[i]->Reset();
    }
}

void RPBilPlayerMgr::Exit() {
    for (int i = 0; i < PLAYER_MAX; i++) {
        mpPlayers[i]->Exit();
    }
}

inline void RPBilPlayerMgr::SetWaiting(bool waiting) {
    bool prev = unkC;
    unkC = waiting;
    if (!prev && unkC) {
        unkD = true;
    }
}

void RPBilPlayerMgr::Calculate() {
    for (int i = 0; i < mPlayerNum; i++) {
        mpPlayers[i]->Calculate();
    }

    unkD = false;

    if (mCurrentPlayer >= 0 && mCurrentPlayer < mPlayerNum &&
        mpPlayers[mCurrentPlayer]->GetState() == RPBilPlayer::STATE_WAIT) {
        SetWaiting(true);
    }
}

void RPBilPlayerMgr::AssignChannels() {
    for (int i = 0; i < PLAYER_MAX; i++) {
        if (i < mPlayerNum) {
            mpPlayers[i]->SetChannel(i);
        }
    }
}

void RPBilPlayerMgr::StartTurn(s32 player, bool ballInHand) {
    unkC = false;
    unkD = false;
    mCurrentPlayer = player;

    for (int i = 0; i < mPlayerNum; i++) {
        if (i == player) {
            if (ballInHand) {
                mpPlayers[i]->ChangeState(RPBilPlayer::STATE_PLACE_BALL);
            } else {
                mpPlayers[i]->ChangeState(RPBilPlayer::STATE_AIM);
            }

            mpPlayers[i]->IncShotCount();
        } else {
            mpPlayers[i]->ChangeState(RPBilPlayer::STATE_INACTIVE);
        }
    }

    RPBilHud* pHud = RPBilHud::sInstance;
    if (mPlayerNum == 1) {
        pHud->Set1pScore(mpPlayers[0]->GetShotCount());
    } else {
        for (int i = 0; i < mPlayerNum; i++) {
            pHud->SetPlayerScore(i, mpPlayers[i]->GetScore());
        }
    }
}

void RPBilPlayerMgr::AddPlayerScore(s32 player, s32 score) {
    mpPlayers[player]->AddScore(score);
}

void RPBilPlayerMgr::IncPlayerFoulCount(s32 player) {
    mpPlayers[player]->IncFoulCount();
}

void RPBilPlayerMgr::UpdatePlayerMaxStat(s32 player, s32 value) {
    mpPlayers[player]->UpdateMaxStat(value);
}

bool RPBilPlayerMgr::IsAorBTriggered() const {
    for (int i = 0; i < mPlayerNum; i++) {
        if (EGG::CoreControllerMgr::instance()
                ->getNthController(mpPlayers[i]->GetChannel())
                ->downTrigger(EGG::cCORE_BUTTON_B) ||
            EGG::CoreControllerMgr::instance()
                ->getNthController(mpPlayers[i]->GetChannel())
                ->downTrigger(EGG::cCORE_BUTTON_A)) {
            return true;
        }
    }

    return false;
}

void RPBilPlayerMgr::SetCursorsVisible(bool visible) {
    for (int i = 0; i < mPlayerNum; i++) {
        mpPlayers[i]->SetCursorsVisible(visible);
    }
}
