// Laser Hockey wait phase: a fixed pause (sFrames) before the next state.
// Unit 0x8021FF3C-0x80220034 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyPhase.h>

s32 RPHkyWaitPhase::sFrames = 80;

RPHkyWaitPhase::~RPHkyWaitPhase() {}

void RPHkyWaitPhase::Start() {
    mTimer = 0;
    mStep = 1;
}

void RPHkyWaitPhase::Create() {
    mStep = 0;
    mTimer = 0;
}

void RPHkyWaitPhase::Load() {}

void RPHkyWaitPhase::Reset() {
    mStep = 0;
    mTimer = 0;
}

void RPHkyWaitPhase::Calculate() {
    switch (mStep) {
    case 0:
        Start();
        break;
    case 1:
        if (++mTimer >= sFrames) {
            mStep = 2;
            mTimer = 0;
        }
        break;
    case 2:
        break;
    }
}

void RPHkyWaitPhase::Exit() {}

void RPHkyWaitPhase::Draw() {}
