#include <Pack/RPParty/RPWlyScene/RPWlyPhase_3.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyField_5.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLyt.h>
#include <Pack/RPSystem/RPSysSceneMgr.h>

RPWlyTargetPhase::~RPWlyTargetPhase() {}

void RPWlyTargetPhase::Start() {
    mStep = 1;
    mTimer = 0;
    RPWlyGameMgr::sInstance->mpBgm->PlayBgm(RPWlyGameMgr::sInstance->mStageNo - 1);
    RPWlyLayoutSet::sInstance->mpDisplay->PlayLevelIn();
    RPWlyLayoutSet::sInstance->mpTimer->PlayIn();
    RPWlyLayoutSet::sInstance->mpQuestion->ShowIn();
    RPSndAudioMgr::instance()->startSystemSe(0x7B);
}

void RPWlyTargetPhase::Init() {
    mStep = 0;
}

void RPWlyTargetPhase::Create() {}

void RPWlyTargetPhase::Reset() {
    mTimer = 0;
    mStep = 0;
}

void RPWlyTargetPhase::Calculate() {
    if (RPWlyGameMgr::sInstance->mState == 1) {
        switch (mStep) {
        case 0:
            Start();
            break;
        case 1:
            if (RPSysSceneMgr::instance()->getCurrentSceneRP()->isSceneDisplay()) {
                if (++mTimer >= 120) {
                    RPWlyLayoutSet::sInstance->mpDisplay->PlayLevelOut();
                    RPWlyLayoutSet::sInstance->mpQuestion->StartOut();
                    RPWlyGameMgr::sInstance->mpWipe->StartClose();
                    mStep = 3;
                    mTimer = 0;
                }
            }
            break;
        case 2:
            if (RPWlyLayoutSet::sInstance->mpQuestion->IsOutDone()) {
                mStep = 3;
                mTimer = 0;
            }
            break;
        case 3:
            break;
        }
    }
}

void RPWlyTargetPhase::Destroy() {}

void RPWlyTargetPhase::Draw() {}
