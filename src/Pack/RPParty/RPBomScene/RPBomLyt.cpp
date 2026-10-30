#include <Pack/RPParty/RPBomScene/RPBomLyt.h>

#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPSystem/RPSysResourceManager.h>


void RPBomLayoutMgr::CreateInstance() {
    sInstance = new RPBomLayoutMgr;
}

void RPBomLayoutMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBomLayoutMgr::~RPBomLayoutMgr() {}

void RPBomLayoutMgr::Create() {
    mpStart = new RPBomLytStart;
    mpLevel = new RPBomLytLevel;
    mpReady = new RPBomLytReady;
    mpPoseSel[0] = new RPBomLytPoseSel(0);
    mpPoseSel[1] = new RPBomLytPoseSel(1);
    mpScore = new RPBomLytScore;
    mpMiss = new RPBomLytMiss;
    mpClear = new RPBomLytClear;
    mpEnd = new RPBomLytEnd;
    for (u32 i = 0; i < 16; i++) {
        mpPoints[i] = new RPBomLytPoint;
    }
    mPointNum = 0;
    mPointNext = 0;
}

void RPBomLayoutMgr::Load() {
    void* pMsgRes = RPSysResourceManager::GetMessageResource("party_message.bmg");
    mpMessage = new RPSysMessage(pMsgRes, NULL);

    EGG::Archive* pArchive = RPSysResourceManager::LoadGameLocalArchive();
    mpStart->Load(pArchive);
    mpLevel->Load(pArchive);
    mpReady->Load(pArchive);
    mpPoseSel[0]->Load(pArchive);
    mpPoseSel[1]->Load(pArchive);
    mpScore->Load(pArchive);
    mpMiss->Load(pArchive);
    mpClear->Load(pArchive);
    mpEnd->Load(pArchive);
    for (u32 i = 0; i < 16; i++) {
        mpPoints[i]->Load(pArchive);
    }
}

void RPBomLayoutMgr::Reset() {
    mpStart->Reset();
    mpLevel->Reset();
    mpReady->Reset();
    mpPoseSel[0]->Reset();
    mpPoseSel[1]->Reset();
    mpScore->Reset();
    mpMiss->Reset();
    mpClear->Reset();
    mpEnd->Reset();
    for (u32 i = 0; i < 16; i++) {
        mpPoints[i]->Reset();
    }
    mPointNum = 0;
    mPointNext = 0;
}

void RPBomLayoutMgr::Calculate() {
    mpStart->Calculate();
    mpLevel->Calculate();
    mpReady->Calculate();
    mpPoseSel[0]->Calculate();
    mpPoseSel[1]->Calculate();
    mpScore->Calculate();
    mpMiss->Calculate();
    mpClear->Calculate();
    mpEnd->Calculate();

    s32 num = mPointNum;
    u32 index = (mPointNext - num) % 16;
    for (; num > 0; num--) {
        mpPoints[index]->Calculate();
        index = (index + 1) % 16;
    }
}

void RPBomLayoutMgr::Exit() {
    mpStart->VF_0x18();
    mpLevel->VF_0x18();
    mpReady->VF_0x18();
    mpPoseSel[0]->VF_0x18();
    mpPoseSel[1]->VF_0x18();
    mpScore->VF_0x18();
    mpMiss->VF_0x18();
    mpClear->VF_0x18();
    mpEnd->VF_0x18();
    for (u32 i = 0; i < 16; i++) {
        mpPoints[i]->VF_0x18();
    }
}

void RPBomLayoutMgr::DrawLayouts() {
    mpStart->Draw();
    mpLevel->Draw();
    mpReady->Draw();
    mpPoseSel[0]->Draw();
    mpPoseSel[1]->Draw();
    mpScore->Draw();
    mpMiss->Draw();
    mpClear->Draw();
    mpEnd->Draw();

    s32 num = mPointNum;
    u32 index = (mPointNext - num) % 16;
    for (; num > 0; num--) {
        mpPoints[index]->Draw();
        index = (index + 1) % 16;
    }
}

void RPBomLayoutMgr::SpawnScorePopup(const nw4r::math::VEC2* pScreenPos, s32 points,
                                     s32 player, BOOL doubled) {
    if (static_cast<u32>(mPointNum) != 16) {
        nw4r::math::VEC2 screenPos = *pScreenPos;
        mpPoints[mPointNext]->Show(&screenPos, points, player, doubled);
        mPointNum++;
        mPointNext = (mPointNext + 1) % 16;
    }
}

void RPBomLayoutMgr::ShowScoreMove(s32 player, s32 points) {
    if (player == 0) {
        mpScore->ShowMoveLeft(points);
    } else {
        mpScore->ShowMoveRight(points);
    }
}
