// Laser Hockey intro phase: start countdown and player balloons.
// Unit 0x8021FE14-0x8021FF3C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyPhase.h>

#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPHkyScene/RPHkyHud.h>
#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>

RPHkyIntroPhase::~RPHkyIntroPhase() {}

void RPHkyIntroPhase::Create() {
    mStep = 0;
}

void RPHkyIntroPhase::Load() {}

void RPHkyIntroPhase::Reset() {
    mTimer = 0;
    mStep = 0;
}

void RPHkyIntroPhase::Calculate() {
    if (RPHkyGame::GetInstance()->GetState() == 0) {
        switch (mStep) {
        case 0:
            mStep = 1;
            mTimer = 0;
            RPHkyHud::GetInstance()->GetStartMsgLyt()->Start();
            RPHkyHud::GetInstance()->GetBalloonLyt()->Show();
            break;
        case 1:
            if (RPHkyHud::GetInstance()->GetStartMsgLyt()->IsDone()) {
                mStep = 2;
                mTimer = 0;
            }
            break;
        case 2:
            break;
        }
    }
}

void RPHkyIntroPhase::Exit() {}

void RPHkyIntroPhase::Draw() {}
