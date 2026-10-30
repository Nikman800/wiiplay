#include <Pack/RPParty/RPBomScene/RPBomGameManager.h>

#include <Pack/RPParty/RPBomScene/RPBomPanel_3.h>
#include <Pack/RPParty/RPBomScene/RPBomPlayer_2.h>
#include <Pack/RPParty/RPBomScene/RPBomQuestionParam.h>
#include <Pack/RPParty/RPBomScene/RPBomScore.h>
#include <Pack/RPParty/RPBomScene/RPBomSound.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPLeadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPMedalResult.h>
#include <Pack/RPParty/RPPartyCommon/RPPauseText.h>
#include <Pack/RPParty/RPPartyCommon/RPResultMenu.h>
#include <Pack/RPParty/RPPartyCommon/RPTextWriter.h>
#include <Pack/RPParty/RPPartyCommon/RPVsWinner.h>
#include <Pack/RPKernel/RPSysEffectBase.h>
#include <egg/gfxe/eggScreen.h>

#include <Pack/RPParty/RPBomScene/RPBomBorder.h>
#include <Pack/RPParty/RPBomScene/RPBomLyt.h>
#include <Pack/RPParty/RPBomScene/RPBomStartBg.h>

// Panel control (RPBomPanel.h conflicts with RPBomPanel_3.h's RPBomPanelPool)
class RPBomPanelCtrl {
public:
    static RPBomPanelCtrl* sInstance;

    void DeactivateAll();
};

extern nw4r::math::VEC3 RP_vecZero;

// Not yet named in their own units (ledger names)
// NONMATCHING-DEBT: defined under this symbol in RPBomBorder.cpp and declared so in RPBomBorder.h (outside this pass).
extern "C" f32 Bom_getScreenHalfWidth();
// These take no argument (r4 is never set by the callers); the ledger names are guesses

namespace {

enum BomMode {
    BOM_MODE_1P,
    BOM_MODE_2P
};

// The game mode read as an enum (decides the original's register choice)
inline BomMode GetMode(const RPBomGameManager* pMgr) {
    return static_cast<BomMode>(pMgr->mMode);
}

inline bool IsPlayerReady(const RPBomPlayer* pPlayer) {
    bool ready = true;
    if (pPlayer->mState != 2 && pPlayer->mState != 3) {
        ready = false;
    }
    return ready;
}

inline bool IsReadyLytIdle(const RPBomLytReady* pLyt) {
    return pLyt->mState == 0;
}

inline f32 GetBubbleScale(const RPBomQuestionParam* pParam) {
    f32 scale;
    if (EGG::Screen::GetTVMode() == 1) {
        scale = pParam->mpFile->mBubbleScale169;
    } else {
        scale = pParam->mpFile->mBubbleScale43;
    }
    return scale;
}

inline s32 GetQuestionCost(const RPBomQuestionParam* pParam, s32 question) {
    s32 cost;
    if (GetMode(RPBomGameManager::sInstance) == BOM_MODE_1P) {
        cost = pParam->mpFile->mQuestions[question].mCost;
    } else {
        f32 scaled = pParam->mpFile->mQuestions[question].mCost;
        scaled *= pParam->mpFile->mTwoPlayerCostScale;
        cost = scaled;
    }
    return cost;
}

inline s32 GetWaveInterval(const RPBomQuestionParam* pParam, s32 question) {
    s32 interval;
    if (GetMode(RPBomGameManager::sInstance) == BOM_MODE_1P) {
        interval = pParam->mpFile->mQuestions[question].mWaveInterval;
    } else {
        f32 scaled = pParam->mpFile->mQuestions[question].mWaveInterval;
        scaled *= pParam->mpFile->mLevels[RPBomGameManager::sInstance->mLevel].mWaveIntervalScale;
        interval = scaled;
    }
    return interval;
}

inline s32 GetWaveNum(const RPBomQuestionParam* pParam, s32 question) {
    s32 num;
    if (GetMode(RPBomGameManager::sInstance) == BOM_MODE_1P) {
        num = pParam->mpFile->mQuestions[question].mWaveNum[0];
    } else {
        num = pParam->mpFile->mQuestions[question].mWaveNum[1];
    }
    return num;
}

// The all-clear burst at the origin
inline void PlayAllClearEffect(const nw4r::math::VEC3& rScale, const nw4r::math::VEC3& rPos) {
    RPSysEffect effect("Bom_all_clear", 4, 0);
    effect.create();
    effect.setPos(rPos);
    effect.setScale(rScale);
    effect.update();
}

} // namespace

