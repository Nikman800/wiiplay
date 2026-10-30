#include <Pack/RPParty/RPWlyScene/RPWlyPhase_2.h>

#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>

RPWlyIntroPhase::~RPWlyIntroPhase() {}

void RPWlyIntroPhase::Init() {
    mStep = 0;
}

void RPWlyIntroPhase::Create() {}

void RPWlyIntroPhase::Reset() {
    mTimer = 0;
    mStep = 0;
}

void RPWlyIntroPhase::Calculate() {
    if (RPWlyGameMgr::sInstance->mState != 0) {
        return;
    }

    switch (mStep) {
    case 0:
        mStep = 1;
        mTimer = 0;
        break;
    case 1:
        mStep = 2;
        mTimer = 0;
        break;
    case 2:
        break;
    }
}

void RPWlyIntroPhase::Destroy() {}

void RPWlyIntroPhase::Draw() {}
