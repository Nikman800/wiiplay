// Fishing player manager (RPFshPlayerMgr): the two players, the "too far" message while
// waiting for a bite and the start of the result screen.
// Unit 0x80208BA4-0x80209118 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshPlayerMgr.h>

#include <Pack/RPParty/RPFshScene/RPFshFishMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshGame.h>
#include <Pack/RPParty/RPFshScene/RPFshMsg.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayer.h>
#include <Pack/RPParty/RPFshScene/RPFshResultLayout.h>
#include <Pack/RPParty/RPFshScene/RPFshScoreLayout.h>
#include <Pack/RPParty/RPFshScene/RPFshStartLayout.h>

f32 RPFshPlayerMgr::sMsgOpenDist = 2.7f;
f32 RPFshPlayerMgr::sMsgCloseDist = 2.5f;

RPFshPlayerMgr* RPFshPlayerMgr::sInstance;

inline RPFshPlayerMgr::RPFshPlayerMgr() {
    mPlayerNum = RPFshGame::GetPlayerNum();
    for (int i = 0; i < PLAYER_MAX; i++) {
        mpPlayers[i] = new RPFshPlayer(i);
    }
}

void RPFshPlayerMgr::CreateInstance() {
    sInstance = new RPFshPlayerMgr();
}

void RPFshPlayerMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshPlayerMgr::~RPFshPlayerMgr() {}

void RPFshPlayerMgr::CreateAll() {
    for (int i = 0; i < PLAYER_MAX; i++) {
        mpPlayers[i]->Create();
    }
}

void RPFshPlayerMgr::Reset() {
    mCatchingPlayer = -1;
    mResultRequested = false;
    mIsMsgOpen = false;

    for (int i = 0; i < PLAYER_MAX; i++) {
        mpPlayers[i]->Reset();
    }
}

void RPFshPlayerMgr::ExitAll() {
    for (int i = 0; i < mPlayerNum; i++) {
        mpPlayers[i]->Exit();
    }
}

void RPFshPlayerMgr::Calculate() {
    RPFshResultLayout* pResult = RPFshResultLayout::GetInstance();
    s32 resultState = pResult->GetState();
    if (pResult->IsShowing()) {
        return;
    }

    if (resultState == RPFshResultLayout::STATE_NONE && mResultRequested) {
        pResult->Start();
    }

    for (int i = 0; i < mPlayerNum; i++) {
        mpPlayers[i]->Calculate();
    }

    bool far = false;
    if (RPFshStartLayout::GetInstance()->GetState() == RPFshStartLayout::STATE_PLAY) {
        for (int i = 0; i < mPlayerNum; i++) {
            RPFshPlayer* pPlayer = mpPlayers[i];
            if (pPlayer->GetState() == RPFshPlayer::STATE_WAIT) {
                if (mIsMsgOpen) {
                    if (pPlayer->GetRemoteDist() > sMsgCloseDist) {
                        far = true;
                    }
                } else if (pPlayer->GetRemoteDist() > sMsgOpenDist) {
                    far = true;
                }
            }
        }

        if (far) {
            RPFshMsg01::GetInstance()->Open();
            mIsMsgOpen = true;
        } else {
            RPFshMsg01::GetInstance()->Close();
            mIsMsgOpen = false;
        }
    }
}

void RPFshPlayerMgr::CalculateRods() {
    if (RPFshResultLayout::GetInstance()->IsShowing()) {
        return;
    }

    for (int i = 0; i < mPlayerNum; i++) {
        mpPlayers[i]->UpdateRod();
    }

    if (RPFshFishMgr::GetInstance()->GetUnk10() == -1 && mCatchingPlayer != -1) {
        mCatchingPlayer = -1;
    }

    mResultRequested = true;
    RPFshPlayer* pPlayer;
    for (int i = 0; i < mPlayerNum; i++) {
        pPlayer = mpPlayers[i];
        if (pPlayer->IsChanging() || pPlayer->GetState() != RPFshPlayer::STATE_IDLE) {
            mResultRequested = false;
        }
    }

    if (mResultRequested) {
        RPFshScoreLayout::GetInstance()->RequestHideAll();
    }
}

void RPFshPlayerMgr::InitPlayers() {
    for (int i = 0; i < mPlayerNum; i++) {
        mpPlayers[i]->Init(0, i);
    }
}

void RPFshPlayerMgr::SetCatchingPlayer(s32 player) {
    for (int i = 0; i < mPlayerNum; i++) {
        RPFshPlayer* pPlayer = GetInstance()->GetPlayer(i);
        if (pPlayer->GetState() != RPFshPlayer::STATE_NONE) {
            if (i == player) {
                pPlayer->ChangeState(RPFshPlayer::STATE_LAND);
            } else {
                pPlayer->ChangeState(RPFshPlayer::STATE_RETURN);
            }
        }
    }
}
