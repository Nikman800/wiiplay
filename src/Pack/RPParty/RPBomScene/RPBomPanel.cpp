#include <Pack/RPParty/RPBomScene/RPBomPanel.h>

void RPBomPanelCtrl::CreateInstance() {
    RPBomPanelCtrl* ctrl = new RPBomPanelCtrl;
    sInstance = ctrl;
}

void RPBomPanelCtrl::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBomPanelCtrl::~RPBomPanelCtrl() {}

void RPBomPanelCtrl::Create() {
    RPBomPanelPool::CreateInstance();
    mpPool = RPBomPanelPool::sInstance;
    mpPool->CreateItems(16);
}

void RPBomPanelCtrl::Load() {
    mpPool->Load();
}

void RPBomPanelCtrl::Reset() {
    mpPool->Reset();
}

void RPBomPanelCtrl::Move() {
    if (RPBomGameManager::sInstance->mWaitTimer > 0) {
        return;
    }
    mpPool->MoveItems();
}

void RPBomPanelCtrl::PreCalc() {
    mpPool->PreCalcItems();
}

void RPBomPanelCtrl::CalcHit() {
    mpPool->CalcHitItems();
}

void RPBomPanelCtrl::PostCalc() {
    mpPool->PostCalcItems();
}

void RPBomPanelCtrl::CalcItems() {
    mpPool->CalcItems();
}

void RPBomPanelCtrl::IncrementAges() {
    mpPool->IncrementAges();
}

void RPBomPanelCtrl::UpdateModels() {
    mpPool->CalcModelMtxItems();
}

void RPBomPanelPool::Destroy() {
    DestroyInstance();
}

void RPBomPanelCtrl::Exit() {
    mpPool->Destroy();
}

void RPBomPanelCtrl::DeactivateAll() {
    mpPool->DeactivateAll();
}