inline void RPBomGameManager::UpdateBgm() {
    mpBgm->Update(RPBomPanelPool::sInstance->CountActiveInRange(),
                   RPBomStartBg::sInstance->mLevel);
}

void RPBomGameManager::StateLevelUp() {
    UpdateBgm();
    bool ready0 = IsPlayerReady(RPBomPlayerManager::sInstance->mpPlayers[0]);
    bool readyIdle = IsReadyLytIdle(RPBomLayoutMgr::sInstance->mpReady);

    switch (mMode) {
    case 0:
        if (ready0 && readyIdle) {
            RPBomStartBg::sInstance->SetFadeTargetFull();
            RPBomBorder::sInstance->StartFlash();
            mpBgm->RequestTrack(0);
            RPBomStartBg::sInstance->SetLevel(mLevel, TRUE);
            RPBomLayoutMgr::sInstance->mpClear->PlayScorePlusEffect();
            RPBomLayoutMgr::sInstance->mpLevel->mDone = false;
            RPBomLayoutMgr::sInstance->mpReady->PlayOut();
            mTimer = 0;
            mState = 8;
        }
        break;
    case 1:
        bool ready1 = IsPlayerReady(RPBomPlayerManager::sInstance->mpPlayers[1]);
        if (ready0 && ready1 && readyIdle) {
            RPBomStartBg::sInstance->SetFadeTargetFull();
            RPBomBorder::sInstance->StartFlash();
            mpBgm->RequestTrack(0);
            RPBomStartBg::sInstance->SetLevel(mLevel, TRUE);
            RPBomLayoutMgr::sInstance->mpClear->PlayScorePlusEffect();
            RPBomLayoutMgr::sInstance->mpLevel->mDone = false;
            RPBomLayoutMgr::sInstance->mpReady->PlayOut();
            mTimer = 0;
            mState = 8;
        }
        break;
    }
}

void RPBomGameManager::StateNextLevelTheme() {
    UpdateBgm();
    if (RPBomLayoutMgr::sInstance->mpLevel->mDone) {
        RPBomBorder::sInstance->mAnimating = true;
        RPBomLayoutMgr::sInstance->mpScore->PlayIn();
        mWaveNum = 0;
        mWaveTimer = 0;
        mTimer = 0;
        mPlayFrame = 0;
        mState = 3;
    } else if (++mTimer == 80) {
        RPBomPlayerManager::sInstance->RestartAllPosing2();
    }
}

void RPBomGameManager::StateGameOver() {
    UpdateBgm();
    if (RPBomLayoutMgr::sInstance->mpEnd->mDone) {
        RPBomPlayerManager::sInstance->SetAllIdle();
        switch (mMode) {
        case 0:
            RPBomLayoutMgr::sInstance->mpClear->SetClearText1P();
            RPCommon::GetInstance()->GetMedalResult()->Start(mpScore->mScores[0]);
            mState = 11;
            break;
        case 1:
            s32 winner = mpScore->GetWinner();
            RPCommon::GetInstance()->GetVsWinner()->Start(winner);
            mState = 11;
            break;
        }
    }
}

