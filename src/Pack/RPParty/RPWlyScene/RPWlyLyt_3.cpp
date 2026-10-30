#include <Pack/RPParty/RPWlyScene/RPWlyLyt_3.h>

void RPWlyPointPopupSet::Init() {
    for (int i = 0; i < POPUP_NUM; ++i) {
        mpPopups[i] = new RPWlyPointPopup;
    }
    for (int i = 0; i < POPUP_NUM; ++i) {
        mpPopups[i]->Init();
    }
}

void RPWlyPointPopupSet::Create() {
    for (int i = 0; i < POPUP_NUM; ++i) {
        mpPopups[i]->Create();
    }
}

void RPWlyPointPopupSet::Reset() {
    mNext = 0;
    for (int i = 0; i < POPUP_NUM; ++i) {
        mpPopups[i]->Reset();
    }
}

void RPWlyPointPopupSet::Calculate() {
    for (int i = 0; i < POPUP_NUM; ++i) {
        mpPopups[i]->Calculate();
    }
}

void RPWlyPointPopupSet::Destroy() {
    for (int i = 0; i < POPUP_NUM; ++i) {
        mpPopups[i]->Destroy();
    }
}

void RPWlyPointPopupSet::Draw() {
    for (int i = 0; i < POPUP_NUM; ++i) {
        mpPopups[i]->Draw();
    }
}

void RPWlyPointPopupSet::ShowNext(s32 player, s32 avatarIdx, s32 points) {
    mpPopups[mNext]->Show(player, avatarIdx, points);
    ++mNext;
    if (mNext >= POPUP_NUM) {
        mNext = 0;
    }
}

bool RPWlyPointPopupSet::AreAllIdle() const {
    bool idle = true;
    for (int i = 0; i < POPUP_NUM; ++i) {
        if (mpPopups[i]->GetState() != 1) {
            idle = false;
            break;
        }
    }
    return idle;
}
