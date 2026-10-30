// Laser Hockey time-up phase: "time up" message for 150 frames.
// Unit 0x802202BC-0x80220438 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyPhase.h>

#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPHkyScene/RPHkyHud.h>
#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>

RPHkyTimeUpPhase::~RPHkyTimeUpPhase() {}

void RPHkyTimeUpPhase::Start() {
    mTimer = 0;
    mStep = 1;
    RPHkyHud::GetInstance()->GetTimeUpMsgLyt()->PlayIn();
    RPHkyHud::GetInstance()->GetGoalMsgLyt()->StartResult();
}

void RPHkyTimeUpPhase::Create() {
    mStep = 0;
    mTimer = 0;
}

void RPHkyTimeUpPhase::Load() {}

void RPHkyTimeUpPhase::Reset() {
    mStep = 0;
    mTimer = 0;
}

void RPHkyTimeUpPhase::Calculate() {
    if (RPHkyGame::GetInstance()->GetState() == 3) {
        switch (mStep) {
        case 0:
            Start();
            break;
        case 1:
            if (++mTimer >= 150) {
                mStep = 2;
                mTimer = 0;
                RPHkyHud::GetInstance()->GetTimeUpMsgLyt()->PlayOut();
                RPHkyHud::GetInstance()->GetGoalMsgLyt()->PlayOut();
            }
            break;
        case 2:
            break;
        }
    }
}

void RPHkyTimeUpPhase::Exit() {}

void RPHkyTimeUpPhase::Draw() {}