void RPBomGameManager::StateAllClear() {
    UpdateBgm();
    if (RPBomLayoutMgr::sInstance->mpEnd->mDone) {
        RPBomPlayerManager::sInstance->SetAllIdle();
        switch (mMode) {
        case 0:
            RPBomLayoutMgr::sInstance->mpClear->SetClearText1P();
            RPCommon::GetInstance()->GetMedalResult()->Start(mpScore->mScores[0]);
            mState = 11;
            break;
        case 1:
            s32 winner = mpScore->GetWinner();
            RPCommon::GetInstance()->GetVsWinner()->Start(winner);
            mState = 11;
            break;
        }
    }
}

void RPBomGameManager::StateResult() {
    UpdateBgm();
    switch (mMode) {
    case 0:
        if (RPCommon::GetInstance()->GetMedalResult()->GetState() == 4) {
            RPCommon::GetInstance()->GetResultMenu()->Start(1, -1);
            mState = 12;
        }
        break;
    case 1:
        if (RPCommon::GetInstance()->GetVsWinner()->GetState() == 5) {
            s32 winner = mpScore->GetWinner();
            RPCommon::GetInstance()->GetResultMenu()->Start(2, winner);
            mState = 12;
        } else if (RPCommon::GetInstance()->GetVsWinner()->IsStateEntered(3)) {
            RPBomLayoutMgr::sInstance->mpClear->SetClearText1P();
        }
        break;
    }
}

void RPBomGameManager::StateEnd() {
    UpdateBgm();
    switch (mMode) {
    case 0:
        if (RPCommon::GetInstance()->GetResultMenu()->GetState() == 4) {
            switch (RPCommon::GetInstance()->GetResultMenu()->GetChoice()) {
            case 0:
                mRetry = true;
                break;
            case 1:
                mQuit = true;
                break;
            }
            mpBgm->Stop();
        }
        break;
    case 1:
        if (RPCommon::GetInstance()->GetResultMenu()->GetState() == 4) {
            switch (RPCommon::GetInstance()->GetResultMenu()->GetChoice()) {
            case 0:
                mRetry = true;
                break;
            case 1:
                mQuit = true;
                break;
            }
            mpBgm->Stop();
        }
        break;
    }
}

void RPBomGameManager::Create() {
    RPBomScore::CreateInstance();
    mpScore = RPBomScore::sInstance;
    RPBomQuestionParam::CreateInstance();
    mpParam = RPBomQuestionParam::sInstance;
    mpBgm = new RPBomBgm;
}

void RPBomGameManager::Load() {
    mpParam->Load();
}

void RPBomGameManager::Reset(s32 playerNum) {
    mPlayerNum = playerNum;
    mTimer = 0;
    mPlayFrame = 0;
    mLevelTimer = 0;
    mWaitTimer = 0;
    mMissTimer = 0;
    mHideLeadCursor = false;
    mSpawning = true;
    mWaveNum = 0;
    mWaveTimer = 0;
    mMissCount = 0;
    mMissMax = mpParam->mpFile->mMissMax;
    mBubbleResult = -1;
    mMissPlayer = -1;
    mForcedMiss = false;
    mMissPending = false;
    mBubbleResultPending = false;
    unk3A = false;
    mMissAllRequest = false;
    mNoMiss[0] = true;
    mNoMiss[1] = true;
    mQuestion = 0;
    mQuestionMax = mpParam->mQuestionNum;
    mLevel = 0;
    mLevelMax = GetLevelNum() - 1;
    if (playerNum == 1) {
        mMode = 0;
    } else if (playerNum == 2) {
        mMode = 1;
    }
    mpScore->Reset(playerNum);
    mpParam->Reset();
    mpBgm->Stop();
    mState = 0;
    mRetry = false;
    mQuit = false;
}

void RPBomGameManager::Start() {
    RPCommon::GetInstance()->GetLeadCursor()->SetTimers(15);
    mHideLeadCursor = false;
    StartGame();
}

void RPBomGameManager::Calculate() {
    (this->*mStateFuncs[mState])();
    RPCommon::GetInstance()->GetLeadCursor()->SetAllEnabled(mHideLeadCursor);
}

