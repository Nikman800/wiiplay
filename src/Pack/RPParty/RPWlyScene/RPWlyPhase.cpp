#include <Pack/RPParty/RPWlyScene/RPWlyPhase.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPMedalResult.h>
#include <Pack/RPParty/RPPartyCommon/RPResultMenu.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPVsWinner.h>
#include <Pack/RPParty/RPWlyScene/RPWlyAvatarMgr.h>

// The phases directly share the game and layout state. Keep the shared headers'
// declarations and layouts unchanged while exposing that state to this unit.
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLyt.h>

inline s32 GetResultScore(s32 player) {
    return RPWlyGameMgr::sInstance->mScores[player];
}

RPWlyResultPhase::~RPWlyResultPhase() {}

void RPWlyResultPhase::Start() {
    mStep = 1;
    mTimer = 0;
    RPSndAudioMgr::instance()->startStream(0x78);
}

void RPWlyResultPhase::CalcTally() {
    RPWlyAvatarMgr::sInstance->HideNonTargets();

    s32 wait = 180;
    if (RPWlyGameMgr::sInstance->mMode == 0) {
        wait = 120;
    }

    if (++mTimer >= wait) {
        mTimer = 0;
        s32 score0 = GetResultScore(0);
        s32 score1 = GetResultScore(1);
        mTallyIdx = -1;
        if (score0 > score1) {
            mTallyIdx = 0;
        } else if (score0 < score1) {
            mTallyIdx = 1;
        }

        if (RPWlyGameMgr::sInstance->mMode == 0) {
            mStep = 3;
            if (RPSceneInfo::spInstance->GetUnk1C() == 100) {
                RPWlyLayoutSet::sInstance->mpComplete->ShowAllClear();
            } else {
                RPWlyLayoutSet::sInstance->mpEndScore->Show();
            }
            RPWlyLayoutSet::sInstance->mpDisplay->StartScoreResult();
        } else if (RPSceneInfo::spInstance->GetUnk1C() == 100) {
            mStep = 4;
            RPWlyLayoutSet::sInstance->mpComplete->ShowAllClear();
        } else {
            mStep = 6;
            RPCommon::GetInstance()->GetVsWinner()->Start(mTallyIdx);
            RPWlyLayoutSet::sInstance->mpTimer->PlayOut();
            RPWlyLayoutSet::sInstance->mpDisplayEnd->PlayOut();
        }
        RPWlyAvatarMgr::sInstance->HideAll();
    }
}

void RPWlyResultPhase::Init() {}

void RPWlyResultPhase::Create() {}

void RPWlyResultPhase::Reset() {
    mStep = 0;
    mTimer = 0;
    mTallyIdx = -1;
    unk50 = true;
}

void RPWlyResultPhase::Calculate() {
    if (RPWlyGameMgr::sInstance->mState == 4) {
        switch (mStep) {
        case 0:
            mStep = 1;
            mTimer = 0;
            RPSndAudioMgr::instance()->startStream(0x78);
            break;
        case 1:
            if (!RPWlyAvatarMgr::sInstance->IsAnyWrongReacting() &&
                RPWlyLayoutSet::sInstance->mpDisplay->mState == 1) {
                mStep = 2;
                mTimer = 0;
                RPWlyLayoutSet::sInstance->mpDisplay->PlayWinnerScore();
            }
            break;
        case 2:
            CalcTally();
            break;
        case 3: {
            s32 wait = 180;
            if (RPSceneInfo::spInstance->GetUnk1C() == 100) {
                wait = 360;
            }
            if (mTimer++ > wait) {
                if (RPSceneInfo::spInstance->GetUnk1C() == 100) {
                    RPWlyLayoutSet::sInstance->mpComplete->PlayOut();
                } else {
                    RPWlyLayoutSet::sInstance->mpEndScore->PlayOut();
                    RPWlyLayoutSet::sInstance->mpDisplayEnd->PlayOut();
                }
                RPWlyLayoutSet::sInstance->mpTimer->PlayOut();
                mStep = 5;
                mTimer = 0;
                RPCommon::GetInstance()->GetMedalResult()->Start(
                    RPSceneInfo::spInstance->GetUnk1C() - 1);
            }
            break;
        }
        case 4: {
            s32 wait = 180;
            if (RPSceneInfo::spInstance->GetUnk1C() == 100) {
                wait = 360;
            }
            if (mTimer++ > wait) {
                mStep = 6;
                mTimer = 0;
                RPCommon::GetInstance()->GetVsWinner()->Start(mTallyIdx);
                if (RPSceneInfo::spInstance->GetUnk1C() == 100) {
                    RPWlyLayoutSet::sInstance->mpComplete->PlayOut();
                } else {
                    RPWlyLayoutSet::sInstance->mpDisplayEnd->PlayOut();
                }
                RPWlyLayoutSet::sInstance->mpTimer->PlayOut();
            }
            break;
        }
        case 5:
            if (RPCommon::GetInstance()->GetMedalResult()->GetState() == 4) {
                mStep = 7;
                mTimer = 0;
                RPCommon::GetInstance()->GetResultMenu()->Start(1, -1);
            }
            break;
        case 6:
            if (RPCommon::GetInstance()->GetVsWinner()->GetState() == 5) {
                mStep = 7;
                mTimer = 0;
                RPCommon::GetInstance()->GetResultMenu()->Start(2, mTallyIdx);
                RPWlyLayoutSet::sInstance->mpDisplay->StartScoreResult();
            }
            break;
        case 7:
            if (RPCommon::GetInstance()->GetResultMenu()->GetState() == 4) {
                if (RPCommon::GetInstance()->GetResultMenu()->GetChoice() == 0) {
                    unk50 = true;
                } else {
                    unk50 = false;
                }
                mStep = 8;
                mTimer = 0;
            }
            break;
        case 8:
            break;
        }
    }
}

void RPWlyResultPhase::Destroy() {}

void RPWlyResultPhase::Draw() {}
