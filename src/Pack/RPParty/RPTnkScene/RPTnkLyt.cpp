// Tanks layout manager (RPTnkLayoutManager): owns every layout and the BMG message.
// Unit 0x8026D670-0x8026DF6C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkLyt.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPTnkScene/RPTnkGameManager.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

RPTnkLayoutManager* RPTnkLayoutManager::sInstance;

void RPTnkLayoutManager::CreateInstance() {
    sInstance = new RPTnkLayoutManager();
}

void RPTnkLayoutManager::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkLayoutManager::~RPTnkLayoutManager() {}

void RPTnkLayoutManager::Create() {
    mpHud = new RPTnkHud();
    mpExMsg = new RPTnkExMsg();
    mpStartMsg = new RPTnkStartMsg();
    mpClearMsg = new RPTnkClearMsg();
    mpFailedMsg = new RPTnkFailedMsg();
    mpCompleteMsg = new RPTnkCompleteMsg();
    for (u32 i = 0; i < 16; i++) {
        mpHitPopups[i] = new RPTnkHitPopup();
    }
    mHitPopupNum = 0;
    mHitPopupNext = 0;
    mpResultLyt = new RPTnkResultLyt();
    mpFade = new RPTnkFade();
}

void RPTnkLayoutManager::Load() {
    void* pMsgRes = RPSysResourceManager::GetMessageResource("party_message.bmg");
    mpMessage = new RPSysMessage(pMsgRes, NULL);
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameLocalArchive();
    mpHud->Load(pArchive);
    mpExMsg->Load(pArchive);
    mpStartMsg->Load(pArchive);
    mpClearMsg->Load(pArchive);
    mpFailedMsg->Load(pArchive);
    mpCompleteMsg->Load(pArchive);
    for (u32 i = 0; i < 16; i++) {
        mpHitPopups[i]->Load(pArchive);
    }
    mpResultLyt->Load(pArchive);
    mpFade->Load(pArchive);
}

void RPTnkLayoutManager::Reset() {
    mpHud->Reset();
    mpExMsg->Reset();
    mpStartMsg->Reset();
    mpClearMsg->Reset();
    mpFailedMsg->Reset();
    mpCompleteMsg->Reset();
    for (u32 i = 0; i < 16; i++) {
        mpHitPopups[i]->Reset();
    }
    mHitPopupNum = 0;
    mHitPopupNext = 0;
    mpResultLyt->Reset();
    mpFade->Reset();
}

void RPTnkLayoutManager::Calculate() {
    s32 state = RPTnkGameManager::sInstance->GetState();
    mpHud->Calculate();
    if (RPTnkGameManager::sInstance->GetStage() == 0) {
        mpExMsg->Calculate();
    }
    if (state == 4) {
        mpStartMsg->Calculate();
    }
    if (state == 6 || state == 7) {
        mpClearMsg->Calculate();
    }
    if (state == 9) {
        mpFailedMsg->Calculate();
    }
    if (state == 8) {
        mpCompleteMsg->Calculate();
    }

    s32 n = mHitPopupNum;
    u32 idx = (mHitPopupNext - n) % 16;
    for (; n > 0; n--) {
        mpHitPopups[idx]->Calculate();
        idx = (idx + 1) % 16;
    }

    if (state == 10 || state == 11 || state == 12) {
        mpResultLyt->Calculate();
    }
    mpFade->Calculate();
}

void RPTnkLayoutManager::Exit() {
    mpHud->VF_0x18();
    mpExMsg->VF_0x18();
    mpStartMsg->VF_0x18();
    mpClearMsg->VF_0x18();
    mpFailedMsg->VF_0x18();
    mpCompleteMsg->VF_0x18();
    for (u32 i = 0; i < 16; i++) {
        mpHitPopups[i]->VF_0x18();
    }
    mpResultLyt->VF_0x18();
    mpFade->VF_0x18();
}

void RPTnkLayoutManager::Draw2D() {
    s32 state = RPTnkGameManager::sInstance->GetState();
    if (RPTnkGameManager::sInstance->GetStage() == 0) {
        mpExMsg->Draw();
    }
    mpHud->Draw();

    s32 n = mHitPopupNum;
    u32 idx = (mHitPopupNext - n) % 16;
    for (; n > 0; n--) {
        mpHitPopups[idx]->Draw();
        idx = (idx + 1) % 16;
    }

    if (state == 4) {
        mpStartMsg->Draw();
    }
    if (state == 6 || state == 7) {
        mpClearMsg->Draw();
    }
    if (state == 9) {
        mpFailedMsg->Draw();
    }
    if (state == 8) {
        mpCompleteMsg->Draw();
    }
    if (state == 10 || state == 11 || state == 12) {
        mpResultLyt->Draw();
    }
    mpFade->Draw();
}

void RPTnkLayoutManager::SpawnHitPopup(const nw4r::math::VEC2& rScreenPos, s32 num, s32 type) {
    if (RPTnkGameManager::sInstance->GetModeIdx() != 0 && mHitPopupNum != 16) {
        mpHitPopups[mHitPopupNext]->Start(rScreenPos, num, type);
        mHitPopupNum++;
        mHitPopupNext = (mHitPopupNext + 1) % 16;
    }
}