void RPBomGameManager::DrawAfterScreenFx() {}

void RPBomGameManager::DrawAfterLayouts() {
    f32 halfWidth = static_cast<u16>(static_cast<u32>(EGG::Screen::GetSizeXMax()) >> 1);
    f32 halfHeight = static_cast<u16>(static_cast<u32>(EGG::Screen::GetSizeYMax()) >> 1);
    RPTextWriter writer;
    writer.Begin(4);
    writer.SetDrawFlag(0x110);
    writer.End();
}

void RPBomGameManager::Exit() {
    RPBomScore::DestroyInstance();
    RPBomQuestionParam::DestroyInstance();
}

BOOL RPBomGameManager::TryAdvanceQuestion() {
    if (mQuestion == mQuestionMax) {
        return FALSE;
    }

    s32 cost = GetQuestionCost(RPBomQuestionParam::sInstance, mQuestion + 1);
    if (mMode == 0) {
        if (RPBomScore::sInstance->mTotals[0] >= cost) {
            mQuestion++;
            RPBomScore::sInstance->AddTotal(0, -cost);
            return TRUE;
        }
    } else if (mMode == 1) {
        if (RPBomScore::sInstance->mTotals[0] + RPBomScore::sInstance->mTotals[1] >= cost) {
            mQuestion++;
            s32 half = -(cost / 2);
            RPBomScore::sInstance->AddTotal(0, half);
            RPBomScore::sInstance->AddTotal(1, half);
            return TRUE;
        }
    }
    return FALSE;
}

void RPBomGameManager::SetPlayerMiss(s32 player) {
    mMissPending = true;
    mMissPlayer = player;
    mQuestion = mpParam->mpFile->mQuestions[mQuestion].mMissNext;
    mpParam->SetQuestion(mQuestion);
    mWaitTimer = mpParam->mpFile->mMissWait;
}

void RPBomGameManager::SetForcedMiss() {
    mMissPending = true;
    mForcedMiss = true;
    mWaitTimer = mpParam->mpFile->mMissWait;
}

void RPBomGameManager::SetBubbleResult(s32 player) {
    mBubbleResult = player;
    mBubbleResultPending = true;
}

void RPBomGameManager::PlaySpecialJingle() {
    mpBgm->RequestTrack(1);
}

// The original's .sdata2 has this constant after the first functions' literal pool and before
// the state functions' constants: the functions above are emitted (in reverse order) before
// it, the ones below in definition order.
#pragma defer_codegen off

const s32 RPBomGameManager::sLevelNum = 16;

