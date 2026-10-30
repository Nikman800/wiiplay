// Shooting Range HUD (RPDucHud): score, start and end layouts and the ring of 32 point popups.
// Unit 0x8024B20C-0x8024B8D0 (dtk/game_units.tsv). Functions in reverse address order.
// The unit's first function (highest address) is RPDucCanMgr::SpawnTarget, the last function of
// the can manager file that follows.
#include <Pack/RPParty/RPDucScene/RPDucHud.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>
#include <Pack/RPSystem.h>


RPDucHud* RPDucHud::sInstance;

void RPDucHud::CreateInstance() {
    sInstance = new RPDucHud();
}

void RPDucHud::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucHud::~RPDucHud() {}

void RPDucHud::Create() {
    mpScoreLayout = new RPDucScoreLayout();
    mpStartLayout = new RPDucStartLayout();
    mpEndLayout = new RPDucEndLayout();
    for (u32 i = 0; i < 32; i++) {
        mpPopups[i] = new RPDucPointPopup();
    }
    mPopupNum = 0;
    mPopupNext = 0;
}

void RPDucHud::Load() {
    void* pMsgRes = RPSysResourceManager::GetMessageResource("party_message.bmg");
    mpMessage = new RPSysMessage(pMsgRes, NULL);

    EGG::Archive* pArchive = RPSysResourceManager::LoadGameLocalArchive();
    mpScoreLayout->Load(pArchive);
    mpStartLayout->Load(pArchive);
    mpEndLayout->Load(pArchive);
    for (u32 i = 0; i < 32; i++) {
        mpPopups[i]->Load(pArchive);
    }
}

void RPDucHud::Reset() {
    mpScoreLayout->Reset();
    mpStartLayout->Reset();
    mpEndLayout->Reset();
    for (u32 i = 0; i < 32; i++) {
        mpPopups[i]->Reset();
    }
    mPopupNum = 0;
    mPopupNext = 0;
}

void RPDucHud::Calculate() {
    mpScoreLayout->Calculate();
    mpStartLayout->Calculate();
    mpEndLayout->Calculate();

    s32 num = mPopupNum;
    u32 idx = (mPopupNext - num) & 31;
    while (num > 0) {
        mpPopups[idx]->Calculate();
        idx = (idx + 1) & 31;
        num--;
    }
}

void RPDucHud::Exit() {
    mpScoreLayout->VF_0x18();
    mpStartLayout->VF_0x18();
    mpEndLayout->VF_0x18();
    for (u32 i = 0; i < 32; i++) {
        mpPopups[i]->VF_0x18();
    }
}

void RPDucHud::DrawLayouts() {
    mpScoreLayout->Draw();
    mpStartLayout->Draw();
    mpEndLayout->Draw();

    s32 num = mPopupNum;
    u32 idx = (mPopupNext - num) & 31;
    while (num > 0) {
        mpPopups[idx]->Draw();
        idx = (idx + 1) & 31;
        num--;
    }
}

void RPDucHud::SetStartBannerVisible(bool visible) {
    if (visible) {
        mpStartLayout->PlayIn();
    } else {
        mpStartLayout->PlayOut();
    }
}

void RPDucHud::SpawnScorePopup(nw4r::math::VEC2 screenPos, s32 points, u32 player, s32 effect,
                               bool combo) {
    if (mPopupNum == 32) {
        return;
    }

    mpPopups[mPopupNext]->Show(screenPos, points, player, effect);
    mPopupNum++;
    mPopupNext = (mPopupNext + 1) & 31;

    s32 type;
    if (!combo || points <= 0) {
        type = 0;
    } else {
        bool bonus = RPDucScoreMgr::sInstance->HasStreakBonus(player);
        type = 1;
        if (bonus) {
            type = 2;
        }
    }
    mpScoreLayout->PlayScoreAnim(player, type);
}

void RPDucHud::ShowBonus(s32 player, s32 bonus) {
    mpScoreLayout->ShowBonus(player, bonus);
    mpScoreLayout->PlayScoreAnim(player, 3);
}

void RPDucHud::SetHitCounterVisible(bool visible) {
    if (visible) {
        mpScoreLayout->ShowHitCounter();
    } else {
        mpScoreLayout->HideHitCounter();
    }
}

bool RPDucHud::IsEndAnimDone() const {
    return mpEndLayout->IsDone();
}
