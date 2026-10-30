#include <Pack/RPParty/RPPartyCommon/RPPauseText_2.h>

RPPauseText::~RPPauseText() {}

RPPauseText::RPPauseText() {
    mpLayout = new RPPauseTextLyt();
}

void RPPauseText::Load() {
    mpLayout->Load();
}

void RPPauseText::Reset() {
    mEnabled[0] = false;
    mEnabled[1] = false;
    mpLayout->Reset();
}

void RPPauseText::Calculate() {
    for (s32 i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
        if (mEnabled[i]) {
            mpLayout->Show(i);
        } else {
            mpLayout->Hide(i);
        }
    }

    mpLayout->Calculate();
}

void RPPauseText::Draw() {
    mpLayout->Draw();
}

void RPPauseText::SetEnabled(bool enabled) {
    mEnabled[0] = enabled;
    mEnabled[1] = enabled;
}