void RPBomGameManager::StateLevelEnd() {
    UpdateBgm();
    if (!RPBomLayoutMgr::sInstance->mpClear->mDone) {
        return;
    }

    if (mLevel == mLevelMax) {
        RPBomStartBg::sInstance->SetFadeTargetFull();
        RPBomPlayerManager::sInstance->SetAllStateResult();
        RPBomPanelCtrl::sInstance->DeactivateAll();
        RPBomBorder::sInstance->mState = 5;
        PlayAllClearEffect(nw4r::math::VEC3(1.0f, 1.0f, 1.0f), nw4r::math::VEC3(0.0f, 0.0f, 0.0f));
        mHideLeadCursor = false;
        mPlayFrame = 0;
        RPBomLayoutMgr::sInstance->mpClear->SetBonusText2P();
        RPBomLayoutMgr::sInstance->mpEnd->Show(TRUE);
        if (mMode == 0) {
            RPCommon::GetInstance()->GetMedalResult()->GetRankForScore(
                RPBomScore::sInstance->mScores[0]);
            RPBomPlayerManager::sInstance->PlayResultAnims(1);
        } else if (mMode == 1) {
            switch (mpScore->GetWinner()) {
            case -1:
                RPBomPlayerManager::sInstance->PlayResultAnims(2);
                break;
            case 0:
                RPBomPlayerManager::sInstance->PlayResultAnims(0);
                break;
            case 1:
                RPBomPlayerManager::sInstance->PlayResultAnims(1);
                break;
            }
        }
        mTimer = 180;
        mState = 10;
    } else {
    mLevel++;
    mQuestion = mpParam->mpFile->mLevels[mLevel].mStartQuestion;
    mLevelTimer = mpParam->mpFile->mLevels[mLevel].mTime * 60;
    mpParam->SetQuestion(mQuestion);
    mSpawning = true;
    RPBomScore::sInstance->ClearTotals();
    mNoMiss[0] = true;
    mNoMiss[1] = true;
    RPBomPlayerManager::sInstance->RestartAllPosing2();
    RPBomPanelCtrl::sInstance->DeactivateAll();

    RPBomQuestionParam* param = mpParam;
    f32 scale = GetBubbleScale(param);
    RPBomSpawnInfo info;
    info.mAngle = 0.0f;
    info.mVel = RP_vecZero;
    info.mRotSpeed = 0.0f;
    info.mScale = scale;
    info.mOrbitRadius = 0.0f;
    info.mOrbitStep = 0.0f;
    s32 pose = param->PickRandomPose();
    mpParam->mForcedPose = pose;

    if (mMode == 0) {
        nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
        u32 flags = 0x80000000;
        flags |= pose;
        RPBomPanelPool::sInstance->SpawnPanel(pos, flags, info);
    } else if (mMode == 1) {
        nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
        pos.x = Bom_getScreenHalfWidth() * -0.4f;
        RPBomPanelPool::sInstance->SpawnPanel(pos, pose | 0x80000000, info);
        pos.x = Bom_getScreenHalfWidth() * 0.4f;
        RPBomPanelPool::sInstance->SpawnPanel(pos, pose | 0x80000000 | 0x10, info);
    }

    RPBomLayoutMgr::sInstance->mpPoseSel[0]->Show();
    if (mMode == 1) {
        RPBomLayoutMgr::sInstance->mpPoseSel[1]->Show();
    }
    RPBomLayoutMgr::sInstance->mpReady->Show();
    mState = 7;
    }
}

void RPBomGameManager::StateWaitBubbles() {
    UpdateBgm();
    mpBgm->HoldJingle(0x10);
    s32 count = RPBomPanelPool::sInstance->CountActiveInRange();
    if (--mWaitTimer > 0 && count != 0) {
        return;
    }

    if (mMissCount == 0) {
        mpBgm->RequestTrack(8);
    } else if (mMissCount == 1) {
        mpBgm->RequestTrack(8);
    } else if (mMissCount == 2) {
        mpBgm->RequestTrack(9);
    }
    mBubbleResult = -1;
    mWaitTimer = 0;
    mState = 3;
}

