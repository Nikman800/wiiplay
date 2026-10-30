// Table Tennis ready phase, and the rally BGM handle shared by the game, ball and result phase.
// Unit 0x8022C6A0-0x8022C8D0 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpPhase.h>

#include <Pack/RPAudio.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>

nw4r::snd::SoundHandle RPPnpReadyPhase::sBgmHandle;

void RPPnpReadyPhase::StartBgm(BOOL multiPlayer) {
    sBgmHandle.Stop(0);
    RPSndAudioMgr::instance()->startSound(&sBgmHandle, (u32)(multiPlayer ? 14 : 13));
}

bool RPPnpReadyPhase::FadeOutBgm() {
    if (sBgmHandle.IsAttachedSound()) {
        sBgmHandle.Stop(60);
        return true;
    }

    return false;
}

RPPnpReadyPhase::~RPPnpReadyPhase() {}

void RPPnpReadyPhase::Create() {
    mStep = 1;
}

void RPPnpReadyPhase::Load() {}

void RPPnpReadyPhase::Reset() {
    mTimer = 0;
}

void RPPnpReadyPhase::Calculate() {
    if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_READY) {
        switch (mStep) {
        case 0:
            mStep = 3;
            mTimer = 0;
            break;
        case 1:
            mStep = 2;
            mTimer = 0;
            break;
        case 2:
            if (++mTimer >= 90) {
                mStep = 5;
                mTimer = 0;
            }
            break;
        case 3:
            mStep = 4;
            mTimer = 0;
            break;
        case 4:
            if (++mTimer >= 40) {
                mStep = 5;
                mTimer = 0;
            }
            break;
        case 5:
            break;
        }
    }
}

void RPPnpReadyPhase::Exit() {}

void RPPnpReadyPhase::Draw() {}
