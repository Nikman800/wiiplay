// Laser Hockey goal phase: "Goal!" message, then a short pause.
// Unit 0x80220438-0x8022061C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyPhase.h>

#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPHkyScene/RPHkyHud.h>
#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>
#include <Pack/RPParty/RPHkyScene/RPHkyPuck.h>

RPHkyGoalPhase::~RPHkyGoalPhase() {}

void RPHkyGoalPhase::Start() {
    mTimer = 0;
    mStep = 1;
    nw4r::math::VEC3 pos = RPHkyGame::GetInstance()->GetPuck()->GetPos();
    RPHkyHud::GetInstance()->GetGoalMsgLyt()->ShowGoal(
        pos.x > 0.0f ? RPHkyGoalMsgLyt::SIDE_LEFT : RPHkyGoalMsgLyt::SIDE_RIGHT);
}

void RPHkyGoalPhase::Create() {
    mStep = 0;
    mTimer = 0;
}

void RPHkyGoalPhase::Load() {}

void RPHkyGoalPhase::Reset() {
    mStep = 0;
    mTimer = 0;
}

void RPHkyGoalPhase::Calculate() {
    if (RPHkyGame::GetInstance()->GetState() == 2) {
        switch (mStep) {
        case 0:
            Start();
            break;
        case 1:
            if (RPHkyHud::GetInstance()->GetGoalMsgLyt()->IsOutDone()) {
                if (++mTimer >= 40) {
                    mStep = 2;
                    mTimer = 0;
                }
            }
            break;
        case 2:
            break;
        }
    }
}

void RPHkyGoalPhase::Exit() {}

void RPHkyGoalPhase::Draw() {}