void RPBomGameManager::StateMiss() {
    bool gameOver = false;
    UpdateBgm();
    if (RPBomPanelPool::sInstance->CountActiveInRange() == 0) {
        if (mMode == 1 && mMissPlayer != -1) {
            RPBomScore::sInstance->ApplyMissPenalty(mMissPlayer);
            mMissPlayer = -1;
        }

        if (--mWaitTimer <= 0) {
            if (mMissCount == mMissMax) {
                gameOver = true;
            } else {
                if (mMissCount == 1) {
                    mpBgm->RequestTrack(8);
                } else if (mMissCount == 2) {
                    mpBgm->RequestTrack(9);
                }
                RPBomScore::sInstance->ClearTotals();
                RPBomPlayerManager::sInstance->RestartAllPosing();
                mWaveNum = 0;
                mWaveTimer = 0;
                mState = 3;
            }
        }
    } else if (--mMissTimer <= 0) {
        RPBomPanelPool::sInstance->MissFrontPanel();
        if (RPBomPanelPool::sInstance->CountActiveInRange() == 0) {
            mMissTimer = 60;
        } else {
            mMissTimer = 5;
        }
    }

    if (gameOver) {
        RPBomPlayerManager::sInstance->SetAllStateResult();
        RPBomPanelCtrl::sInstance->DeactivateAll();
        mHideLeadCursor = false;
        mPlayFrame = 0;
        if (RPBomLayoutMgr::sInstance->mpStart->mState == 4) {
            RPBomLayoutMgr::sInstance->mpStart->PlayThemeOut();
        }
        RPBomLayoutMgr::sInstance->mpPoseSel[0]->PlayOut();
        if (mMode == 1) {
            RPBomLayoutMgr::sInstance->mpPoseSel[1]->PlayOut();
        }
        RPBomLayoutMgr::sInstance->mpEnd->Show(FALSE);
        RPBomLayoutMgr::sInstance->mpScore->PlayOut();
        RPBomLayoutMgr::sInstance->mpClear->SetClearText2P();
        if (mMode == 0) {
            RPCommon::GetInstance()->GetMedalResult()->GetRankForScore(
                RPBomScore::sInstance->mScores[0]);
        } else if (mMode == 1) {
            switch (mpScore->GetWinner()) {
            case -1:
                RPBomPlayerManager::sInstance->PlayResultAnims(2);
                break;
            case 0:
                RPBomPlayerManager::sInstance->PlayResultAnims(0);
                break;
            case 1:
                RPBomPlayerManager::sInstance->PlayResultAnims(1);
                break;
            }
        }
        mTimer = 180;
        mState = 9;
    }
}

void RPBomGameManager::StatePlay() {
    UpdateBgm();
    if (mMissPending) {
        mMissPending = false;
        if (mForcedMiss) {
            mMissCount = mMissMax;
        } else {
            mMissCount++;
        }

        if (mMissCount == 1) {
            mpBgm->RequestTrack(4);
        } else if (mMissCount == 2) {
            mpBgm->RequestTrack(5);
        } else if (mMissCount == mMissMax) {
            if (mMode == 0) {
                if (RPCommon::GetInstance()->GetMedalResult()->GetRankForScore(
                        RPBomScore::sInstance->mScores[0]) != -1) {
                    mpBgm->RequestTrack(6);
                } else {
                    mpBgm->RequestTrack(7);
                }
            } else if (mMode == 1) {
                mpBgm->RequestTrack(6);
            }
        }

        if (mMissPlayer == -1) {
            mNoMiss[0] = false;
            mNoMiss[1] = false;
        } else {
            mNoMiss[mMissPlayer] = false;
        }

        RPBomLayoutMgr::sInstance->mpMiss->Show(mMissPlayer);
        mMissTimer = 25;
        mWaitTimer = mpParam->mpFile->mMissWait;
        mState = 4;
        return;
    }

    s32 count = RPBomPanelPool::sInstance->CountActiveInRange();
    if (mBubbleResultPending) {
        mBubbleResultPending = false;
        if (count != 0) {
            mWaitTimer = mpParam->mpFile->mBubbleWait;
            mpBgm->RequestTrack(2);
            mState = 5;
            return;
        }

        mWaitTimer = 0;
        if (mMissCount == 0) {
            mpBgm->RequestTrack(8);
        } else if (mMissCount == 1) {
            mpBgm->RequestTrack(8);
        } else if (mMissCount == 2) {
            mpBgm->RequestTrack(9);
        }
        mBubbleResult = -1;
    }

    if (mMissAllRequest) {
        RPBomPanelPool::sInstance->MissAll();
        mMissAllRequest = false;
    }

    s32 interval = GetWaveInterval(RPBomQuestionParam::sInstance, mQuestion);
    if (mSpawning) {
        if (count == 0) {
            mWaveTimer = 0;
            mWaveNum = GetWaveNum(mpParam, mQuestion);
        } else if (++mWaveTimer >= interval) {
            mWaveTimer = 0;
            mWaveNum = GetWaveNum(mpParam, mQuestion);
        }

        if (mWaveNum > 0) {
            mpParam->NextPattern(mQuestion);
            if (mpParam->GenerateWave()) {
                mpParam->SpawnBubbles();
            }
            mWaveNum--;
        }

        if (--mLevelTimer <= 0) {
            mLevelTimer = 0;
            mSpawning = false;
        }
    } else if (count == 0) {
        RPBomStartBg::sInstance->SetFadeTargetDim();
        if (RPBomLayoutMgr::sInstance->mpStart->mState == 4) {
            RPBomLayoutMgr::sInstance->mpStart->PlayThemeOut();
        }
        RPBomLayoutMgr::sInstance->mpPoseSel[0]->PlayOut();
        if (mMode == 1) {
            RPBomLayoutMgr::sInstance->mpPoseSel[1]->PlayOut();
        }
        RPBomLayoutMgr::sInstance->mpScore->PlayOut();
        RPBomLayoutMgr::sInstance->mpClear->UpdateStates();
        if (mLevel == mLevelMax) {
            mpBgm->RequestTrack(10);
        } else {
            mpBgm->RequestTrack(3);
        }
        if (mMode == 0 && mLevel == 0) {
            RPCommon::GetInstance()->GetPauseText()->SetEnabled(0, false);
        }
        RPBomPlayerManager::sInstance->SetPlayerStateWin(0, mNoMiss[0]);
        if (mMode == 1) {
            RPBomPlayerManager::sInstance->SetPlayerStateWin(1, mNoMiss[1]);
        }
        mTimer = 180;
        mState = 6;
        return;
    }

    mpParam->UpdateSpeed(mQuestion);
    mPlayFrame++;
}

void RPBomGameManager::StateShowTheme() {
    UpdateBgm();
    if (RPBomLayoutMgr::sInstance->mpLevel->mDone) {
        RPBomLayoutMgr::sInstance->mpScore->PlayIn();
        mWaveNum = 0;
        mWaveTimer = 0;
        mTimer = 0;
        mPlayFrame = 0;
        mState = 3;
    } else if (++mTimer == 80) {
        f32 scale = GetBubbleScale(mpParam);
        RPBomPlayerManager::sInstance->StartAllPosing(scale, mpParam->mPoses[0]);
    }
}

void RPBomGameManager::StateWaitReady() {
    bool ready0 = IsPlayerReady(RPBomPlayerManager::sInstance->mpPlayers[0]);
    bool titleDone = RPBomLayoutMgr::sInstance->mpStart->mState == 2;

    switch (mMode) {
    case 0:
        if (ready0 && titleDone) {
            RPBomBorder::sInstance->StartFlash();
            RPBomStartBg::sInstance->PlayWipe();
            RPBomLayoutMgr::sInstance->mpStart->ShowTheme();
            mpBgm->RequestTrack(0);
            if (mMode == 0 && mLevel == 0) {
                RPCommon::GetInstance()->GetPauseText()->SetEnabled(0, true);
            }
            mTimer = 0;
            mState = 2;
        }
        break;
    case 1:
        bool ready1 = IsPlayerReady(RPBomPlayerManager::sInstance->mpPlayers[1]);
        if (ready0 && ready1 && titleDone) {
            RPBomBorder::sInstance->StartFlash();
            RPBomStartBg::sInstance->PlayWipe();
            RPBomLayoutMgr::sInstance->mpStart->ShowTheme();
            mpBgm->RequestTrack(0);
            mTimer = 0;
            mState = 2;
        }
        break;
    }
}

void RPBomGameManager::StateIntro() {
    if (RPBomBorder::sInstance->mState != 3) {
        return;
    }

    RPBomQuestionParam* param = mpParam;
    f32 scale = GetBubbleScale(param);
    RPBomSpawnInfo info;
    info.mAngle = 0.0f;
    info.mVel = RP_vecZero;
    info.mRotSpeed = 0.0f;
    info.mScale = scale;
    info.mOrbitRadius = 0.0f;
    info.mOrbitStep = 0.0f;
    s32 pose = param->mPoses[0];

    if (mMode == 0) {
        nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
        u32 flags = 0x80000000;
        flags |= pose;
        RPBomPanelPool::sInstance->SpawnPanel(pos, flags, info);
    } else if (mMode == 1) {
        nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
        pos.x = Bom_getScreenHalfWidth() * -0.4f;
        RPBomPanelPool::sInstance->SpawnPanel(pos, pose | 0x80000000, info);
        pos.x = Bom_getScreenHalfWidth() * 0.4f;
        RPBomPanelPool::sInstance->SpawnPanel(pos, pose | 0x80000000 | 0x10, info);
    }

    RPBomLayoutMgr::sInstance->mpPoseSel[0]->Show();
    if (mMode == 1) {
        RPBomLayoutMgr::sInstance->mpPoseSel[1]->Show();
    }
    RPBomLayoutMgr::sInstance->mpStart->ShowTitle();
    mHideLeadCursor = true;
    mState = 1;
}

void RPBomGameManager::StartGame() {
    mTimer = 0;
    mPlayFrame = 0;
    mLevelTimer = 0;
    mWaitTimer = 0;
    mMissTimer = 0;
    mSpawning = true;
    mWaveNum = 0;
    mWaveTimer = 0;
    mMissCount = 0;
    mMissMax = mpParam->mpFile->mMissMax;
    mBubbleResult = -1;
    mMissPlayer = -1;
    mForcedMiss = false;
    mMissPending = false;
    mBubbleResultPending = false;
    unk3A = false;
    mMissAllRequest = false;
    mNoMiss[0] = true;
    mNoMiss[1] = true;
    mpScore->ResetAll();

    mLevel = 0;
    mQuestion = mpParam->mpFile->mLevels[mLevel].mStartQuestion;
    mQuestionMax = mpParam->mQuestionNum;
    mLevelMax = GetLevelNum() - 1;
    mLevelTimer = mpParam->mpFile->mLevels[mLevel].mTime * 60;
    RPBomStartBg::sInstance->SetLevel(0, FALSE);
    mpParam->SetQuestion(mQuestion);

    f32 scale = GetBubbleScale(mpParam);
    s32 pose = mpParam->mPoses[1];
    RPBomPlayerManager::sInstance->StartAllPosing(scale, pose);
    RPBomLayoutMgr::sInstance->mpPoseSel[0]->SetPose(pose);
    if (mMode == 1) {
        RPBomLayoutMgr::sInstance->mpPoseSel[1]->SetPose(pose);
    }

    RPBomPanelCtrl::sInstance->DeactivateAll();
    RPBomBorder::sInstance->StartIntro();
    mState = 0;
}

RPBomGameManager::~RPBomGameManager() {}

RPBomGameManager::RPBomGameManager() {
    mStateFuncs[0] = &RPBomGameManager::StateIntro;
    mStateFuncs[1] = &RPBomGameManager::StateWaitReady;
    mStateFuncs[2] = &RPBomGameManager::StateShowTheme;
    mStateFuncs[3] = &RPBomGameManager::StatePlay;
    mStateFuncs[4] = &RPBomGameManager::StateMiss;
    mStateFuncs[5] = &RPBomGameManager::StateWaitBubbles;
    mStateFuncs[6] = &RPBomGameManager::StateLevelEnd;
    mStateFuncs[7] = &RPBomGameManager::StateLevelUp;
    mStateFuncs[8] = &RPBomGameManager::StateNextLevelTheme;
    mStateFuncs[9] = &RPBomGameManager::StateGameOver;
    mStateFuncs[10] = &RPBomGameManager::StateAllClear;
    mStateFuncs[11] = &RPBomGameManager::StateResult;
    mStateFuncs[12] = &RPBomGameManager::StateEnd;
}

void RPBomGameManager::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

void RPBomGameManager::CreateInstance() {
    sInstance = new RPBomGameManager;
}
